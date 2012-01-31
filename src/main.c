#include <ctype.h>
#include <strings.h>
#include <errno.h>
#include <limits.h>
#include "ioutil.h"
#include "prog.h"

int main(int argc, char *argv[]) {

    if (argc == 1) {
        print_usage();
        return EXIT_SUCCESS;
    } else if (argc < 3) {
        print_usage_error("Insufficient arguments");
        return EXIT_FAILURE;
    }

    mbdev_t *dev = init(argv[1], argv[2]);
    mbcmd_t *cmd = mbcmd_init();

    while (next_cmd(dev, cmd)) {
        send_cmd(dev, cmd);
    }

    mbcmd_free(cmd);
    mbdev_free(dev);

    return 0;
}

