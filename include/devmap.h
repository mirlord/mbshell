#include <iniparser.h>
#include <modbus.h>

typedef struct {
    modbus_t *ctx;
    dictionary *map;
    char *prog_f;
    FILE *prog;
    int prog_lc;
} mbdev_t;

typedef struct {
    int addr;
    int bits;
    int nb_regs;
} mbreg_t;

mbreg_t *reg_spec(mbdev_t *dev, mbreg_t *reg, char *reg_name);

void reg_spec_free(mbreg_t *reg);

mbdev_t *init(char *devmap_f, char *prog_f);

void mbdev_free(mbdev_t *dev);

