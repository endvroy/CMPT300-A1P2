#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    return 0;
}