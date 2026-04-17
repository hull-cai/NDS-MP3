# NDS MP3 Player (Homebrew)

This project builds a Nintendo DS homebrew `.nds` app with a richer, more navigable "Spotify-inspired" handheld interface:

- **Dual-input navigation**: D-pad/buttons + stylus touch shortcuts.
- **Library views**: Tracks, Artists, Albums, Playlists, and Now Playing screens.
- **Playlist workflow**: Create playlists quickly and add tracks from the browser.
- **Remix mode**: Shuffle/re-order the active playlist instantly.
- **Transport controls**: Play, stop, previous, next.
- **Playback behavior controls**: Repeat Off / Repeat One / Repeat All.
- **Metadata indexing**: ID3v1 title/artist/album parsing.

## Current backend status

- ✅ Library scan, grouping, UI navigation, and playlist/remix logic are implemented.
- ⚠️ MP3 decode backend still needs to be linked for real hardware audio playback.

## Build

1. Install **devkitPro** with `libnds` and `libfat`.
2. Set environment variables:

```bash
export DEVKITPRO=/opt/devkitpro
export DEVKITARM=$DEVKITPRO/devkitARM
```

3. Build:

```bash
make
```

Expected output:

- `nds-mp3-player.nds`

## In-app control instructions (top-screen help overlay)

Press `Select` anytime to show/hide the **controls help area**.

### Hardware buttons (all DS face/shoulder/system inputs mapped)

- `D-Pad Left/Right`: Switch between views
- `D-Pad Up/Down`: Move selection in current view
- `A`: Play selected track
- `B`: Stop playback (or close help overlay)
- `X`: Create new playlist and make it active
- `Y`: Add selected track to active playlist
- `L`: Previous track
- `R`: Next track
- `Start`: Remix (shuffle) active playlist
- `Select`: Toggle instructions/help overlay

### Touchscreen controls (bottom action bar)

Tap the lower touchscreen zones:

- Row 1: `<<` (Prev), `>` (Play), `[]` (Stop), `>>` (Next), `Mix` (Remix), `Rep` (Repeat mode)
- Row 2: `New Playlist`, `Add Track`

## Folder layout expected on SD card

```text
/music
  Artist A/
    Album 1/
      track01.mp3
```
