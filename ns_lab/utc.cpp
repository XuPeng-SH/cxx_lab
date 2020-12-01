#define __GNU_SOURCE
#include <sched.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <string>

#include "utc.h"

#define STACK_SIZE (1024*1024)
static char child_stack[STACK_SIZE];
char* const child_args[] = {
    "/bin/bash", NULL
};

int child_main(void* args) {
    printf("In child-process!\n");
    std::string hostname = "UTCLAB";
    int ret = sethostname(hostname.c_str(), hostname.size());
    if (ret == -1) {
        printf("Something went wrong with sethostname()! %s\n", strerror(errno));
    }
    execv(child_args[0], child_args);
    return 1;
}

int utc_main() {
    printf("UTC main start: \n");
    int child_pid = clone(child_main, child_stack + STACK_SIZE,
            // If not set CLONE_NEWUTC, the child process will change the gloabl hostname.
            // If set CLONE_NEWUTS, the child process will only change its own uts hostname
            SIGCHLD | CLONE_NEWUTS,
            NULL);
    waitpid(child_pid, NULL, 0);
    printf("UTC main end\n");
    return 0;
}
