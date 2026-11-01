HARRY POTTER AND THE PHILOSOPHER'S STONE - PC PORT
==================================================

A native Windows port of the original Xbox game, made by static recompilation.
It contains no game data: you need the files from your own copy of the Xbox disc.


MINIMUM SYSTEM REQUIREMENTS
---------------------------

  OS:       Windows 10 or 11, 64-bit
  CPU:      4 threads with SSE4.1 (Intel Core 2nd generation / AMD FX or newer)
  Memory:   4 GB RAM
  Graphics: DirectX 12 card (feature level 11_0) with a current driver:
            NVIDIA GeForce GTX 600 series, AMD Radeon HD 7000 series,
            Intel HD Graphics 500 series or newer
  Storage:  about 3.1 GB (3 GB game files + the port)
  Input:    keyboard and mouse, or an Xbox controller (360, One or Series)

Windows "N" editions also need Microsoft's Media Feature Pack.


INSTALLING
----------

The easiest way is the Barthes Launcher, one launcher for all three Barthes
Recompiled Harry Potter ports:
    https://github.com/Barthes-Recompiled/Barthes-Launcher/releases/latest
It downloads this port, unpacks your disc image (.iso / .xiso) or copies an
extracted disc, checks every game file against the disc's, installs the
build for your disc's edition (PAL or USA) and keeps it up to date. Your
game folder, saves and settings are never touched by an update.

To install by hand:

1. Download the release for your disc from
       https://github.com/Barthes-Recompiled/HP1-Recompiled/releases/latest
   (PAL for a European disc, USA for a North American one) and unzip it
   into a folder of its own, for example C:\Games\HP1. It holds:
       hpps.exe
       README.txt
       CHANGES.txt
       licenses\
       shadercache\base_shaders.bin
2. Make a folder named "game" beside hpps.exe and put the files of your own
   disc in it, so that game\default.xbe exists. A disc image (.iso / .xiso)
   has to be extracted first: the Barthes Launcher does it for you, or use
   a tool such as extract-xiso.

Updating from an earlier version: replace the files above and keep your
hpps.toml, saves\ and shadercache\ (your settings, saved games and the
shaders already built on your PC stay as they are).

The port creates these itself on the first run:
    hpps.toml      your settings (F10 and the in-game settings
                   change them)
    saves\         your saved games; saves\xbox holds the game's Xbox
                   disk images, so the whole folder can be moved or backed
                   up as it is
    hpps_run.log   what the game printed on its last start (send it
                   with a problem report)
    shadercache\   shaders built on your PC (the first visit to an area
                   prepares them; later visits load at once)


PLAYING
-------

Start hpps.exe, or press Play in the Barthes Launcher.

