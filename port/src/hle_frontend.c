/**
 * "Quit To Desktop" on the main menu and the pause menu: on a PC the desktop stands in for the dashboard.
 *
 * The main menu's input is sub_0001F200 and its drawing sub_0001C380: four rows (New Game, Continue, Options,
 * Extras -- shown as Language on the Xbox), the chosen one in [0x325334] (0..3, wrapping). Row i is drawn at
 * sub_00168D70(x, y) = (400 - 15 i, 295 + 37 i) -- (392 - 15 i, 310 + 33 i) when [0x32E210] is 2 (11 changes y
 * only) -- in a colour and scale that ease toward their targets each frame (lit: white, growing to 1.4 / 1.15;
 * others bluer the further they are). A fifth row, index 4, goes after them and leaves the way the title's own
 * dashboard launches do: XLaunchNewImage(NULL, NULL) (sub_00153FC4) -> HalReturnToFirmware, which ends the
 * process.
 *
 * The four rows and the port's two share the room the four had (sub_00168D70 while the menu draws them,
 * title_row_y): a step 0.84 of the game's, the last row a little above the game's fourth, so a lit row clears the
 * copyright line and the first stays clear of the logo. While the fifth row is chosen the title's input step does not run; down from the last row or up
 * from the first picks it, as the title's own wrap would. Choosing it asks first: the rows (the text draw
 * sub_00169A40, a printf) give way to "Quit To Desktop?" over the game's GUI_YES / GUI_NO in the rows' style,
 * No lit; up / down picks, A or START answers, B (as the options' accept-changes screen, 0x1FB80) is No.
 *
 * The pause menu is input sub_000307F0 and drawing sub_00030F00, state [0x32E638] (2 the rows, 0x10 the "sure
 * quit" box with Yes / No in [0x32E7F8], left / right toggles, A answers); rows 0..9 from the key table at
 * 0x211720 (GUI_QUIT the last), chosen in [0x32E644], wrapping; row i at y 90 + 32 i (the 32.0 at 0x1FB530, which
 * nothing else in the drawing reads) in an easing colour (lit white; else r 0.2 - d/60, g 0.64 - 0.05 d,
 * b 0.99 - 0.04 d, a 0.95 - 0.05 d) and scale (4 dt a frame, to 1.4 / 1.2 in mode 11), and a "select" hint at
 * (320, 416). Row 10, "Quit To Desktop", goes below GUI_QUIT the way the main menu's fifth row does (the input
 * step's own jump table ignores an index past 9; up from it is the step's own decrement); the rows close up to a
 * 30 step while the pause draws and the hint moves to 430, clear of it, its button sprite (sub_00030510's
 * sub_00169F70, 270-318 x 390-438 for the A) down with it. Choosing it opens the game's own box,
 * titled "Quit To Desktop?"; its Yes ends the process, No goes back to the row.
 *
 * Built into the recompile with --exclude-manual port/src/hle_frontend.c (regen.sh); until a regen the generated
 * bodies of sub_0001F200, sub_0001C380, sub_000307F0, sub_00030F00, sub_00030510 (recomp_0000.c), sub_00168D70,
 * sub_00169A40 and sub_00169F70 (recomp_0004.c) are renamed _gen by hand. RECOMP_QUIT_ITEM=0 leaves both menus as they were.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RECOMP_GENERATED_CODE
#include "recomp/gen/recomp_types.h"

#define ARG(i) MEM32(g_esp + 4 + 4 * (i))

enum {
    kRow = 0x325334, kRows = 4, kSettingsRow = 4, kQuitRow = 5, kMode = 0x32E210, kBusy = 0x32536D,
    kPadState = 0x325364, kDown = 0x356BD0, kStickY = 0x356BA8, kUp = 0x356BB0, kConfirmA = 0x356C14,
    kConfirmB = 0x356BE4, kBack = 0x356C74, kSndMove = 0x322DA0, kSndConfirm = 0x322D9C, kSndBack = 0x322DA4,
    kFrameTime = 0x322D48, kKeyYes = 0x1FB580, kKeyNo = 0x1FB578,
    kPState = 0x32E638, kPList = 2, kPSure = 0x10, kPRow = 0x32E644, kPYes = 0x32E7F8, kPDt = 0x32E62C,
    kPRows = 10, kPQuitRow = 10, kPStep = 0x1FB530, kKeySelect = 0x2118E0,
};

extern void sub_0001F200_gen(void);
extern void sub_0001C380_gen(void);
extern void sub_000307F0_gen(void);
extern void sub_00030F00_gen(void);
extern void sub_00168D70_gen(void);
extern void sub_00169A40_gen(void);
extern void sub_00030510_gen(void);
extern void sub_00169F70_gen(void);
extern void sub_00168D90(void);
extern void sub_00168E60(void);
extern void sub_00168DC0(void);
extern void sub_00168DF0(void);
extern void sub_00163AF0(void);
extern void sub_00163BF0(void);
extern void sub_0002A7C0(void);
extern void sub_0002A9F0(void);
extern void sub_000306C0(void);
extern void sub_00030510(void);
extern void sub_00153FC4(void);
extern void sub_0015CCA0(void);
extern void __stdcall xbox_HalReturnToFirmware(unsigned long routine);
uint32_t xbox_HeapAlloc(uint32_t size, uint32_t alignment);

static int g_in_menu;               /* the title's rows are being drawn: they take their places among the six */
static float g_col[4] = {0.2f, 0.64f, 0.99f, 0.95f}, g_scale = 1.0f;  /* the fifth row's easing colour / scale */
static int g_ask, g_ask_yes;        /* the main menu asks; Yes is lit */
static float g_ask_col[2][4] = {{0.2f, 0.64f, 0.99f, 0.95f}, {0.2f, 0.64f, 0.99f, 0.95f}}, g_ask_scale[2] = {1, 1};
static int g_pause_draw;            /* the pause menu is drawing */
static int g_pause_ask;             /* the pause menu's box is up for row 10 */
static int g_pause_hint;            /* the pause menu's hint is drawing its button */
static float g_pcol[3][4] = {{0.2f, 0.64f, 0.99f, 0.95f}, {0.2f, 0.64f, 0.99f, 0.95f}, {0.2f, 0.64f, 0.99f, 0.95f}},
             g_pscale[3] = {1, 1, 1};  /* the port's rows' easing colours / scales */
