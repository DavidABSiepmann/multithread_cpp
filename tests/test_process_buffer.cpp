#include <gtest/gtest.h>
#include "process_thread.h"
#include "bench_metrics.h"

TEST(ProcessBuffer, IncrementsProcessed) {
    reset_processed_items();
    process_B b;
    int val = 0;
    b.process_buffer(&val);
    EXPECT_GE(get_processed_items(), 1);
}
