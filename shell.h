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
} Job;

// constructors
Process *new_process();

Job *new_job(size_t total_processes);

// functions to parse input
size_t _parse_line(char ***commands, size_t *n, char *line);

void _parse_raw_command(Process *p, char *raw_command);

Job *parse_line_into_job(char *line);

int has_bg_sign(Job *job);

// functions to launch jobs
void exec(Process *process, pid_t gid, int in_file, int out_file);

void launch_job(Job *job, int bg);

// functions to deal with fg jobs
void put_job_in_fg(Job *job, int cont);

void wait_for_fg_job(Job *job);

// functions to deal with bg jobs
void _assign_bg_num(Job *job);

void put_job_in_bg(Job *job, int cont);

void print_bg_job(Job *job, char status);

Job *_pop_bg_job_with_num(size_t bg_n);

// function to get job status
char get_job_status(Job *job);

// functions to change job status
void mark_job_as_running(Job *job);

void resume_job_running_status(Job *job);

// functions to periodically update and clean jobs
void refresh_all_bg_process_status();

void clean_bg_jobs();

void free_job(Job *job);

#endif
