#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include "prog.h"
#include "ioutil.h"

// Size-limit for the whole command string, including non-interpreted chars
#define CMD_SIZE_LIMIT 255
#define CMD_READ 0
#define CMD_WRITE 1

char *trim_whitespace(char *str) {

    char *end;

    // Trim leading space
    while(isspace(*str)) str++;

    if(*str == 0) {  // All spaces?
        return str;
    }

    // Trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace(*end)) end--;

    // Write new null terminator
    *(end+1) = 0;

    return str;
}

char *strip_comments(char *str) {
    char *sh_idx = index(str, '#');
    if (sh_idx != NULL) {
        *sh_idx = 0;
    }
    return str;
}

mbcmd_t *parse_cmd(mbdev_t *dev, mbcmd_t *cmd, char **dstptr, char **srcptr) {

    if (fgets(cmd->buf, CMD_BUF_SIZE, dev->prog) == NULL) {
        return NULL;
    }
    dev->prog_lc++;

    char *cmd_s = trim_whitespace(strip_comments(cmd->buf));

    // ignore empty string
    if (strlen(cmd_s) == 0) {
        return parse_cmd(dev, cmd, dstptr, srcptr);
    }

    char *sep_idx = index(cmd_s, '=');
    if (sep_idx == NULL) {
        fprintf(stderr, "Incorrect input string: file %s, line %i", dev->prog_f, dev->prog_lc);
        exit(EXIT_FAILURE);
    }
    *sep_idx = 0;

    *dstptr = trim_whitespace(cmd_s);
    *srcptr = trim_whitespace(++sep_idx);
    return cmd;
}

mbcmd_t *next_cmd(mbdev_t *dev, mbcmd_t *cmd) {

    char *dst, *src;
    if (parse_cmd(dev, cmd, &dst, &src) == NULL) {
        return NULL;
    }

    if (index(dst, '$') == dst) {
        cmd->rw = CMD_READ;
        strcpy(cmd->dst, ++dst);
        strcpy(cmd->src, src);
        reg_spec(dev, cmd->reg_spec, src);
    } else {
        cmd->rw = CMD_WRITE;
        strcpy(cmd->dst, dst);
        reg_spec(dev, cmd->reg_spec, dst);
        strcpy(cmd->src, src);
    }

    MSG("raw command: [%s] <-- [%s]", dst, src);

    return cmd;
}

/**
 *
 * - mapped alias
 * - variable
 * - expression
 * - value itself
 */
uint16_t *eval_src_raw_value(mbdev_t *dev, mbcmd_t* cmd) {

    #define src cmd->src
    char *endptr;
    errno = 0;
    long long int raw_val = strtoll(src, &endptr, 0);
    if ((errno == ERANGE && (raw_val == LONG_MAX || raw_val == LONG_MIN))
            || (errno != 0 && raw_val == 0)
            || (endptr == src)) {

        char *reg_src_cfg = malloc(8 + strlen(src));
        sprintf(reg_src_cfg, "values:%s", src);
        char *raw_src_str = iniparser_getstring(dev->map, reg_src_cfg, "0x00");
        free(reg_src_cfg);
        raw_val = strtol(raw_src_str, NULL, 0);
    }
    for (int i = 0; i < REG_VALUE_MAX_SIZE / 2; i++) {
        cmd->value_buf[i] = (uint16_t) (raw_val >> 16 * i);
    }
    #undef src
    return cmd->value_buf;
}

void write_cmd(mbdev_t *dev, mbcmd_t *cmd) {

    uint16_t *raw_value = eval_src_raw_value(dev, cmd);
    for (int i = 0; i < REG_VALUE_MAX_SIZE / 2; i++) {
        /* vals_a[i] = vals_a[0]; */
        MSG("reg[%d]=%d (0x%X)", i, raw_value[i], raw_value[i]);
    }

    errno = 0;
    /* int rc = modbus_write_register(dev->ctx, raw_addr, raw_value[0]); */
    int rc = modbus_write_registers(dev->ctx, cmd->reg_spec->addr, cmd->reg_spec->nb_regs, raw_value);
    if (rc == -1) {
        // Error not always means, that a command was not executed or
        // it's impossible to continue. Just print and go further.
        MSG("E: %s", modbus_strerror(errno));
    }
}

void read_cmd(mbdev_t *dev, mbcmd_t *cmd) {

    errno = 0;
    int rc = modbus_read_registers(dev->ctx, cmd->reg_spec->addr, cmd->reg_spec->nb_regs, cmd->value_buf);
    if (rc == -1) {
        MSG("E: %s", modbus_strerror(errno));
    }
    if (strcmp(cmd->dst, "_") == 0) {
        for (int i = 0; i < rc; i++) {
            MSG("reg[%d]=%d (0x%X)", i, cmd->value_buf[i], cmd->value_buf[i]);
        }
    }
}

void *send_cmd(mbdev_t *dev, mbcmd_t *cmd) {

    if (cmd->rw == CMD_WRITE) {
        write_cmd(dev, cmd);
    } else {
        read_cmd(dev, cmd);
    }
}

void mbcmd_free(mbcmd_t *cmd) {
    reg_spec_free(cmd->reg_spec);
    free(cmd);
}

mbcmd_t *mbcmd_init() {
    mbcmd_t *cmd = malloc(sizeof(mbcmd_t));
    cmd->reg_spec = malloc(sizeof(mbreg_t));
    return cmd;
}