On the first start, pick your aspect ratio (4:3 or 16:9), display mode,
resolution and controls with "Settings" in the title menu or the pause menu
(Gameplay, Graphics and Controls, in the game's own font), or F10 for every
option. Aspect ratio and speaker changes apply at the next start.

Only one copy of the game runs at a time: starting it again brings the
running game to the front.

Quitting: "Quit To Desktop" in the title menu or the pause menu, or Alt+F4.

Keys that always work
  F10          Options (see below)
  Alt+Enter    switch between windowed and borderless
  Alt+F4       quit the game

Controller
  An Xbox 360 / One / Series controller plays like the original Xbox pad.
  The original pad's White and Black buttons are LB and RB.
  To play with keyboard and mouse while a controller is connected, turn
  off "Use controller" (F10 > Controls).

Keyboard and mouse (defaults; change them under F10 > Controls)
  Move                  W A S D
  Camera                mouse, or the arrow keys
  A                     Space
  B                     E
  X                     left mouse button
  Y                     right mouse button
  Black / White         R / Q
  Left / right trigger  Shift / Ctrl
  D-pad                 1 2 3 4 (up, down, left, right)
  Start / Back          Escape / Tab
  Stick clicks          C (left), middle mouse button (right)
  Sneak (toggle)        Caps Lock: Harry keeps his sneaking pace until you
                        press it again ("Sneaking" shows in the corner)
  Pace of the keys      mouse wheel: down Sneak < Walk < Run up (Run is the
                        default; also F10 > Controls > Movement keys)

Sneaking: on a controller, push the left stick gently. Keys always move at
full speed, so use the sneak toggle; a controller button can toggle it too
(F10 > Controls > Sneak toggle button).


ARCHIPELAGO
-----------

Philosopher's Stone can be played in an Archipelago multiworld randomizer
(https://archipelago.gg): spells, Wizard Cards, beans and other pickups are
shuffled across every game in the room.

The room's host needs hpps.apworld, attached to the release on GitHub, and
your player YAML for the game.

To play, open the game's Archipelago page in the Barthes Launcher, tick
"Play this game in an Archipelago multiworld", enter the room's server
(for example archipelago.gg:38281), your slot name and the password if the
room has one, and press Play. The game connects on its own; the password is
kept in saves\archipelago\connection.toml and never passes through the
command line or a log.

In a randomizer game the pause menu gains a Hogwarts Travel page, for going
back to any day or set piece you have unlocked. With the apworld's Skip
Tutorial option (on by default) a new game starts on Day 1 in the dormitory;
Day 0 stays on the Travel page.

Without the Barthes Launcher, set the environment variables
RECOMP_AP_SERVER (server:port) and RECOMP_AP_SLOT (slot name) before
starting hpps.exe, and put password = "..." in
saves\archipelago\connection.toml if the room has one.


OPTIONS (F10)
-------------

F10 opens the options over the game; most changes apply at once and are saved
to hpps.toml. "Defaults (this page)" resets the page you are on (Graphics,
Controls, ...) and leaves the others; the aspect ratio stays.

Graphics
  Display
    Aspect ratio         4:3 (black bars at the sides), 16:9 (the game's
                         camera widened; menus and movies keep 4:3) or
                         Stretch. Applies at the next start.
    Cutscene bars        (16:9) cutscenes fill the screen and keep the
                         original black bars over the picture, with the
                         subtitles in the bottom bar
    Display mode         Windowed, Borderless (covers the monitor) or
                         Fullscreen (switches the monitor's resolution)
    Window size /
    Resolution           the window's size, or the fullscreen mode; Desktop
                         uses the monitor's own
    Vsync                waits for the monitor's refresh (no tearing)
    Frame rate           the game's rate (60 fps), 30 fps, or Unlocked
                         (experimental: the game was made for 60, some timing
                         may misbehave)
  Resolution
    Internal resolution  the lines the 3D is drawn at: 480 is the Xbox's own;
                         720 to 2160 sharpen it; "Follow window" matches the
                         window
    Render scale %       with "Follow window": the render size as a share of
                         the window's
    Max render height    with "Follow window": the most lines it follows the
                         window to (1080 by default, so a 4K screen keeps
                         60 fps on modest cards)
  Anti-aliasing and filtering
    MSAA                 smooths polygon edges: off, 2x, 4x, 8x
    SMAA                 smooths edges over the finished picture; combines
                         with MSAA
    Anisotropic filtering  sharper textures at an angle: the game's own, 2x,
                         4x, 8x or 16x
  Picture
    Brightness, Contrast 50 leaves the picture as the game draws it
    FPS counter          shows the frame rate
  Post-processing
    Bloom, Bloom strength  a glow around bright parts of the 3D scene (not
                         the HUD), off by default
    Sharpening           contrast-adaptive sharpening, off by default
    Field of view (experimental)
                         80 to 130 % of the game's; 100 is the game's own

Controls
  Keyboard and mouse
    Keyboard and mouse play as controller 1 - alongside a controller
    Mouse sensitivity, Invert mouse Y
    Movement keys        the pace W A S D move Harry at: Sneak, Walk or Run;
                         the mouse wheel steps it in game
    Key bindings         click a binding, then press a key or mouse button
                         (Escape clears it); "Default keys" restores them
  Controller
    Use controller       off: the game ignores the controller (and it does
                         not vibrate); keyboard and mouse still play
    Left / right stick deadzone %      how far a stick moves before it counts
    Left / right stick sensitivity %   the response past the deadzone
    Invert camera X / Y
    Sneak toggle button  a controller button that toggles sneaking (it then
                         does nothing else), or None
    Vibration, Vibration strength %
    Button remapping     which pad button each game button reads;
                         "Default buttons" restores them

Audio
  Speakers               Stereo, or 5.1 surround: the console's own speaker
                         mix, each channel to its own speaker. Needs Windows'
                         speaker setup set to 5.1 (or a 5.1 HDMI receiver).
                         Applies at the next start.

Accessibility
  Colour blindness
    Correction filter    Off, Protanopia (red-weak), Deuteranopia
                         (green-weak) or Tritanopia (blue-weak): shifts the
                         colours you confuse toward ones you can tell apart
    Strength %

Enhancements
  Bertie Bott's beans
    Announce bean flavours on pickup
                         off: the flavour is no longer said aloud when Harry
                         picks up a bean (on by default)

Debug
  Logs
    Write debug logs     writes hpps_run.log and xbox_kernel.log beside the
                         game from the next start. Off by default; turn it on
                         to report a problem, and send both files with your
                         report. With it off, a crash still leaves
                         hpps_crash.log beside the game with the crash report.


TROUBLESHOOTING
---------------

Game files are damaged or missing
    "Verify files" on the game's page in the Barthes Launcher compares
    every file with the disc's and names any that are damaged or missing.
    Add your disc again there (or copy those files again from your disc)
    to repair them. A damaged file can crash the game or lose sound or
    pictures where it is used.

"Failed to load default.xbe"
    The game files are missing: check that game\default.xbe exists beside
    hpps.exe.

The game will not start / closes at once
    Update your graphics driver (DirectX 12 is required). Then turn on
    F10 > Debug > Write debug logs (or set debug_logs = true in hpps.toml),
    start again and keep hpps_run.log.

A short pause the first time you enter an area
    Shaders are being prepared for your graphics card; the loading picture
    stays up meanwhile. It happens once per area. From the next start the
    port prepares everything it has seen ahead, while you are in the menus
    ("Preparing shaders" shows in the corner), so those areas load at once.


LICENCES AND SOURCE CODE
------------------------

The port's own code is under the MIT licence. It also contains code from the
xemu project (the Xbox audio processor emulation), which stays under the GNU
Lesser General Public License, version 2.1 or later. The licence texts and the
list of third-party code are in the licenses folder beside this file.

The complete source code, including that code and any changes made to it, is
available at:
    https://github.com/Barthes-Recompiled/HP1-Recompiled
It is built on MiddleDucky's port:
    https://github.com/MiddleDucky/HP-PS-Xbox-Recomp

The Harry Potter game itself is not included and is not covered by these
licences.
