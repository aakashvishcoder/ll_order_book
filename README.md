# ll_order_book

A single-node limit order book and matching engine in C++17, built as a way to
actually get hands-on with the data structures and concurrency patterns
HFT-style systems use, rather than just reading about them.

## What's actually working

- **`OrderBook`** — price-time priority matching. Bids and asks are each a
  `std::map<price, std::list<Order>>` (descending for bids, ascending for
  asks), so best price is always `begin()`. Orders are allocated from a
  `MemoryPool` instead of the heap.
- **`OrderBookManager`** — a registry of order books keyed by symbol, guarded
  by a `shared_mutex` so lookups (`getBook`) don't block each other.
- **`SPSCQueue`** — a lock-free single-producer/single-consumer ring buffer.
  `main.cpp` uses one to hand orders from a "market data" thread to the
  matching thread without locking.
- **`AuditLog` / `StateRecovery`** — every add/cancel/fill is appended to a
  binary log (magic header + version so corrupt/legacy files get detected and
  rotated aside instead of silently misread). On startup, `recoverState()`
  replays the log back into a fresh `OrderBook`, capped at 100k events so a
  huge or corrupted log can't hang startup.
- **`LatencyMonitor`** — lock-free min/max/avg tracking for per-order
  processing time.
- **`AnalyticsEngine`** — top-of-book and depth-5 bid/ask imbalance.
- **Optional CUDA path** — if CMake finds a CUDA compiler, `GPUAnalytics`
  runs a Monte Carlo-style stress test: given a book snapshot and a batch of
  hypothetical order sizes, it estimates execution price / slippage on the
  GPU (`src/analytics_kernel.cu`). Without CUDA, this is skipped with a CMake
  warning and the rest builds normally.

`src/main.cpp` ties this together as a stress test: it creates an `AAPL`
book, replays any existing audit log, then fires 1,000,000 random limit
orders through the SPSC queue into a single matching thread while printing
throughput, latency stats, and top-5 depth at the end.

## Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/orderbook_app
./build/stress_test
```

Unit tests are pulled in via `FetchContent` (GoogleTest 1.14.0):

```bash
ctest --test-dir build
```

The CUDA path needs `nvcc` on your `PATH`; CMake detects it automatically
(`check_language(CUDA)`) and defines `LL_HAS_CUDA_ANALYTICS` when available.

## What's scaffolding, not working yet

Wanted to note this up front instead of letting someone find out the hard
way:

- **`FixParser::parseNewOrderSingle`** is declared but the body is empty —
  no actual FIX message parsing happens yet.
- **`NetworkListener`** opens a non-blocking socket but the epoll/IOCP event
  loop is still just comments (`start()` doesn't actually listen for
  anything). `SmartOrderRouter` and `MarketDataBroadcaster` are built against
  this, so they compile and run but aren't hooked up to real network I/O.
- **`MemoryPool::deallocate()`** is currently a no-op — it's a bump allocator
  with no free-list, so pool capacity (`OrderBookManager`'s default is 2M
  orders) is a hard ceiling per process lifetime, not per book.
- **`MarketDataBroadcaster::publishUpdate`** doesn't fill in the timestamp or
  size fields on the update it queues.

None of that blocks the core matching engine / audit / recovery path, which
is what `stress_test` and `unit_tests` actually exercise.
