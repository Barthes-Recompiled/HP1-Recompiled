"""Build port/ap/catalog.json, the Archipelago ID source shared by hpps.exe's bridge and the apworld, from a census of
the game (Harry Potter Xbox Decomps/tools: psscripts.py and pscensus.py dumps of each level, made from the user's own
disc; they stay outside the repository). The catalog keeps only IDs, display names and the keys the bridge matches.

    python make_catalog.py <census dir> [catalog.json]

Location keys (what the bridge sees):
  task:<id>:added / task:<id>:completed      AddTask / SetTaskComplete(<id>) (hpps_ScriptCommand)
  script:<level>:<script>:<command>:<args>    that command run by that script (challenge tiers, gifts, card collects)
  trade:<level>:<trader>:<card>               a card trader's offer taken (the trade gives item <card> + 100)
  enemy:<level>:<type>:<name>                 that enemy defeated; enemytype:<type> the first of a type
  scene:<level>:<script>                      that script run (secrets, Peeves, passwords, Quidditch/hoop stages)
  object:<level>:<name>                       the placed pickup or chest's item given (sub_00041D20 from it)
  spell:<n>:first_cast, beans:<n>, cards:<n>  firsts and counters
IDs are given in a fixed order (by key, sorted), so rebuilding from the same census gives the same catalog; the version
string carries a hash of the keys and the bridge refuses a slot built from another catalog."""
import collections
import hashlib
import json
import os
import re
import sys

GAME = "Harry Potter and the Philosopher's Stone Recomp"
LOCATION_BASE = 74100000
ITEM_BASE = 74110000

LEVELS = {6: "Day 0", 7: "Night 0", 8: "Day 1", 9: "Lumos Lesson", 10: "Night 1", 11: "Day 2", 12: "Spongify Lesson",
          13: "Night 2", 15: "Day 3", 16: "Diffindo Lesson", 17: "Night 3", 19: "Day 4", 20: "Incendio Lesson",
          21: "Night 4", 23: "Day 5", 25: "Night 5", 26: "Devil's Snare", 27: "Flying Keys", 28: "Wizard Chess",
          29: "Quirrell", 31: "Day 6"}
HUBS = [6, 7, 8, 10, 11, 13, 15, 17, 19, 21, 23, 25, 31]
# set pieces: their own level, or a script inside a hub (level, entry script)
DUNGEONS = collections.OrderedDict([
    ("Lumos Lesson", (9, None)), ("Spongify Lesson", (12, None)), ("Diffindo Lesson", (16, None)),
    ("Incendio Lesson", (20, None)), ("Troll", (13, "TrollIntro")), ("Library", (17, "LibraryStealthSection")),
    ("Ingredients", (23, "IngredientsCheck")), ("Quidditch", (23, "HPE53Script")),
    ("Norbert", (21, "GoToHagrid01")),  # night 4's Hagrid pursuit (the Norbert night's playable part)
    ("Devil's Snare", (26, None)), ("Flying Keys", (27, None)),
    ("Wizard Chess", (28, None)), ("Quirrell", (29, None)),
])
# No Forbidden Forest pass: night4ingredients (22) is not on the Xbox disc and no script mentions the forest (the user's
# decision, 2026-10-30)
DROPPED_DUNGEONS = set()
# quest items (inventory items the scripts test with QuizInventory): progression for the set pieces that need them
QUEST_ITEMS = {"Flobberworm Mucus": 41, "Moondew": 42, "Puffapod Spores": 43, "Potion Vial": 21, "Wiggentree Bark": 38,
               "Dittany": 39, "Asphodel": 40, "Owl Treat": 20}
# spells are inventory items too (QuizInventory(10) = has Incendio): the number is the item given
SPELLS = {"Flipendo": 1, "Lumos": 2, "Diffindo": 5, "Spongify": 8, "Incendio": 10, "Alohomora": 12}
ENEMIES = ("Troll", "TrollBathRoom", "Gytrash", "GytrashBoss", "Horklump", "VenomousTentacular", "DevilSnare",
           "Filch", "FlyingBook", "Doxy", "Ghost", "Percy", "Imp", "Bowtruckle", "ProtrudingRoot", "StaticShooter",
           "RotDev")
