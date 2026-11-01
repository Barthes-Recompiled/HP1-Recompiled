# Harry Potter and the Philosopher's Stone — Xbox to PC port

A native Windows port of the original Xbox game *Harry Potter and the Philosopher's Stone* (2003), made by
**static recompilation**: the game's x86 code is translated to C and compiled for Windows, with the Xbox kernel,
Direct3D 8, DirectSound and input replaced by native code. No emulator. Part of
[Barthes Recompiled](https://github.com/Barthes-Recompiled), beside
[Chamber of Secrets](https://github.com/Barthes-Recompiled/HP2-Recompiled) and
[Prisoner of Azkaban](https://github.com/Barthes-Recompiled/HP3-Recompiled).

**Current release: 1.0.1**, for the PAL and USA discs. Built on, and with endless thanks to,
[MiddleDucky's port](#thank-you-middleducky).

Features beyond the original: resolutions up to 4K with MSAA/SMAA and anisotropic filtering, true 16:9
(widened 3D, 2D kept in shape), windowed/borderless/fullscreen, keyboard and mouse with rebinding, controller
remapping, deadzones and vibration strength, colour-blind correction filters, frame-rate caps up to 240 or
unlocked, settings pages inside the game's own title and pause menus, cutscene fast-forward, "Quit To Desktop"
in the game's menus, and an [Archipelago](#archipelago) multiworld randomizer.

**This repository contains no game content.** To play or build it you need the files from your own copy of the
Xbox disc. The build regenerates the recompiled code from your `default.xbe`.

## Quick start

1. Download **`barthes_launcher.exe`** from the
   [Barthes Launcher releases](https://github.com/Barthes-Recompiled/Barthes-Launcher/releases/latest) and put it in
   a folder of its own, for example `C:\Games\Barthes`.
2. Start it, pick **Philosopher's Stone** on the shelf and press **Install**.
3. Press **Add disc image or folder...** and choose your `.iso` / `.xiso`, or the `default.xbe` of a disc you
   already extracted. The launcher checks that it is this game's disc, unpacks it, checks every file and installs
   the build for your disc (PAL or USA).
4. Press **Play**.

The launcher keeps the port up to date from this repository's releases and never touches your disc files, saves or
settings when it updates.

## Setting up by hand

1. Download the [latest release](https://github.com/Barthes-Recompiled/HP1-Recompiled/releases/latest) for your
   disc: **PAL** for the European *Philosopher's Stone*, **USA** for the North American *Sorcerer's Stone*. Unzip it
   into a folder of its own, for example `C:\Games\HP1`.
2. Make a folder named `game` beside `hpps.exe` and put your disc's files in it, so that `game\default.xbe` exists.
   A disc image has to be extracted first (the Barthes Launcher does this for you; [extract-xiso](https://github.com/XboxDev/extract-xiso)
   works too).
3. Start `hpps.exe`.

```
C:\Games\HP1\
  hpps.exe
  README.txt, CHANGES.txt, licenses\
  shadercache\         shaders (shipped ones, plus those built on your PC)
  game\default.xbe     ... and the rest of your disc
  hpps.toml            your settings (made on the first start)
  saves\               your saved games, and the game's Xbox disk images in saves\xbox
```

To update by hand, unzip the new release over the old one; `hpps.toml`, `saves\` and `shadercache\` stay as they
are. The whole folder is portable: move it or back it up as it is.

**Requirements:** Windows 10 or 11 (64-bit), a DirectX 12 graphics card (GeForce GTX 600, Radeon HD 7000, Intel HD
500 or newer) with a current driver, 4 GB RAM, about 3.1 GB of disk. Keyboard and mouse, or an Xbox 360 / One /
Series controller (other pads through Steam Input or DS4Windows).

## Which disc

Both editions work: the European (PAL) *Philosopher's Stone* disc, which the code is written for, and the North
American *Sorcerer's Stone* disc, built through the region profiles in `port/region/`. A build is made for one disc
and says so if it is given the other, so each release has a `PAL` and a `USA` download.

## Settings

- **Settings** in the title menu and the pause menu: Gameplay, Graphics and Controls pages in the game's own font,
  including Button Mapping (press the action, then the controller button).
- **F10** anywhere: every option, over the game.
- **Alt+Enter** switches windowed / borderless; **Alt+F4** quits, as does **Quit To Desktop** in the menus.

Both change the same settings and save them to `hpps.toml`. Highlights: resolutions up to 4K with MSAA, SMAA and
anisotropic filtering; true 16:9 with the original cutscene bars kept; windowed, borderless or fullscreen;
frame-rate caps of 30, 60, 120, 144, 165 and 240 or unlocked (experimental); keyboard and mouse with rebinding;
controller remapping, deadzones and vibration strength; colour-blind correction filters; 5.1 surround; cutscene
fast-forward (hold Start or Back) and start-up logo skip, both on by default. Every option and the default keys are
listed in [port/README.txt](port/README.txt), which also ships in the release.

## Archipelago

Philosopher's Stone can be played in an [Archipelago](https://archipelago.gg) multiworld randomizer.

- **Host:** the room needs `hpps.apworld`, attached to each release. Tested on the PAL disc.
- **Player:** on the game's **Archipelago** page in the Barthes Launcher, tick *Play this game in an Archipelago
  multiworld*, enter the room's server (for example `archipelago.gg:38281`), your slot name and the password, and
  press **Play**. The game connects on its own.
- **In the game:** the pause menu gains a **Hogwarts Travel** page for returning to any day or set piece you have
  unlocked. With the apworld's *Skip Tutorial* option (on by default) a new game starts on Day 1 in the dormitory;
  Day 0 stays on the Travel page.

Without the launcher, set `RECOMP_AP_SERVER` (server:port) and `RECOMP_AP_SLOT` (slot name) before starting
`hpps.exe`, and put `password = "..."` in `saves\archipelago\connection.toml` if the room has one.

## Troubleshooting

| Problem | What to do |
|---|---|
| "Failed to load default.xbe" | The disc files are missing: check that `game\default.xbe` exists beside `hpps.exe`. |
| The game says it is the wrong disc | Download the other build (`PAL` or `USA`), or let the Barthes Launcher pick it. |
| Crashes, missing sound or pictures in one place | A disc file may be damaged: **Verify files** on the game's page in the Barthes Launcher names it; add your disc again to repair. |
| The game won't start or closes at once | Update your graphics driver (DirectX 12 is required). Every start writes `hpps_run.log` beside `hpps.exe`; send it with your report. For more detail turn on F10 > Debug > *Write debug logs* and start again. A crash always leaves `hpps_crash.log`. |
| A short pause the first time you enter an area | Shaders are being built for your card, once per area; later starts prepare them ahead in the menus. |
| Keyboard doesn't move Harry while a pad is plugged in | Turn off F10 > Controls > *Use controller*. |
| Archipelago won't connect | Check the server address and port, the slot name's spelling and the password; the room must use the `hpps.apworld` from the same release. |

Problems and questions: [open an issue](https://github.com/Barthes-Recompiled/HP1-Recompiled/issues). What changed
in each release is in [CHANGES.md](CHANGES.md).

## Building

See [port/DEVELOPING.md](port/DEVELOPING.md): MSYS2 (UCRT64) setup, regenerating the code from the XBE, building,
debugging tools and how the port works.

## Thank you, MiddleDucky

This fork exists only because of **[MiddleDucky](https://github.com/MiddleDucky)**, who made the port itself: the
recompilation, the renderer work, the audio, the launcher, and every fix and option in
[HP-PS-Xbox-Recomp](https://github.com/MiddleDucky/HP-PS-Xbox-Recomp). Everything here is built on that work, and
each upstream release (v0.3.0 most recently) is merged in with gratitude. Thank you, MiddleDucky, for bringing this
game to the PC and for sharing it so generously. The fork only adds a few things on top (the North American disc,
settings pages inside the game, cutscene and logo skips, and Archipelago). If you enjoy this port, go and star
MiddleDucky's repositories: it is their work you are playing.

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
