#ifndef PP_MOVES_H
#define PP_MOVES_H
#include "game.h"
typedef enum {
  PP_ELECTRIC,
  PP_GRASS,
  PP_FIRE,
  PP_WATER,
  PP_NORMAL,
  PP_BUG, PP_POISON, PP_FLYING, PP_GROUND, PP_ROCK, PP_FAIRY, PP_PSYCHIC, PP_FIGHTING, PP_ICE, PP_STEEL, PP_GHOST, PP_DARK, PP_DRAGON
} pp_move_type;
typedef struct {
  const char *name, *type_name, *description;
  pp_move_type type;
  unsigned short power;
} pp_move;
#define PP_MOVE_SLOTS 4
#define PP_MOVE_COUNT (PP_SPECIES * PP_MOVE_SLOTS)
#define PP_MOVE_FAMILY_COUNT 79
#define PP_MOVE_CATALOG_COUNT (PP_MOVE_FAMILY_COUNT * PP_MOVE_SLOTS)
extern const pp_move pp_moves[PP_MOVE_CATALOG_COUNT];
extern const char *const pp_move_family_names[PP_MOVE_FAMILY_COUNT];
/* Catalog entries can precede playable pets. Gameplay uses pp_move_at. */
const pp_move *pp_move_catalog_at(unsigned species, unsigned slot);
const pp_move *pp_battle_move(const pp_game *, unsigned side, unsigned slot);
const pp_move *pp_move_at(unsigned species, unsigned slot);
unsigned pp_move_form_power(unsigned national_id, unsigned slot);
unsigned pp_battle_move_power(const pp_game *, unsigned side, unsigned slot);
#endif
