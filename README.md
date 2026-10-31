# Harry Potter and the Philosopher's Stone — Xbox to PC port

A native Windows port of the original Xbox game *Harry Potter and the Philosopher's Stone* (2003), made by
**static recompilation**: the game's x86 code is translated to C and compiled for Windows, with the Xbox kernel,
Direct3D 8, DirectSound and input replaced by native code. No emulator.

Features beyond the original: resolutions up to 4K with MSAA/SMAA and anisotropic filtering, true 16:9
(widened 3D, 2D kept in shape), windowed/borderless/fullscreen, keyboard and mouse with rebinding, controller
remapping, deadzones and vibration strength, colour-blind correction filters, frame-rate caps up to 240 or
unlocked, settings pages inside the game's own title and pause menus, cutscene fast-forward, and "Quit To Desktop"
in the game's menus.

**This repository contains no game content.** To play or build it you need the files from your own copy of the
Xbox disc. The build regenerates the recompiled code from your `default.xbe`.

## Which disc

Both releases work: the European (PAL) *Philosopher's Stone* disc, which the code is written for, and the North
American *Sorcerer's Stone* disc, built through the region profiles in `port/region/` (see
[port/DEVELOPING.md](port/DEVELOPING.md), section 3.1). A build is made for one disc and says so if it is given the
other, so each release has a `PAL` and a `USA` download.

## Playing

The easiest way is the **[Barthes Launcher](https://github.com/Barthes-Recompiled/Barthes-Launcher/releases/latest)**,
one launcher for all three Barthes Recompiled Harry Potter ports: it installs the port, unpacks and checks your disc,
picks the build for your disc's edition and keeps it up to date.

To set it up by hand, download the [latest release](https://github.com/Barthes-Recompiled/HP1-Recompiled/releases/latest)
for your disc, put your disc's files or the .iso in a `game` folder beside the executables and start
`hpps_launcher.exe`. Requirements, controls and every option are in [port/README.txt](port/README.txt); what changed
in each release is in [CHANGES.md](CHANGES.md).

Archipelago multiworld randomizer support is coming soon.

## Building

See [port/DEVELOPING.md](port/DEVELOPING.md): MSYS2 (UCRT64) setup, regenerating the code from the XBE, building,
debugging tools and how the port works.

## Thank you, MiddleDucky

This fork exists only because of **[MiddleDucky](https://github.com/MiddleDucky)**, who made the port itself: the
recompilation, the renderer work, the audio, the launcher, and every fix and option in
[HP-PS-Xbox-Recomp](https://github.com/MiddleDucky/HP-PS-Xbox-Recomp). Everything here is built on that work, and
each upstream release (v0.3.0 most recently) is merged in with gratitude. Thank you, MiddleDucky, for bringing this
game to the PC and for sharing it so generously. The fork only adds a few things on top (the North American disc,
settings pages inside the game, cutscene and logo skips, and soon Archipelago).

## Credits and licences

The port's own code is MIT licensed, © 2026 MiddleDucky ([LICENSE](LICENSE)). The parts below keep their own
licences and copyright.

* **xboxrecomp** — the static recompilation toolkit this port is built on (runtime, kernel layer, pipeline),
  MIT, by sp00nz. Its own readme: [README.xboxrecomp.md](README.xboxrecomp.md); licence [LICENSES/xboxrecomp-MIT.txt](LICENSES/xboxrecomp-MIT.txt);
  third-party parts (including LGPL code from xemu) in [NOTICE](NOTICE) and [LICENSES/](LICENSES/).
* **SCDA-XBERecomp** by TGP482 — the D3D8-to-Direct3D 12 renderer the port's graphics are based on, MIT
  ([port/src/scda/LICENSE.SCDA](port/src/scda/LICENSE.SCDA)).
* **Dear ImGui** (MIT, port/third_party/imgui) and **SMAA** (MIT, Jorge Jimenez et al.).

### Thanks

These projects were references for hardware behaviour, XDK details or fixes
(no code taken unless listed above):

* [xemu](https://github.com/xemu-project/xemu) — Xbox hardware behaviour; the APU code listed in NOTICE
* [XbSymbolDatabase](https://github.com/Cxbx-Reloaded/XbSymbolDatabase) — XDK function addresses
* [Cxbx-Reloaded](https://github.com/Cxbx-Reloaded/Cxbx-Reloaded) and [xboxdevwiki](https://xboxdevwiki.net) — kernel and hardware reference
* [Nightfire Windows Port](https://github.com/limeylicious/Nightfire-Windows-Port) — the same lifter bugs, found first
* [DefJamFFNY-recomp](https://github.com/vyanhursky/DefJamFFNY-recomp) — single-core guest thread pinning
* [Unicorn Engine](https://github.com/unicorn-engine/unicorn) — the debug-only difftest build

*Harry Potter* and all related names and artwork are trademarks of and © Warner Bros. Entertainment Inc. The game
is © Electronic Arts. This is an unofficial fan project, not affiliated with or endorsed by Warner Bros.,
Electronic Arts or Microsoft.

This project has been aided by Claude Code.
