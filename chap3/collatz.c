#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, const char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <number>\n", argv[0]);
        return 1;
    }
    int n = 0; sscanf(argv[1], "%d", &n);
    if (n <= 0) {
        fprintf(stderr, "Usage: %s <number>\n", argv[0]);
        return 1;
    }
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return 1;
    } else if (pid == 0) {
        while (n > 1) {
            printf("%d, ", n);
            if (n % 2 == 1) {
                n = 3 * n + 1;
            } else {
                n /= 2;
            }
        }
        puts("1.");
    } else {
        wait(NULL);
    }
}
