/* The Archipelago bridge's C interface (bridge.cpp), used by hle_ap.c on the title's own thread.
 *
 * Off unless RECOMP_AP_SERVER and RECOMP_AP_SLOT are set (the password, if any, is read from
 * <exe dir>\saves\archipelago\connection.toml, password = "...", so it never passes through the command line or a
 * log). Everything here runs on the game thread: the network thread (client.cpp) only queues packets. */
#ifndef HPPS_AP_BRIDGE_H
#define HPPS_AP_BRIDGE_H

#ifdef __cplusplus
extern "C" {
#endif

/* 1 if RECOMP_AP_SERVER and RECOMP_AP_SLOT are set (read once): the rest does nothing otherwise */
int hpps_ap_enabled(void);
/* once a frame Harry is updated: connect, drain packets, (re)send checks */
void hpps_ap_frame(void);
/* a location's catalog key happened (unknown keys are ignored); safe to repeat */
void hpps_ap_location(const char *key);
/* 1 if the catalog has that location key (whether or not it is checked yet) */
int hpps_ap_known(const char *key);
/* the goal is done */
void hpps_ap_goal(void);
/* the next received item to give in the game: its catalog kind ("card", "spell", "beans", ...) and value (card
 * number, spell number, 0); returns 0 when there is none. hpps_ap_item_given() after giving it moves the received
 * index on and saves it */
int hpps_ap_next_item(const char **kind, int *value);
void hpps_ap_item_given(void);
/* 1 while a slot is bound (connected or offline with a session), for the cards goal and the travel menu */
int hpps_ap_active(void);
int hpps_ap_cards_goal(void);
/* 1 if the slot owns that item (a pass, by catalog name) */
int hpps_ap_has(const char *item_name);
/* the travel window's list: the days and nights in order, each followed by its set pieces (a set piece with a level of
 * its own goes under the day before it). name is the day's or the set piece's, script the set piece's entry script in
 * its hub or "", unlocked 1 when the slot owns what it takes (the day's unlock; a set piece's pass and its hub's
 * unlock). Returns 0 past the end. */
int hpps_ap_travel_entry(int i, const char **name, int *level, const char **script, int *dungeon, int *unlocked);

#ifdef __cplusplus
}
#endif

#endif
