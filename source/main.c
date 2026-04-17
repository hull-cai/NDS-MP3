#include <fat.h>
#include <nds.h>
#include <stdio.h>
#include <string.h>

#include "library.h"
#include "player.h"
#include "playlist.h"

typedef enum {
    VIEW_LIBRARY = 0,
    VIEW_ARTISTS,
    VIEW_ALBUMS,
    VIEW_PLAYLISTS,
    VIEW_NOW_PLAYING,
    VIEW_COUNT
} ViewMode;

typedef enum {
    REPEAT_OFF = 0,
    REPEAT_ONE,
    REPEAT_ALL
} RepeatMode;

typedef struct {
    MediaLibrary lib;
    ViewMode view;
    RepeatMode repeat_mode;
    int selected_index;
    int active_playlist;
    int now_playing_track;
    bool shuffle_enabled;
    bool is_playing;
    bool show_help;
    char status_line[64];
} AppState;

typedef enum {
    TOUCH_NONE = 0,
    TOUCH_PREV,
    TOUCH_PLAY,
    TOUCH_STOP,
    TOUCH_NEXT,
    TOUCH_REMIX,
    TOUCH_REPEAT,
    TOUCH_NEW_PL,
    TOUCH_ADD
} TouchAction;

static const char* kViewTitles[] = {"Library", "Artists", "Albums", "Playlists", "Now Playing"};

static int item_count_for_view(const AppState* app) {
    switch (app->view) {
        case VIEW_LIBRARY: return app->lib.track_count;
        case VIEW_ARTISTS: return app->lib.artist_count;
        case VIEW_ALBUMS: return app->lib.album_count;
        case VIEW_PLAYLISTS: return app->lib.playlist_count;
        case VIEW_NOW_PLAYING: return app->lib.track_count;
        default: return 0;
    }
}

static const char* repeat_label(RepeatMode mode) {
    switch (mode) {
        case REPEAT_ONE: return "ONE";
        case REPEAT_ALL: return "ALL";
        default: return "OFF";
    }
}

static void set_status(AppState* app, const char* msg) {
    strncpy(app->status_line, msg, sizeof(app->status_line) - 1);
    app->status_line[sizeof(app->status_line) - 1] = '\0';
}

static void draw_header(const AppState* app) {
    iprintf("\x1b[0;0H================================");
    iprintf("\x1b[1;0H  NDS MUSIC - %-11s       ", kViewTitles[app->view]);
    iprintf("\x1b[2;0H================================");
    iprintf("\x1b[3;0HShuffle:%s Repeat:%s Help:%s", app->shuffle_enabled ? "ON " : "OFF", repeat_label(app->repeat_mode), app->show_help ? "ON" : "OFF");
}

static void draw_help_overlay(void) {
    iprintf("\x1b[5;0H--- DS BUTTON MAP ----------------");
    iprintf("\x1b[6;0HD-Pad: Move selection / view");
    iprintf("\x1b[7;0HA: Play selected track");
    iprintf("\x1b[8;0HB: Stop (or close this help)");
    iprintf("\x1b[9;0HL/R: Previous / Next track");
    iprintf("\x1b[10;0HX: Create new playlist");
    iprintf("\x1b[11;0HY: Add selected track");
    iprintf("\x1b[12;0HStart: Remix active playlist");
    iprintf("\x1b[13;0HSelect: Toggle help overlay");
    iprintf("\x1b[14;0HStylus: Tap transport bar");
    iprintf("\x1b[15;0H--------------------------------");
}

static void draw_footer(const AppState* app) {
    iprintf("\x1b[20;0H--------------------------------");
    iprintf("\x1b[21;0HTouch: [<<][>][[]][>>][Mix][Rep]");
    iprintf("\x1b[22;0HTouch: [New Playlist][Add Track]");
    iprintf("\x1b[23;0H%-32s", app->status_line);
}

