# Design Decisions & Trade-offs

Este documento explica as decisões arquiteturais e trade-offs do projeto.

---

## 🎯 Decision 1: Pipeline com 4 Threads Fixos (vs. Dynamic Pool)

### Decisão
Usar **4 threads hardcoded** em vez de um thread pool dinâmico.

### Justificativa

| Aspecto | 4 Threads Fixos | Thread Pool Dinâmico |
|---------|---|---|
| **Simplicidade** | ✅ Fácil entender fluxo | ❌ Complexidade extra |
| **Previsibilidade** | ✅ Comportamento determinístico | ❌ Variável conforme carga |
| **Footprint Embarcado** | ✅ Memória fixa, sem overhead | ⚠️ Precisa allocator dinâmico |
| **Real-time Predictability** | ✅ Latência previsível | ⚠️ Jitter de scheduling |
| **Didática** | ✅ Conceitos claros, fácil aprender | ⚠️ Abstração pesada |
| **Escalabilidade** | ❌ Fixo, não adapta | ✅ Adapta à carga |

### Contexto de Design

**Este é um projeto educacional** focado em demonstrar padrões de concorrência em **contexto de sistemas embarcados** (IoT, microcontroladores, edge computing), não em escalabilidade para centenas de threads.

Em sistemas embarcados:
- Recursos são **limitados** (RAM, CPU cores)
- Previsibilidade > flexibilidade
- Comportamento determinístico é crítico
- Overhead dinâmico é inaceitável

Para um projeto de **escalabilidade real** (100+ threads, workload dinâmico), considere:
- **Work-stealing queue** (Boost.Context)
- **Async/await patterns** (C++20 coroutines)
- **Reactor pattern** (libuv, Boost.Asio)

---

## 🎯 Decision 2: Singleton para ProfilePrinter (vs. Dependency Injection)

### Decisão
`ProfilePrinter` como **Singleton** em vez de injetar instância.

### Justificativa

```cpp
// ❌ Alternativa (Dependency Injection)
Pipeline(BenchConfig *cfg, ProfilePrinter *prof);
process_A(source_A *src, ProfilePrinter *prof);
process_B(source_B *src, ProfilePrinter *prof);

// ✅ Escolhido (Singleton)
ProfilePrinter::get().start("event");
```

**Por quê Singleton?**
- Logging é um **cross-cutting concern** (afeta todo sistema)
- Apenas **uma instância** precisa existir (arquivos não duplicam)
- **Thread-safety garantido** (magic static C++11)
- Encurta assinatura de construtores

**Trade-off**
- ❌ Menos testável (global implícito)
- ✅ Mais simples (sem passagem de parâmetros)
- ✅ Pode ser "mockado" em testes (ver `test_profile_silencer.cpp`)

---

## 🎯 Decision 3: Mutex per Buffer (vs. Global Lock)

### Decisão
Cada buffer (`source_A`, `source_B`) tem **próprio mutex** em vez de lock global.

### Justificativa

```cpp
// ❌ Global lock (simples mas ineficiente)
std::mutex g_lock;
void source_A::run() {
  std::lock_guard<std::mutex> lk(g_lock);
  // Tudo serializado
}

// ✅ Escolhido (Per-buffer lock)
class source_A {
  std::mutex mtx;  // Próprio lock
  // Apenas buffer protegido
};
```

**Vantagem**: Reduz **lock contention**
- Threads diferentes operam em buffers diferentes
- Concorrência real entre source_A, source_B, process_A, process_B

**Desvantagem**: Requer disciplina
- Risco de deadlock se locks tomados em ordem diferente
- Mitigation: sempre acquire in same order (source_A → source_B → process)

---

## 🎯 Decision 4: Condition Variable para Change Detection

### Decisão
Usar **std::condition_variable** para notificar leitores de novo dado, com wait explícito.

### Justificativa

```cpp
// ✅ Escolhido (Condition Variable - semântica clara)
class source_A {
  std::condition_variable cv;
  void run() {
    data = produce();
    cv.notify_all();  // Acordar leitores quando há novo dado
  }
};

void process_A::run() {
  std::unique_lock<std::mutex> lk(mtx);
  cv.wait(lk, [this] { return has_new_data(); });  // Espera explícita
  process(buffer);
}

// ❌ Alternativa (Index polling - fácil de esquecer sleep)
class source_A {
  uint8_t index = 0;  // Simples contador
};

void process_A::run() {
  while (source->read(&buffer) == same_index) {
    std::this_thread::sleep_for(10ms);  // Fácil esquecer ou configurar errado
  }
}
```

