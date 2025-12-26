#include <gtest/gtest.h>
#include "pipeline.h"
#include "bench_metrics.h"

TEST(Pipeline, StartStopNoCrash) {
    reset_processed_items();

    BenchConfig cfg;
    Pipeline mt(&cfg);
    mt.start();

    // short run
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    mt.stop();

    long long p = get_processed_items();
    EXPECT_GE(p, 0);
}
