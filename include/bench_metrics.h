#ifndef BENCH_METRICS_H
#define BENCH_METRICS_H

#include <atomic>

inline std::atomic<long long>& processed_items_storage() {
    static std::atomic<long long> inst{0};
    return inst;
}

inline void reset_processed_items() { processed_items_storage().store(0); }
inline void inc_processed_items(long long v=1) { processed_items_storage().fetch_add(v); }
inline long long get_processed_items() { return processed_items_storage().load(); }

#endif // BENCH_METRICS_H
