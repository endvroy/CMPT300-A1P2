#define _GNU_SOURCE

#include "shell.h"
#include <deque>
#include <termios.h>

using namespace std;

typedef deque<Job *> JobList;

// global variables
int shell_gid;
JobList bg_jobs;
size_t bg_num = 1;
//struct termios shell_tmodes;

Process *new_process() {
    Process *process = (Process *) malloc(sizeof(process));
    return process;
}

Job *new_job(size_t total_processes) {
    Job *job = (Job *) malloc(sizeof(Job));
    job->total_processes = total_processes;
    job->processes = (Process **) malloc(total_processes * sizeof(Process *));
    job->gid = 0;
    return job;
}

size_t _parse_line(char ***commands, size_t *n, char *line) {
    if (*commands == NULL) {
        *n = 10;
        *commands = (char **) malloc((*n) * sizeof(char *));
    }

    char *remaining = line;
    char *command;
    size_t i = 0;
    while (1) {
        command = strsep(&remaining, "|");
        if (remaining == NULL) {    // end of line
            if (i >= *n) {  // not enough size
                *n = i + 2;
                *commands = (char **) realloc(*commands, (*n) * sizeof(char *));
            }
            (*commands)[i] = command;
            (*commands)[i + 1] = NULL; // append a NULL ptr
            return i + 1;
        }
        else {
            if (i >= *n) {  // not enough size
                *n = 2 * (*n);
                *commands = (char **) realloc(*commands, (*n) * sizeof(char *));
            }
            (*commands)[i] = command;
            i++;
        }
    }
}

void _parse_raw_command(Process *p, char *raw_command) {
    size_t n = 10;
    if (p->argv == NULL) {
        p->argv = (char **) malloc(n * sizeof(char *));
    }

    char *remaining = raw_command;
    char *orig_arg;
    size_t i = 0;
    while (1) {
        orig_arg = strsep(&remaining, " \n");
        char *arg = (char *) malloc(strlen(orig_arg) * sizeof(char));
        strcpy(arg, orig_arg);

        if (remaining == NULL) {    // end of raw_command
            if (*arg == '\0' || isspace(*arg)) {
                if (i >= n) {  // not enough size
                    n = i + 1;
                    p->argv = (char **) realloc(p->argv, n * sizeof(char *));
                }
                p->argv[i + 1] = NULL; // append a NULL ptr
                p->argc = i;
                return;
            }
            if (i >= n) {  // not enough size
                n = i + 2;
                p->argv = (char **) realloc(p->argv, n * sizeof(char *));
            }
            p->argv[i] = arg;
            p->argv[i + 1] = NULL; // append a NULL ptr
            p->argc = i + 1;
            return;
        }
        else {
            if (*arg == '\0' || isspace(*arg)) {
                continue;
            }
            else {
                if (i >= n) {  // not enough size
                    n = 2 * n;
                    p->argv = (char **) realloc(p->argv, n * sizeof(char *));
                }
                p->argv[i] = arg;
                i++;
            }
        }
    }
}

Job *parse_line_into_job(char *line) {
    char **raw_commands = NULL; // need cleaning up
    size_t commands_n = 0;
    size_t total_raw_commands = _parse_line(&raw_commands, &commands_n, line);

    Job *job = new_job(total_raw_commands);

    for (size_t i = 0; i < total_raw_commands; i++) {
        Process *p = new_process();
        _parse_raw_command(p, raw_commands[i]);
        job->processes[i] = p;
    }
    free(raw_commands);
    return job;
}

void exec(Process *process, pid_t gid, int in_file, int out_file) {
    signal(SIGINT, SIG_DFL);

    pid_t pid = getpid();
    if (gid == 0) {
        gid = pid;
    }
    setpgid(pid, gid);

    if (gid == 0) {
        gid = pid;
    }
    setpgid(pid, gid);

    if (in_file != STDIN_FILENO) {
        dup2(in_file, STDIN_FILENO);
        close(in_file);
    }
    if (out_file != STDOUT_FILENO) {
        dup2(out_file, STDOUT_FILENO);
        close(out_file);
    }

    if (execvp(process->argv[0], process->argv) == -1) {
        if (errno == ENOENT) {
            fprintf(stderr, "%s: command not found\n", process->argv[0]);
        }
        else {
            perror(process->argv[0]);
        }
        exit(-1);
    }
}

