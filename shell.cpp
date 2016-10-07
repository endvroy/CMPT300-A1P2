#define _GNU_SOURCE

#include "shell.h"
#include <deque>

using namespace std;

typedef deque<Job *> JobList;

// shell's internal variables
int shell_gid;
JobList bg_jobs;
size_t bg_num = 1;

// constructors
Process *new_process() {
    Process *process = (Process *) malloc(sizeof(process));
    return process;
}

Job *new_job(size_t total_processes) {
    Job *job = (Job *) malloc(sizeof(Job));
    job->total_processes = total_processes;
    job->processes = (Process **) malloc(total_processes * sizeof(Process *));
    job->gid = 0;
    job->bg_num = 0;
    return job;
}

// functions to parse input
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
        orig_arg = strsep(&remaining, " \n\t");


        if (remaining == NULL) {    // end of raw_command
            if (*orig_arg == '\0' || isspace(*orig_arg)) {
                if (i >= n) {  // not enough size
                    n = i + 1;
                    char *arg = (char *) malloc((strlen(orig_arg) + 1) * sizeof(char));
                    strcpy(arg, orig_arg);
                    p->argv = (char **) realloc(p->argv, n * sizeof(char *));
                }
                p->argv[i] = NULL; // append a NULL ptr
                p->argc = i;
                return;
            }
            if (i >= n) {  // not enough size
                n = i + 2;
                char *arg = (char *) malloc((strlen(orig_arg) + 1) * sizeof(char));
                strcpy(arg, orig_arg);
                p->argv = (char **) realloc(p->argv, n * sizeof(char *));
            }
            p->argv[i] = orig_arg;
            p->argv[i] = NULL; // append a NULL ptr
            p->argc = i + 1;
            return;
        }
        else {
            if (*orig_arg == '\0' || isspace(*orig_arg)) {
                continue;
            }
            else {
                if (i >= n) {  // not enough size
                    n = 2 * n;
                    char *arg = (char *) malloc((strlen(orig_arg) + 1) * sizeof(char));
                    strcpy(arg, orig_arg);
                    p->argv = (char **) realloc(p->argv, n * sizeof(char *));
                }
                char *arg = (char *) malloc((strlen(orig_arg) + 1) * sizeof(char));
                strcpy(arg, orig_arg);
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

// functions to launch jobs
void exec(Process *process, pid_t gid, int in_file, int out_file) {
    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);

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
    mark_job_as_running(job);
    if (bg) {
        put_job_in_bg(job, 0);
    }
    else {
        put_job_in_fg(job, 0);
    }
}

// functions to deal with fg jobs
void put_job_in_fg(Job *job, int cont) {
//    mark_job_as_running(job);
    // put job in fg
    resume_job_running_status(job);
    tcsetpgrp(STDIN_FILENO, job->gid);

    if (cont) {
        if (kill(-job->gid, SIGCONT) < 0)
            perror("kill (SIGCONT)");
    }

    wait_for_fg_job(job);

    // Put the shell back in fg
    tcsetpgrp(STDIN_FILENO, shell_gid);

}

void wait_for_fg_job(Job *job) {
    int status;
    pid_t pid;

    while (1) {
        pid = waitpid(-job->gid, &status, WUNTRACED);
        int errsv = errno;
        if (pid == 0) {
            break;
        }
        else if (pid < 0) {
            if (errsv != ECHILD) {
                perror("waitpid");
            }
            break;
        }
        else {
            for (size_t i = 0; i < job->total_processes; i++) {
                if (job->processes[i]->pid == pid) {
                    if (WIFSTOPPED(status)) {
                        job->processes[i]->status = 's';
                    }
                    else {
                        job->processes[i]->status = 't';
                    }
                }
            }
            char job_status = get_job_status(job);
//            printf("job status: %c\n", job_status);
            if (job_status == 's') {
                _assign_bg_num(job);
                bg_jobs.push_back(job);
                print_bg_job(job, 's');
                return;
            }
            else if (job_status == 't') {
                free_job(job);
                return;
            }
        }
    };
}

// functions to deal with bg jobs
void _assign_bg_num(Job *job) {
    if (job->bg_num == 0) {
        if (bg_jobs.empty()) {
            bg_num = 1;
        }
        job->bg_num = bg_num;
        bg_num++;
    }
}

void put_job_in_bg(Job *job, int cont) {
//    mark_job_as_running(job);
    resume_job_running_status(job);
    _assign_bg_num(job);
    bg_jobs.push_back(job);

    if (cont) {
        if (kill(-job->gid, SIGCONT) < 0) {
            perror("kill (SIGCONT)");
        }
    }
    char job_status = get_job_status(job);
    print_bg_job(job, job_status);
}

void print_bg_job(Job *job, char status) {
    printf("[%zu]  ", job->bg_num);
    if (status == 'r') {
        printf("%-20s ", "Running");
    }
    if (status == 't') {
        printf("%-20s ", "Done");
    }
    if (status == 's') {
        printf("%-20s ", "Stopped");
    }

    for (size_t i = 0; i < job->total_processes; i++) {
        for (size_t j = 0; j < job->processes[i]->argc; j++) {
            printf("%s ", job->processes[i]->argv[j]);
        }
        if (i + 1 < job->total_processes) {
            printf("| ");
        }
    }
    putchar('\n');
}

Job *_pop_bg_job_with_num(size_t bg_n) {
    for (auto it_job = bg_jobs.begin(); it_job != bg_jobs.end(); it_job++) {
        if ((*it_job)->bg_num == bg_n) {
            Job *job = *it_job;
            bg_jobs.erase(it_job);
            return job;
        }
    }
    return NULL;
}

// function to get job status
char get_job_status(Job *job) {
    for (size_t i = 0; i < job->total_processes; i++) {
        if (job->processes[i]->status == 's') {
            return 's';
        }
        if (job->processes[i]->status == 'r') {   // not stopped but has one still running
            return 'r';
        }
    }
    return 't';
}

// functions to change job status
void mark_job_as_running(Job *job) {
    for (size_t i = 0; i < job->total_processes; i++) {
        job->processes[i]->status = 'r';
    }
}

void resume_job_running_status(Job *job) {
    for (size_t i = 0; i < job->total_processes; i++) {
        if (job->processes[i]->status == 's') {
            job->processes[i]->status = 'r';
        }
    }
}

// functions to periodically update and clean jobs
void refresh_all_bg_process_status() {
    pid_t pid;
    int status;
    for (auto it_job = bg_jobs.begin(); it_job != bg_jobs.end(); it_job++) {
        Job *job = *it_job;
        while (1) {
            pid = waitpid(-job->gid, &status, WNOHANG);
            int errsv = errno;
//            printf("pid = %d\n", pid);
//            if (errsv == ECHILD) {
//                printf("no child\n");
//            }
            if (pid == 0) {
                break;
            }
            else if (pid < 0) {
                if (errsv != ECHILD) {
                    perror("waitpid");
                }
                break;
            }
            else {
                for (size_t i = 0; i < job->total_processes; i++) {
                    if (job->processes[i]->pid == pid) {
                        if (WIFSTOPPED(status)) {
                            job->processes[i]->status = 's';
                        }
                        else {
                            job->processes[i]->status = 't';
                        }
                    }
                }
            }
        }
    }
}

void clean_bg_jobs() {
    auto it_job = bg_jobs.begin();
    while (it_job != bg_jobs.end()) {
        Job *job = *it_job;
        char job_status = get_job_status(job);
        if (job_status == 't') {
            print_bg_job(job, job_status);
            it_job = bg_jobs.erase(it_job);
            free_job(job);
        }
        else {
            it_job++;
        }
    }
}

void free_job(Job *job) {
    for (size_t i = 0; i < job->total_processes; i++) {
        Process *process = job->processes[i];
        for (size_t j = 0; j < process->argc; j++) {
            free(process->argv[j]);
        }
        free(process->argv);
    }
    free(job->processes);
}

int main(void) {
    signal(SIGINT, SIG_IGN);    // ignore ^C
    signal(SIGQUIT, SIG_IGN);   // ignore ^D
    signal(SIGTSTP, SIG_IGN);   // prevent the shell from being stopped
    signal(SIGTTOU, SIG_IGN);   // prevent the shell from stopping itself

    shell_gid = getpid();
    setpgid(shell_gid, shell_gid);
    tcsetpgrp(STDIN_FILENO, shell_gid);

    char cwd[4096];

    char *line = NULL;
    size_t line_size = 0;

    while (1) {
        int bad_line = 0;
        if (getcwd(cwd, 4096) == NULL) {
            fprintf(stderr, "path length exceeds limit");
            exit(-1);
        }

        refresh_all_bg_process_status();
        clean_bg_jobs();

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
                char job_status = get_job_status(job1);
                print_bg_job(job1, job_status);
            }
        }
        else if (strcmp(job->processes[0]->argv[0], "fg") == 0) {
            if (job->processes[0]->argc == 1) {
                if (bg_jobs.empty()) {
                    fprintf(stderr, "fg: no back ground job\n");
                    continue;
                }
                else {
                    Job *job1 = bg_jobs.back();
                    bg_jobs.pop_back();
                    put_job_in_fg(job1, 1);
                }
            }
            else {
                size_t bg_n = (size_t) atoi(job->processes[0]->argv[1] + 1);
                Job *job1 = _pop_bg_job_with_num(bg_n);
                if (job1 == NULL) {
                    fprintf(stderr, "fg: %zu: no such job\n", bg_n);
                    continue;
                }
                else {
                    put_job_in_fg(job1, 1);
                }
            }
        }

        else if (strcmp(job->processes[0]->argv[0], "bg") == 0) {
            if (job->processes[0]->argc == 1) {
                if (bg_jobs.empty()) {
                    fprintf(stderr, "bg: no back ground job\n");
                    continue;
                }
                else {
                    Job *job1 = bg_jobs.back();
                    bg_jobs.pop_back();
                    put_job_in_bg(job1, 1);
                }
            }
            else {
                size_t bg_n = (size_t) atoi(job->processes[0]->argv[1] + 1);
                Job *job1 = _pop_bg_job_with_num(bg_n);
                if (job1 == NULL) {
                    fprintf(stderr, "bg: %zu: no such job\n", bg_n);
                    continue;
                }
                else {
                    put_job_in_bg(job1, 1);
                }
            }
        }
        else {
            int bg = has_bg_sign(job);
            launch_job(job, bg);
        }
    }

// end of main loop
// cleaning up
    free(line);
    for (auto job:bg_jobs) {
        free_job(job);
    }
    return 0;
}
