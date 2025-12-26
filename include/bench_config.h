#ifndef BENCH_CONFIG_H
#define BENCH_CONFIG_H

#include <cstdint>
#include <string>
#include <cstdio>

struct BenchConfig {
    int threads = 4; // not used for now
    int producers = 1;
    int consumers = 1;
    int batch_size = 1;
    int queue_capacity = 0;
    int work_us = 0; // microseconds of simulated work per processed item
    int duration_s = 1; // seconds
    int warmup = 0; // number of warmup runs
    int repeats = 1; // number of repeats
    unsigned int seed = 0;
    std::string out_file = ""; // optional path to append per-run CSV results
    std::string profile_file = ""; // file path to write profile events (thread,time,status)
};


#endif // BENCH_CONFIG_H