static int g_page_draw;             /* a page is up: the title's rows and hint are not drawn */
/* the settings / travel pages (below): which is up and where (0 none, 1 over the title, 2 in the pause menu) */
enum { kPageTravel, kPageSettings, kPageGameplay, kPageGraphics, kPageControls, kPageButtons };
/* the Button mapping page's mode: its rows, waiting for a pad button (input.cpp), or asking whether to swap */
enum { kMapRows, kMapWait, kMapAsk };
static struct {
    int page, sel, top, open_travel, where;
    int map_mode, map_game, map_button, map_other;  /* the game button asked for, the pad button got, its owner */
    float col[64][4], scale[64];
} g_pg;
static void page_input(void);
static void page_draw(void);
static void page_open_at(int page, int where);
static float g_last_x, g_last_y;    /* the last text position the game set, as it set it */

static int enabled(void)
{
    static int on = -1;
    if (on < 0) {
        const char *e = getenv("RECOMP_QUIT_ITEM");
        on = !(e && *e == '0');
    }
    return on;
}

/* a recompiled function on this guest thread, arguments pushed right to left and a return address; esp and the
 * callee-saved registers as before whoever pops */
static uint32_t call_guest(void (*fn)(void), int n, const uint32_t *args)
{
    uint32_t s_esp = g_esp, s_ebx = g_ebx, s_esi = g_esi, s_edi = g_edi, s_ebp = g_ebp;
    uint32_t sp = s_esp;
    for (int i = n; i-- > 0;) {
        sp -= 4;
        MEM32(sp) = args[i];
    }
    sp -= 4;
    MEM32(sp) = 0;
    g_esp = sp;
    fn();
    g_esp = s_esp, g_ebx = s_ebx, g_esi = s_esi, g_edi = s_edi, g_ebp = s_ebp;
    return g_eax;
}

static uint32_t fb(float f)
{
    uint32_t u;
    memcpy(&u, &f, 4);
    return u;
}

static float bf(uint32_t u)
{
    float f;
    memcpy(&f, &u, 4);
    return f;
}

/* the pad's per-button record the title's menus read (word +0x10 of the 24-byte entry the index names) */
static int16_t pad(uint32_t index_var)
{
    uint32_t tbl = MEM32(MEM32(kPadState) + 0x10);
    return (int16_t)MEM16(tbl + MEM32(index_var) * 24 + 0x10);
}

static int pressed_down(void) { return pad(kDown) != 0 || pad(kStickY) == -1; }
static int pressed_up(void) { return pad(kUp) != 0 || pad(kStickY) == 1; }
static int pressed_confirm(void) { return pad(kConfirmA) != 0 || pad(kConfirmB) != 0; }

static void sound(uint32_t var)
{
    uint32_t a[2] = {MEM32(var), 0};
    call_guest(sub_00163BF0, 1, a);
    call_guest(sub_00163AF0, 2, a);
}

static void quit_to_desktop(void)
{
    fprintf(stderr, "[MENU] Quit To Desktop\n");
    uint32_t a[2] = {0, 0};
    call_guest(sub_00153FC4, 2, a);  /* XLaunchNewImage(NULL, NULL): does not come back */
    xbox_HalReturnToFirmware(2);
}

static int wide_mode(void) { return MEM32(kMode) == 2 || MEM32(kMode) == 11; }
static float row_step(void) { return wide_mode() ? 33.0f : 37.0f; }
/* the six title rows (the game's four, then Settings and Quit To Desktop) on a step a little closer than the game's,
 * the last one a little above where the game puts its fourth: a lit row (1.4 x) clears the copyright line below and
 * the first row stays clear of the logo above, at 4:3 and 16:9 alike */
static float title_row_y(float i)
{
    float last = (wide_mode() ? 310.0f : 295.0f) + 3.0f * row_step() - 8.0f;
    return last - (5.0f - i) * row_step() * 0.84f;
}

static uint32_t guest_text(const char *s, uint32_t *va)
{
    if (!*va) {
        size_t n = strlen(s) + 1;
        *va = xbox_HeapAlloc((uint32_t)n, 16);
        if (*va) memcpy((void *)XBOX_PTR(*va), s, n);
    }
    return *va;
}

static uint32_t quit_text(void)
{
    static uint32_t va;
    return guest_text("Quit To Desktop", &va);
}

static uint32_t ask_text(void)
{
    static uint32_t va;
    return guest_text("Quit To Desktop?", &va);
}

/* a line of the game's text (sub_0002A9F0, by key) */
static uint32_t game_text(uint32_t key)
{
    uint32_t a[1] = {key};
    return call_guest(sub_0002A9F0, 1, a);
}

static float ease(float cur, float target)
{
    /* up 0.04 a frame, down 0.02, as the title's rows */
    if (cur < target) return cur + 0.04f > target ? target : cur + 0.04f;
    if (cur > target) return cur - 0.02f < target ? target : cur - 0.02f;
    return cur;
}

static void set_align(uint32_t a0, uint32_t a1)
{
    uint32_t align[4] = {a0, a1, 0, 0};
    call_guest(sub_00168DC0, 4, align);
}

static void draw_text(const float col[4], float scale, float x, float y, uint32_t text)
{
    uint32_t c[4] = {fb(col[0]), fb(col[1]), fb(col[2]), fb(col[3])};
    call_guest(sub_00168D90, 4, c);
    uint32_t sc[1] = {fb(scale)};
    call_guest(sub_00168E60, 1, sc);
    uint32_t pos[2] = {fb(x), fb(y)};
    call_guest(sub_00168D70_gen, 2, pos);
    uint32_t t[1] = {text};
    call_guest(sub_00169A40_gen, 1, t);
}

