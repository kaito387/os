#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>

#define SIZE 4 << 16

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <number>\n", argv[0]);
        return 1;
    }
    int n = 0; sscanf(argv[1], "%d", &n);
    if (n <= 0) {
        fprintf(stderr, "Usage: %s <number>\n", argv[0]);
        return 1;
    }

    int fd = shm_open("/myshm", O_CREAT | O_RDWR, 0666);
    ftruncate(fd, SIZE);
    int *ptr = mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return 1;
    } else if (pid == 0) {
        while (n > 1) {
            *ptr++ = n;
            if (n % 2 == 1) {
                n = n * 3 + 1;
            } else {
                n /= 2;
            }
        }
        *ptr++ = 1;
    } else {
        wait(NULL);
        for (; *ptr > 1; ++ptr) {
            printf("%d, ", *ptr);
        }
        puts("1.");
        shm_unlink("/myshm");
    }
    return 0;
}
