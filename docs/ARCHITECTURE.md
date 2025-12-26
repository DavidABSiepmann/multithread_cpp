# Architecture Documentation

## 📐 Visão Geral do Sistema

Este documento descreve a arquitetura do pipeline multithreaded, incluindo componentes, fluxo de dados, padrões de sincronização e decisões de design.

---

## 🏗️ Arquitetura de Alto Nível

```
┌────────────────────────────────────────────────────────────────┐
│                        PIPELINE (4 THREADS)                    │
├────────────────────────────────────────────────────────────────┤
│                                                                │
│  ┌──────────────┐      ┌──────────────┐      ┌──────────────┐  │
│  │   Source A   │──┬──▶│  Process A   │      │  Process B   │  │
│  │  (Producer)  │  │   │  (Consumer)  │      │  (Consumer)  │  │
│  └──────────────┘  │   └──────────────┘      └──────────────┘  │
│                    │                                │          │
│                    │   ┌──────────────┐             │          │
│                    └──▶│   Source B   │─────────────┘          │
│                        │ (Processor)  │                        │
│                        └──────────────┘                        │
│                                                                │
└────────────────────────────────────────────────────────────────┘
```

### Fluxo de Dados

1. **Source A** (thread 1): Simula coleta de dados contínua
   - Produz: `buffer_source_A` {int data, uint8_t index}
   - Período: ~50ms (configurable)

2. **Source B** (thread 2): Processa dados de Source A
   - Consome: buffer_source_A
   - Processa: data += 1000
   - Produz: `buffer_source_B` {int data, uint8_t index}

3. **Process A** (thread 3): Consumidor de Source A
   - Consome: buffer_source_A
   - Trabalho simulado: 250ms por item (configurable)

4. **Process B** (thread 4): Consumidor de Source B com métrica
   - Consome: buffer_source_B
   - Incrementa contador `processed_items` (métrica principal)
   - Trabalho simulado: 57ms por item (configurable)

---

## 📦 Componentes Principais

### 1. `thread_base` (include/thread_utils.h)

**Responsabilidade**: Base class polimórfica para todos os workers

```cpp
class thread_base {
  std::atomic<bool> active{false};        // Estado thread-safe com memory_order
  std::thread worker_thread;
  std::mutex mtx;

  void start();      // Cria std::thread, chama run()
  void stop();       // Define active=false, join()
  virtual void run() = 0;  // Override em subclasses
  virtual ~thread_base() = default;
};
```

**Padrões**:
- ✅ **RAII**: destrutor virtual garante cleanup correto
- ✅ **Atomic with Memory Ordering**: `active.load(std::memory_order_acquire)` para thread safety
- ✅ **Template Method**: `start()/stop()` ativa `run()` virtual

---

### 2. `source_A` (include/source_threads.h)

**Responsabilidade**: Simular coleta de dados com sincronização por index

```cpp
class source_A : public thread_base {
  buffer_source_A temp_buffer;
  std::mutex mtx;
  int latched_index = 0;

  void run() override;              // Loop de produção
  uint8_t read(buffer_source_A *);  // Reader com índice
};

struct buffer_source_A {
  int data;
  uint8_t index;  // Incrementa a cada novo dado
};
```

**Sincronização**: Mutex protege leitura/escrita de buffer e index

**Detalhe**: Index permite detecção de "novo dado" sem comparar valor (funciona mesmo se data repetir)

---

### 3. `source_B` (include/source_process_threads.h)

**Responsabilidade**: Processar dados de Source A e produzir Source B

```cpp
class source_B : public thread_base {
  source_A *source;  // Dependência injetada
  buffer_source_B temp_buffer;
  std::mutex mtx;
  int latched_index = 0;

  void run() override;
  void process_buffer(int *buffer);
  uint8_t read(buffer_source_B *);
};
```

**Sincronização**: Similar a Source A (mutex + index pattern)

**Processamento**: Lê de Source A → transforma (data += 1000) → escreve em Source B

---

### 4. `process_A` e `process_B` (include/process_thread.h)

**Responsabilidade**: Consumir dados e simular processamento

```cpp
class process_A : public thread_base {
  source_A *source;
  BenchConfig *cfg;  // Config injetada (not global!)
  
  void run() override;
  void process_buffer(int *buffer);
};

class process_B : public thread_base {
  source_B *source;
  BenchConfig *cfg;

  void run() override;
  void process_buffer(int *buffer);  // Chama inc_processed_items(1)
};
```

**Métrica**: `process_B::process_buffer()` incrementa contador atomicamente

**Trabalho Simulado**: `std::this_thread::sleep_for(microseconds)` baseado em `cfg->work_us`

---

### 5. `Pipeline` (include/pipeline.h)

**Responsabilidade**: Orquestrar criação e ciclo de vida de workers

```cpp
class Pipeline {
  source_A source_Captura;
  source_B process_cap_gen;
  process_A process_Captura;
  process_B process_gen;
  std::vector<std::thread> workers;

  Pipeline(BenchConfig *cfg = nullptr);
  void start();
  void stop();
};
```

