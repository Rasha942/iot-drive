# IOT Drive — Distributed Network Block Device

A distributed block-storage system written in modern C++ (C++20). The host kernel
sees an ordinary block device (via [NBD](https://en.wikipedia.org/wiki/Network_block_device)),
but every read and write is transparently striped and **mirrored (RAID 10)** across a
cluster of independent storage nodes ("minions") that communicate over the network.

This was my software-engineering capstone project. It brings together event-driven
networking, a thread pool, and a set of classic design patterns into a working,
fault-tolerant storage server.

---

## How it works

A **master** node exposes a virtual block device to the local kernel. Block
operations arriving on that device are handled by an event loop and dispatched to a
pool of worker threads, which replicate each operation across multiple **minion**
nodes over UDP and reconcile their responses.

```
            ┌────────────────────── Master ──────────────────────┐
 Kernel ──▶ │  NBD ──▶ Reactor ──▶ ThreadPool ──▶ Command         │
 (block I/O)│                                        │            │
            │                                        ▼            │
            │                              RAID10 ──▶ MinionProxy  │──▶ UDP ──▶ Minions
            │                                        │            │           (replicas)
            │   ResponseManager ◀── Reactor ◀────────┘            │
            └─────────────────────────────────────────────────────┘
```

See [`architecture.puml`](architecture.puml) for the full component flow and
[`reactor-flow.puml`](reactor-flow.puml) for the request/response sequence. Both
render with any PlantUML viewer.

### Design patterns used

| Pattern        | Where                                                        |
|----------------|-------------------------------------------------------------|
| **Reactor**    | `reactor.*` — single-threaded `select()` event loop         |
| **Thread pool**| `threadpool.*` — bounded worker pool with a priority queue  |
| **Command**    | `command.*` — read/write operations as first-class objects  |
| **Factory**    | `factory.*` — builds commands from message keys             |
| **Singleton**  | `singleton.*`, `handleton.*` — process-wide services        |
| **Proxy**      | `inputproxy.*`, `*_nbd.*` — remote minions as local objects |
| **Mediator**   | `input_mediator.*` — wires callbacks into the reactor       |

Other notable pieces: a `DirMonitor` that hot-loads command plugins from the
`plugins/` directory at runtime, an asynchronous `Logger`, and a `ResponseManager`
that tracks per-request UIDs so replicated writes can be confirmed.

---

## Building

### Requirements

- Linux with the `nbd` kernel module (`sudo modprobe nbd`)
- `g++` with C++20 support (tested on g++ 13)
- [`libmysqlcppconn-dev`](https://dev.mysql.com/downloads/connector/cpp/) (MySQL Connector/C++)
- [SimpleIni](https://github.com/brofield/simpleini) headers in `/usr/include/simpleini`

### Compile

The build is target-driven. Pick `master` or `minion`:

```bash
make TARGET=master      # → progs/master.out  + libs/libmaster.so
make TARGET=minion      # → progs/minion.out  + libs/libminion.so
make help               # usage and options
make TARGET=master run  # build and run
```

`make TARGET=<t> DEBUG=0` produces a release build; `make TARGET=<t> vlg` runs it
under Valgrind.

> **Verified end to end** on Ubuntu 24.04 (g++ 13.3, MySQL Connector/C++ 1.1.12).
> Both targets build cleanly, the cluster comes up (3 minions + master), the master
> attaches a 12 MB `/dev/nbd0`, and a write/read round-trip through the device returns
> byte-for-byte identical data across the replicated minions. `master.out` requires the
> `nbd` kernel module (`sudo modprobe nbd`); the demo below does this for you.

### Run a local cluster

Once both targets are built, [`scripts/demo.sh`](scripts/demo.sh) brings up 3 minions
and a master, then writes a pattern through `/dev/nbd0`, reads it back, and verifies
the round-trip across the cluster. The master attaches an NBD device, so run it as root:

```bash
sudo ./scripts/demo.sh
```

---

## Configuration

`config.ini.example` documents the intended cluster layout — minion count, per-minion
transport options (the config format reserves UDP / Bluetooth / MQTT; this build wires
the UDP path), ports, and the MySQL connection used for metadata. Copy it to
`config.ini` and fill in your own values before running:

```bash
cp config.ini.example config.ini
```

> No real credentials are committed. Replace the `<...>` placeholders with your own.

---

## Project layout

```
include/    Public headers (one per component)
src/        Implementation
test/       Standalone test drivers (master_test, minion_test)
plugins/    Runtime-loadable command plugins
libs/ objs/ progs/   Build output (kept empty in git)
```

---

## Notes

This repository is the cleaned-up, canonical version of the project, extracted from a
larger development workspace. The networking layer binds to `127.0.0.1` for a
single-machine demo; minion endpoints are configurable for a real multi-host cluster.
