#!/usr/bin/env bash
# Disassemble + recompile default.xbe into src/recomp/gen (run from anywhere). Every hand-written override file is
# listed with --exclude-manual so its sub_X keeps the generated body as sub_X_gen.
set -e
cd "$(dirname "$0")/.."
# the XBE's layout (sections, imports, entry), which disasm reads as game/default_analysis.json
python3 -m tools.xbe_parser game/default.xbe --json game/default_analysis.json
python3 -m tools.disasm game/default.xbe --text-only --extra-sections D3D,D3DX,XGRPH,DSOUND,XMV,XPP \
    --seed-functions port/icall_seeds.json
python3 -m tools.func_id game/default.xbe
python3 -m tools.abi_analysis game/default.xbe
python3 -m tools.recomp game/default.xbe --all --split 1000 --gen-dir port/src/recomp/gen --game-name hpps \
    --exclude-manual port/src/hle_dsound.c \
    --exclude-manual port/src/hle_video.c \
    --exclude-manual port/src/hle_input.c \
    --exclude-manual port/src/hle_xapi.c \
    --exclude-manual port/src/hle_debug.c \
    --exclude-manual port/src/hle_anim.c \
    --exclude-manual port/src/hle_trainer.c \
    --exclude-manual port/src/hle_frontend.c \
    --exclude-manual port/src/hle_world.c \
    --exclude-manual port/src/scda/xdk/hle_d3d8.cpp
# hooks inside generated functions (debug traces) that an override file can't express
python3 port/tools/patch_gen.py port/src/recomp/gen
