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
#include <sys/capability.h>

#include "ns.h"

#define STACK_SIZE (1024*1024)
static char child_stack[STACK_SIZE];
char* const child_args[] = {
    "/bin/bash", NULL
};

void set_uid_map(pid_t pid, int inside_id, int outside_id, int length) {
    char path[256];
    sprintf(path, "/proc/%d/uid_map", pid);
    printf("%s\n", path);
    FILE* uid_map = fopen(path, "w");
    fprintf(uid_map, "%d %d %d", inside_id, outside_id, length);
    int ret = fclose(uid_map);
    if (ret == -1) {
        printf("Something went wrong with fclose()! %s\n", strerror(errno));
    }
    printf("fclose ret=%d\n", ret);
}

void set_gid_map(pid_t pid, int inside_id, int outside_id, int length) {
    char path[256];
    sprintf(path, "/proc/%d/gid_map", pid);
    FILE* gid_map = fopen(path, "w");
    fprintf(gid_map, "%d %d %d", inside_id, outside_id, length);
    fclose(gid_map);
}

int child_main(void* args) {
    printf("In child-process main!\n");
    std::string hostname = "UTCLAB";
    int ret = sethostname(hostname.c_str(), hostname.size());
    if (ret == -1) {
        printf("Something went wrong with sethostname()! %s\n", strerror(errno));
    }

    execv(child_args[0], child_args);
    return 1;
}

int child_main2(void* args) {
    printf("In child-process main2!\n");
    set_uid_map(getpid(), 0, 1000, 1);
    set_gid_map(getpid(), 0, 1000, 1);

    std::string hostname = "UTCLAB";
    int ret = sethostname(hostname.c_str(), hostname.size());
    if (ret == -1) {
        printf("Something went wrong with sethostname()! %s\n", strerror(errno));
    }

    cap_t caps;
    printf("eUID=%ld; eGID=%ld; ", (long)geteuid(), (long)getegid());
    caps = cap_get_proc();
    printf("Caps: %s\n", cap_to_text(caps, NULL));
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

int ipc_utc_main() {
    printf("IPC-UTC main start: \n");
    int child_pid = clone(child_main, child_stack + STACK_SIZE,
            SIGCHLD | CLONE_NEWUTS | CLONE_NEWIPC,
            NULL);
    waitpid(child_pid, NULL, 0);
    printf("IPC-UTC main end\n");
    return 0;
}

int pid_ipc_utc_main() {
    printf("PID-IPC-UTC main start: \n");
    int child_pid = clone(child_main, child_stack + STACK_SIZE,
            SIGCHLD | CLONE_NEWUTS | CLONE_NEWIPC | CLONE_NEWPID,
            NULL);
    waitpid(child_pid, NULL, 0);
    printf("PID-IPC-UTC main end\n");
    return 0;
}

int ns_pid_ipc_utc_main() {
    printf("NS-PID-IPC-UTC main start: \n");
    int child_pid = clone(child_main, child_stack + STACK_SIZE,
            SIGCHLD | CLONE_NEWUTS | CLONE_NEWIPC | CLONE_NEWPID | CLONE_NEWNS,
            NULL);
    waitpid(child_pid, NULL, 0);
    printf("NS-PID-IPC-UTC main end\n");
    return 0;
}

int user_ns_pid_ipc_utc_main() {
    printf("USER-NS-PID-IPC-UTC main start: \n");
    char path[256];
    sprintf(path, "/proc/%d/uid_map", getpid());
    printf("%s\n", path);
    int child_pid = clone(child_main2, child_stack + STACK_SIZE,
            SIGCHLD | CLONE_NEWUSER,
            /* SIGCHLD | CLONE_NEWUSER | CLONE_NEWUTS, */
            /* SIGCHLD | CLONE_NEWUTS | CLONE_NEWIPC | CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWUSER, */
            NULL);
    waitpid(child_pid, NULL, 0);
    printf("NS-PID-IPC-UTC main end\n");
    return 0;
}
