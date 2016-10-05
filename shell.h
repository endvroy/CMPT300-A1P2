#ifndef SHELL_H
#define SHELL_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <wait.h>
#include <errno.h>

typedef struct Process {
    size_t argc;
    char **argv;
    pid_t pid;
    char status;
} Process;

typedef struct Job {
    size_t total_processes;
    Process **processes;
    pid_t gid;
    size_t bg_num;
//    struct termios tmodes;
} Job;

void put_job_in_fg(Job *job, int cont);

void put_job_in_bg(Job *job, int cont);

Process *new_process();

Job *new_job(size_t total_processes);

size_t _parse_line(char ***commands, size_t *n, char *line);

void _parse_raw_command(Process *p, char *raw_command);

Job *parse_line_into_job(char *line);

void exec(Process *process, pid_t gid, int in_file, int out_file);

void launch_job(Job *job, int bg);

void put_job_in_fg(Job *job, int cont);

void put_job_in_bg(Job *job, int cont);

int has_bg_sign(Job *job);

Job *pop_bg_job_with_num(size_t bg_n);

void assign_bg_num(Job *job);

void print_bg_job(Job *job, char status);

void delete_job(Job *job);

void mark_job_as_running(Job *job);

#endif