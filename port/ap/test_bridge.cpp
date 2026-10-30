// Host test of the Archipelago bridge without the game (src/ap/bridge.cpp + client.cpp against a local AP server).
//   test_bridge <phase>   with RECOMP_AP_SERVER / RECOMP_AP_SLOT set, run from a folder with saves\ beside the exe
// phase 1: connect, send two checks by key, take every received item ("give" it), report the counts
// phase 2: a new run of the same slot: the session must come back (checks made, items given) and give nothing twice
#include "bridge.h"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>

static int pump(int seconds, int* given) {
  auto end = std::chrono::steady_clock::now() + std::chrono::seconds(seconds);
  while (std::chrono::steady_clock::now() < end) {
    hpps_ap_frame();
    const char* kind;
    int value;
    while (hpps_ap_next_item(&kind, &value)) {
      std::printf("give %s %d\n", kind, value);
      hpps_ap_item_given();
      ++*given;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
  }
  return hpps_ap_active();
}

int main(int argc, char** argv) {
  int phase = argc > 1 ? std::atoi(argv[1]) : 1;
  int given = 0;
  if (!hpps_ap_enabled()) {
    std::printf("RECOMP_AP_SERVER / RECOMP_AP_SLOT not set\n");
    return 2;
  }
  if (phase == 1) hpps_ap_location("cards:1");  // before connecting: kept as pending
  int bound = pump(4, &given);
  std::printf("phase %d: bound %d, items given this run %d\n", phase, bound, given);
  if (phase == 1) {
    hpps_ap_location("task:6:added");
    hpps_ap_location("beans:10");
    pump(3, &given);
    std::printf("phase 1: after checks, items given this run %d\n", given);
  }
  std::printf("has Day 0 Pass: %d, has Night 0 Pass: %d\n", hpps_ap_has("Day 0 Pass"), hpps_ap_has("Night 0 Pass"));
  return bound ? 0 : 1;
}
