#include "id3.h"

#include <stdio.h>
#include <string.h>

static void trim_field(char* field, size_t size) {
    for (size_t i = size; i > 0; --i) {
        if (field[i - 1] == ' ' || field[i - 1] == '\0') {
            field[i - 1] = '\0';
        } else {
            break;
        }
    }
}

bool id3_load_metadata(const char* path, Track* out_track) {
    FILE* f = fopen(path, "rb");
    if (!f) {
        return false;
    }

    if (fseek(f, -128, SEEK_END) != 0) {
        fclose(f);
        return false;
    }

    unsigned char tag[128];
    if (fread(tag, 1, sizeof(tag), f) != sizeof(tag)) {
        fclose(f);
        return false;
    }

    fclose(f);

    if (memcmp(tag, "TAG", 3) != 0) {
        return false;
    }

    memset(out_track->title, 0, sizeof(out_track->title));
    memset(out_track->artist, 0, sizeof(out_track->artist));
    memset(out_track->album, 0, sizeof(out_track->album));

    memcpy(out_track->title, tag + 3, 30);
    memcpy(out_track->artist, tag + 33, 30);
    memcpy(out_track->album, tag + 63, 30);

    trim_field(out_track->title, sizeof(out_track->title));
    trim_field(out_track->artist, sizeof(out_track->artist));
    trim_field(out_track->album, sizeof(out_track->album));

    return true;
}
