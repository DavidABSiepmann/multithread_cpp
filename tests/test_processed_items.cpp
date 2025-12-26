#include <gtest/gtest.h>
#include "bench_metrics.h"

TEST(BenchMetrics, IncrementAndReset) {
    reset_processed_items();
    EXPECT_EQ(get_processed_items(), 0);

    inc_processed_items(5);
    EXPECT_EQ(get_processed_items(), 5);

    reset_processed_items();
    EXPECT_EQ(get_processed_items(), 0);
}