**Por quê Condition Variable?**
- ✅ **Semântica clara**: intenção explícita (wait for event)
- ✅ **Correto por padrão**: não precisa "lembrar" de adicionar sleep
- ✅ **Standard pattern**: reconhecido por todo desenvolvedor C++
- ✅ **Eficiente em embarcado**: thread dorme até event, não consome CPU
- ✅ **Educacional**: demonstra padrão produtor-consumidor correto

**Trade-off vs. Polling**
- ⚠️ Ligeiramente mais complexo (unique_lock + lambda)
- ✅ Mas evita armadilha comum: "esquecer sleep" em polling
- ✅ Comportamento previsível e determinístico

---

## 🎯 Decision 5: Atomic<bool> com Memory Ordering (vs. Volatile)

### Decisão
`std::atomic<bool> active` com `memory_order_acquire/release`, não `volatile bool`.

### Justificativa

```cpp
// ❌ Fraco (volatile não garante atomicidade)
volatile bool active = true;
bool status = active;  // Pode ser torn read em ARM32!

// ✅ Escolhido (Explicit memory ordering)
std::atomic<bool> active{false};
active.store(true, std::memory_order_release);
bool status = active.load(std::memory_order_acquire);
```

**Por quê Atomic?**
- ✅ **Atomicidade**: leitura indivisível mesmo em múltiplos cores
- ✅ **Memory ordering**: acquire/release previne reordering
- ✅ **Portabilidade**: funciona em x86, ARM, RISC-V
- ✅ **Explícito**: intenção clara para code review

**Memory Ordering Escolhido**
```cpp
store(..., std::memory_order_release)  // Writer
load(..., std::memory_order_acquire)   // Reader
```

**Rationale**: Sufficient for synchronization point (não precisa sequentially_consistent overhead)

---

## 🎯 Decision 6: Config por Pointer (vs. Global/Const Constructor)

### Decisão
Passar `BenchConfig *cfg` a cada worker, não usar global ou const constructor.

### Justificativa

```cpp
// ❌ Global (untestable)
extern BenchConfig benchConfig;
class process_A {
  void process() { sleep_for(benchConfig.work_us); }
};

// ⚠️ Const Constructor (não mudável)
class process_A {
  const int work_us;
  process_A(int w) : work_us(w) {}
};

// ✅ Escolhido (Pointer, testable)
class process_A {
  BenchConfig *cfg;
  process_A(source_A *src, BenchConfig *c) : cfg(c) {}
  void process() { sleep_for(cfg->work_us); }
};
```

**Por quê Pointer?**
- ✅ **Testável**: injetar config diferente por teste
- ✅ **Flexível**: alterar config em runtime (se necessário)
- ✅ **Decoupled**: workers não conhecem origem de config
- ✅ **Simples**: alternativa a Strategy pattern pesada

**Trade-off**
- ⚠️ Nullptr risk (mitigado: sempre passe válido na Pipeline)
- ⚠️ Não sabe se config muda durante run (acceptable: config static durante benchmark)

---

## 🎯 Decision 7: Header-Only vs. .cpp Split

### Decisão
**Header-only declarations** + **separate .cpp files** para implementações pesadas.

### Antes (Header-Only)
```cpp
// include/process_thread.h
class process_A {
  void run() override {  // ❌ 50 linhas de código aqui
    while (active.load(std::memory_order_acquire)) {
      // ... lógica complexa
    }
  }
};
```

**Problemas**
- ❌ Recompila tudo se muda processo_A
- ❌ Headers gigantes (poluição)
- ❌ Namespace pollution (`using` directives)

### Depois (Split Declaration/Definition)
```cpp
// include/process_thread.h (declaration only)
class process_A : public thread_base {
  void run() override;  // ✅ Só declaração
  void process_buffer(int *buffer);
};

// src/process_thread.cpp (implementation)
void process_A::run() {  // ✅ Corpo aqui
  while (active.load(std::memory_order_acquire)) {
    // ... 50 linhas
  }
}
```

**Vantagens**
- ✅ Mais rápido recompilar (headers lean)
- ✅ Namespace clean (sem `using` em headers)
- ✅ Melhor para CI/CD (parallelização)
- ✅ Code organization clara

---

## 🎯 Decision 8: RAII Locks (vs. Manual Lock/Unlock)

### Decisão
`std::lock_guard<std::mutex>` automático vs. `mtx.lock()` + `mtx.unlock()` manual.

### Antes (Manual - Perigoso)
```cpp
mtx.lock();
buffer = temp_buffer;
if (buffer.value == invalid) {
  mtx.unlock();
  return error;  // ❌ Leak: mutex ainda locked!
}
mtx.unlock();
```

### Depois (RAII - Seguro)
```cpp
{
  std::lock_guard<std::mutex> lk(mtx);  // Auto-lock
  buffer = temp_buffer;
  if (buffer.value == invalid) {
    return error;  // ✅ Auto-unlocked ao sair
  }
}  // ~lock_guard() libera mutex
```

