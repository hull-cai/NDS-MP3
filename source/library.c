#include "library.h"

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "id3.h"

static bool has_mp3_extension(const char* name) {
    const char* ext = strrchr(name, '.');
    if (!ext) return false;
    return strcasecmp(ext, ".mp3") == 0;
}

static int upsert_artist(MediaLibrary* lib, const char* name) {
    const char* effective = (name && name[0] != '\0') ? name : "Unknown Artist";

    for (int i = 0; i < lib->artist_count; ++i) {
        if (strcmp(lib->artists[i].name, effective) == 0) {
            return i;
        }
    }

    if (lib->artist_count >= MAX_ARTISTS) {
        return -1;
    }

    ArtistGroup* artist = &lib->artists[lib->artist_count];
    memset(artist, 0, sizeof(*artist));
    strncpy(artist->name, effective, sizeof(artist->name) - 1);
    return lib->artist_count++;
}

static int upsert_album(MediaLibrary* lib, const char* name) {
    const char* effective = (name && name[0] != '\0') ? name : "Unknown Album";

    for (int i = 0; i < lib->album_count; ++i) {
        if (strcmp(lib->albums[i].name, effective) == 0) {
            return i;
        }
    }

    if (lib->album_count >= MAX_ALBUMS) {
        return -1;
    }

    AlbumGroup* album = &lib->albums[lib->album_count];
    memset(album, 0, sizeof(*album));
    strncpy(album->name, effective, sizeof(album->name) - 1);
    return lib->album_count++;
}

static void assign_track_to_groups(MediaLibrary* lib, int track_idx) {
    Track* track = &lib->tracks[track_idx];

    int artist_idx = upsert_artist(lib, track->artist);
    if (artist_idx >= 0) {
        ArtistGroup* group = &lib->artists[artist_idx];
        if (group->track_count < MAX_TRACKS) {
            group->track_indices[group->track_count++] = track_idx;
        }
    }

    int album_idx = upsert_album(lib, track->album);
    if (album_idx >= 0) {
        AlbumGroup* group = &lib->albums[album_idx];
        if (group->track_count < MAX_TRACKS) {
            group->track_indices[group->track_count++] = track_idx;
        }
    }
}

static bool scan_recursive(MediaLibrary* lib, const char* path) {
    DIR* dir = opendir(path);
    if (!dir) {
        return false;
    }

    struct dirent* entry;
    char full_path[MAX_PATH_LEN];

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        if (entry->d_type == DT_DIR) {
            scan_recursive(lib, full_path);
            continue;
        }

        if (!has_mp3_extension(entry->d_name) || lib->track_count >= MAX_TRACKS) {
            continue;
        }

        Track* track = &lib->tracks[lib->track_count];
        memset(track, 0, sizeof(*track));
        strncpy(track->path, full_path, sizeof(track->path) - 1);

        if (!id3_load_metadata(track->path, track)) {
            strncpy(track->title, entry->d_name, sizeof(track->title) - 1);
            strncpy(track->artist, "Unknown Artist", sizeof(track->artist) - 1);
            strncpy(track->album, "Unknown Album", sizeof(track->album) - 1);
        }

        assign_track_to_groups(lib, lib->track_count);
        lib->track_count++;
    }

    closedir(dir);
    return true;
}

void library_init(MediaLibrary* lib) {
    memset(lib, 0, sizeof(*lib));
}

bool library_scan_directory(MediaLibrary* lib, const char* root) {
    return scan_recursive(lib, root);
}

int library_find_artist(const MediaLibrary* lib, const char* artist_name) {
    for (int i = 0; i < lib->artist_count; ++i) {
        if (strcmp(lib->artists[i].name, artist_name) == 0) {
            return i;
        }
    }

    return -1;
}

int library_find_album(const MediaLibrary* lib, const char* album_name) {
    for (int i = 0; i < lib->album_count; ++i) {
        if (strcmp(lib->albums[i].name, album_name) == 0) {
            return i;
        }
    }

    return -1;
}
