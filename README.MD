# Multithreaded Pipeline Benchmark

[![CI](https://github.com/DavidABSiepmann/multithread_cpp/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/DavidABSiepmann/multithread_cpp/actions/workflows/ci.yml)
[![Sanitizers](https://github.com/DavidABSiepmann/multithread_cpp/actions/workflows/sanitizers.yml/badge.svg?branch=main)](https://github.com/DavidABSiepmann/multithread_cpp/actions/workflows/sanitizers.yml)
[![Nightly Stress](https://github.com/DavidABSiepmann/multithread_cpp/actions/workflows/nightly-stress.yml/badge.svg)](https://github.com/DavidABSiepmann/multithread_cpp/actions/workflows/nightly-stress.yml)
[![Release](https://img.shields.io/github/v/release/DavidABSiepmann/multithread_cpp)](https://github.com/DavidABSiepmann/multithread_cpp/releases)

Um projeto C++ profissional que demonstra um **pipeline de processamento multithreaded** com coleta de dados, processamento paralelo e benchmarking reproducível com análise de perfil em tempo real.

---

## 📋 Descrição Curta

Este projeto implementa um **pipeline de dados em 4 estágios** onde múltiplas threads trabalham cooperativamente:
1. **Source A**: coleta de dados simulada
2. **Source B**: processamento e transformação dos dados
3. **Process A**: consumidor de Source A
4. **Process B**: consumidor de Source B com métrica de throughput

O projeto inclui **profiling de eventos**, **benchmarking automatizado** e **testes unitários completos** com suporte a sanitizers (TSAN/ASAN).

---

## 💡 Motivação

Este repositório foi desenvolvido para demonstrar:

- **Padrões de concorrência robustos**: uso de `std::atomic`, `std::lock_guard` (RAII) e sincronização thread-safe
- **Profiling de eventos em produção**: sistema de logging com nanosecond-precision para análise de timeline
- **Benchmarking reproducível**: framework customizado para medir throughput com variações paramétricas
- **Boas práticas de C++17**: encapsulamento, polimorfismo, RAII, memory ordering
- **Qualidade de código**: testes unitários (GoogleTest), CI-ready, sem warnings
- **Documentação e portfólio**: código limpo, bem comentado, pronto para code review

---

## 🔧 Instalação

### Pré-requisitos

- **C++17** ou superior
- **CMake 3.15+**
- **Python 3.7+** (para geração de gráficos)
- **Git**

### Build via CMake

```bash
# Clone o repositório
git clone https://github.com/DavidABSiepmann/multithread_cpp.git
cd multithread_cpp

# Build rápido (Release)
./scripts/build.sh

# Ou manualmente
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
```

### Opções de CMake

```bash
# Com sanitizers (para testes)
cmake .. -DENABLE_SANITIZERS=ON -DCMAKE_BUILD_TYPE=Debug

# Com documentação Doxygen
cmake .. -DBUILD_DOCS=ON
```

---

## 🚀 Como Usar

### Exemplo 1: Benchmark Rápido

```bash
# Executar um bench de 3 segundos com trabalho de 10µs
./build/pipelines_cpp \
  --duration 3 \
  --work-us 10 \
  --repeats 3 \
  --out outputs/results.csv \
  --profile outputs/profile_events.csv
```

### Exemplo 2: Grade Completa de Benchmarks

```bash
# Roda automaticamente múltiplas combinações
./scripts/run_bench.sh
```

Isso gera:
- `outputs/results.csv` — tabela de throughput (threads, duration, work_us, throughput_items_s)
- `outputs/profile_events.csv` — timeline de eventos (thread_id, timestamp_ns, event_name)

### Exemplo 3: Gerar Gráficos

```bash
# Ambos os gráficos (profile + results)
python3 bench/grafico.py

# Ou separadamente
python3 bench/grafico.py --profile-only
python3 bench/grafico.py --results-only
```

Gera imagens em `assets/profile.png` e `assets/results.png`.

### Parâmetros da Linha de Comando

```
--duration S          Duração de cada execução (segundos, default: 1)
--work-us US          Trabalho simulado por item (microsegundos, default: 50)
--warmup N            Número de runs de warmup (default: 0)
--repeats R           Quantas repetições por célula (default: 1)
--threads N           Valor registrado para número de threads (default: 4)
--out FILE            CSV de resultados (append mode)
--profile FILE        CSV de eventos de profiling (nanosecond precision)
--help                Mostra esta mensagem
```

---

## 📊 Resultados / Benchmarks

### Saída de Resultados

O CSV de resultados segue este formato:

```csv
threads,duration_s,work_us,run,processed,throughput_items_s
4,1,50,1,8234,8234.5
4,1,50,2,8401,8401.2
4,1,100,1,4567,4567.8
```

**Interpretação:**
- **throughput_items_s**: itens processados por segundo (métrica principal)
- Compare entre diferentes `work_us` para avaliar escalabilidade
- Compare entre diferentes `duration_s` para avaliar estabilidade

### Profiling Timeline

O gráfico de profile mostra:
- Eixo X: tempo (segundos)
- Eixo Y: threads (source_A, source_B, process_A, process_B)
- Cores: status de cada thread (running, waiting, etc.)

Útil para detectar:
- **Gargalos**: threads que ficam idle enquanto outras trabalham
- **Balanceamento**: distribuição desigual de carga
- **Contention**: competição por locks visível como padrões intermitentes

---

## 🛠️ Tecnologias

| Componente | Tecnologia | Versão |
|-----------|-----------|--------|
| **Linguagem** | C++ | 17 |
| **Build System** | CMake | 3.15+ |
| **Threading** | std::thread, std::atomic, std::mutex | STL |
| **Profiling** | std::chrono (nanosecond precision) | STL |
| **Testes** | GoogleTest (GTest) | FetchContent |
| **Sanitizers** | TSAN, ASAN | GCC/Clang |
| **Gráficos** | Matplotlib (Python) | 3.0+ |
| **Documentação** | Doxygen | Markdown |
| **Versionamento** | Git | - |

**Características de Code Quality:**
- ✅ Zero warnings (C++17 strict mode)
- ✅ RAII (Resource Acquisition Is Initialization)
- ✅ Memory-safe (atomic operations com memory ordering)
- ✅ No global externs (config por dependency injection)
- ✅ Header-only headers (rápida recompilação)

---

## ✅ Como Rodar Testes

### Testes Unitários (GoogleTest)

```bash
# Build + run rápido
./scripts/run_tests.sh
```

Ou manualmente:

```bash
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . -j
./tests/unit_tests --gtest_color=yes
```

### Testes com Sanitizers

```bash
# Com TSAN (Thread Sanitizer) para detectar race conditions
mkdir -p build && cd build
cmake .. -DENABLE_SANITIZERS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build . -j
./tests/unit_tests
```

### Cobertura de Testes

Testes incluem:

| Teste | Propósito |
|-------|----------|
| `CLI.WritesFiles` | Validar argumentos CLI e escrita de arquivos |
| `Pipeline.StartStopNoCrash` | Verificar ciclo de vida (start/stop) |
| `ProcessBuffer.IncrementsProcessed` | Validar contador de items processados |
| `BenchMetrics.IncrementAndReset` | Testar thread-safety de métricas |
| `ThreadBase.StartStop` | Validar sincronização de threads |
| `Throughput.Smoke` | Teste de integração (smoke test) |

---

## 📚 Documentação Adicional

Para detalhes técnicos, veja:

- **[ARCHITECTURE.md](./docs/ARCHITECTURE.md)** — Design do pipeline, componentes e fluxo de dados
- **[Code Style](./docs/ARCHITECTURE.md#code-style)** — Convenções usadas no projeto