static void draw_list(const AppState* app) {
    for (int row = 0; row < 14; ++row) {
        iprintf("\x1b[%d;0H                                ", 5 + row);
    }

    if (app->show_help) {
        draw_help_overlay();
        return;
    }

    int total = item_count_for_view(app);
    int start = app->selected_index - 5;
    if (start < 0) start = 0;

    for (int row = 0; row < 14; ++row) {
        int index = start + row;
        if (index >= total) continue;

        char marker = (index == app->selected_index) ? '>' : ' ';
        if (app->view == VIEW_LIBRARY || app->view == VIEW_NOW_PLAYING) {
            const Track* t = &app->lib.tracks[index];
            iprintf("\x1b[%d;0H%c %-16.16s %-13.13s", 5 + row, marker, t->title, t->artist);
        } else if (app->view == VIEW_ARTISTS) {
            const ArtistGroup* g = &app->lib.artists[index];
            iprintf("\x1b[%d;0H%c %-22.22s %3d", 5 + row, marker, g->name, g->track_count);
        } else if (app->view == VIEW_ALBUMS) {
            const AlbumGroup* g = &app->lib.albums[index];
            iprintf("\x1b[%d;0H%c %-22.22s %3d", 5 + row, marker, g->name, g->track_count);
        } else if (app->view == VIEW_PLAYLISTS) {
            const Playlist* p = &app->lib.playlists[index];
            iprintf("\x1b[%d;0H%c %-22.22s %3d", 5 + row, marker, p->name, p->track_count);
        }
    }
}

static void render_ui(const AppState* app) {
    draw_header(app);
    draw_list(app);
    draw_footer(app);
}

static int resolve_selected_track(const AppState* app) {
    if (app->view == VIEW_LIBRARY || app->view == VIEW_NOW_PLAYING) {
        return app->selected_index;
    }

    if (app->view == VIEW_PLAYLISTS && app->active_playlist >= 0 && app->active_playlist < app->lib.playlist_count) {
        const Playlist* p = &app->lib.playlists[app->active_playlist];
        if (app->selected_index >= 0 && app->selected_index < p->track_count) {
            return p->track_indices[app->selected_index];
        }
    }

    return -1;
}

static void play_selected(AppState* app) {
    int track_idx = resolve_selected_track(app);
    if (track_idx < 0 || track_idx >= app->lib.track_count) {
        set_status(app, "No playable track in this view");
        return;
    }

    app->now_playing_track = track_idx;
    app->is_playing = player_play_mp3_file(app->lib.tracks[track_idx].path);
    set_status(app, app->is_playing ? "Playing track" : "Playback backend unavailable");
}

static void next_track(AppState* app) {
    if (app->lib.track_count <= 0) return;

    if (app->repeat_mode == REPEAT_ONE && app->now_playing_track >= 0) {
        play_selected(app);
        return;
    }

    int next = app->now_playing_track + 1;
    if (next >= app->lib.track_count) {
        if (app->repeat_mode == REPEAT_ALL) next = 0;
        else {
            set_status(app, "Reached end of queue");
            return;
        }
    }

    app->selected_index = next;
    play_selected(app);
}

static void prev_track(AppState* app) {
    if (app->lib.track_count <= 0) return;
    int prev = app->now_playing_track - 1;
    if (prev < 0) prev = 0;
    app->selected_index = prev;
    play_selected(app);
}

static void cycle_repeat(AppState* app) {
    app->repeat_mode = (RepeatMode)((app->repeat_mode + 1) % 3);
    set_status(app, "Repeat mode updated");
}

static void create_playlist(AppState* app) {
    char name[32];
    snprintf(name, sizeof(name), "Mix %d", app->lib.playlist_count + 1);
    int idx = playlist_create(&app->lib, name);
    if (idx >= 0) {
        app->active_playlist = idx;
        set_status(app, "Created new playlist");
    } else {
        set_status(app, "Playlist limit reached");
    }
}

static void add_selected_to_playlist(AppState* app) {
    int tr = resolve_selected_track(app);
    if (app->active_playlist >= 0 && tr >= 0 && playlist_add_track(&app->lib, app->active_playlist, tr)) {
        set_status(app, "Added track to active playlist");
    } else {
        set_status(app, "Unable to add track");
    }
}