void launch_job(Job *job, int bg) {
    int data_pipe[2];
    int in_file = STDIN_FILENO;
    int out_file;
    for (size_t i = 0; i < job->total_processes; i++) {
        if (i + 1 == job->total_processes) {
            out_file = STDOUT_FILENO;
        }
        else {
            pipe(data_pipe);
            out_file = data_pipe[1];
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            return;
        }
        else if (pid == 0) {    // child
            exec(job->processes[i], job->gid, in_file, out_file);
        }
        else {   // parent
            job->processes[i]->pid = pid;
            if (job->gid == 0) {
                job->gid = pid;
            }
            setpgid(pid, job->gid);

            // close pipes in parent
            if (in_file != STDIN_FILENO) {
                close(in_file);
            }
            if (out_file != STDOUT_FILENO) {
                close(out_file);
            }
            in_file = data_pipe[0];


        }
    }
    if (bg) {
        put_job_in_bg(job, 0);
    }
    else {
        put_job_in_fg(job, 0);
    }
}

void assign_bg_num(Job *job) {
    job->bg_num = bg_num;
    bg_num++;
}

void print_bg_job(Job *job, char *status) {
    printf("[%zu] %s %s\n", job->bg_num, status, job->processes[0]->argv[0]);
}

void put_job_in_fg(Job *job, int cont) {
    // put job in fg
    tcsetpgrp(STDIN_FILENO, job->gid);

    if (cont) {
        if (kill(-job->gid, SIGCONT) < 0)
            perror("kill (SIGCONT)");
    }

    int status;
    waitpid(-job->gid, &status, WUNTRACED);
    if (WIFSTOPPED(status)) {
        assign_bg_num(job);
        bg_jobs.push_back(job);
        print_bg_job(job, "Stopped");
    }
    // Put the shell back in fg
    tcsetpgrp(STDIN_FILENO, shell_gid);

}

void put_job_in_bg(Job *job, int cont) {
    assign_bg_num(job);
    bg_jobs.push_back(job);

    if (cont) {
        if (kill(-job->gid, SIGCONT) < 0) {
            perror("kill (SIGCONT)");
        }
    }
    print_bg_job(job, "Running");
}

int has_bg_sign(Job *job) {
    Process *last_process = job->processes[job->total_processes - 1];
    if (strcmp(last_process->argv[last_process->argc - 1], "&") == 0) {
        last_process->argv[last_process->argc - 1] = NULL;
        free(last_process->argv[last_process->argc]);
        last_process->argc--;
        return 1;
    }
    else {
        return 0;
    }
}

int main(void) {
    signal(SIGINT, SIG_IGN);    // ignore ^C
    signal(SIGTTOU, SIG_IGN);   // prevent the shell from stopping itself

    shell_gid = getpid();
    setpgid(shell_gid, shell_gid);
    tcsetpgrp(STDIN_FILENO, shell_gid);
//    tcgetattr(STDIN_FILENO, &shell_tmodes);

    char cwd[4096];

    char *line = NULL;
    size_t line_size = 0;

    while (1) {
        int bad_line = 0;
        if (getcwd(cwd, 4096) == NULL) {
            fprintf(stderr, "path length exceeds limit");
            exit(-1);
        }

        printf("mysh:%s> ", cwd);
        getline(&line, &line_size, stdin);
        Job *job = parse_line_into_job(line);
        if (job->total_processes == 1 && job->processes[0]->argc == 0) {
            continue; //empty_line
        }
        for (size_t i = 0; i < job->total_processes; i++) {
            if (job->processes[i]->argc == 0) {
                bad_line = 1;
                break;
            }
        }
        if (bad_line) {
            fprintf(stderr, "bad input\n");
            continue;
        }

        if (strcmp(job->processes[0]->argv[0], "exit") == 0) {
            break;
        }
        else if (strcmp(job->processes[0]->argv[0], "cd") == 0) {
            if (chdir(job->processes[0]->argv[1]) != 0) {
                perror("cd");
            };
        }
        else if (strcmp(job->processes[0]->argv[0], "jobs") == 0) {
            for (auto job1 : bg_jobs) {
                print_bg_job(job1, "unimplemented");
            }
        }
        else {
            int bg = has_bg_sign(job);
            launch_job(job, bg);

//            // for test purposes
//            if (bg) {
//                printf("background job\n");
//            }
//            else {
//                printf("foreground job\n");
//            }
//            for (size_t i = 0; i < job->total_processes; i++) {
//                printf("command %zu:\n---------------\n", i);
//                for (size_t j = 0; j < job->processes[i]->argc; j++) {
//                    printf("arg %zu: %s\n", j, job->processes[i]->argv[j]);
//                }
//                putchar('\n');
//            }
        }
    }

    // end of main loop
    // cleaning up
    free(line);
    return 0;

}