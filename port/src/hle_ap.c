/**
 * The game side of the Archipelago bridge (src/ap/bridge.cpp): what in the title counts as a check, and giving the
 * items the multiworld sends. Everything here runs on the title's own thread: hpps_ApScript from hle_debug.c's
 * hpps_ScriptCommand (every level-script command, just before its handler), hpps_ApFrame from Harry's update
 * (hle_trainer.c). Without RECOMP_AP_SERVER / RECOMP_AP_SLOT the bridge is off and nothing here acts.
 *
 * Checks are keyed as port/ap/catalog.json (make_catalog.py) names them, from the same data: a script command's key is
 * built from the instruction's own argument strings in the level's script, as the census read them, not from their
 * values at run time. The interpreter sub_0014DD30(ctx, ...) keeps the script context as its first argument (at the
 * hook, argv + 0x4C8: its frame is 0x4D0 bytes and four pushes); ctx +0x10 the script record (+0 name, +6 instruction
 * count, +0x28 code), +0x18 the instruction being run (8 bytes: u16 command, u16 argument count, u32 argument string
 * pointers).
 *
 * Counters, once a frame: wizard cards owned (the 101 counts at 0x329BD4) and beans collected (rises of 0x329B8A).
 *
 * Items: a card through sub_0003E9A0(card) (the title's own: the inventory slot, a health bar each 20th); beans into
 * 0x329B8A up to the bag (0x329B8C); a health bar sub_00152160(); the big bag 150; quest items and spells through the
 * title's give-item sub_00041D20(item) (spells are inventory items of their number), a spell also onto the first free
 * button (sub_0014FEB0(slot, spell), then sub_000342E0() for the abilities). Passes are kept by the bridge (the travel menu).
 * Not yet: placed pickups and enemies as checks, the vanilla rewards of checks left in place.
 */

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define RECOMP_GENERATED_CODE
#include "recomp/gen/recomp_types.h"
#include "ap/bridge.h"

extern void sub_0003E9A0(void);
extern void sub_00152160(void);
extern void sub_0014FEB0(void);
extern void sub_00041D20(void);
extern void sub_000342E0(void);

#define LEVEL_NOW    0x356CC4u
#define CMD_TABLE    0x356F90u
#define CARDS        0x329BD4u
#define BEANS        0x329B8Au
#define BEAN_BAG     0x329B8Cu
#define SPELL_SLOTS  0x329DD5u
#define CTX_FROM_ARGV 0x4C8u

static int guest_ptr(uint32_t p) { return (p >= 0x00011000u && p < 0x04000000u) || (p >= 0x80000000u && p < 0x84000000u); }

static const char *guest_str(uint32_t p)
{
    if (!guest_ptr(p)) return "";
    const char *c = (const char *)XBOX_PTR(p);
    int n = 0;
    while (n < 96 && c[n] >= 0x20 && c[n] < 0x7F) n++;
    return n < 96 && c[n] == 0 ? c : "";
}

/* an argument string as the census and make_catalog.py keep it: spaces and quotes off both ends */
static void clean_arg(const char *s, char *out, size_t size)
{
    while (*s == ' ' || *s == '\t' || *s == '"') s++;
    size_t n = strlen(s);
    while (n && (s[n - 1] == ' ' || s[n - 1] == '\t' || s[n - 1] == '"')) n--;
    if (n >= size) n = size - 1;
    memcpy(out, s, n);
    out[n] = 0;
}

static int scene_script(const char *name)
{
    static const char *const kWords[] = {"secret", "peeve", "password", "fatlady", "fat_lady"};
    char low[96];
    size_t i;
    for (i = 0; name[i] && i < sizeof low - 1; i++) low[i] = (char)(name[i] >= 'A' && name[i] <= 'Z' ? name[i] + 32 : name[i]);
    low[i] = 0;
    for (size_t k = 0; k < sizeof kWords / sizeof kWords[0]; k++)
        if (strstr(low, kWords[k])) return 1;
    return 0;
}