/* the main menu's input step */
void sub_0001F200(void)
{
    if (!enabled()) {
        sub_0001F200_gen();
        return;
    }
    if (g_ask) {
        if (pressed_down() || pressed_up()) {
            g_ask_yes = !g_ask_yes;
            sound(kSndMove);
        } else if (pressed_confirm() && !MEM8(kBusy)) {
            sound(kSndConfirm);
            if (g_ask_yes) quit_to_desktop();
            g_ask = 0;
        } else if (pad(kBack) != 0) {
            sound(kSndBack);
            g_ask = 0;
        }
        g_esp += 4;  /* the step's ret */
        return;
    }
    if (g_pg.where == 1) {  /* a settings page over the title */
        page_input();
        g_esp += 4;  /* the step's ret */
        return;
    }
    int row = (int32_t)MEM32(kRow);
    if (row < kRows) {
        int down = pressed_down(), up = pressed_up();
        sub_0001F200_gen();
        int after = (int32_t)MEM32(kRow);
        /* the title wrapped past its last or first row: the port's rows come in between */
        if (down && row == kRows - 1 && after == 0) MEM32(kRow) = kSettingsRow;
        else if (up && !down && row == 0 && after == kRows - 1) MEM32(kRow) = kQuitRow;
        return;
    }
    if (pressed_down()) {
        MEM32(kRow) = row == kSettingsRow ? kQuitRow : 0;
        sound(kSndMove);
    } else if (pressed_up()) {
        MEM32(kRow) = row == kQuitRow ? kSettingsRow : kRows - 1;
        sound(kSndMove);
    } else if (pressed_confirm() && !MEM8(kBusy)) {
        sound(kSndConfirm);
        if (row == kSettingsRow) {
            page_open_at(kPageSettings, 1);
        } else {
            g_ask = 1;
            g_ask_yes = 0;
        }
    }
    g_esp += 4;  /* the step's ret */
}

/* text position (x, y) */
void sub_00168D70(void)
{
    float x = bf(ARG(0)), y = bf(ARG(1));
    g_last_x = x, g_last_y = y;
    /* the title's rows a row higher while the menu draws them (not the copyright line) */
    if (g_in_menu && !(x == 320.0f && y == 423.0f))  /* the game's row i, in its place among the six */
        ARG(1) = fb(title_row_y((y - (wide_mode() ? 310.0f : 295.0f)) / row_step()));
    /* the pause menu's "select" hint below row 10 */
    if (g_pause_draw && x == 320.0f && y == 416.0f) ARG(1) = fb(430.0f);
    sub_00168D70_gen();
}

/* the hint's button sprite (x0, y0, x1, y1, ...) */
void sub_00030510(void)
{
    g_pause_hint = g_pause_draw;
    sub_00030510_gen();
    g_pause_hint = 0;
}

/* a 2D sprite (x0, y0, x1, y1, ...): the pause hint's button goes down to its text at 430 */
void sub_00169F70(void)
{
    if (g_page_draw && g_pause_hint) {
        g_esp += 4;  /* the title's hint button, under a page: the page draws its own */
        return;
    }
    if (g_pause_hint) {
        ARG(1) = fb(bf(ARG(1)) + 14.0f);
        ARG(3) = fb(bf(ARG(3)) + 14.0f);
    }
    sub_00169F70_gen();
}

static void pause_row(void);
static float pause_step(void);

/* text draw (format, ...) */
void sub_00169A40(void)
{
    if (g_page_draw) {
        g_esp += 4;  /* a page is up: the title's row text gives way (its ret, undrawn) */
        return;
    }
    if (g_in_menu && g_ask && !(g_last_x == 320.0f && g_last_y == 423.0f)) {
        g_esp += 4;  /* the rows give way to the question: their ret, undrawn */
        return;
    }
    if (g_pause_draw && g_pause_ask && g_last_x == 320.0f && g_last_y == 200.0f)
        ARG(0) = ask_text();  /* the box's title (GUI_SUREQUIT) */
    if (g_pause_draw && MEM32(kPState) == kPList && g_last_y == 90.0f + pause_step() * (kPRows - 1) &&
        (g_last_x == 90.0f || g_last_x == 128.0f)) {
        float x = g_last_x;
        uint32_t s_esp = g_esp;
        sub_00169A40_gen();
        uint32_t ret_esp = g_esp;
        g_esp = s_esp;
        g_last_x = x;
        pause_row();
        g_esp = ret_esp;
        return;
    }
    sub_00169A40_gen();
}

/* one of the main menu's row places (index i of five, shifted as the rows are), its colour and scale eased */
/* the title's text settings as its copyright line left them (0x1C82D), after the port drew */
static void title_restore(void)
{
    set_align(1, 0);
    uint32_t col0[4] = {fb(0.75f), fb(0.75f), 0, fb(1.0f)};
    call_guest(sub_00168D90, 4, col0);
    uint32_t sc0[1] = {fb(0.9f)};
    call_guest(sub_00168E60, 1, sc0);
}

static void menu_row(int i, uint32_t text, float col[4], float *scale, int lit, float d)
{
    float target[4] = {0.2f - d * 0.02f, 0.64f - d * 0.0833333f, 0.99f - d * 0.0666667f, 0.95f - d * 0.1f};
    for (int k = 0; k < 4; k++) col[k] = ease(col[k], lit ? 1.0f : target[k]);
    float step = bf(MEM32(kFrameTime)) * 0.01f, top = MEM32(kMode) == 2 ? 1.15f : 1.4f;
    *scale += lit ? step : -step;
    if (*scale > top) *scale = top;
    if (*scale < 1.0f) *scale = 1.0f;
    float x = (MEM32(kMode) == 2 ? 392.0f : 400.0f) - 15.0f * i;
    float y = title_row_y((float)i);
    set_align(0, 1);  /* the rows' alignment (0x1C46D) */
    draw_text(col, *scale, x, y, text);
}

