#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

size_t parse_line(char **commands[], size_t *n, char *line) {
    if (*commands == NULL) {
        *n = 10;
        *commands = malloc((*n) * sizeof(char *));
    }

    char *remaining = line;
    char *command;
    size_t i = 0;
    while (1) {
        command = strsep(&remaining, "|");
        if (remaining == NULL) {    // end of line
            if (i >= *n) {  // not enough size
                *n = i + 2;
                *commands = realloc(*commands, (*n) * sizeof(char *));
            }
            (*commands)[i] = command;
            (*commands)[i + 1] = NULL; // append a NULL ptr
            return i;
        }
        else {
            if (i >= *n) {  // not enough size
                *n = 2 * (*n);
                *commands = realloc(*commands, (*n) * sizeof(char *));
            }
            (*commands)[i] = command;
            i++;
        }
    }
}

size_t parse_raw_command(char ***argv, size_t *n, char *line) {
    if (*argv == NULL) {
        *n = 10;
        *argv = malloc((*n) * sizeof(char *));
    }

    char *remaining = line;
    char *arg;
    size_t i = 0;
    while (1) {
        arg = strsep(&remaining, " \n");

        if (remaining == NULL) {    // end of line
            if (*arg == '\0' || isspace(*arg)) {
                if (i >= *n) {  // not enough size
                    *n = i + 1;
                    *argv = realloc(*argv, (*n) * sizeof(char *));
                }
                (*argv)[i + 1] = NULL; // append a NULL ptr
                return i;
            }
            if (i >= *n) {  // not enough size
                *n = i + 2;
                *argv = realloc(*argv, (*n) * sizeof(char *));
            }
            (*argv)[i] = arg;
            (*argv)[i + 1] = NULL; // append a NULL ptr
            return i + 1;
        }
        else {
            if (*arg == '\0' || isspace(*arg)) {
                continue;
            }
            else {
                if (i >= *n) {  // not enough size
                    *n = 2 * (*n);
                    *argv = realloc(*argv, (*n) * sizeof(char *));
                }
                (*argv)[i] = arg;
                i++;
            }
        }
    }
}

size_t parse_line_into_commands(char ****argvs, size_t **argcs, size_t *n, char *line) {
    if (*argvs == NULL) {
        *n = 10;
        *argvs = malloc((*n) * sizeof(char **));
    }
    if (*argcs == NULL) {
        *n = 10;
        *argcs = malloc((*n) * sizeof(int));
    }

    char **raw_commands = NULL; // need cleaning up
    size_t commands_n = 0;   // need cleaning up
    size_t total_raw_commands = parse_line(&raw_commands, &commands_n, line);

    if (total_raw_commands > *n) {
        *n = total_raw_commands;
        *argvs = malloc((*n) * sizeof(char **));
        *argcs = malloc((*n) * sizeof(int));
    }

    char **argv = NULL;
    for (size_t i = 0; i < total_raw_commands; i++) {
        (*argcs)[i] = parse_raw_command(&argv, &commands_n, raw_commands[i]);
    }

}


int main(void) {
    char *line = NULL;
    size_t size = 0;
    getline(&line, &size, stdin);

    char **raw_commands = NULL;
    size_t n = 0;
    parse_line(&raw_commands, &n, line);

    int i = 0;
    while (1) {
        if (raw_commands[i] == NULL) {
            break;
        }
        else {
            printf("%s\n", raw_commands[i]);
            i++;
        }
    }

    printf("--------------------------------\n");

    char **argv = NULL;
    size_t size1 = 0;
    size_t argc = parse_raw_command(&argv, &size1, raw_commands[1]);
    printf("%zu\n", argc);
    i = 0;
    while (1) {
        if (argv[i] == NULL) {
            break;
        }
        else {
            printf("%s\n", argv[i]);
            i++;
        }
    }

    return 0;
}