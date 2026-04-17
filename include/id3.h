#ifndef ID3_H
#define ID3_H

#include <stdbool.h>

#include "media_types.h"

bool id3_load_metadata(const char* path, Track* out_track);

#endif
