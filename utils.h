/* utils.h - Общие утилиты и проверки персонажей */
#ifndef __UTILS_H__
#define __UTILS_H__

#include "typedefs.h"

// Forward declaration (полное определение в pers.h)
typedef struct fs_pers_s fs_pers_t;

// Проверки статуса (реализации после pers.h)
int pers_is_bot(const fs_pers_t *p);
int pers_is_player(const fs_pers_t *p);
int pers_is_alive(const fs_pers_t *p);
int pers_is_dead(const fs_pers_t *p);
int pers_has_flag(const fs_pers_t *p, int flag);
int is_teammate(const fs_pers_t *p1, const fs_pers_t *p2);
int is_opponent(const fs_pers_t *p1, const fs_pers_t *p2);
double pers_hp_ratio(const fs_pers_t *p);
double pers_mp_ratio(const fs_pers_t *p);
int pers_hp_percent(const fs_pers_t *p);

#endif
