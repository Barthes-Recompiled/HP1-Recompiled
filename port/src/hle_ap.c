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

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define RECOMP_GENERATED_CODE
#include "recomp/gen/recomp_types.h"
#include "ap/bridge.h"
#include "input/xinput_xbox.h"

static void travel(void);

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
    /* a character spoken to (<Name>Talk) and a pursuit's stage, as make_catalog.py's TALK_RX / PURSUIT_RX */
    size_t n = strlen(name);
    if (n > 4 && !strcmp(name + n - 4, "Talk")) return 1;
    return strstr(low, "pursuit") || strstr(low, "throughdoor") || strstr(low, "gotohagrid") || strstr(low, "makeronrun");
}

/* enemies the level's scripts set up (name -> type), for their defeat's key */
static struct { uint32_t level; char name[64], type[40]; } g_enemy[256];
static int g_nenemy;

static void enemy_remember(uint32_t level, const char *name, const char *type)
{
    for (int i = 0; i < g_nenemy; i++)
        if (g_enemy[i].level == level && !_stricmp(g_enemy[i].name, name)) return;
    if (g_nenemy == 256) g_nenemy = 0;
    g_enemy[g_nenemy].level = level;
    snprintf(g_enemy[g_nenemy].name, sizeof g_enemy[0].name, "%s", name);
    snprintf(g_enemy[g_nenemy].type, sizeof g_enemy[0].type, "%s", type);
    g_nenemy++;
}

/* hle_trainer.c: a watched enemy's CurrentHealth reached 0 */
void hpps_ApEnemyDown(uint32_t ent)
{
    if (!hpps_ap_enabled()) return;
    const char *name = guest_str(MEM32(ent + 0x0C));
    uint32_t level = MEM32(LEVEL_NOW);
    char key[200];
    for (int i = 0; i < g_nenemy; i++)
        if (g_enemy[i].level == level && !_stricmp(g_enemy[i].name, name)) {
            snprintf(key, sizeof key, "enemy:%u:%s:%s", level, g_enemy[i].type, g_enemy[i].name);
            hpps_ap_location(key);
            snprintf(key, sizeof key, "enemytype:%s", g_enemy[i].type);
            hpps_ap_location(key);
            return;
        }
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
    /* an enemy set up (SetAI<type>(name, ...)): its key, for when it is defeated (hpps_ApEnemyDown) */
    if (!strncmp(cmd, "SetAI", 5) && argc && cmd[5]) {
        snprintf(key, sizeof key, "enemy:%u:%s:%s", level, cmd + 5, a[0]);
        if (hpps_ap_known(key)) enemy_remember(level, a[0], cmd + 5);
    }
    /* the goal: Quirrell defeated (level 29's VoldemortDeadScript, which goes on to day 6) */
    if (level == 29 && !strcmp(script, "VoldemortDeadScript")) hpps_ap_goal();
}

/* every cutscene the title starts by name passes sub_000CEC60(name): the StartCutScene command (sub_0004BC00) and the
 * level transitions (sub_000D0C10 / sub_000D0D40), which play some cutscenes (the Forbidden Forest's among them) that
 * no level script starts. It looks the name up in the table at [0x218888] and keeps the index at [0x33FEF4]. A check
 * keyed cutscene:<name> (lower case), so the Forest counts when it plays. Found by DeepSeek reading the generated code. */
