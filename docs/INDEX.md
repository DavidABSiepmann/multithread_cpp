# 📚 Documentation Index

Bem-vindo à documentação do **Multithreaded Pipeline Benchmark**! Use este índice para encontrar o que procura.

---

## 🚀 Começar Rápido

- **[QUICKSTART.md](./QUICKSTART.md)** (5 min) — Setup, primeiro benchmark, gráficos
  - Clone, build, execute benchmark em 30 segundos
  - Parâmetros comuns e troubleshooting básico

---

## 🏗️ Arquitetura & Design

- **[ARCHITECTURE.md](./ARCHITECTURE.md)** (30 min) — Deep dive no design
  - Visão geral do sistema (4 threads, fluxo de dados)
  - Componentes: thread_base, source_A/B, process_A/B, Pipeline
  - ProfilePrinter, BenchConfig, BenchMetrics
  - Padrões: Template Method, Singleton, Dependency Injection, RAII, Atomics
  - Sincronização thread-safe (mutex + RAII, atomic + memory_order)
  - Testing strategy e performance considerations

- **[DECISIONS.md](./DECISIONS.md)** (20 min) — Por quê decidimos assim?
  - Decision 1: 4 threads fixos vs. dynamic pool
  - Decision 2: Singleton para ProfilePrinter
  - Decision 3: Mutex per buffer (lock contention)
  - Decision 4: Index-based change detection vs. condvar
  - Decision 5: Atomic<bool> com memory ordering vs. volatile
  - Decision 6: Config injetada vs. global/const
  - Decision 7: Header-only vs. .cpp split
  - Decision 8: RAII locks vs. manual
  - Decision 9: FILE* vs. std::ofstream
  - Escalabilidade e otimizações futuras

---

## 📖 Guias Práticos

- **[../README.md](../README.MD)** (Main) — Overview completo do projeto
  - Descrição curta
  - Motivação (padrões, profiling, benchmarking)
  - Instalação (CMake, pré-requisitos)
  - Como usar (exemplos, parâmetros)
  - Resultados/Benchmarks (interpretação de CSV)
  - Tecnologias
  - Testes (rodar, sanitizers)
  - Licença (MIT)

- **[../CONTRIBUTING.md](../CONTRIBUTING.md)** — Development guidelines
  - Code style (C++17, naming, headers)
  - Building (release, debug, sanitizers)
  - Testing (unit, smoke, TSAN)
  - Adding features (processo)
  - Debugging e profiling

---

## 📁 Estrutura de Arquivos

```
multithread_cpp/
├── README.MD              ← Start here! Overview do projeto
├── LICENSE                ← MIT License
├── CONTRIBUTING.md        ← Development guidelines
│
├── docs/                  ← You are here
│   ├── INDEX.md          ← This file
│   ├── QUICKSTART.md     ← 5-minute setup
│   ├── ARCHITECTURE.md   ← Technical deep dive
│   └── DECISIONS.md      ← Design rationale
│
├── include/              ← Headers (declaration-only)
│   ├── thread_utils.h    ← thread_base (RAII, atomic<bool>)
│   ├── source_threads.h  ← source_A (producer)
│   ├── source_process_threads.h ← source_B (processor)
│   ├── process_thread.h  ← process_A, process_B (consumers)
│   ├── pipeline.h        ← Pipeline (orchestrator)
│   ├── profile_print.h   ← ProfilePrinter (singleton logger)
│   ├── bench_config.h    ← BenchConfig struct
│   └── bench_metrics.h   ← Atomics (processed_items counter)
│
├── src/                  ← Implementations (.cpp files)
│   ├── main.cpp
│   ├── thread_utils.cpp
│   ├── source_threads.cpp
│   ├── source_process_threads.cpp
│   ├── process_thread.cpp
│   ├── profile_print.cpp
│   └── bench_globals.cpp (empty placeholder)
│
├── tests/                ← GoogleTest unit tests
│   ├── test_pipeline.cpp
│   ├── test_process_buffer.cpp
│   ├── test_profile_silencer.cpp
│   ├── test_processed_items.cpp
│   ├── test_throughput_smoke.cpp
│   └── CMakeLists.txt
│
├── bench/                ← Benchmarking utilities
│   └── grafico.py        ← Graph generation (matplotlib)
│
├── scripts/              ← Build automation
│   ├── build.sh
│   ├── run_tests.sh
│   ├── run_bench.sh
│   └── format.sh
│
├── outputs/              ← Generated CSV data
│   ├── results.csv       ← Throughput data
│   └── profile_events.csv ← Timeline events
│
└── assets/               ← Generated PNG graphs
    ├── profile.png
    └── results.png
```

---

## 🎓 Learning Path (Sugerido)

### Iniciante (30 min)
1. [README.md](../README.MD) — Overview e motivação
2. [QUICKSTART.md](./QUICKSTART.md) — Setup e primeiro benchmark
3. `./build/pipelines_cpp --help` — Ver argumentos disponíveis

### Intermediário (1.5 h)
1. [ARCHITECTURE.md](./ARCHITECTURE.md) — Entender design
2. Ler `include/thread_utils.h` → `include/pipeline.h` (header structure)
3. Ler `src/main.cpp` → `src/process_thread.cpp` (implementations)
4. Rodar testes: `./scripts/run_tests.sh`

### Avançado (2-3 h)
1. [DECISIONS.md](./DECISIONS.md) — Entender trade-offs
2. [CONTRIBUTING.md](../CONTRIBUTING.md) — Contribution guidelines
3. Experimente:
   - Mudar `--work-us` e ver impacto em throughput
   - Ativar sanitizers: `cmake .. -DENABLE_SANITIZERS=ON`
   - Modificar número de threads em `pipeline.h`

---

## ❓ FAQ Rápido

### Como executar um benchmark?
```bash
./build/pipelines_cpp --duration 5 --work-us 50 --out outputs/results.csv --profile outputs/profile_events.csv
python3 bench/grafico.py
```

### Como rodar testes?
```bash
./scripts/run_tests.sh
```

### Como detectar race conditions?
```bash
cmake .. -DENABLE_SANITIZERS=ON -DCMAKE_BUILD_TYPE=Debug
./build/tests/unit_tests
```

### Qual é a métrica principal?
**throughput_items_s** no arquivo `outputs/results.csv` — itens processados por segundo

### Como é o pipeline?
Source A → Process A  
Source A → Source B → Process B

Tudo em paralelo, sincronizado por mutexes e atomics.

### Por que 4 threads?
Simples, determinístico, bom para benchmarking. Veja [DECISIONS.md](./DECISIONS.md#-decision-1-pipeline-com-4-threads-fixos-vs-dynamic-pool)

---

## 🔗 Referências Externas

- **C++ Reference**: https://en.cppreference.com/ (std::atomic, std::thread, std::chrono)
- **GoogleTest**: https://github.com/google/googletest/blob/main/docs/primer.md
- **ThreadSanitizer**: https://clang.llvm.org/docs/ThreadSanitizer
- **CMake**: https://cmake.org/documentation/

---

## 📞 Contato & Issues

- **Questions?** Abra uma issue no GitHub
- **Encontrou bug?** Descreva em issue com reproduction steps
- **Quer contribuir?** Veja [CONTRIBUTING.md](../CONTRIBUTING.md)

---

**Última Atualização**: 2025  
**Status**: Production-ready ✓  
**Licença**: MIT (veja [LICENSE](../LICENSE))

Happy benchmarking! 🚀
