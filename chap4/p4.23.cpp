#include <pthread.h>
#include <iostream>

void *print_primes(void *args) {
    int n = *(int *)args;
    std::cout << "Prime numbers up to " << n << ": ";
    for (int i = 2; i <= n; i++) {
        bool is_prime = true;
        for (int j = 2; j <= i / 2; j++) {
            if (i % j == 0) {
                is_prime = false;
                break;
            }
        }
        if (is_prime) {
            std::cout << i << " ";
        }
    }
    std::cout << std::endl;
    pthread_exit(nullptr);
}

int main() {
    int n;
    std::cin >> n;
    pthread_t tid;
    pthread_create(&tid, nullptr, print_primes, &n);
    pthread_join(tid, nullptr);
    return 0;
}