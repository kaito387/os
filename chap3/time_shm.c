#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <command> [args...]\n", argv[0]);
        return 1;
    }

    const int SIZE = 4096;
    int fd = shm_open("/myshm", O_CREAT | O_RDWR, 0666);
    ftruncate(fd, SIZE);
    struct timeval *ptr = mmap(NULL, sizeof(struct timeval), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); 
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return 1;
    } else if (pid == 0) {
        struct timeval start;
        gettimeofday(&start, NULL);
        *ptr = start;
        // write(fd[1], &start, sizeof start);
        execvp(argv[1], argv + 1);
        perror("execvp failed");
        return 1;
    } else {
        wait(NULL);
        struct timeval start, end;
        gettimeofday(&end, NULL);
        start = *ptr;
        // read(fd[0], &start, sizeof start);

        double elapsed = (end.tv_sec - start.tv_sec) * 1e3 +
                         (end.tv_usec - start.tv_usec) / 1e3;
        printf("Elapsed time: %.3f ms\n", elapsed);
        shm_unlink("/myshm");
    }
    return 0;
}