**Design**:
- Constructor: instancia 4 workers (passando dependências)
- `start()`: chama start() em cada worker (lança threads)
- `stop()`: chama stop() em cada worker (join + cleanup)
- **Ordem**: Source A → Source B → Processos (paralelamente)

---

### 6. `ProfilePrinter` (include/profile_print.h)

**Responsabilidade**: Logging thread-safe de eventos com nanosecond precision

```cpp
class ProfilePrinter {
  static ProfilePrinter& get();      // Singleton
  bool open_file(const std::string &path);
  void start(const char *event);     // Log START
  void stop(const char *event);      // Log STOP
  void write_line(const char *name, long long t, int status);
  void mute();    // Silencia output (útil em testes)
  void unmute();
};
```

**Formato de Saída**:
```csv
thread_id,timestamp_ns,event_name,status
2,1702123456789012345,process_A_start,1
2,1702123456789012400,process_A_stop,0
```

**Nanosecond Precision**: Usa `std::chrono::system_clock::now()` cast para nanosegundos

---

### 7. `BenchConfig` (include/bench_config.h)

**Responsabilidade**: Encapsular todos os parâmetros de benchmark

```cpp
struct BenchConfig {
  int work_us;          // Trabalho simulado (microsegundos)
  int duration_s;       // Duração de cada run (segundos)
  int warmup_runs;
  int repeats;
  int threads_count;
  std::string results_file;
  std::string profile_file;
  // ... mais campos
};
```

**Injeção de Dependência**: Passada por pointer a todos os workers que precisam

---

### 8. `BenchMetrics` (include/bench_metrics.h)

**Responsabilidade**: Contadores thread-safe para throughput

```cpp
inline std::atomic<long long>& processed_items_storage() {
  static std::atomic<long long> inst{0};
  return inst;
}

// Helpers
inline void inc_processed_items(long long v = 1) {
  processed_items_storage().fetch_add(v, std::memory_order_release);
}

inline long long get_processed_items() {
  return processed_items_storage().load(std::memory_order_acquire);
}

inline void reset_processed_items() {
  processed_items_storage().store(0, std::memory_order_release);
}
```

**Sem Globals**: Encapsulado em função static (nenhum extern global visível)

---

## 🔄 Padrões de Design

### 1. Template Method (thread_base)

```cpp
void thread_base::start() {
  active.store(true, std::memory_order_release);
  worker_thread = std::thread([this] { this->run(); });
}

// Subclass override:
void source_A::run() {
  while (active.load(std::memory_order_acquire)) {
    // produzir dados
  }
}
```

**Benefício**: Centraliza lógica de criação de thread; subclasses só definem `run()`

---

### 2. Singleton (ProfilePrinter)

```cpp
class ProfilePrinter {
  static ProfilePrinter& get() {
    static ProfilePrinter instance;
    return instance;
  }
private:
  ProfilePrinter();
  ~ProfilePrinter();
};
```

**Benefício**: Garante única instância; thread-safe (magic static C++11)

---

### 3. Dependency Injection

**Antes** (não-recomendado):
```cpp
extern BenchConfig benchConfig;  // Global!
extern source_A source_instance;

class process_A {
  void process() {
    sleep_for(benchConfig.work_us);  // Difícil testar
  }
};
```

**Depois** (recomendado):
```cpp
class process_A {
  BenchConfig *cfg;

  process_A(source_A *src, BenchConfig *c) : cfg(c) {}
  void process() {
    sleep_for(cfg->work_us);  // Testável, injetável
  }
};
```

**Benefício**: Código testável, sem acoplamento a globals

---

### 4. RAII (Resource Acquisition Is Initialization)

**Locks**:
```cpp
void source_A::read(buffer_source_A *dado) {
  std::lock_guard<std::mutex> lk(mtx);  // Locked aqui
  *dado = temp_buffer;
  // Automaticamente unlocked ao sair do escopo
}
```

**Benefício**: Exception-safe; não precisa de try/finally manual

---

### 5. Atomic with Memory Ordering

```cpp
// Writer (process_B)
inc_processed_items(1);  // .fetch_add(..., std::memory_order_release)

// Reader (test)
long long count = get_processed_items();  // .load(..., std::memory_order_acquire)
```

**Benefício**: Garante visibility de mudanças entre threads; mais rápido que mutex em read-heavy workload

---

## 🔒 Sincronização Thread-Safe

### Estratégia 1: Mutex + RAII (para estruturas compartilhadas)

**Usado em**: Source A/B buffers, ProfilePrinter FILE*

```cpp
std::lock_guard<std::mutex> lk(mtx);  // Cria lock
// Seção crítica
// ~lock_guard() destrutor libera lock
```

### Estratégia 2: Atomic + Memory Ordering (para contadores)

**Usado em**: `active` flag, `processed_items` counter

```cpp
active.store(false, std::memory_order_release);      // Escrita
bool is_active = active.load(std::memory_order_acquire);  // Leitura
```

