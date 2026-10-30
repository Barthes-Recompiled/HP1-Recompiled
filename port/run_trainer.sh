#!/usr/bin/env bash
# The debug trainer (hpps_trainer.exe) under Wine, beside a running hpps.exe (start either first): noclip, any
# spell on the buttons, infinite health, saved positions. Same Wine prefix as the game, so they find each other.
cd "$(dirname "$0")/run" || exit 1
export WINEDEBUG=${WINEDEBUG:--all}
exec wine ./hpps_trainer.exe "$@"
