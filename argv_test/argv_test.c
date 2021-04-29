#include "io.h"
#include "system_call.h"

void main(int argc, char *argv[])
{
    int pid = sys_getpid();
    printf("Argv Test, pid %d\n", pid);
    for (int i = 0; i < argc; ++i) {
        printf("argv[%d]: %s\n", i, argv[i]);
    }

    // char *fork_argv[] = {"fork_test", 0};
    // exec("fork_test", fork_argv);
    
    while (1) {
        // sys_schedule();
    }
}