extern void sub_000CEC60_gen(void);
void sub_000CEC60(void)
{
    if (hpps_ap_enabled()) {
        char name[64], key[96];
        snprintf(name, sizeof name, "%s", guest_str(MEM32(g_esp + 4)));
        for (char *c = name; *c; c++) *c = (char)tolower((unsigned char)*c);
        snprintf(key, sizeof key, "cutscene:%s", name);
        fprintf(stderr, "[AP] cutscene %s (level %u)%s\n", name, MEM32(LEVEL_NOW), hpps_ap_known(key) ? ": a check" : "");
        hpps_ap_location(key);
    }
    sub_000CEC60_gen();
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

/* the bridge's own gives (hpps_ApFrame) pass the reward swap below */
static int g_giving;

/* a search's result: sub_000942E0 (Harry's search of a jar, bookcase or chest) gives it with the searched object at
 * [esi + 0x25D0] (its +0x0C name, "group.WizF2_Jar3"); the key drops the group and is lower case, as the catalog */
#define SEARCH_FIRST 0x000942E0u
#define SEARCH_END   0x00095370u
static void search_check(uint32_t ret)
{
    if (ret < SEARCH_FIRST || ret >= SEARCH_END || !guest_ptr(g_esi)) return;
    uint32_t spot = MEM32(g_esi + 0x25D0);
    const char *name = guest_ptr(spot) ? guest_str(MEM32(spot + 0x0C)) : "";
    const char *dot = strchr(name, '.');
    char low[96], key[160];
    size_t i;
    if (dot) name = dot + 1;
    for (i = 0; name[i] && i < sizeof low - 1; i++) low[i] = (char)(name[i] >= 'A' && name[i] <= 'Z' ? name[i] + 32 : name[i]);
    low[i] = 0;
    if (!low[0]) return;
    snprintf(key, sizeof key, "search:%s", low);
    hpps_ap_location(key);
    snprintf(key, sizeof key, "search:%u:%s", MEM32(LEVEL_NOW), low);
    hpps_ap_location(key);
}

/* while a slot is bound the multiworld hands out cards, spells and quest items: the title's own gives of them (a
 * check's vanilla reward) are left out, the check made; beans and the rest stay the title's */
static int swapped(uint32_t item)
{
    static const uint8_t kOwn[] = {1, 2, 5, 8, 10, 12, 20, 21, 38, 39, 40, 41, 42, 43};  /* spells, quest items */
    if (g_giving || !hpps_ap_active()) return 0;
    if (item >= 101 && item <= 201) return 1;
    for (size_t i = 0; i < sizeof kOwn; i++)
        if (item == kOwn[i]) return 1;
    return 0;
}

/* sub_0003E9A0(card 1..101), cdecl: one wizard card */
extern void sub_0003E9A0_gen(void);
void sub_0003E9A0(void)
{
    uint32_t ret = MEM32(g_esp), card = MEM32(g_esp + 4);
    if (hpps_ap_enabled()) search_check(ret);
    if (hpps_ap_enabled() && swapped(card + 100)) {
        fprintf(stderr, "[AP] wizard card %u from the game left out (the multiworld's instead)\n", card);
        g_esp += 4;
        return;
    }
    sub_0003E9A0_gen();
}

/* sub_00041D20(item), cdecl: an inventory item (101..201 the cards, through sub_0003E9A0) */
extern void sub_00041D20_gen(void);
void sub_00041D20(void)
{
    uint32_t ret = MEM32(g_esp), item = MEM32(g_esp + 4) & 0xFFFF;
    if (hpps_ap_enabled()) search_check(ret);
    if (hpps_ap_enabled() && !(item >= 101 && item <= 201) && swapped(item)) {
        fprintf(stderr, "[AP] item %u from the game left out (the multiworld's instead)\n", item);
        g_eax = 0;
        g_esp += 4;
        return;
    }
    sub_00041D20_gen();
}

/* placed pickups (beans, cards, frogs, sweets in the catalog as object:<level>:<name>): the entities near Harry (the
 * ring through +0 from the player's entity) are followed; one that leaves the ring, or stops being an entity, while
 * Harry was close to it is taken as picked up */
#define PLAYER       0x3565B0u
#define PICKUP_NEAR  300.0f
static struct { uint32_t ent; float dist; int seen; char key[96]; } g_pick[128];
static int g_npick;

static int entity_pos(uint32_t e, float p[3])
{
    uint32_t obj = MEM32(e + 0x28);
    if (!guest_ptr(obj)) return 0;
    uint32_t m = obj + 0x40 + 0x40 * (MEM32(obj + 0x2C) & 1);
    p[0] = MEMF(m + 0x30), p[1] = MEMF(m + 0x34), p[2] = MEMF(m + 0x38);
    return 1;
}

static void pickups(void)
{
    static uint32_t last_level;
    uint32_t harry = MEM32(PLAYER), level = MEM32(LEVEL_NOW);
    float hp[3];
    if (level != last_level) {  /* a new level: everything leaves the ring at once, nothing was picked up */
        last_level = level;
        g_npick = 0;
    }
    if (!guest_ptr(harry) || !entity_pos(harry, hp)) return;
    for (int i = 0; i < g_npick; i++) g_pick[i].seen = 0;
    uint32_t e = MEM32(harry);
    for (int n = 0; n < 4096 && guest_ptr(e) && e != harry; n++, e = MEM32(e)) {
        if (MEM32(e + 8) != e) break;
        const char *name = guest_str(MEM32(e + 0x0C));
        if (!*name || strchr(name, '.')) continue;
        int k;
        for (k = 0; k < g_npick && g_pick[k].ent != e; k++) {}
        if (k == g_npick) {
            char key[96];
            snprintf(key, sizeof key, "object:%u:%s", level, name);
            if (!hpps_ap_known(key) || g_npick == 128) continue;
            g_pick[k].ent = e;
            snprintf(g_pick[k].key, sizeof g_pick[k].key, "%s", key);
            g_npick++;
        }
        float p[3];
        if (entity_pos(e, p)) {
            float dx = p[0] - hp[0], dy = p[1] - hp[1], dz = p[2] - hp[2];
            g_pick[k].dist = sqrtf(dx * dx + dy * dy + dz * dz);
        }
        g_pick[k].seen = 1;
    }
    int n = 0;
    for (int i = 0; i < g_npick; i++) {
        if (!g_pick[i].seen) {
            if (g_pick[i].dist < PICKUP_NEAR) hpps_ap_location(g_pick[i].key);
            continue;  /* gone: dropped from the list */
        }
        g_pick[n++] = g_pick[i];
    }
    g_npick = n;
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
    travel();  /* before the connection check: the entries are gated by the passes held */
    if (!hpps_ap_active()) return;
    const char *kind;
    int value;
    for (int n = 0; n < 4 && hpps_ap_next_item(&kind, &value); n++) {
        g_giving = 1;
        give(kind, value);
        g_giving = 0;
        hpps_ap_item_given();
    }
    pickups();
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

/* travel (the pause menu's Travel page, hle_frontend.c; the F10 > Debug window, scda/app/menu.cpp): opened with both
 * triggers held and Back pressed; the choice is carried out here,
 * on the title's thread, by the trainer's warp (through the hub) and, for a set piece inside a hub, its entry script */
extern void hpps_FrontendOpenTravel(void);  /* hle_frontend.c: the pause menu's Travel page */
extern void hpps_TrainerTravel(uint32_t level, const char *script);
static volatile LONG g_travel_pick = -1;

void hpps_ApTravelRequest(int entry) { InterlockedExchange(&g_travel_pick, entry); }

void hpps_ApPad(XBOX_GAMEPAD *g)
{
    static int back_was;
    static DWORD fired;
    if (!hpps_ap_enabled()) return;
    int back = (g->wButtons & 0x0020) != 0, triggers = g->bAnalogButtons[6] > 128 && g->bAnalogButtons[7] > 128;
    if (back && !back_was && triggers) {
        hpps_FrontendOpenTravel();
        fired = GetTickCount();
        fprintf(stderr, "[AP] travel window (triggers %u/%u, Back)\n", g->bAnalogButtons[6], g->bAnalogButtons[7]);
    }
    back_was = back;
    /* the combo's Back is not the Remembrall's: kept from the game while the triggers are held and for half a second
     * after the combo (a Back let go after the triggers) */
    if (g->bAnalogButtons[6] > 64 || g->bAnalogButtons[7] > 64 || GetTickCount() - fired < 500)
        g->wButtons &= (uint16_t)~0x0020;
}

static void travel(void)
{
    LONG pick = InterlockedExchange(&g_travel_pick, -1);
    if (pick < 0) return;
    const char *name, *script;
    int level, dungeon, unlocked;
    if (!hpps_ap_travel_entry((int)pick, &name, &level, &script, &dungeon, &unlocked) || !unlocked) return;
    fprintf(stderr, "[AP] travel: %s (level %d)\n", name, level);
    hpps_TrainerTravel((uint32_t)level, script);
}
