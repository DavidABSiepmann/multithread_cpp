#include "process_thread.h"
#include <chrono>

// process_A implementations
void process_A::run(void)
{
    if (!cap) {
        printf("[process_A] Capture não inicado!!\n");
        return;
    }

    buffer_source_A val;
    cap->read(&val);

    startProfile("pcA");
    process_buffer(&val.data);
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    stopProfile("pcA");
}

void process_A::process_buffer(int *buffer)
{
    if (cfg && cfg->work_us > 0) {
        std::this_thread::sleep_for(std::chrono::microseconds(cfg->work_us));
    }
}

// process_B implementations
void process_B::run(void)
{
    if (!cap) {
        printf("[process_B] Capture não inicado!!\n");
        return;
    }

    buffer_source_B val;
    cap->read(&val);

    startProfile("pcB");
    process_buffer(&val.data);
    std::this_thread::sleep_for(std::chrono::milliseconds(57));
    stopProfile("pcB");
}

void process_B::process_buffer(int *buffer)
{
    if (cfg && cfg->work_us > 0) {
        std::this_thread::sleep_for(std::chrono::microseconds(cfg->work_us));
    }
    inc_processed_items(1);
}