**Regra**: Sempre usa acquire (leitura) / release (escrita) em sincronização cross-thread

---

## 📊 Fluxo de Execução (Timeline)

```
Time │ Source A │ Source B │ Process A │ Process B
─────┼──────────┼──────────┼───────────┼──────────
  0  │ Produce  │  Ready   │  Ready    │  Ready
  10 │ Produce  │ Process  │  Waiting  │  Waiting
  20 │ Produce  │ Produce  │  Process  │  Waiting
  30 │ Waiting  │ Process  │  Process  │  Process
  40 │ Produce  │ Waiting  │  Process  │  Waiting
  50 │ Produce  │ Produce  │  Waiting  │  Waiting
```

**Observações**:
- Cada thread trabalha independentemente
- Sincronização apenas quando ler/escrever buffer compartilhado
- Process B incrementa métrica a cada buffer processado

---

## 🧪 Testing Strategy

### Unit Tests (GoogleTest)

1. **CLI.WritesFiles**: Valida parsing de argumentos
2. **Pipeline.StartStopNoCrash**: Verifica init/cleanup lifecycle
3. **ThreadBase.StartStop**: Testa thread creation/joining
4. **ProcessBuffer.IncrementsProcessed**: Valida métrica de throughput
5. **BenchMetrics.IncrementAndReset**: Thread-safety de contador
6. **Throughput.Smoke**: Integração end-to-end (polling-based)

### Sanitizer Testing

```bash
cmake .. -DENABLE_SANITIZERS=ON -DCMAKE_BUILD_TYPE=Debug
./tests/unit_tests
# Detecta: data races (TSAN), memory leaks (ASAN)
```

---

## 🚀 Performance Considerations

### 1. Lock Contention

**Problema**: Se múltiplos threads competem pelo mesmo mutex, performance degrada

**Solução atual**: Cada buffer tem próprio mutex (lock per buffer, não global)

**Alternativa futura**: Lock-free data structures (ring buffer sem mutex)

### 2. Sleep Precision

**Problema**: `std::this_thread::sleep_for()` pode ter ~1ms jitter em Linux

**Solução**: Aceitar jitter; não usar para timing crítico (usa `chrono::steady_clock` para profiling)

### 3. Index-Based Change Detection

**Vantagem**: Detecção de novo dado sem comparar valores (zero overhead)

**Desvantagem**: Índice pode overflow (uint8_t wraps a 256); aceitável pois buffer muda raramente

---

## 📈 Escalabilidade

**Atual**: 4 threads fixed (1 source_A, 1 source_B, 1 process_A, 1 process_B)

**Possíveis melhorias**:
- [ ] Múltiplas instâncias de source_A (paralelizar coleta)
- [ ] Múltiplas instâncias de process_B (scale consumidor)
- [ ] Thread pool dinâmico
- [ ] Work-stealing queue (em vez de buffers único-index)

---

## 📝 Code Style

### Convenções

- **Naming**: snake_case para variáveis/funções, PascalCase para classes/structs
- **Headers**: Declaration-only (implementações em .cpp)
- **Includes**: Forward declarations quando possível
- **Const**: `const` em parâmetros const, métodos readonly marcados `const`
- **References**: Prefer `const T&` em parâmetros; pointer apenas para ownership
- **Memory**: Stack allocation preferred; heap apenas se necessário

### Header Quality Checklist

- ✅ Sem `using` directives (evita poluição de namespace)
- ✅ Sem implementações pesadas (rápida recompilação)
- ✅ Include guards ou `#pragma once`
- ✅ Forward declarations antes de tipos incompletos
- ✅ Virtual destrutor em base classes
- ✅ `override` keyword em métodos virtuais

---

## 🔧 Troubleshooting

### Build falha com "undefined reference"

**Causa**: Nova classe adicionada em header mas implementação não em .cpp

**Solução**: Mover corpo de método de header → src/arquivo.cpp; deixar só declaração em header

### Teste flakiness (intermitentemente falha)

**Causa**: Fixed sleep pode ser insuficiente em CI lenta

**Solução**: Usar polling com timeout:
```cpp
auto start = std::chrono::high_resolution_clock::now();
while (get_processed_items() < expected) {
  if (std::chrono::high_resolution_clock::now() - start > 2s)
    break;  // timeout
  std::this_thread::sleep_for(10ms);
}
```

### TSAN detecta race condition

**Causa**: Acesso concorrente sem sincronização

**Solução**:
1. Identificar variável em questão
2. Proteger com `std::lock_guard<std::mutex>` OU `std::atomic<T>`
3. Se atomic, adicionar memory ordering explícito

---

## 📚 Referências

- **C++ Standard**: cppreference.com (std::atomic, std::thread, std::chrono)
- **CMake**: cmake.org/documentation
- **GoogleTest**: github.com/google/googletest/blob/main/docs/primer.md
- **TSAN**: clang.llvm.org/docs/ThreadSanitizer

---

**Última Atualização**: 2025  
**Status**: Production-ready ✓
