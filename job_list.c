#include "job_list.h"

JobNode *_new_job_node(Job *job) {
    JobNode *job_node = malloc(sizeof(JobNode));
    if (job_node == NULL) {
        return NULL;
    }
    job_node->job = job;
    job_node->next = NULL;
    return job_node;
}

JobList *new_list() {
    JobList *list = malloc(sizeof(JobList));
    if (list == NULL) {
        return NULL;
    }
    list->head = list->tail = NULL;
    return list;
}

int list_append(JobList *list, Job *job) {
    JobNode *new_node = _new_job_node(job);
    if (new_node == NULL) {
        return -1;
    }
    if (list->head == NULL) {   // empty list
        list->head = list->tail = new_node;
        return 0;
    }
    else {
        list->tail->next = new_node;
        list->tail = new_node;
        return 0;
    }
}

//JobNode *_list_find_node(JobList *list, size_t job_no) {
//    JobNode *node = list->head;
//
//}
//
//Job *list_pop(JobList *list, size_t job_no) {
//    if (list->head == node) {
//        list->head = list->head->next;
//    }
//    if (list->tail == node) {
//
//    }
//}