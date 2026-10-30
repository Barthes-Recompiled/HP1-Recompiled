#!/usr/bin/env bash
# run_win.sh behind failsafes, for test runs (Claude's scripted ones above all):
#  - its own systemd scope: memory capped (SAFE_MEM, default 16G), at most SAFE_CPU of the cores (default 1600% =
#    16 of 24), low CPU weight and nice 10, so the desktop always keeps cores and memory;
#  - a GPU watchdog on the kernel log: the first amdgpu page fault / ring timeout / reset closes the game at once,
#    instead of letting it keep feeding a wedged GPU until the driver resets it (2026-10-20 that took X down);
#  - SAFE_TIMEOUT seconds (default 0 = none) kills it however it is stuck.
# Everything else (HPPS_EXE, RECOMP_*) passes through to run_win.sh.
here="$(cd "$(dirname "$0")" && pwd)"
exe="${HPPS_EXE:-hpps.exe}"
log="$here/logs/safe_run.log"
mkdir -p "$here/logs"
: > "$log"

kill_game() {
    echo "[SAFE] $1 -- closing $exe" | tee -a "$log" >&2
    pkill -x "$exe"
    sleep 2
    pkill -9 -x "$exe"
}

# GPU watchdog: only messages from after the start
journalctl -k -f -n 0 -o short-monotonic 2>/dev/null | while read -r line; do
    case "$line" in
        *amdgpu*"page fault"*|*amdgpu*"timeout"*|*amdgpu*"GPU reset"*|*amdgpu*"wedged"*)
            echo "[SAFE] kernel: $line" >> "$log"
            kill_game "GPU fault reported by the kernel"
            ;;
    esac
done &
watch_pid=$!

timer_pid=
if [ "${SAFE_TIMEOUT:-0}" -gt 0 ]; then
    (sleep "$SAFE_TIMEOUT"; kill_game "time limit ${SAFE_TIMEOUT}s reached") &
    timer_pid=$!
fi

systemd-run --user --scope --quiet --collect \
    -p MemoryMax="${SAFE_MEM:-16G}" -p MemorySwapMax=0 \
    -p CPUQuota="${SAFE_CPU:-1600%}" -p CPUWeight=50 \
    nice -n 10 "$here/run_win.sh" "$@"
status=$?

[ -n "$timer_pid" ] && kill "$timer_pid" 2>/dev/null
pkill -P "$watch_pid" 2>/dev/null
kill "$watch_pid" 2>/dev/null
grep -q '^\[SAFE\]' "$log" && cat "$log" >&2
exit $status