**Garantias RAII**
- ✅ Exception-safe (destrói mesmo com erro)
- ✅ Não esquecer unlock
- ✅ Menos linhas de código
- ✅ Mais expressivo

---

## 🎯 Decision 9: Logging com std::ofstream (Stream Moderno)

### Decisão
**std::ofstream** (stream C++ moderno) em vez de `FILE* + fprintf`.

### Justificativa

```cpp
// ✅ Escolhido (std::ofstream - RAII, C++ correto)
class ProfilePrinter {
  std::ofstream log_file;
  
  void write_line(const char *name, long long t, int status) {
    log_file << t << "," << name << "," << status << "\n";
    log_file.flush();  // Explícito quando necessário
  }
};

// ❌ Alternativa (FILE* - manual, C-style)
void ProfilePrinter::write_line(...) {
  fprintf(fp, "%lld,%s,%d\n", timestamp_ns, name, status);
  // Precisa fflush() manual e close() manual
}
```

**Por quê std::ofstream?**
- ✅ **RAII correto**: destrutor fecha arquivo automaticamente
- ✅ **Type-safe**: não precisa format strings (evita %lld errado)
- ✅ **Modern C++**: consistente com resto do projeto (std::atomic, std::thread)
- ✅ **Extensível**: fácil adicionar serializadores customizados
- ✅ **Educacional**: demonstra RAII em ação

**Performance**
- ✅ Comparável a FILE* em sistemas modernos
- ✅ Stream buffering automático (flush on close)
- Se necessário: `log_file.flush()` para sincronização imediata

**Trade-off vs. FILE***
- ✅ Mais "C++" moderno
- ✅ Menos propenso a erros (não esquecer fflush/fclose)
- ⚠️ Digitação um pouco mais verbosa (operador <<)

---

## 📈 Escalabilidade: Quando Mudar

| Cenário | Considere | Razão |
|---------|-----------|-------|
| 100+ threads | Thread pool | 4 threads fixos fica bottleneck |
| Workload variável | Dynamic pool | Ajustar conforme carga |
| Milhões de eventos/s | Lock-free queue | Mutex contention domina |
| Sub-microsecond latency | Busy-wait spinning | Polling + sleep jitter inaceitável |
| Heterogeneous workload | Work-stealing | Balancear cores diferentes |

---

## 🚀 Otimizações Futuras

### 1. Lock-Free Ring Buffer
```cpp
// Em vez de:
std::mutex + single index

// Usar:
// boost::lockfree::ring_buffer<Data>
// Reduz contention em high throughput
```

### 2. NUMA-Aware Allocation
```cpp
// Se rodando em NUMA machines
// numa_alloc_onnode() para buffers
// Reduz remote memory access latency
```

### 3. CPU Pinning
```cpp
// Pinnar cada thread a core específico
// Reduz cache misses, context switches
cpu_set_t set;
CPU_SET(core_id, &set);
pthread_setaffinity_np(tid, sizeof(cpu_set_t), &set);
```

---

## 📝 Resumo: Filosofia de Design

### Contexto: Projeto Educacional para Sistemas Embarcados

**Este projeto é um teste/showcase educacional**, não um aplicativo de produção em escala. O objetivo é evidenciar **bons padrões de concorrência** em contexto de **sistemas embarcados** (IoT, edge computing, microcontroladores).

Princípios:

| Princípio | Aplicação | Contexto |
|-----------|-----------|----------|
| **Simplicidade** | 4 threads fixos, não dynamic pool | Embarcado: recursos limitados |
| **Explícito melhor que Implícito** | `std::atomic` com memory_order, não `volatile` | Clareza de intenção para code review |
| **RAII** | `lock_guard`, não manual lock/unlock | Exception-safety automático |
| **Testable** | Config injetada, não global | Dependency injection para flexibilidade |
| **Padrão Standard** | Condition variable, não polling | Semântica clara, reconhecida |
| **Modern C++** | std::ofstream, não FILE* | Consistência com C++17 |  
| **Measurable** | ProfilePrinter com nanosecond precision | Profiling e observabilidade |

### Escalabilidade

Para um projeto **de produção com requisitos de escalabilidade** (100+ threads, workload dinâmico):
- A implementação seria **completamente diferente**
- Usaria: thread pools, lock-free queues, async I/O, etc.
- Este projeto demonstra **conceitos fundamentais**, não otimizações de escala

### Foco Didático

A arquitetura foi projetada para:
1. Ensinar padrões corretos (RAII, atomics, condvar)
2. Ser fácil de entender (4 threads fixos, não abstrações pesadas)
3. Ser previsível em embarcado (memória fixa, comportamento determinístico)
4. Incluir profiling/benchmarking para observabilidade

