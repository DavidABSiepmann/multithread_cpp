## Contributing & Development Guide

Se você quer contribuir ou estender este projeto, aqui estão as guidelines.

---

### Code Style

- **C++17**: Use recursos modernos (auto, structured bindings, std::optional)
- **Naming**: `snake_case` para variáveis/funções, `PascalCase` para classes
- **Headers**: Declaration only (implementações em .cpp)
- **No using directives**: Evite `using namespace std` ou `using std::vector` em headers
- **RAII**: Sempre use `std::lock_guard`, `unique_ptr`, etc.
- **Atomics**: Use `std::atomic<T>` com memory_order explícito, nunca `volatile`

### Building

```bash
# Build padrão
./scripts/build.sh

# Com sanitizers (TSAN/ASAN)
mkdir -p build && cd build
cmake .. -DENABLE_SANITIZERS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build . -j$(nproc)
./tests/unit_tests
```

### Testing

```bash
# Testes unitários
./scripts/run_tests.sh

# Teste específico
./build/tests/unit_tests --gtest_filter=ThreadBase.StartStop

# Com verbose output
./build/tests/unit_tests --gtest_color=yes -v
```

### Adding a New Feature

1. **Create header** in `include/`
2. **Create .cpp file** in `src/`
3. **Add tests** in `tests/`
4. **Update README.md** if public API changes
5. **Run full test suite** and verify sanitizers clean

### Directory Structure

```
include/         ← Headers (declaration only)
src/             ← Implementations (.cpp files)
tests/           ← GoogleTest unit tests
docs/            ← Architecture & design docs
  ├── ARCHITECTURE.md    ← System design & components
  ├── DECISIONS.md       ← Design trade-offs explained
  └── QUICKSTART.md      ← 30-second setup guide
bench/           ← Benchmarking scripts (grafico.py)
scripts/         ← Build/test automation
outputs/         ← Generated CSV files
assets/          ← Generated PNG graphs
```

### Performance Profiling

```bash
# Run with profiling enabled
./build/pipelines_cpp --duration 5 --work-us 50 --profile outputs/profile.csv --out outputs/results.csv

# Generate timeline
python3 bench/grafico.py --profile-only

# Open assets/profile.png to see thread activity
```

### Debugging

```bash
# Build with debug symbols
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . -j

# Run with GDB
gdb --args ./tests/unit_tests
(gdb) run
(gdb) bt  # backtrace on crash
```

### Thread Sanitizer (Race Condition Detection)

```bash
mkdir -p build && cd build
cmake .. -DENABLE_SANITIZERS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build . -j
./tests/unit_tests
# Look for "WARNING: ThreadSanitizer:"
```

---

**Questions?** Open an issue or check [ARCHITECTURE.md](docs/ARCHITECTURE.md) for design details.
