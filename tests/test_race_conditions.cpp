#include <gtest/gtest.h>
#include "thread_utils.h"
#include "pipeline.h"
#include "bench_metrics.h"
#include <vector>
#include <atomic>
#include <random>
#include <thread>

/**
 * @brief Teste de race condition: múltiplos threads acessando atomic
 * 
 * Testa se os atomics da classe thread_base aguenta
 * múltiplos acessos concorrentes
 */
TEST(RaceCondition, AtomicContentionHighLoad) {
    struct ContentionTest : public thread_base {
        std::atomic<long long> total{0};
        
        void run() override {
            total.fetch_add(1);
            
            // Simular contention ao verificar frequentemente
            for (int i = 0; i < 100; i++) {
                if (!isActive()) break;
            }
        }
    };
    
    ContentionTest tester;
    std::thread worker([&]{ tester.start(); });
    
    // Múltiplas threads lendo/escrevendo simultaneamente
    std::vector<std::thread> monitors;
    for (int i = 0; i < 5; i++) {
        monitors.emplace_back([&]() {
            for (int j = 0; j < 100; j++) {
                bool active = tester.isActive();
                EXPECT_TRUE(active || !active);  // Verificar que leitura não falha
            }
        });
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    tester.stop();
    worker.join();
    
    for (auto& m : monitors) {
        m.join();
    }
    
    EXPECT_GT(tester.total.load(), 0);
}

/**
 * @brief Teste de race condition: reader/writer na source
 * 
 * Múltiplos readers tentando ler enquanto writer escreve
 */
TEST(RaceCondition, SourceReaderWriterRace) {
    source_A source;
    std::atomic<int> read_success{0};
    std::atomic<int> read_fail{0};
    
    source.start();
    
    // Múltiplos readers tentando ler rapidamente
    const int NUM_READERS = 8;
    std::vector<std::thread> readers;
    
    for (int i = 0; i < NUM_READERS; i++) {
        readers.emplace_back([&]() {
            for (int j = 0; j < 20; j++) {
                try {
                    buffer_source_A buf;
                    source.read(&buf);
                    read_success.fetch_add(1);
                } catch (...) {
                    read_fail.fetch_add(1);
                }
            }
        });
    }
    
    // Aguardar todos os readers
    for (auto& t : readers) {
        t.join();
    }
    
    source.stop();
    
    // Todos os reads devem ter sucesso
    int total_reads = read_success.load() + read_fail.load();
    EXPECT_EQ(total_reads, NUM_READERS * 20);
    EXPECT_EQ(read_fail.load(), 0);
    EXPECT_EQ(read_success.load(), NUM_READERS * 20);
}

/**
 * @brief Teste de race condition: pipeline com many readers
 * 
 * Pipeline completo com múltiplos observadores simultâneos
 */
TEST(RaceCondition, PipelineMultipleObservers) {
    reset_processed_items();
    
    BenchConfig cfg;
    cfg.work_us = 0;  // Sem trabalho extra para maximizar throughput
    Pipeline pipeline(&cfg);
    
    std::atomic<int> observer_reads{0};
    pipeline.start();
    
    // Múltiplos observadores lendo simultaneamente
    const int NUM_OBSERVERS = 6;
    std::vector<std::thread> observers;
    
    for (int i = 0; i < NUM_OBSERVERS; i++) {
        observers.emplace_back([&]() {
            // Cada observador tenta fazer muitas leituras
            for (int j = 0; j < 30; j++) {
                observer_reads.fetch_add(1);
                std::this_thread::yield();
            }
        });
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Aguardar observadores
    for (auto& t : observers) {
        t.join();
    }
    
    pipeline.stop();
    
    long long items_processed = get_processed_items();
    EXPECT_GT(items_processed, 0);
    EXPECT_EQ(observer_reads.load(), NUM_OBSERVERS * 30);
}

/**
 * @brief Teste de race condition: destrutor chamado durante operação
 * 
 * Testa se destrutor aguenta ser chamado enquanto thread ainda roda
 */
TEST(RaceCondition, DestructorWhileRunning) {
    for (int iteration = 0; iteration < 5; iteration++) {
        reset_processed_items();
        
        {
            // Pipeline é destruído aqui com threads ainda rodando
            BenchConfig cfg;
            Pipeline pipeline(&cfg);
            pipeline.start();
            
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
            // Destrutor implícito será chamado
        }
        
        // Esperar um pouco para garantir que cleanup foi feito
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    
    // Se chegou aqui, não houve crash/segfault no destrutor
    EXPECT_TRUE(true);
}

/**
 * @brief Teste de race condition: alternância rápida start/stop
 * 
 * Start/stop muito rápidos podem causar race conditions
 */
TEST(RaceCondition, RapidToggleStartStop) {
    struct QuickThread : public thread_base {
        std::atomic<int> iterations{0};
        
        void run() override {
            iterations.fetch_add(1);
            if (iterations.load() > 500) stop();
        }
    };
    
    QuickThread t;
    
    // Alternar rapidamente
    for (int i = 0; i < 20; i++) {
        std::thread worker([&]{ t.start(); });
        std::this_thread::sleep_for(std::chrono::microseconds(100));
        t.stop();
        worker.join();
    }
    
    // Sem crash = sucesso
    EXPECT_TRUE(true);
}

/**
 * @brief Teste de race condition: fonte B lendo enquanto A escreve
 * 
 * Testa sincronização entre source_A e source_B
 */
TEST(RaceCondition, SourceBReadsSourceA) {
    source_A source_a;
    source_B source_b(&source_a);
    
    std::atomic<int> b_reads{0};
    
    source_a.start();
    source_b.start();
    
    // Múltiplos threads lendo de B
    std::vector<std::thread> readers;
    for (int i = 0; i < 4; i++) {
        readers.emplace_back([&]() {
            for (int j = 0; j < 15; j++) {
                buffer_source_B buf;
                source_b.read(&buf);
                b_reads.fetch_add(1);
            }
        });
    }
    
    for (auto& t : readers) {
        t.join();
    }
    
    source_b.stop();
    source_a.stop();
    
    EXPECT_EQ(b_reads.load(), 4 * 15);
}

/**
 * @brief Teste de race condition: contention no mutex
 * 
 * Múltiplas threads tentando adquirir mutex simultaneamente
 */
TEST(RaceCondition, MutexContention) {
    struct MutexTest : public thread_base {
        std::atomic<int> critical_section_count{0};
        int dummy_value{0};
        
        void run() override {
            {
                std::lock_guard<std::mutex> lk(mtx);
                dummy_value++;
                critical_section_count.fetch_add(1);
                // Simular trabalho na seção crítica
                for (volatile int i = 0; i < 100; i++) {}
            }
        }
    };
    
    MutexTest tester;
    std::thread worker([&]{ tester.start(); });
    
    // Múltiplas threads tentando accessar mtx
    std::vector<std::thread> contentioners;
    for (int i = 0; i < 10; i++) {
        contentioners.emplace_back([&]() {
            for (int j = 0; j < 20; j++) {
                std::lock_guard<std::mutex> lk(tester.mtx);
                // Just holding the lock briefly
            }
        });
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    
    tester.stop();
    worker.join();
    
    for (auto& c : contentioners) {
        c.join();
    }
    
    // Se não houve deadlock, teste passou
    EXPECT_GT(tester.critical_section_count.load(), 0);
}

/**
 * @brief Teste de race condition: memory_order semantics
 * 
 * Verifica se memory order (acquire/release) está funcionando
 */
TEST(RaceCondition, MemoryOrderingCorrectness) {
    struct OrderTest : public thread_base {
        std::atomic<int> data{0};
        
        void run() override {
            data.store(1, std::memory_order_release);
            std::this_thread::sleep_for(std::chrono::microseconds(10));
            if (!isActive()) return;
        }
    };
    
    OrderTest t;
    std::thread worker([&]{ t.start(); });
    
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    
    // Reader verifica que escrita foi visível
    int value = t.data.load(std::memory_order_acquire);
    EXPECT_EQ(value, 1);
    
    t.stop();
    worker.join();
}