void hpps_ApScript(uint32_t entry, uint32_t argv)
{
    static int told;
    if (!hpps_ap_enabled()) return;
    uint32_t ctx = MEM32(argv + CTX_FROM_ARGV);
    uint32_t rec = guest_ptr(ctx) ? MEM32(ctx + 0x10) : 0;
    uint32_t pc = guest_ptr(ctx) ? MEM32(ctx + 0x18) : 0;
    if (!guest_ptr(rec) || pc >= MEM16(rec + 6) || !*guest_str(MEM32(rec))) {
        if (!told++) fprintf(stderr, "[AP] script context not where expected (ctx %08X): script checks off\n", ctx);
        return;
    }
    const char *cmd = (const char *)XBOX_PTR(entry);
    const char *script = guest_str(MEM32(rec));
    uint32_t ins = MEM32(rec + 0x28) + 8 * pc;
    uint32_t argc = MEM16(ins + 2), args = MEM32(ins + 4);
    uint32_t level = MEM32(LEVEL_NOW);
    char a[8][96], joined[600], key[800];
    joined[0] = 0;
    for (uint32_t i = 0; i < argc && i < 8; i++) {
        clean_arg(guest_str(MEM32(args + 4 * i)), a[i], sizeof a[i]);
        if (i) strncat(joined, "|", sizeof joined - strlen(joined) - 1);
        strncat(joined, a[i], sizeof joined - strlen(joined) - 1);
    }
    if (!strcmp(cmd, "AddTask") && argc) {
        snprintf(key, sizeof key, "task:%s:added", a[0]);
        hpps_ap_location(key);
    } else if (!strcmp(cmd, "SetTaskComplete") && argc) {
        snprintf(key, sizeof key, "task:%s:completed", a[0]);
        hpps_ap_location(key);
    } else if (!strcmp(cmd, "AwardSpellChallengePoints") || !strcmp(cmd, "CollectInventoryItem") ||
               !strcmp(cmd, "CollectWizardCard") || !strcmp(cmd, "CollectWizardCardPack")) {
        snprintf(key, sizeof key, "script:%u:%s:%s:%s", level, script, cmd, joined);
        hpps_ap_location(key);
    } else if (!strcmp(cmd, "SetQuidLevel") || !strcmp(cmd, "FlyStartHoops") || !strcmp(cmd, "SnitchChaseStart") ||
               !strcmp(cmd, "FlyQuizHoops") || !strcmp(cmd, "SetQuidHoopCount")) {
        snprintf(key, sizeof key, "scene:%u:%s", level, script);
        hpps_ap_location(key);
    }
    if (scene_script(script)) {
        snprintf(key, sizeof key, "scene:%u:%s", level, script);
        hpps_ap_location(key);
    }
    /* the goal: Quirrell defeated (level 29's VoldemortDeadScript, which goes on to day 6) */
    if (level == 29 && !strcmp(script, "VoldemortDeadScript")) hpps_ap_goal();
}

/* runs a recompiled function on this guest thread, the arguments pushed right to left and a return address */
static void call_guest(void (*fn)(void), int n, const uint32_t *args)
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
}

static int give(const char *kind, int value)
{
    if (!strcmp(kind, "card") && value >= 1 && value <= 101) {
        uint32_t arg = (uint32_t)value;
        call_guest(sub_0003E9A0, 1, &arg);
    } else if (!strcmp(kind, "beans") || !strcmp(kind, "sweet")) {
        int add = !strcmp(kind, "beans") ? 5 : 3, bag = (int16_t)MEM16(BEAN_BAG), now = (int16_t)MEM16(BEANS);
        MEM16(BEANS) = (uint16_t)(now + add > bag ? bag : now + add);
    } else if (!strcmp(kind, "health")) {
        call_guest(sub_00152160, 0, NULL);
    } else if (!strcmp(kind, "bag")) {
        MEM16(BEAN_BAG) = 150;
    } else if (!strcmp(kind, "frog")) {
        MEMF(0x329B90) = MEMF(0x329B94);  /* a chocolate frog: health filled */
    } else if (!strcmp(kind, "quest") && value > 0) {
        uint32_t arg = (uint32_t)value;
        call_guest(sub_00041D20, 1, &arg);  /* the title's give-item: the inventory slot the scripts test */
    } else if (!strcmp(kind, "spell") && value > 0) {
        /* a spell is an inventory item of its number (the scripts test QuizInventory(10) for Incendio) */
        uint32_t arg = (uint32_t)value;
        call_guest(sub_00041D20, 1, &arg);
        for (uint32_t s = 0; s < 3; s++)
            if (MEM8(SPELL_SLOTS + s) == (uint8_t)value) return 1;
        for (uint32_t s = 0; s < 3; s++)
            if (MEM8(SPELL_SLOTS + s) == 0) {
                uint32_t args2[2] = {s, (uint32_t)value};
                call_guest(sub_0014FEB0, 2, args2);
                call_guest(sub_000342E0, 0, NULL);
                return 1;
            }
        fprintf(stderr, "[AP] spell %d: no free button (kept as owned)\n", value);
    }
    return 1;  /* passes: the bridge keeps them */
}

void hpps_ApFrame(void)
{
    if (!hpps_ap_enabled()) return;
    hpps_ap_frame();
    if (!hpps_ap_active()) return;
    const char *kind;
    int value;
    for (int n = 0; n < 4 && hpps_ap_next_item(&kind, &value); n++) {
        give(kind, value);
        hpps_ap_item_given();
    }
    /* counters */
    static int cards_seen, beans_last = -1, beans_total;
    int owned = 0;
    for (uint32_t c = 0; c < 101; c++) owned += MEM16(CARDS + 2 * c) != 0;
    char key[32];
    while (cards_seen < owned) {
        snprintf(key, sizeof key, "cards:%d", ++cards_seen);
        hpps_ap_location(key);
    }
    int goal = hpps_ap_cards_goal();
    if (goal && owned >= goal) hpps_ap_goal();
    int beans = (int16_t)MEM16(BEANS);
    if (beans_last >= 0 && beans > beans_last) {
        int before = beans_total;
        beans_total += beans - beans_last;
        for (int n = (before / 10 + 1) * 10; n <= beans_total; n += 10) {
            snprintf(key, sizeof key, "beans:%d", n);
            hpps_ap_location(key);
        }
    }
    beans_last = beans;
}
