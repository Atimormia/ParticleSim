#pragma once
#include <chrono>
#include <functional>
#include <string>
#include <numeric>

class BenchmarkHelper
{
public:
    struct Result
    {
        double average_us; // per-iteration average (microseconds)
        double total_ms;   // total time (milliseconds)
        int iterations;

        std::string tostring()
        {
            std::ostringstream ss;
            ss << "Update average: " << average_us << " us\n";
            ss << "Total: " << total_ms << " ms over " << iterations << " iterations";
            return ss.str();
        }
    };

    // Runs benchmark on a callable F
    template <typename F>
    static Result run(F func,
                      int warmup_iterations = 200,
                      int measure_iterations = 5000)
    {
        // Warm-up: reduce cold-cache and branch-predictor effects
        for (int i = 0; i < warmup_iterations; i++)
            func();

        auto start = clock::now();

        for (int i = 0; i < measure_iterations; i++)
            func();

        auto end = clock::now();

        long long total_us =
            std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        Result r;
        r.total_ms = total_us / 1000.0f;
        r.iterations = measure_iterations;
        r.average_us = total_us / (float)measure_iterations;

        return r;
    }

private:
    using clock = std::chrono::high_resolution_clock;
};
