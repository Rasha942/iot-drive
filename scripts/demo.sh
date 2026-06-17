#!/usr/bin/env bash
#
# demo.sh — bring up a local IOT Drive cluster and verify the distributed I/O path.
#
# Starts 3 minion storage nodes (UDP) and 1 master node, which attaches a 12 MB
# block device at /dev/nbd0. It then writes a random pattern through the block
# device, flushes caches, reads it back, and confirms the round-trip — exercising
# the full NBD -> reactor -> RAID10 -> minion replication path.
#
# The master needs root (NBD kernel module + /dev/nbd0), so run this with sudo:
#
#     sudo ./scripts/demo.sh
#
# Requirements: `make TARGET=minion` and `make TARGET=master` already built, the
# `nbd` kernel module available, and the runtime libraries reachable (embedded
# rpath, or set DEMO_LIB_PATH to a dir containing libmysqlcppconn / libmysqlclient).

set -uo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

NBD_DEV=/dev/nbd0
export LD_LIBRARY_PATH="${DEMO_LIB_PATH:-}:$ROOT/libs:${LD_LIBRARY_PATH:-}"
PIDS=()

log()  { echo -e "\033[1;34m[demo]\033[0m $*"; }
fail() { echo -e "\033[1;31m[demo] FAIL:\033[0m $*"; exit 1; }

cleanup() {
    log "shutting down cluster..."
    for p in "${PIDS[@]:-}"; do kill "$p" 2>/dev/null; done
    pkill -f 'progs/master.out' 2>/dev/null
    pkill -f 'progs/minion.out' 2>/dev/null
    command -v nbd-client >/dev/null && nbd-client -d "$NBD_DEV" 2>/dev/null
    rm -f /tmp/iot_write.bin /tmp/iot_read.bin
}
trap cleanup EXIT INT TERM

# --- sanity ------------------------------------------------------------------
[[ -x progs/minion.out && -x progs/master.out ]] || \
    fail "binaries missing — run: make TARGET=minion && make TARGET=master"
[[ $EUID -eq 0 ]] || log "warning: not root; master will likely fail to attach $NBD_DEV"

# --- 1. minions --------------------------------------------------------------
for n in 1 2 3; do
    ./progs/minion.out "$n" >"/tmp/iot_minion$n.log" 2>&1 &
    PIDS+=($!)
    log "started minion $n (pid $!)"
done
sleep 2
log "minion UDP sockets:"
ss -lunp 2>/dev/null | grep -E ':70[0-9]0' || log "  (no minion ports detected — check /tmp/iot_minion*.log)"

# --- 2. master ---------------------------------------------------------------
./progs/master.out >/tmp/iot_master.log 2>&1 &
PIDS+=($!)
log "started master (pid $!); waiting for $NBD_DEV to attach..."

attached=0
for _ in $(seq 1 20); do
    if [[ -b "$NBD_DEV" ]] && [[ "$(blockdev --getsize64 "$NBD_DEV" 2>/dev/null || echo 0)" -gt 0 ]]; then
        attached=1; break
    fi
    sleep 1
done
[[ $attached -eq 1 ]] || { sed 's/^/[master] /' /tmp/iot_master.log; fail "$NBD_DEV did not attach (see log above)"; }
log "$NBD_DEV attached — size $(blockdev --getsize64 "$NBD_DEV") bytes"

# --- 3. verify distributed read/write round-trip -----------------------------
log "writing 32 KB random pattern through $NBD_DEV ..."
dd if=/dev/urandom of=/tmp/iot_write.bin bs=4096 count=8 status=none
dd if=/tmp/iot_write.bin of="$NBD_DEV" bs=4096 count=8 conv=fsync status=none

blockdev --flushbufs "$NBD_DEV" 2>/dev/null   # drop cache so the read hits the minions

log "reading it back ..."
dd if="$NBD_DEV" of=/tmp/iot_read.bin bs=4096 count=8 status=none

if cmp -s /tmp/iot_write.bin /tmp/iot_read.bin; then
    log "\033[1;32mROUND-TRIP OK\033[0m — data written and read back match across the cluster."
else
    fail "data mismatch — written and read-back patterns differ."
fi

log "demo complete; cleaning up."
