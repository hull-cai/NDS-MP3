#include "playlist.h"

#include <stdlib.h>
#include <string.h>

int playlist_create(MediaLibrary* lib, const char* name) {
    if (lib->playlist_count >= MAX_PLAYLISTS) {
        return -1;
    }

    Playlist* playlist = &lib->playlists[lib->playlist_count];
    memset(playlist, 0, sizeof(*playlist));
    strncpy(playlist->name, name, sizeof(playlist->name) - 1);
    return lib->playlist_count++;
}

bool playlist_add_track(MediaLibrary* lib, int playlist_idx, int track_idx) {
    if (playlist_idx < 0 || playlist_idx >= lib->playlist_count) {
        return false;
    }

    if (track_idx < 0 || track_idx >= lib->track_count) {
        return false;
    }

    Playlist* playlist = &lib->playlists[playlist_idx];
    if (playlist->track_count >= MAX_PLAYLIST_TRACKS) {
        return false;
    }

    playlist->track_indices[playlist->track_count++] = track_idx;
    return true;
}

bool playlist_remove_track(MediaLibrary* lib, int playlist_idx, int position) {
    if (playlist_idx < 0 || playlist_idx >= lib->playlist_count) {
        return false;
    }

    Playlist* playlist = &lib->playlists[playlist_idx];
    if (position < 0 || position >= playlist->track_count) {
        return false;
    }

    for (int i = position; i < playlist->track_count - 1; ++i) {
        playlist->track_indices[i] = playlist->track_indices[i + 1];
    }
    playlist->track_count--;
    return true;
}

bool playlist_remix(MediaLibrary* lib, int playlist_idx, unsigned int seed) {
    if (playlist_idx < 0 || playlist_idx >= lib->playlist_count) {
        return false;
    }

    Playlist* playlist = &lib->playlists[playlist_idx];
    if (playlist->track_count < 2) {
        return false;
    }

    srand(seed);
    for (int i = playlist->track_count - 1; i > 0; --i) {
        int j = rand() % (i + 1);
        int tmp = playlist->track_indices[i];
        playlist->track_indices[i] = playlist->track_indices[j];
        playlist->track_indices[j] = tmp;
    }

    return true;
}
