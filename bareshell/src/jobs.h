#ifndef JOBS_H
#define JOBS_H

#include <sys/types.h>

typedef enum {
    JOB_STATUS_SUBMITTED,   // Just submitted/assembled
    JOB_STATUS_RUNNING,     // Currently executing
    JOB_STATUS_FINISHED,    // Completed successfully
    JOB_STATUS_TERMINATED   // Killed or crashed
} JobStatus;

typedef struct {
    int id;                 // Internal PID (1, 2, 3...)
    pid_t pid;              // OS PID
    char* filename;         // Source filename (e.g. "test.asm")
    char* binary_filename;  // Binary filename (e.g. "test.bin")
    JobStatus status;
} Job;

#endif
