#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main() {
    pid_t pid = fork();  // 创建子进程

    if (pid < 0) {
        perror("fork failed");
        exit(1);
    } else if (pid == 0) {
        // 子进程
        printf("Child process (PID=%d) exiting...\n", getpid());
        exit(0);  // 子进程退出，但父进程尚未 wait
    } else {
        // 父进程
        printf("Parent process (PID=%d) sleeping. Child PID=%d\n", getpid(), pid);
        printf("Check 'ps -el' in another terminal to see the zombie process.\n");
        sleep(30);  // 父进程暂停 30 秒，子进程成为僵尸
        printf("Parent process exiting. Zombie child should now be reaped.\n");
    }

    return 0;
}
