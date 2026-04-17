#ifndef MEDIA_TYPES_H
#define MEDIA_TYPES_H

#include <stdbool.h>
#include <stddef.h>

#define MAX_TITLE_LEN 64
#define MAX_ARTIST_LEN 64
#define MAX_ALBUM_LEN 64
#define MAX_PATH_LEN 256

#define MAX_TRACKS 1024
#define MAX_ARTISTS 128
#define MAX_ALBUMS 256
#define MAX_PLAYLISTS 32
#define MAX_PLAYLIST_TRACKS 512

typedef struct {
    char title[MAX_TITLE_LEN];
    char artist[MAX_ARTIST_LEN];
    char album[MAX_ALBUM_LEN];
    char path[MAX_PATH_LEN];
} Track;

typedef struct {
    char name[MAX_ARTIST_LEN];
    int track_indices[MAX_TRACKS];
    int track_count;
} ArtistGroup;

typedef struct {
    char name[MAX_ALBUM_LEN];
    int track_indices[MAX_TRACKS];
    int track_count;
} AlbumGroup;

typedef struct {
    char name[MAX_TITLE_LEN];
    int track_indices[MAX_PLAYLIST_TRACKS];
    int track_count;
} Playlist;

typedef struct {
    Track tracks[MAX_TRACKS];
    int track_count;

    ArtistGroup artists[MAX_ARTISTS];
    int artist_count;

    AlbumGroup albums[MAX_ALBUMS];
    int album_count;

    Playlist playlists[MAX_PLAYLISTS];
    int playlist_count;
} MediaLibrary;

#endif
