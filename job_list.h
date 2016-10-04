#ifndef JOB_LIST_H
#define JOB_LIST_H

#include "shell.h"

typedef struct JobNode {
    Job *job;
    struct JobNode *next;
} JobNode;

typedef struct JobList {
    JobNode *head;
    JobNode *tail;
} JobList;


#endif //CMPT300_A1P2_JOB_LIST_H
