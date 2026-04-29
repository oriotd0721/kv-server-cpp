# kv-server-cpp

A multithreaded in-memory key-value server written in C++17, built from scratch using raw Winsock TCP sockets. Conceptually similar to a minimal Redis — one shared store, multiple concurrent clients, clean text protocol.

Personal project, developed independently, inspired by topics covered in theOperating System course at the University of Geneva (networking, concurrency, shared mutable state).

---

## What it does

When the server starts, it opens a TCP socket on port 6379 and waits for connections. Each client that connects gets its own thread. The client sends plain-text commands, the server parses them, updates or queries an in-memory hash map, and sends back a response. All access to the shared store is protected by a mutex so concurrent clients never corrupt each other's data.

---

## Commands

| Command | Response |
|---------|----------|
| `SET key value` | `OK` |
| `GET key` | the value, or `NOT_FOUND` |
| `DEL key` | `OK` if deleted, `NOT_FOUND` if absent |
| `EXIT` | `BYE`, then the connection closes |

### Example session

```
SET username oriance
OK

GET username
oriance

DEL username
OK

GET username
NOT_FOUND

EXIT
BYE
```

---

## Build

Requires Visual Studio 2022 or 2026 (MSVC) and CMake 3.16+.

If a `build/` directory already exists from a previous configuration, delete it first:

```powershell
Remove-Item -Recurse -Force build
```

Then configure and build:

```powershell
cmake -S . -B build -G "Visual Studio 18 2026"
cmake --build build --config Release
```

Replace `"Visual Studio 18 2026"` with `"Visual Studio 17 2022"` if you are on VS 2022.

The binary is produced at `build\Release\server.exe`.

---

## Run

```powershell
.\build\Release\server.exe
```

The server prints `KV server listening on port 6379` and blocks, waiting for clients.

---

## Connect

Use any TCP client — `telnet`, `netcat`, or PowerShell:

```powershell
telnet 127.0.0.1 6379
```

---

## Architecture

```
kv-server-cpp/
├── src/
│   ├── main.cpp        — creates the server and calls run()
│   ├── server.cpp      — Winsock init, TCP accept loop, one thread per client
│   ├── parser.cpp      — tokenises text lines into typed Command structs
│   └── kv_store.cpp    — unordered_map store with mutex protection
├── include/
│   ├── server.hpp
│   ├── parser.hpp
│   └── kv_store.hpp
└── CMakeLists.txt
```

**KVStore** wraps `std::unordered_map<std::string, std::string>` with a `mutable std::mutex`. Every `set`, `get`, and `del` acquires a `std::lock_guard` before touching the map — this is the core thread-safety guarantee.

**Parser** converts a raw text line into a typed `Command` struct. Missing arguments or unknown verbs produce an `UNKNOWN` command whose `error` field is sent back to the client.

**Server** runs an infinite accept loop. Each accepted socket is moved into a detached `std::thread`. Because `run()` never returns, the `Server` object always outlives every client thread.

---

## Possible extensions

- Persistence (append-only log or periodic snapshot)
- TTL / key expiry
- `KEYS` and `PING` commands
- Graceful shutdown on Ctrl-C
- Custom CLI client

---

## License

MIT — see [LICENSE](LICENSE).
