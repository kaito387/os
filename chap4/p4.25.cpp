#include <iostream>
#include <random>
#include <chrono>
#include <omp.h>

int main() {
    const long long total_points = 100'000'000;

    long long in_circle = 0;
    #pragma omp parallel reduction(+:in_circle)
    {
        // Each thread gets its own independent RNG
        std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count() 
                         + omp_get_thread_num());
        std::uniform_real_distribution<double> dist(-1.0, 1.0);

        #pragma omp for
        for (long long i = 0; i < total_points; ++i) {
            double x = dist(rng);
            double y = dist(rng);
            if (x*x + y*y <= 1.0) {
                in_circle++;
            }
        }
    } // implicit barrier here

    double pi_estimate = (double)in_circle / total_points * 4.0;
    std::cout << "Estimated Pi: " << pi_estimate << std::endl;    
    return 0;
}