SCENE_RX = re.compile(r"(?i)secret|peeve|password|fatlady|fat_lady")
QUID_CMDS = {"SetQuidLevel", "FlyStartHoops", "SnitchChaseStart", "FlyQuizHoops", "SetQuidHoopCount"}
PICKUPS = [("Bean", r"(?i)^(big_)?bean(?!bag)"), ("Chest", r"(?i)chest"), ("Wizard Card", r"(?i)card"),
           ("Chocolate Frog", r"(?i)chocfrog|frog"), ("Sweet", r"(?i)cauldron|pasty|cake|liquorice"),
           ("Gnome", r"(?i)gnome"), ("Secret", r"(?i)secret|search"), ("Item", r"(?i)owltreat|dungbomb|stinkpel")]
ORDER = [6, 7, 8, 9, 10, 11, 12, 13, 15, 16, 17, 19, 20, 21, 23, 25, 26, 27, 28, 29]


def clean(s):
    return s.strip().strip('"')


def region_of(level):
    return LEVELS.get(level, "Level %d" % level)


def main():
    census = sys.argv[1]
    out = sys.argv[2] if len(sys.argv) > 2 else os.path.join(os.path.dirname(__file__), "catalog.json")
    rows = []
    for level in ORDER:
        p = os.path.join(census, "l%d.tsv" % level)
        if os.path.exists(p):
            for ln in open(p, encoding="utf-8"):
                f = ln.rstrip("\n").split("\t")
                if len(f) >= 4:
                    rows.append((int(f[0]), f[1], int(f[2]), f[3], f[4] if len(f) > 4 else ""))
    locs = {}  # key -> (name, region, group)

    def add(key, name, region, group):
        if key not in locs:
            locs[key] = (name, region, group)

    tasks = {}
    for lv, script, _, cmd, args in rows:
        a = [clean(x) for x in args.split("|")] if args else []
        reg = region_of(lv)
        if cmd == "AddTask" and len(a) > 1:
            tasks.setdefault(a[0], (a[1], reg))
        elif cmd == "AwardSpellChallengePoints":
            add("script:%d:%s:%s:%s" % (lv, script, cmd, "|".join(a)),
                "%s - %s (%s)" % (reg, script, "/".join(a)), reg, "Challenge Points")
        elif cmd == "CollectInventoryItem":
            add("script:%d:%s:%s:%s" % (lv, script, cmd, "|".join(a)),
                "%s - %s gift %s" % (reg, script, "/".join(a)), reg, "Gifts")
        elif cmd in ("CollectWizardCard", "CollectWizardCardPack"):
            add("script:%d:%s:%s:%s" % (lv, script, cmd, "|".join(a)),
                "%s - %s card" % (reg, script), reg, "Wizard Cards")
        elif cmd == "SetAITrader" and a:
            for card in a[1:]:
                add("trade:%d:%s:%s" % (lv, a[0], card), "%s - %s trade card %s" % (reg, a[0], card), reg,
                    "Card Trades")
        elif cmd.startswith("SetAI") and cmd[5:] in ENEMIES and a:
            add("enemy:%d:%s:%s" % (lv, cmd[5:], a[0]), "%s - defeat %s %s" % (reg, cmd[5:], a[0]), reg, "Enemies")
            add("enemytype:%s" % cmd[5:], "First %s defeated" % cmd[5:], "Hogwarts", "Enemy Types")
        elif cmd in QUID_CMDS:
            add("scene:%d:%s" % (lv, script), "%s - %s" % (reg, script), reg, "Quidditch")
        if SCENE_RX.search(script):
            add("scene:%d:%s" % (lv, script), "%s - %s" % (reg, script), reg, "Scenes")
    for tid, (title, reg) in tasks.items():
        add("task:%s:added" % tid, "Task %s added (%s)" % (title.replace("TASK_", "").title(), reg), reg, "Tasks")
        add("task:%s:completed" % tid, "Task %s completed" % title.replace("TASK_", "").title(), reg, "Tasks")

    prev = set()
    for level in ORDER:
        p = os.path.join(census, "e%d.txt" % level)
        if not os.path.exists(p):
            continue
        recs = set()
        for ln in open(p):
            parts = ln.split()
            if len(parts) == 2 and parts[0] != "entities:":
                recs.add((parts[0], parts[1]))
        fresh = {r for r in recs if r not in prev}  # records left from the level before are skipped
        prev = recs
        reg = region_of(level)
        for _, name in sorted(fresh):
            if "." in name or re.search(r"(?i)sfx|^fx|cam|light|dummy|pos$|dum|align", name):
                continue
            for kind, rx in PICKUPS:
                if re.search(rx, name):
                    add("object:%d:%s" % (level, name), "%s - %s %s" % (reg, kind, name), reg, kind + "s")
                    break

    # search spots: the castle's searchable jars, bookcases and chests, the same 176 in the day and night archives
    # (pscensus.py ... pos: search_day1.txt, search_night0.txt); one check per spot, and per hub behind an option
    spots = set()
    for f in ("search_day1.txt", "search_night0.txt"):
        p = os.path.join(census, f)
        if os.path.exists(p):
            for ln in open(p):
                parts = ln.split()
                if len(parts) == 5 and re.search(r"(?i)jar|chest|bookcase", parts[1]) and                         not re.search(r"(?i)done|dum|fade|snap|sfx", parts[1]):
                    spots.add(parts[1].split(".", 1)[1].lower())
    for spot in sorted(spots):
        add("search:%s" % spot, "Search %s" % spot, "Castle", "Search Spots")
        for lv in HUBS[1:]:
            add("search:%d:%s" % (lv, spot), "%s - Search %s" % (LEVELS[lv], spot), LEVELS[lv], "Search Spots per Hub")

    for spell in SPELLS:
        add("spell:%d:first_cast" % SPELLS[spell], "First cast of %s" % spell, "Hogwarts", "Spells")
    for n in range(10, 501, 10):
        add("beans:%d" % n, "%d beans collected" % n, "Hogwarts", "Bean Milestones")
    for n in range(1, 102):
        add("cards:%d" % n, "%d wizard cards owned" % n, "Hogwarts", "Card Milestones")

    keys = sorted(locs)
    locations = [{"id": LOCATION_BASE + i, "key": k, "name": locs[k][0], "region": locs[k][1], "group": locs[k][2]}
                 for i, k in enumerate(keys)]
    seen = collections.Counter(l["name"] for l in locations)
    for l in locations:
        if seen[l["name"]] > 1:
            l["name"] += " [%d]" % (l["id"] - LOCATION_BASE)

    items = collections.OrderedDict()
    nid = ITEM_BASE
    for lv in HUBS:
        items["%s Pass" % LEVELS[lv]] = {"id": nid, "kind": "pass", "level": lv, "class": "progression"}; nid += 1
    for name, (lv, script) in DUNGEONS.items():
        if name in DROPPED_DUNGEONS:
            continue
        items["%s Pass" % name] = {"id": nid, "kind": "dungeon", "level": lv, "script": script,
                                   "class": "progression"}; nid += 1
    for spell, n in SPELLS.items():
        items[spell] = {"id": nid, "kind": "spell", "spell": n, "class": "progression"}; nid += 1
    for name, n in QUEST_ITEMS.items():
        items[name] = {"id": nid, "kind": "quest", "item": n, "class": "progression"}; nid += 1
    for card in range(1, 102):
        items["Wizard Card %d" % card] = {"id": nid, "kind": "card", "card": card, "class": "useful"}; nid += 1
    for name, kind, cls in (("Health Bar", "health", "useful"), ("Big Bean Bag", "bag", "useful"),
                            ("Bertie Bott's Beans", "beans", "filler"), ("Chocolate Frog", "frog", "filler"),
                            ("Cauldron Cake", "sweet", "filler")):
        items[name] = {"id": nid, "kind": kind, "class": cls}; nid += 1

    digest = hashlib.sha256("\n".join(keys).encode() + json.dumps(list(items)).encode()).hexdigest()[:12]
    catalog = {"version": "hpps-2-" + digest, "game": GAME, "location_base": LOCATION_BASE, "item_base": ITEM_BASE,
               "regions": {"hubs": [LEVELS[l] for l in HUBS],
                           "dungeons": [d for d in DUNGEONS if d not in DROPPED_DUNGEONS]},
               "items": items, "locations": locations}
    with open(out, "w", encoding="utf-8", newline="\n") as f:
        json.dump(catalog, f, indent=1)
        f.write("\n")
    groups = collections.Counter(l["group"] for l in locations)
    print("%s: %d locations, %d items" % (catalog["version"], len(locations), len(items)))
    for g, n in groups.most_common():
        print("  %5d %s" % (n, g))


if __name__ == "__main__":
    main()
