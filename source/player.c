#include "player.h"

#include <nds.h>
#include <stdio.h>

bool player_init(void) {
    // Placeholder audio init.
    // For full MP3 playback you can integrate a decoder such as minimp3 or Helix,
    // decode to PCM, then stream PCM using libnds sound channels.
    return true;
}

bool player_play_mp3_file(const char* path) {
    iprintf("\x1b[12;0HNow playing:\n%-30s", path);
    iprintf("\x1b[15;0HMP3 decode backend not linked in this template.");
    return false;
}

void player_stop(void) {
    soundKill(-1);
}