static void remix_active_playlist(AppState* app) {
    app->shuffle_enabled = !app->shuffle_enabled;
    if (app->active_playlist >= 0) {
        if (playlist_remix(&app->lib, app->active_playlist, (unsigned int)REG_VCOUNT + app->lib.track_count)) {
            set_status(app, "Remixed active playlist");
        } else {
            set_status(app, "Need at least 2 tracks to remix");
        }
    }
}

static TouchAction decode_touch_action(const touchPosition* touch) {
    if (touch->py >= 160 && touch->py < 176) {
        if (touch->px < 40) return TOUCH_PREV;
        if (touch->px < 80) return TOUCH_PLAY;
        if (touch->px < 120) return TOUCH_STOP;
        if (touch->px < 160) return TOUCH_NEXT;
        if (touch->px < 210) return TOUCH_REMIX;
        return TOUCH_REPEAT;
    }

    if (touch->py >= 176) {
        if (touch->px < 128) return TOUCH_NEW_PL;
        return TOUCH_ADD;
    }

    return TOUCH_NONE;
}

static void handle_touch_action(AppState* app, TouchAction action) {
    switch (action) {
        case TOUCH_PREV: prev_track(app); break;
        case TOUCH_PLAY: play_selected(app); break;
        case TOUCH_STOP:
            player_stop();
            app->is_playing = false;
            set_status(app, "Stopped");
            break;
        case TOUCH_NEXT: next_track(app); break;
        case TOUCH_REMIX: remix_active_playlist(app); break;
        case TOUCH_REPEAT: cycle_repeat(app); break;
        case TOUCH_NEW_PL: create_playlist(app); break;
        case TOUCH_ADD: add_selected_to_playlist(app); break;
        default: break;
    }
}

int main(void) {
    consoleDemoInit();

    AppState app;
    memset(&app, 0, sizeof(app));
    library_init(&app.lib);
    app.view = VIEW_LIBRARY;
    app.active_playlist = -1;
    app.now_playing_track = -1;
    app.show_help = true;
    set_status(&app, "Welcome - scanning /music");

    if (!fatInitDefault()) {
        iprintf("FAT init failed. Check DLDI patch.\n");
        while (1) swiWaitForVBlank();
    }

    player_init();
    library_scan_directory(&app.lib, "/music");

    app.active_playlist = playlist_create(&app.lib, "Quick Mix");
    set_status(&app, "Loaded library. Press SELECT for help");

    while (1) {
        scanKeys();
        int keys = keysDown();

        if (!app.show_help) {
            if (keys & KEY_RIGHT) {
                app.view = (ViewMode)((app.view + 1) % VIEW_COUNT);
                app.selected_index = 0;
            }
            if (keys & KEY_LEFT) {
                app.view = (ViewMode)((app.view + VIEW_COUNT - 1) % VIEW_COUNT);
                app.selected_index = 0;
            }

            int total_items = item_count_for_view(&app);
            if (keys & KEY_DOWN) {
                if (total_items > 0) app.selected_index = (app.selected_index + 1) % total_items;
            }
            if (keys & KEY_UP) {
                if (total_items > 0) {
                    app.selected_index--;
                    if (app.selected_index < 0) app.selected_index = total_items - 1;
                }
            }

            if (keys & KEY_A) play_selected(&app);
            if (keys & KEY_X) create_playlist(&app);
            if (keys & KEY_Y) add_selected_to_playlist(&app);
        }

        if (keys & KEY_B) {
            if (app.show_help) {
                app.show_help = false;
                set_status(&app, "Help closed");
            } else {
                player_stop();
                app.is_playing = false;
                set_status(&app, "Stopped");
            }
        }

        if (keys & KEY_R) next_track(&app);
        if (keys & KEY_L) prev_track(&app);
        if (keys & KEY_START) remix_active_playlist(&app);
        if (keys & KEY_SELECT) {
            app.show_help = !app.show_help;
            set_status(&app, app.show_help ? "Help open" : "Help closed");
        }

        if (keys & KEY_TOUCH) {
            touchPosition touch;
            touchRead(&touch);
            handle_touch_action(&app, decode_touch_action(&touch));
        }

        render_ui(&app);
        swiWaitForVBlank();
    }

    return 0;
}