/* the main menu's drawing */
void sub_0001C380(void)
{
    if (!enabled()) {
        sub_0001C380_gen();
        return;
    }
    uint32_t s_esp = g_esp;
    if (g_pg.where == 1) {  /* a settings page: over the menu's moving background, the title's logo and rows left out */
        page_draw();
        title_restore();
        g_esp = s_esp + 4;  /* the drawing's ret */
        return;
    }
    g_in_menu = 1;
    sub_0001C380_gen();
    g_in_menu = 0;
    uint32_t ret_esp = g_esp;
    g_esp = s_esp;

    int sel = (int32_t)MEM32(kRow);
    uint32_t text = quit_text();
    static uint32_t settings_va;
    uint32_t settings = guest_text("Settings", &settings_va);
    static float settings_col[4] = {0.2f, 0.64f, 0.99f, 0.95f}, settings_scale = 1.0f;
    if (g_ask) {
        /* in the rows' places: the question, then Yes and No */
        static const float white[4] = {1, 1, 1, 1};
        uint32_t q = ask_text(), yes = game_text(kKeyYes), no = game_text(kKeyNo);
        if (q) {
            set_align(0, 1);
            draw_text(white, 1.0f, (MEM32(kMode) == 2 ? 392.0f : 400.0f) - 15.0f * 2,
                      title_row_y(2.0f), q);
        }
        if (yes) menu_row(3, yes, g_ask_col[0], &g_ask_scale[0], g_ask_yes, 1);
        if (no) menu_row(4, no, g_ask_col[1], &g_ask_scale[1], !g_ask_yes, 1);
    } else {
        /* the port's rows, as the title's loop would draw rows 4 and 5 (0x1C4B0) */
        float d = (float)(sel > kSettingsRow ? sel - kSettingsRow : kSettingsRow - sel);
        if (settings) menu_row(kSettingsRow, settings, settings_col, &settings_scale, sel == kSettingsRow, d);
        d = (float)(sel > kQuitRow ? sel - kQuitRow : kQuitRow - sel);
        if (text) menu_row(kQuitRow, text, g_col, &g_scale, sel == kQuitRow, d);
    }
    title_restore();
    g_esp = ret_esp;
}

/* ── the pause menu's own rows and pages ──────────────────────────────────────────────────────────────────────────
 * Below the title's ten rows (GUI_QUIT the last) come rows of the port's own, in the rows' style: "Hogwarts Travel"
 * (with an Archipelago slot set), "Settings" and "Quit To Desktop". Travel and Settings open a page: a pause state of
 * its own (kPPage) in which the title's drawing runs as for its row list (the panel behind, its rows and hint left
 * undrawn) and the page's rows are drawn in their place with the same text call, colours, scale and hint, scrolling
 * when longer than the screen; the pad as the title's menus read it (up / down, left / right, A, B) and the title's
 * menu sounds. Settings lists Gameplay, Graphics and Controls; Gameplay's rows are app_InGameOption* (app.cpp), the
 * same settings the F10 panel changes. Controls opens with Button Mapping: each game button and the pad button it
 * reads (hpps.toml pad_*); A waits for a pad button (input.cpp, the game sees the pad at rest meanwhile), one already
 * in use asks whether to swap the two, and holding B, Escape or ten idle seconds cancel. The travel window's both-triggers + Back shortcut (hle_ap.c) opens the pause
 * menu on the Travel page. */

enum { kPPage = 0x20, kStartBtn = 0x356CB0, kLeft = 0x356CA4, kRight = 0x356C18, kStickX = 0x356C6C };
enum { kXTravel, kXSettings, kXQuit };
/* the Archipelago bridge (src/ap, hle_ap.c), weak: a build without it has no Travel row */
extern int hpps_ap_enabled(void) __attribute__((weak));
extern int hpps_ap_travel_entry(int i, const char **name, int *level, const char **script, int *dungeon,
                                int *unlocked) __attribute__((weak));
extern void hpps_ApTravelRequest(int entry) __attribute__((weak));
extern int app_PageOptionCount(int page);  /* scda/app/app.cpp: 0 Gameplay, 1 Graphics, 2 Controls */
extern const char *app_PageOptionLabel(int page, int i);
extern const char *app_PageOptionValue(int page, int i);
extern void app_PageOptionStep(int page, int i, int dir);
extern void app_PadCaptureStart(void);  /* scda/app/input.cpp: the Button mapping page's wait and the remapping */
extern int app_PadCaptureResult(int *button);
extern const char *app_PadInputLabel(int i);
extern int app_PadInputCount(void);
extern int app_PadMapGet(int game);
extern void app_PadMapSet(int game, int button, int swap);
extern void app_PadMapReset(void);
extern void sub_001684D0(void);

static int extras(int kind[3])
{
    int n = 0;
    if (hpps_ap_enabled && hpps_ap_travel_entry && hpps_ApTravelRequest && hpps_ap_enabled()) kind[n++] = kXTravel;
    kind[n++] = kXSettings;
    kind[n++] = kXQuit;
    return n;
}

static uint32_t extra_text(int kind)
{
    static uint32_t va[3];
    static const char *const kText[3] = {"Hogwarts Travel", "Settings", "Quit To Desktop"};
    return guest_text(kText[kind], &va[kind]);
}

/* the row step while the pause menu draws: the ten rows and the extras above the hint at 430 */
static float pause_step(void)
{
    int kind[3];
    return extras(kind) > 2 ? 26.0f : 28.0f;
}

/* ── pages ── */
#define PAGE_ROWS_SHOWN 9
#define PAGE_TITLE_Y    52.0f   /* the page's name */
#define PAGE_AREA_TOP   100.0f  /* the rows' tops (centred text hangs from y) keep below the name, lit rows included */
#define PAGE_AREA_END   372.0f  /* and above the "..." and the hint at 430 */
#define PAGE_STEP       30.0f
#define PAGE_TEXT_H     30.0f   /* the room a row takes per unit of scale, which a lit row grows by */
#define PAGE_COL_GAP    8.0f    /* half the gap between a row's label and value columns */
#define PAGE_LIT_SCALE  1.2f    /* a lit row grows less than the menus' 1.4: long values stay clear of the edges */

/* a flat rectangle in the 640 x 480 frame, drawn as the letterbox bars draw theirs (0x2A600: a state block for an
 * untextured quad, then sub_00169F70 with a colour; the menus' 2D state is already up) */
