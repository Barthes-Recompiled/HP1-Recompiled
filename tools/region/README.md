# tools/region

`xbe_align.py` maps guest addresses from one regional build of a game to another (PAL to NTSC-U, for example).
It needs Python 3 and the standard library. If `capstone` is installed it is used to find pointers in code
more precisely.

```
py -3 tools/region/xbe_align.py map   --from game/default.xbe --to other/default.xbe -o map.json
py -3 tools/region/xbe_align.py addrs --map map.json --from game/default.xbe --to other/default.xbe \
    port/src/hle_*.c port/src/main.c port/icall_seeds.json
```

`--from` is the XBE the repo is built against. `--to` is the other region's XBE.

## Map

Regional builds are usually the same program with a few functions or data items changed, so the map is a list
of segments: `{from_start, from_end, delta, section, confidence, method}`.

- **bytes**: the segment was found by comparing a section with its namesake in the other image. Any 32-bit
  value that points into the image is masked first, so relocated pointers compare equal.
- **bss-pointers / bss-extend**: bss has no bytes to compare, so these segments come from the pointers into it
  found in aligned code and data.

Every `bytes` segment is checked. Each absolute pointer inside it is moved to the other image, and the value
there must equal the pointer's target mapped through the table. `pointers_checked` and `pointers_ok` record the
result. A segment that fails most of its checks is dropped and listed in `unmapped`.

`unmapped` lists the ranges that exist in only one image, with `side` set to `from` or `to`.

## Address check

`addrs` finds `sub_XXXXXXXX` names and hex literals inside the image in the files you give it, and classifies
each one:

- **high**: in a high-confidence segment, and the code at both addresses matches. Relocated pointers and rel32
  branches whose targets map onto each other count as matching.
- **ambiguous**: anything that maps but isn't high.
- **unmapped**: the address is in no segment.

A literal that is an exclusive range end, such as the end of `.text`, shows up as unmapped.

The map and the reports contain only addresses, counts and hashes. Keep them, and the XBEs, out of the
repository like other per-game tool output.
