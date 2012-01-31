#include <strings.h>
#include <errno.h>
#include "devmap.h"
#include "ioutil.h"

char *reg_raw_spec(mbdev_t *dev, char *reg_name) {
    char *reg_param = malloc(11 + strlen(reg_name));
    sprintf(reg_param, "registers:%s", reg_name);
    char *spec = iniparser_getstring(dev->map, reg_param, "0:8");
    free(reg_param);

    return spec;
}

mbreg_t *reg_spec(mbdev_t *dev, mbreg_t *reg, char *reg_name) {

    char *reg_spec = reg_raw_spec(dev, reg_name);
    reg->bits = 8;
    char *sep_idx = index(reg_spec, ':');
    if (sep_idx != NULL) {
        *sep_idx = 0;
        reg->bits = atoi(++sep_idx);
    }
    reg->addr = atoi(reg_spec);
    reg->nb_regs = (reg->bits - 1) / 16 + 1;
    return reg;
}

void reg_spec_free(mbreg_t *reg) {
    free(reg);
}

mbdev_t *init(char *devmap_f, char *prog_f) {

    MSG("init");

    mbdev_t *dev = malloc(sizeof(mbdev_t));
    dev->map = iniparser_load(devmap_f);
    dev->prog_f = prog_f;
    dev->prog = fopen(prog_f, "r");
    dev->prog_lc = 0;

    dev->ctx = modbus_new_rtu(
            iniparser_getstring(dev->map, "config:dev", "/dev/ttyUSB0"),
            iniparser_getint(dev->map, "config:speed", 9600),
            iniparser_getstring(dev->map, "config:parity", "N")[0],
            iniparser_getint(dev->map, "config:data_bit", 8),
            iniparser_getint(dev->map, "config:stop_bit", 1));
    if (dev->ctx == NULL) {
       print_usage_error("Unable to create libmodbus context\n");
    }
    modbus_set_debug(dev->ctx, iniparser_getint(dev->map, "config:debug", 0));

    int rto_msec = iniparser_getint(dev->map, "config:timeout", 0);
    if (rto_msec != 0) {
        struct timeval rto;
        rto.tv_sec = rto_msec / 1000;
        rto.tv_usec = rto_msec % 1000;
        modbus_set_response_timeout(dev->ctx, &rto);
        MSG("%i.%i", rto.tv_sec, rto.tv_usec);
    }

    MSG("connecting...");
    modbus_set_slave(dev->ctx, iniparser_getint(dev->map, "config:slave", 0));
    if (modbus_connect(dev->ctx) == -1) {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(dev->ctx);
        exit(1);
    }
    return dev;
}

void mbdev_free(mbdev_t *dev) {
    modbus_free(dev->ctx);
    fclose(dev->prog);
    iniparser_freedict(dev->map);
    free(dev);
}