static void draw_rect(float x0, float y0, float x1, float y1, const float col[4])
{
    static uint32_t va;
    if (!va) va = xbox_HeapAlloc(0x80, 16);
    if (!va) return;
    uint8_t *m = (uint8_t *)XBOX_PTR(va);
    memset(m, 0, 0x80);
    memcpy(m, col, 16);                   /* the colour (the bars' four floats under the block) */
    uint32_t st = va + 0x10;              /* the state block, as the bars fill it */
    MEM32(st + 0x18) = 3;
    MEM32(st + 0x28) = 1;
    MEM32(st + 0x48) = 6;
    MEM32(st + 0x4C) = 2;
    uint32_t a[1] = {st};
    call_guest(sub_0015CCA0, 1, a);
    uint32_t q[8] = {fb(x0), fb(y0), fb(x1), fb(y1), 0, 0, va, fb(1.0f)};
    call_guest(sub_00169F70_gen, 8, q);
}
typedef struct { char text[96]; int enabled, indent; } PageRow;

static int page_rows(int page, PageRow *rows, int max)
{
    int n = 0;
    if (page == kPageTravel && hpps_ap_travel_entry) {
        const char *name, *script;
        int level, dungeon, unlocked;
        for (int i = 0; n < max && hpps_ap_travel_entry(i, &name, &level, &script, &dungeon, &unlocked); i++, n++) {
            snprintf(rows[n].text, sizeof rows[n].text, "%s%s", name, unlocked ? "" : "  (locked)");
            rows[n].enabled = unlocked, rows[n].indent = dungeon;
        }
    } else if (page == kPageSettings) {
        static const char *const kSub[3] = {"Gameplay", "Graphics", "Controls"};
        for (; n < 3; n++) {
            snprintf(rows[n].text, sizeof rows[n].text, "%s", kSub[n]);
            rows[n].enabled = 1, rows[n].indent = 0;
        }
    } else if (page == kPageButtons && g_pg.map_mode == kMapAsk) {
        /* the pad button got already plays another game button: swap the two, or leave both */
        snprintf(rows[0].text, sizeof rows[0].text, "%s is used by %s", app_PadInputLabel(g_pg.map_button),
                 app_PadInputLabel(g_pg.map_other));
        snprintf(rows[1].text, sizeof rows[1].text, "Swap (%s gets %s)", app_PadInputLabel(g_pg.map_other),
                 app_PadInputLabel(app_PadMapGet(g_pg.map_game)));
        snprintf(rows[2].text, sizeof rows[2].text, "Cancel");
        rows[0].enabled = 0, rows[1].enabled = rows[2].enabled = 1;
        rows[0].indent = rows[1].indent = rows[2].indent = 0;
        n = 3;
    } else if (page == kPageButtons) {
        /* each game button and the pad button it reads, then the defaults */
        for (int i = 0; n < max && i < app_PadInputCount(); i++, n++) {
            int wait = g_pg.map_mode == kMapWait && i == g_pg.map_game;
            snprintf(rows[n].text, sizeof rows[n].text, "%s:  %s", app_PadInputLabel(i),
                     wait ? "press a button" : app_PadInputLabel(app_PadMapGet(i)));
            rows[n].enabled = 1, rows[n].indent = 0;
        }
        if (n < max) {
            snprintf(rows[n].text, sizeof rows[n].text, "Reset to defaults");
            rows[n].enabled = 1, rows[n].indent = 0, n++;
        }
    } else {
        int opts = page - kPageGameplay;
        if (page == kPageControls && n < max) {  /* Controls opens with the Button mapping page */
            snprintf(rows[n].text, sizeof rows[n].text, "Button mapping");
            rows[n].enabled = 1, rows[n].indent = 0, n++;
        }
        for (int i = 0; n < max && i < app_PageOptionCount(opts); i++, n++) {
            snprintf(rows[n].text, sizeof rows[n].text, "%s:  %s", app_PageOptionLabel(opts, i),
                     app_PageOptionValue(opts, i));
            rows[n].enabled = 1, rows[n].indent = 0;
        }
    }
    return n;
}

static const char *page_title(int page)
{
    switch (page) {
    case kPageTravel: return "Hogwarts Travel";
    case kPageSettings: return "Settings";
    case kPageGameplay: return "Gameplay";
    case kPageGraphics: return "Graphics";
    case kPageButtons: return "Button Mapping";
    default: return "Controls";
    }
}

static void page_open_at(int page, int where)
{
    PageRow rows[64];
    g_pg.map_mode = kMapRows;
    int n = page_rows(page, rows, 64);
    g_pg.page = page, g_pg.sel = -1, g_pg.top = 0;
    for (int i = 0; i < n && g_pg.sel < 0; i++)
        if (rows[i].enabled) g_pg.sel = i;
    for (int i = 0; i < 64; i++) {
        g_pg.col[i][0] = 0.2f, g_pg.col[i][1] = 0.64f, g_pg.col[i][2] = 0.99f, g_pg.col[i][3] = 0.95f;
        g_pg.scale[i] = 1.0f;
    }
    g_pg.where = where;
    if (where == 2) MEM32(kPState) = kPPage;
}

static void page_open(int page) { page_open_at(page, 2); }

/* back out of a page: a sub-page to Settings, a page to the pause menu's row list */
static void page_back(void)
{
    if (g_pg.page == kPageButtons) {
        if (g_pg.map_mode == kMapAsk) {  /* the swap question's B: as Cancel */
            g_pg.map_mode = kMapRows, g_pg.sel = g_pg.map_game;
            return;
        }
        page_open_at(kPageControls, g_pg.where);
        g_pg.sel = 0;
        return;
    }
    if (g_pg.page == kPageGameplay || g_pg.page == kPageGraphics || g_pg.page == kPageControls) {
        int from = g_pg.page;
        page_open_at(kPageSettings, g_pg.where);
        g_pg.sel = from - kPageGameplay;
        return;
    }
    if (g_pg.where == 2) MEM32(kPState) = kPList;
    g_pg.where = 0;
}

