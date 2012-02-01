#include "devmap.h"

// Size-limit for the whole command string, including non-interpreted chars
#define CMD_BUF_SIZE 2048
#define RVALUE_BUF_L sizeof(long long int) / 2
#define CMD_READ 0
#define CMD_WRITE 1

typedef struct {
    char buf[CMD_BUF_SIZE];
    int rw;
    char dst[CMD_BUF_SIZE];
    char src[CMD_BUF_SIZE];
    mbreg_t *reg_spec;
    uint16_t value_buf[RVALUE_BUF_L];
} mbcmd_t;

char *trim_whitespace(char *str);

char *strip_comments(char *str);

mbcmd_t *next_cmd(mbdev_t *dev, mbcmd_t *cmd);

void mbcmd_free(mbcmd_t *cmd);

mbcmd_t *mbcmd_init();

void write_cmd(mbdev_t *dev, mbcmd_t *cmd);

void read_cmd(mbdev_t *dev, mbcmd_t *cmd);

void *send_cmd(mbdev_t *dev, mbcmd_t *cmd);

