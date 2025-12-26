#include <gtest/gtest.h>
#include "thread_utils.h"
#include <thread>
#include <atomic>

struct TestThread : public thread_base {
    std::atomic<int> counter{0};
    void run() override {
        counter.fetch_add(1);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if(counter.load() > 1000) stop();
    }
};

TEST(ThreadBase, StartStop) {
    TestThread t;
    std::thread thr([&]{ t.start(); });

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    EXPECT_TRUE(t.isActive());

    t.stop();
    thr.join();

    EXPECT_FALSE(t.isActive());
    EXPECT_GT(t.counter.load(), 0);
}
