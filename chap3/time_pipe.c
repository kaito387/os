#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>

#define READ_END 0
#define WRITE_END 1

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <command> [args...]\n", argv[0]);
        return 1;
    }

    int fd[2];
    if (pipe(fd) == -1) {
        perror("Pipe falied");
        return 1;
    }
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return 1;
    } else if (pid == 0) {
        close(fd[READ_END]);
        struct timeval start;
        gettimeofday(&start, NULL);
        write(fd[WRITE_END], &start, sizeof start);
        close(fd[WRITE_END]);
        execvp(argv[1], argv + 1);
        perror("execvp failed");
        return 1;
    } else {
        close(fd[WRITE_END]);
        wait(NULL);
        struct timeval start, end;
        gettimeofday(&end, NULL);
        read(fd[READ_END], &start, sizeof start);
        close(fd[READ_END]);

        double elapsed = (end.tv_sec - start.tv_sec) * 1e3 +
                         (end.tv_usec - start.tv_usec) / 1e3;
        printf("Elapsed time: %.3f ms\n", elapsed);
    }
    return 0;
}