/* leave the pause menu as its resume row does (0x3097B): state 1, the menu's two sounds stopped */
static void pause_resume(void)
{
    MEM32(kPState) = 1;
    uint32_t a[2] = {MEM32(0x32E800), 0};
    call_guest(sub_001684D0, 2, a);
    a[0] = MEM32(0x32E858);
    call_guest(sub_001684D0, 2, a);
    MEM32(0x338A94) = MEM32(0x1FAA0C);
}

/* the Button mapping page's answer from the pad (input.cpp): a free button is taken, one in use asks to swap */
static void map_wait(void)
{
    int button, r = app_PadCaptureResult(&button);
    if (!r) return;
    g_pg.map_mode = kMapRows;
    if (r < 0) {
        sound(kSndBack);
        return;
    }
    g_pg.map_other = -1;
    for (int j = 0; j < app_PadInputCount(); j++)
        if (j != g_pg.map_game && app_PadMapGet(j) == button) g_pg.map_other = j;
    if (g_pg.map_other < 0 || app_PadMapGet(g_pg.map_game) == button) {
        app_PadMapSet(g_pg.map_game, button, 0);
        sound(kSndConfirm);
        return;
    }
    g_pg.map_button = button, g_pg.map_mode = kMapAsk, g_pg.sel = 2, g_pg.top = 0;  /* Cancel lit */
    sound(kSndMove);
}

static void page_input(void)
{
    if (g_pg.page == kPageButtons && g_pg.map_mode == kMapWait) {  /* the game sees the pad at rest meanwhile */
        map_wait();
        return;
    }
    PageRow rows[64];
    int n = page_rows(g_pg.page, rows, 64);
    int down = pressed_down(), up = pressed_up(), a = pad(kConfirmA) != 0, b = pad(kBack) != 0;  /* B: the menus' back */
    int left = pad(kLeft) != 0 || pad(kStickX) == -1, right = pad(kRight) != 0 || pad(kStickX) == 1;
    if (b) {
        sound(kSndBack);
        page_back();
        return;
    }
    if ((down || up) && n) {
        int dir = down ? 1 : -1, c = g_pg.sel;
        for (int k = 0; k < n; k++) {
            c = (c + dir + n) % n;
            if (rows[c].enabled) break;
        }
        if (c != g_pg.sel && rows[c].enabled) {
            g_pg.sel = c;
            sound(kSndMove);
        }
    }
    if (g_pg.sel >= 0 && g_pg.sel < n) {
        if (g_pg.sel < g_pg.top) g_pg.top = g_pg.sel;
        if (g_pg.sel >= g_pg.top + PAGE_ROWS_SHOWN) g_pg.top = g_pg.sel - PAGE_ROWS_SHOWN + 1;
    }
    if (g_pg.page == kPageButtons) {
        if (!a || g_pg.sel < 0 || g_pg.sel >= n || !rows[g_pg.sel].enabled) return;
        if (g_pg.map_mode == kMapAsk) {
            if (g_pg.sel == 1) app_PadMapSet(g_pg.map_game, g_pg.map_button, 1);
            sound(g_pg.sel == 1 ? kSndConfirm : kSndBack);
            g_pg.map_mode = kMapRows, g_pg.sel = g_pg.map_game;
        } else if (g_pg.sel == app_PadInputCount()) {
            app_PadMapReset();
            sound(kSndConfirm);
        } else {
            g_pg.map_game = g_pg.sel, g_pg.map_mode = kMapWait;
            app_PadCaptureStart();
            sound(kSndConfirm);
        }
        return;
    }
    if (g_pg.page == kPageControls && g_pg.sel == 0) {
        if (!a) return;
        sound(kSndConfirm);
        page_open_at(kPageButtons, g_pg.where);
        return;
    }
    if (g_pg.page >= kPageGameplay && (left || right || a) && g_pg.sel >= 0) {
        app_PageOptionStep(g_pg.page - kPageGameplay, g_pg.page == kPageControls ? g_pg.sel - 1 : g_pg.sel,
                           left ? -1 : 1);
        sound(kSndMove);
        return;
    }
    if (!a || g_pg.sel < 0 || g_pg.sel >= n || !rows[g_pg.sel].enabled) return;
    sound(kSndConfirm);
    if (g_pg.page == kPageTravel) {
        if (hpps_ApTravelRequest) hpps_ApTravelRequest(g_pg.sel);
        g_pg.where = 0;
        pause_resume();
    } else if (g_pg.page == kPageSettings) {
        page_open_at(kPageGameplay + g_pg.sel, g_pg.where);
    }
}

/* a row's text (n bytes of s) in a guest buffer of its own, which the game's printf reads as its format */
static int page_text(uint32_t *va, const char *s, size_t n)
{
    if (!*va) *va = xbox_HeapAlloc(96, 16);
    if (!*va) return 0;
    char *dst = (char *)XBOX_PTR(*va);
    size_t k = 0;
    for (size_t j = 0; j < n && s[j] && k < 94; j++) {
        if (s[j] == '%') dst[k++] = '%';  /* a literal % for the printf */
        dst[k++] = s[j];
    }
    dst[k] = 0;
    return 1;
}

