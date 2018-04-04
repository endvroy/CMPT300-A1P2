# my_shell

My_shell that supports pipline, job control and graceful handling.

### Compilation
- C++11 standard, POSIX.1-2008, and 4.4BSD required for compilation

### Implementation Details
This project refers to the FAQ on https://www.gnu.org/software/libc/manual/html_node/Implementing-a-Shell.html#Implementing-a-Shell, in the following Aspects:

- The design of Process and Job structs
- An optional 'continue' parameter in put_job_in_fg and put_job_in_bg
- Setting up pipes in launch_job

Except for these aspect, my_shell has a unique design:

- Distinction of foreground jobs and background jobs. A data structure tracks backgound jobs and checks its status periodically. 
- Use of different data structures. This project uses extendable arrays in Job, and C++ deque to track all jobs
- The design of job and process status. This project uses process status including 'r' for running, 's' for stopped, and 't' for terminated. This project does not distinguish whether a process exiting normally or being killed by a interrupt. When a process ends its execution, it is 'terminated'.
- Clear job status. Job status are mutually exclusive in this project, in contrast to the original material.
- This project trims anything I think as unnecesary in the original material.

### Usage
- run executable compiled to start
- 'exit' to exit (Ctrl+D dosn't work)