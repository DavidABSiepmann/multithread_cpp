#include <gtest/gtest.h>
#include "pipeline.h"
#include "bench_config.h"
#include "bench_metrics.h"

TEST(Throughput, Smoke) {
    reset_processed_items();
    // Ensure work is light via local config
    BenchConfig cfg;
    cfg.work_us = 0;

    Pipeline mt(&cfg);
    mt.start();

    // Poll for processed items, up to a timeout, to avoid flaky timing-dependent failures
    auto t0 = std::chrono::steady_clock::now();
    const auto timeout = std::chrono::milliseconds(2000);
    while (std::chrono::steady_clock::now() - t0 < timeout) {
        if (get_processed_items() >= 1) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    mt.stop();

    long long p = get_processed_items();
    // Expect at least some processing occurred
    EXPECT_GE(p, 1);
}
