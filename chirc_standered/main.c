#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>


#include "server.h"

char password[50] = "123";
int port = PORT;
int log_level = 0;

void parse_arguments(int argc, char* argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "p:o:qv")) != -1) {
        switch (opt) {
        case 'p':
            port = atoi(optarg);
            break;
        case 'o':
            strncpy(password, optarg, sizeof(password) - 1);
            password[sizeof(password) - 1] = '\0';
            break;
        case 'q':
            log_level = 0; // quiet (only errors)
            break;
        case 'v':
            if (log_level < 2) {
                log_level++;
            }
            break;
        default:
            fprintf(stderr, "Usage: %s [-p port] [-o operator_password] [-q] [-v]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }
}

void log_message(int level, const char* message) {
    if (level <= log_level) {
        switch (level) {
        case 0:
            printf("ERROR: %s\n", message);
            break;
        case 1:
            printf("WARN: %s\n", message);
            break;
        case 2:
            printf("INFO: %s\n", message);
            break;
        }
    }
}

int main(int argc, char* argv[]) {

    parse_arguments(argc, argv);

    log_message(2, "Starting server");

    init_server(port, password);

    return 0;
}
