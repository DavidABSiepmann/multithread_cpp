# Quick Start Guide

## ⚡ 30 segundos para começar

### 1. Clone e Build

```bash
git clone https://github.com/DavidABSiepmann/multithread_cpp.git
cd multithread_cpp
./scripts/build.sh
```

### 2. Execute um Benchmark Simples

```bash
./build/pipelines_cpp \
  --duration 1 \
  --work-us 50 \
  --repeats 3 \
  --out outputs/results.csv \
  --profile outputs/profile_events.csv
```

### 3. Gere um Gráfico

```bash
python3 bench/grafico.py
```

Abra `assets/profile.png` para ver a timeline de threads.

---

## 🧪 Rodando Testes

```bash
# Testes rápidos (com saída limpa)
./scripts/run_tests.sh

# Ou diretamente
./build/tests/unit_tests --gtest_color=yes
```

**Esperado**: 6 testes passando em ~2 segundos

---

## 📊 Benchmark Completo (3-5 minutos)

```bash
# Roda uma grade de valores (predefinida em run_bench.sh)
./scripts/run_bench.sh

# Gera dois gráficos
python3 bench/grafico.py
```

Resultado:
- `outputs/results.csv` — tabela de throughput
- `outputs/profile_events.csv` — timeline de eventos
- `assets/profile.png` — gráfico timeline
- `assets/results.png` — gráfico de throughput

---

## 🎯 Parâmetros Comuns

### Benchmark Curto (1s, pouco trabalho)

```bash
./build/pipelines_cpp --duration 1 --work-us 10
```

**Uso**: Teste rápido, validação

### Benchmark Médio (10s, trabalho moderado)

```bash
./build/pipelines_cpp --duration 10 --work-us 100 --repeats 5
```

**Uso**: Análise de comportamento

### Benchmark Longo (60s, máximo de precisão)

```bash
./build/pipelines_cpp --duration 60 --work-us 50 --repeats 10
```

**Uso**: Relatório final, análise estatística

---

## 🔧 Troubleshooting Rápido

| Problema | Solução |
|----------|---------|
| Build falha | `rm -rf build && ./scripts/build.sh` |
| Teste falha | Executar com `--gtest_filter=<TestName>` para debug |
| Gráfico não aparece | Verificar que `outputs/` existe e `python3` está instalado |
| Threads lentos | Aumentar `--duration` ou `--repeats` para melhor precisão |

---

## 📁 Estrutura de Saída

Após executar benchmark, você terá:

```
outputs/
  ├── results.csv           # Tabela: threads, duration_s, work_us, run, processed, throughput
  └── profile_events.csv    # Timeline: thread_id, timestamp_ns, event_name, status

assets/
  ├── profile.png           # Gráfico timeline (thread activity)
  └── results.png           # Gráfico throughput (items/sec)
```

---

## 📖 Próximos Passos

1. Leia [ARCHITECTURE.md](./ARCHITECTURE.md) para entender o design
2. Explore `include/` e `src/` para ver implementação
3. Modifique `--work-us` para simular diferentes workloads
4. Rode com `--help` para ver todos os argumentos

---

**Pronto!** Você tem um benchmark multithreaded profissional rodando. 🚀
