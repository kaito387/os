#include <iostream>
#include <pthread.h>
#include <cstdlib>
#include <climits>

// Global variables shared among threads
double average;
int maximum;
int minimum;

// Structure to pass arguments to threads
struct ThreadArgs {
    int *numbers;
    int count;
};

// Thread 1: Calculate average
void *calc_average(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    int *numbers = args->numbers;
    int count = args->count;

    double sum = 0;
    for (int i = 0; i < count; i++) {
        sum += numbers[i];
    }
    average = sum / count;

    pthread_exit(nullptr);
}

// Thread 2: Calculate maximum
void *calc_maximum(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    int *numbers = args->numbers;
    int count = args->count;

    int max_val = INT_MIN;
    for (int i = 0; i < count; i++) {
        if (numbers[i] > max_val) {
            max_val = numbers[i];
        }
    }
    maximum = max_val;

    pthread_exit(nullptr);
}

// Thread 3: Calculate minimum
void *calc_minimum(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    int *numbers = args->numbers;
    int count = args->count;

    int min_val = INT_MAX;
    for (int i = 0; i < count; i++) {
        if (numbers[i] < min_val) {
            min_val = numbers[i];
        }
    }
    minimum = min_val;

    pthread_exit(nullptr);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <num1> <num2> ... <numN>" << std::endl;
        return 1;
    }

    int count = argc - 1;
    int *numbers = new int[count];

    // Parse command-line arguments
    for (int i = 0; i < count; i++) {
        numbers[i] = std::atoi(argv[i + 1]);
    }

    ThreadArgs args = {numbers, count};

    pthread_t tid_avg, tid_max, tid_min;

    // Create three worker threads
    pthread_create(&tid_avg, nullptr, calc_average, &args);
    pthread_create(&tid_max, nullptr, calc_maximum, &args);
    pthread_create(&tid_min, nullptr, calc_minimum, &args);

    // Wait for all threads to finish
    pthread_join(tid_avg, nullptr);
    pthread_join(tid_max, nullptr);
    pthread_join(tid_min, nullptr);

    // Output results
    std::cout << "The average value is " << average << std::endl;
    std::cout << "The minimum value is " << minimum << std::endl;
    std::cout << "The maximum value is " << maximum << std::endl;

    delete[] numbers;
    return 0;
}