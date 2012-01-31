#include <stdio.h>
#include <stdlib.h>

void print_usage() {
    printf("%s\n", "./onitex <map-file>.map <prog-file>.mbp");
}

void print_usage_error(char *error_msg) {
    fprintf(stderr, "%s\n", error_msg);
    print_usage();
}

