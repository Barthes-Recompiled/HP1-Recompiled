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

1. Put the games files or in a folder of your choice:
       hpps_launcher.exe
       hpps.exe
       README.txt
2. Copy the game files from your disc or the .xiso file into a folder named "game" in the same folder with the executables.

The port creates these itself on the first run:
    hpps.toml      your settings (the launcher and F10 change them)
    saves\         your saved games
    shadercache\   shaders built on your PC (the first visit to an area
                   prepares them; later visits load at once)


PLAYING
-------

Start the game with hpps_launcher.exe. Opening hpps.exe directly only offers
to open the launcher.

The launcher sets the aspect ratio (4:3 or 16:9), display, graphics and
controls, then starts the game with Play. If you turn its page off, the
launcher starts the game straight away; hold Shift while opening it to see
the page again.

Keys that always work
  F10          Options (see below)
  Alt+Enter    switch between windowed and borderless
  Alt+F4       quit the game

Controller
  An Xbox 360 / One / Series controller plays like the original Xbox pad.
  The original pad's White and Black buttons are LB and RB.

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

Sneaking: on a controller, push the left stick gently. Keys always move at
full speed, so use the sneak toggle; a controller button can toggle it too
(F10 > Controls > Sneak toggle button).


OPTIONS (F10)
-------------

F10 opens the options over the game; most changes apply at once and are saved
to hpps.toml. "Defaults" resets everything except the aspect ratio. "Show the
launcher at startup" turns the launcher's page on or off.

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

Controls
  Keyboard and mouse
    Keyboard and mouse play as controller 1 - alongside a controller
    Mouse sensitivity, Invert mouse Y
    Key bindings         click a binding, then press a key or mouse button
                         (Escape clears it); "Default keys" restores them
  Controller
    Left / right stick deadzone %      how far a stick moves before it counts
    Left / right stick sensitivity %   the response past the deadzone
    Invert camera X / Y
    Sneak toggle button  a controller button that toggles sneaking (it then
                         does nothing else), or None
    Vibration, Vibration strength %
    Button remapping     which pad button each game button reads;
                         "Default buttons" restores them

Accessibility
  Colour blindness
    Correction filter    Off, Protanopia (red-weak), Deuteranopia
                         (green-weak) or Tritanopia (blue-weak): shifts the
                         colours you confuse toward ones you can tell apart
    Strength %

Debug
  Logs
    Write debug logs     writes hpps_run.log and xbox_kernel.log beside the
                         game from the next start. Off by default; turn it on
                         to report a problem, and send both files with your
                         report.


TROUBLESHOOTING
---------------

"Failed to load default.xbe"
    The game files are missing: check that game\default.xbe exists beside
    hpps.exe.

The game will not start / closes at once
    Update your graphics driver (DirectX 12 is required). Then turn on
    F10 > Debug > Write debug logs (or set debug_logs = true in hpps.toml),
    start again and keep hpps_run.log.

A short pause the first time you enter an area
    Shaders are being prepared for your graphics card; the loading picture
    stays up meanwhile. It happens once per area.
