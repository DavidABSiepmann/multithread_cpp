#include <gtest/gtest.h>
#include "pipeline.h"
#include "bench_metrics.h"
#include "profile_print.h"
#include <vector>
#include <atomic>

/**
 * @brief Teste de stress: pipeline com muitos ciclos
 * 
 * Verifica se o pipeline consegue processar muitos ciclos sem
 * vazamento de memória ou deadlock
 */
TEST(StressTest, HighThroughputLongDuration) {
    reset_processed_items();
    
    BenchConfig cfg;
    cfg.work_us = 10;  // Trabalho mínimo para testar throughput
    
    Pipeline pipeline(&cfg);
    pipeline.start();
    
    // Deixar rodando por mais tempo para acumular eventos
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    pipeline.stop();
    
    long long p = get_processed_items();
    // Deve processar muitos itens em 500ms
    EXPECT_GT(p, 10);
    EXPECT_LT(p, 10000);  // Não deve explodir exponencialmente
}

/**
 * @brief Teste de stress: múltiplos start/stop rápidos
 * 
 * Verifica se o pipeline aguenta múltiplos ciclos de
 * start e stop sem deadlock ou crash
 */
TEST(StressTest, RapidStartStop) {
    for (int i = 0; i < 10; i++) {
        reset_processed_items();
        
        BenchConfig cfg;
        Pipeline pipeline(&cfg);
        
        pipeline.start();
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        pipeline.stop();
        
        // Esperar um pouco para evitar race nas threads
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    // Se chegou aqui, não houve deadlock/crash
    EXPECT_TRUE(true);
}

/**
 * @brief Teste de stress: múltiplos pipelines simultâneos
 * 
 * Verifica se múltiplos pipelines conseguem rodar em paralelo
 * sem interferência um do outro
 */
TEST(StressTest, MultiplePipelinesParallel) {
    reset_processed_items();
    
    const int NUM_PIPELINES = 3;
    std::vector<Pipeline*> pipelines;
    std::vector<std::thread> threads;
    
    // Criar múltiplos pipelines
    for (int i = 0; i < NUM_PIPELINES; i++) {
        BenchConfig *cfg = new BenchConfig();
        cfg->work_us = 5;
        pipelines.push_back(new Pipeline(cfg));
    }
    
    // Iniciar todos em paralelo
    for (auto p : pipelines) {
        p->start();
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Parar todos
    for (auto p : pipelines) {
        p->stop();
    }
    
    // Cleanup
    for (auto p : pipelines) {
        delete p;
    }
    
    long long items = get_processed_items();
    EXPECT_GT(items, 0);
}

/**
 * @brief Teste de stress: ProfilePrinter com muitas escritas
 * 
 * Verifica se o ProfilePrinter consegue lidar com muitas
 * chamadas de escrita simultâneas (não testa sincronização externa)
 */
TEST(StressTest, ProfilePrinterStress) {
    std::string temp_file = "/tmp/stress_profile_test.csv";
    
    // Remover arquivo se existir
    std::remove(temp_file.c_str());
    
    // Usar o open_file() em vez de set_stream() para inicializar corretamente
    bool opened = ProfilePrinter::get().open_file(temp_file);
    EXPECT_TRUE(opened);
    
    // Simular múltiplas escritas rápidas
    const int ITERATIONS = 50;
    for (int i = 0; i < ITERATIONS; i++) {
        ProfilePrinter::get().write_line("stress_test", 
                                         (long long)i * 1000, 
                                         i % 2);
    }
    
    // Esperar um pouco para garantir flush
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    // Verificar que o arquivo existe
    std::ifstream check(temp_file);
    EXPECT_TRUE(check.is_open());
    check.close();
    
    // Cleanup
    std::remove(temp_file.c_str());
}

/**
 * @brief Teste de stress: condition variable com múltiplos readers
 * 
 * Verifica se a condition variable aguenta múltiplos readers/writers
 */
TEST(StressTest, CondVarMultipleWaiters) {
    source_A source;
    std::atomic<int> read_count{0};
    
    // Thread produtor
    source.start();
    
    // Múltiplos threads consumidores
    const int NUM_READERS = 4;
    std::vector<std::thread> readers;
    
    for (int i = 0; i < NUM_READERS; i++) {
        readers.emplace_back([&]() {
            buffer_source_A buf;
            for (int j = 0; j < 10; j++) {
                source.read(&buf);
                read_count.fetch_add(1);
            }
        });
    }
    
    // Aguardar todos os readers terminarem
    for (auto& t : readers) {
        t.join();
    }
    
    source.stop();
    
    // Cada reader fez 10 leituras = 4 * 10 = 40 leituras
    EXPECT_EQ(read_count.load(), NUM_READERS * 10);
}

/**
 * @brief Teste de stress: alternância rápida de mute/unmute
 * 
 * Verifica se ProfilePrinter aguenta alternâncias rápidas
 */
TEST(StressTest, MuteUnmuteCycles) {
    std::string temp_file = "/tmp/mute_test.csv";
    
    std::ofstream out(temp_file);
    ProfilePrinter::get().set_stream(std::move(out));
    
    // Alternar rapidamente entre muted/unmuted
    for (int i = 0; i < 50; i++) {
        if (i % 2 == 0) {
            ProfilePrinter::get().mute();
        } else {
            ProfilePrinter::get().unmute();
        }
        ProfilePrinter::get().write_line("mute_test", i, 0);
    }
    
    std::ifstream check(temp_file);
    EXPECT_TRUE(check.is_open());
    check.close();
    
    std::remove(temp_file.c_str());
}
