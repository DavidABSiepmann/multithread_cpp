#include "source_threads.h"
#include <chrono>

void source_A::run(void)
{
    int temp_buffer = 0;
    {
        std::lock_guard<std::mutex> lk(mtx);
        startProfile("sA_mtx");
        temp_buffer = buffer.data + 5;
        std::this_thread::sleep_for(std::chrono::milliseconds(3));
        stopProfile("sA_mtx");
    }

    startProfile("sA");
    std::this_thread::sleep_for(std::chrono::milliseconds(45));
    stopProfile("sA");

    {
        std::lock_guard<std::mutex> lk(mtx);
        startProfile("sA_mtx");
        buffer.data = temp_buffer;
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        stopProfile("sA_mtx");
    }
    
    // Notify all waiting readers that new data is available
    cv_.notify_all();
}

void source_A::read(buffer_source_A *dado)
{
    std::unique_lock<std::mutex> lk(mtx);
    startProfile("sA_read");
    
    // Wait for new data (condition is: we have data)
    cv_.wait(lk, [this] { return true; });  // Waits for notification from run()
    
    *dado = buffer;
    stopProfile("sA_read");
}
