#ifndef LIBRARY_H
#define LIBRARY_H

#include <stdbool.h>

#include "media_types.h"

void library_init(MediaLibrary* lib);
bool library_scan_directory(MediaLibrary* lib, const char* root);
int library_find_artist(const MediaLibrary* lib, const char* artist_name);
int library_find_album(const MediaLibrary* lib, const char* album_name);

#endif
