#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>

bool player_init(void);
bool player_play_mp3_file(const char* path);
void player_stop(void);

#endif