/* the page in the rows' place, drawn after the title's (its own rows and hint undrawn: g_page_draw) */
static void page_draw(void)
{
    PageRow rows[64];
    int n = page_rows(g_pg.page, rows, 64);
    float dt = bf(MEM32(kPDt));
    if (dt == 0.0f) dt = 1.0f / 30.0f;
    float top = PAGE_LIT_SCALE;
    uint32_t sh[2] = {4, 5};
    static const float title_col[4] = {1.0f, 0.86f, 0.45f, 1.0f};
    static uint32_t title_va[6];
    /* a still, dark backdrop: over the title it covers the menu's moving background; in the pause menu it darkens the
     * game behind */
    static const float backdrop[4] = {0.02f, 0.025f, 0.07f, 1.0f}, dim[4] = {0.0f, 0.0f, 0.02f, 0.82f};
    draw_rect(0.0f, 0.0f, 640.0f, 480.0f, g_pg.where == 1 ? backdrop : dim);
    set_align(1, 0);
    call_guest(sub_00168DF0, 2, sh);
    draw_text(title_col, 1.3f, 320.0f, PAGE_TITLE_Y, guest_text(page_title(g_pg.page), &title_va[g_pg.page]));
    /* the shown rows as one block, centred between the name and the hint */
    int shown = n < PAGE_ROWS_SHOWN ? n : PAGE_ROWS_SHOWN;
    float row0 = (PAGE_AREA_TOP + PAGE_AREA_END) * 0.5f - PAGE_STEP * (float)(shown - 1) * 0.5f;
    if (row0 < PAGE_AREA_TOP) row0 = PAGE_AREA_TOP;
    /* the colours and sizes ease first; a lit row's growth then pushes the rows below it down (the text hangs from
     * y), and the block moves up by half of it to stay centred */
    float grow = 0.0f;
    for (int r = 0; r < PAGE_ROWS_SHOWN && g_pg.top + r < n; r++) {
        int i = g_pg.top + r, lit = i == g_pg.sel;
        float d = (float)(i > g_pg.sel ? i - g_pg.sel : g_pg.sel - i);
        float target[4];
        if (d > 3.0f) d = 3.0f;  /* on the dark backdrop the far rows stay readable */
        target[0] = 0.2f + 0.5f - d * 0.08f, target[1] = 0.64f + 0.2f - d * 0.05f, target[2] = 1.0f,
        target[3] = 1.0f - d * 0.08f;
        if (!rows[i].enabled) target[0] = target[1] = target[2] = 0.45f, target[3] = 0.6f;
        float *col = i < 64 ? g_pg.col[i] : g_pg.col[63], *sc = i < 64 ? &g_pg.scale[i] : &g_pg.scale[63];
        for (int k = 0; k < 4; k++) col[k] = ease(col[k], lit ? 1.0f : target[k]);
        *sc += lit ? dt * 4.0f : -dt * 4.0f;
        if (*sc > top) *sc = top;
        if (*sc < 1.0f) *sc = 1.0f;
        grow += (*sc - 1.0f) * PAGE_TEXT_H;
    }
    float y = row0 - grow * 0.5f;
    for (int r = 0; r < PAGE_ROWS_SHOWN && g_pg.top + r < n; r++) {
        int i = g_pg.top + r;
        float *col = i < 64 ? g_pg.col[i] : g_pg.col[63], sc = i < 64 ? g_pg.scale[i] : g_pg.scale[63];
        /* "label:  value" as two columns about the centre, the label right-aligned and the value left-aligned, so the
         * values line up down the page; a row without a value stays centred */
        static uint32_t row_va[PAGE_ROWS_SHOWN], val_va[PAGE_ROWS_SHOWN];
        const char *sep = strstr(rows[i].text, ":  ");
        float x = rows[i].indent ? 335.0f : 320.0f;
        if (sep) {
            if (!page_text(&row_va[r], rows[i].text, (size_t)(sep + 1 - rows[i].text)) ||
                !page_text(&val_va[r], sep + 3, strlen(sep + 3)))
                continue;
            set_align(2, 0);
            call_guest(sub_00168DF0, 2, sh);
            draw_text(col, sc, x - PAGE_COL_GAP, y, row_va[r]);
            set_align(0, 0);
            call_guest(sub_00168DF0, 2, sh);
            draw_text(col, sc, x + PAGE_COL_GAP, y, val_va[r]);
        } else {
            if (!page_text(&row_va[r], rows[i].text, strlen(rows[i].text))) continue;
            set_align(1, 0);  /* a centred column */
            call_guest(sub_00168DF0, 2, sh);
            draw_text(col, sc, x, y, row_va[r]);
        }
        y += PAGE_STEP + (sc - 1.0f) * PAGE_TEXT_H;
    }
    if (n > PAGE_ROWS_SHOWN) {  /* more above / below */
        static uint32_t up_va, down_va;
        static const float more[4] = {0.6f, 0.75f, 0.95f, 0.9f};
        set_align(1, 0);
        if (g_pg.top > 0) draw_text(more, 0.9f, 320.0f, row0 - grow * 0.5f - 20.0f, guest_text("...", &up_va));
        if (g_pg.top + PAGE_ROWS_SHOWN < n)
            draw_text(more, 0.9f, 320.0f, row0 + grow * 0.5f + PAGE_STEP * (float)(shown - 1) + 26.0f,
                      guest_text("...", &down_va));
    }
    if (g_pg.page == kPageButtons && g_pg.map_mode == kMapWait) {
        /* waiting for a pad button the question takes the hint's place (A does not select meanwhile), clear of the
         * rows and their "..." */
        static uint32_t wait_va;
        static const float wait_col[4] = {1.0f, 0.86f, 0.45f, 1.0f};
        set_align(1, 1);
        draw_text(wait_col, 0.9f, 320.0f, 430.0f,
                  guest_text("Press a button for it. Hold B or press Esc to cancel", &wait_va));
    } else {
        /* the hint the pause menu's rows have: the A button and "select" */
        static const float hint[4] = {0.183f, 0.596f, 0.85f, 1.0f};
        set_align(0, 1);
        uint32_t c[4] = {fb(hint[0]), fb(hint[1]), fb(hint[2]), fb(hint[3])};
        call_guest(sub_00168D90, 4, c);
        uint32_t sc1[1] = {fb(1.0f)};
        call_guest(sub_00168E60, 1, sc1);
        g_pause_hint = 1;
        call_guest(sub_00030510_gen, 0, NULL);
        g_pause_hint = 0;
        uint32_t pos[2] = {fb(320.0f), fb(430.0f)};
        call_guest(sub_00168D70_gen, 2, pos);
        uint32_t t[1] = {game_text(kKeySelect)};
        if (t[0]) call_guest(sub_00169A40_gen, 1, t);
    }
    /* the port everything here stands on */
    static uint32_t thanks_va;
    static const float thanks_col[4] = {0.55f, 0.65f, 0.8f, 0.85f};
    set_align(1, 0);
    draw_text(thanks_col, 0.6f, 320.0f, 458.0f,
              guest_text("Huge thanks to MiddleDucky, whose wonderful port makes all of this possible", &thanks_va));
}

