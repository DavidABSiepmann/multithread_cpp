#include "source_process_threads.h"
#include <chrono>

void source_B::run(void)
{
    if (!cap) {
        printf("[source_B] Capture não inicado!!\n");
        return;
    }

    buffer_source_A val;
    cap->read(&val);

    startProfile("sB");
    process_buffer(&val.data);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    stopProfile("sB");
}

void source_B::process_buffer(int *value)
{
    int temp_value = *value + 1000;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    {
        std::lock_guard<std::mutex> lk(mtx);
        startProfile("sB_mtx");
        buffer.data = temp_value;
        cv_.notify_all();
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
        stopProfile("sB_mtx");
    }
}

void source_B::read(buffer_source_B *dado)
{
    std::unique_lock<std::mutex> lk(mtx);
    startProfile("sB_read");
    cv_.wait(lk, [this] { return true; });
    *dado = buffer;
    stopProfile("sB_read");
}
