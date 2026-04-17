#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <stdbool.h>

#include "media_types.h"

int playlist_create(MediaLibrary* lib, const char* name);
bool playlist_add_track(MediaLibrary* lib, int playlist_idx, int track_idx);
bool playlist_remove_track(MediaLibrary* lib, int playlist_idx, int position);
bool playlist_remix(MediaLibrary* lib, int playlist_idx, unsigned int seed);

#endif