/* the pause menu's input step */
void sub_000307F0(void)
{
    if (!enabled()) {
        sub_000307F0_gen();
        return;
    }
    uint32_t state = MEM32(kPState);
    /* the travel shortcut while playing: the game's own Start opens the pause menu, then the page */
    if (g_pg.open_travel && state == 0) {
        uint32_t tbl = MEM32(MEM32(kPadState) + 0x10);
        MEM16(tbl + MEM32(kStartBtn) * 24 + 0x10) = 1;
        sub_000307F0_gen();
        if (MEM32(kPState) != 0) {
            g_pg.open_travel = 0;
            page_open(kPageTravel);
        }
        return;
    }
    if (g_pg.open_travel && state != kPPage) {  /* the pause menu the combo's Start opened */
        g_pg.open_travel = 0;
        page_open(kPageTravel);
        g_esp += 4;  /* the step's ret */
        return;
    }
    if (state == kPPage) {
        page_input();
        g_esp += 4;  /* the step's ret */
        return;
    }
    int kind[3], nx = extras(kind), last = kPRows + nx - 1;
    int row = (int32_t)MEM32(kPRow);
    if (state != kPSure) g_pause_ask = 0;
    int confirm = pad(kConfirmA) != 0, down = pressed_down(), up = pressed_up();
    if (state == kPSure && g_pause_ask && confirm && MEM32(kPYes) == 1) {
        uint32_t a[1] = {1};
        call_guest(sub_000306C0, 1, a);  /* the box's own click */
        quit_to_desktop();
    }
    sub_000307F0_gen();
    if (state != kPList || MEM32(kPState) != kPList) return;
    int after = (int32_t)MEM32(kPRow);
    if (row >= kPRows) {
        if (confirm) {
            int k = kind[row - kPRows < nx ? row - kPRows : nx - 1];
            if (k == kXQuit) {
                /* as GUI_QUIT opens the box (0x30BA7), No lit; the step has clicked */
                MEM32(kPYes) = 0;
                MEM32(kPState) = kPSure;
                g_pause_ask = 1;
            } else {
                page_open(k == kXTravel ? kPageTravel : kPageSettings);
            }
            MEM32(kPRow) = (uint32_t)row;
        } else if (after > last) {
            MEM32(kPRow) = 0;  /* down from the last wraps to the first row */
        }
    } else if (!confirm && ((up && row == 0 && after == kPRows - 1) || (down && !up && row == kPRows - 1 && after == 0))) {
        /* the step wrapped past the title's last or first row: the port's rows come in between */
        MEM32(kPRow) = (uint32_t)(up && row == 0 ? last : kPRows);
    }
}

/* the pause menu's drawing: the rows closer together while it runs; on a page, the panel without the title's rows */
void sub_00030F00(void)
{
    if (!enabled()) {
        sub_00030F00_gen();
        return;
    }
    uint32_t step = MEM32(kPStep);
    MEM32(kPStep) = fb(pause_step());
    g_pause_draw = 1;
    if (MEM32(kPState) == kPPage) {
        MEM32(kPState) = kPList;
        g_page_draw = 1;
        sub_00030F00_gen();
        g_page_draw = 0;
        MEM32(kPState) = kPPage;
        page_draw();
    } else {
        sub_00030F00_gen();
    }
    g_pause_draw = 0;
    MEM32(kPStep) = step;
}

/* the port's rows under GUI_QUIT, drawn as the pause menu's loop draws its rows (0x310F7), and the hint when one is
 * lit */
static void pause_row(void)
{
    int kind[3], nx = extras(kind);
    int sel = (int32_t)MEM32(kPRow);
    float dt = bf(MEM32(kPDt));
    if (dt == 0.0f) dt = 1.0f / 30.0f;
    float top = call_guest(sub_0002A7C0, 0, NULL) == 11 ? 1.2f : 1.4f, step = pause_step();
    int any_lit = 0;
    for (int x = 0; x < nx; x++) {
        int row = kPRows + x, lit = sel == row;
        float d = (float)(sel > row ? sel - row : row - sel);
        float target[4] = {0.2f - d / 60.0f, 0.64f - d * 0.05f, 0.99f - d * 0.04f, 0.95f - d * 0.05f};
        for (int k = 0; k < 4; k++) {
            float t = lit ? 1.0f : target[k];
            if (g_pcol[x][k] < t) g_pcol[x][k] = g_pcol[x][k] + 0.04f > t ? t : g_pcol[x][k] + 0.04f;
            else if (g_pcol[x][k] > t) g_pcol[x][k] = g_pcol[x][k] - 0.04f < t ? t : g_pcol[x][k] - 0.04f;
        }
        g_pscale[x] += lit ? dt * 4.0f : -dt * 4.0f;
        if (g_pscale[x] > top) g_pscale[x] = top;
        if (g_pscale[x] < 1.0f) g_pscale[x] = 1.0f;
        uint32_t text = extra_text(kind[x]);
        if (!text) continue;
        set_align(0, 1);
        uint32_t sh[2] = {4, 5};
        call_guest(sub_00168DF0, 2, sh);
        draw_text(g_pcol[x], g_pscale[x], g_last_x, 90.0f + step * row, text);
        any_lit |= lit;
    }
    if (any_lit) {
        /* the hint the loop's end gives rows 0..5 and 9 (0x319FC) */
        static const float hint[4] = {0.183f, 0.596f, 0.85f, 1.0f};
        set_align(0, 1);
        uint32_t c[4] = {fb(hint[0]), fb(hint[1]), fb(hint[2]), fb(hint[3])};
        call_guest(sub_00168D90, 4, c);
        uint32_t sc[1] = {fb(1.0f)};
        call_guest(sub_00168E60, 1, sc);
        call_guest(sub_00030510, 0, NULL);
        uint32_t pos[2] = {fb(320.0f), fb(430.0f)};
        call_guest(sub_00168D70_gen, 2, pos);
        uint32_t t[1] = {game_text(kKeySelect)};
        if (t[0]) call_guest(sub_00169A40_gen, 1, t);
    }
}

/* hle_ap.c's travel shortcut (both triggers + Back): the pause menu opens on the Travel page */
void hpps_FrontendOpenTravel(void) { g_pg.open_travel = 1; }
