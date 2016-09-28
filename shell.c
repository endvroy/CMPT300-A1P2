#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void parse_line(char **commands[], size_t *n, char *line) {
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
            return;
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

void parse_command(char **argv[], size_t *n, char *line) {
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
                return;
            }
            if (i >= *n) {  // not enough size
                *n = i + 2;
                *argv = realloc(*argv, (*n) * sizeof(char *));
            }
            (*argv)[i] = arg;
            (*argv)[i + 1] = NULL; // append a NULL ptr
            return;
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

int main(void) {
    char *line = NULL;
    size_t size = 0;
    getline(&line, &size, stdin);

    char **commands = NULL;
    size_t n = 0;
    parse_line(&commands, &n, line);

    int i = 0;
    while (1) {
        if (commands[i] == NULL) {
            break;
        }
        else {
            printf("%s\n", commands[i]);
            i++;
        }
    }

    printf("--------------------------------\n");

    char **argv = NULL;
    size_t size1 = 0;
    parse_command(&argv, &size1, commands[1]);
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