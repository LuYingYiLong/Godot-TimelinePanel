# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Godot-TimelinePanel is a C++ GDExtension for Godot 4.5+ that provides timeline panel UI controls. It supports multi-track timelines with instant keys and clip (range) keys, time/frame/beat counting units with BPM, and a connection editor for bezier curves.

## Build Commands

Build system: SCons (primary), C++20, MSVC on Windows.

```bash
# Debug build
scons target=template_debug

# Release build
scons target=template_release

# Clean build
scons -c target=template_debug
```

Output goes to `addons/timeline_panel/bin/`. The `godot-cpp` submodule (branch `4.5`) provides the GDExtension bindings — build it first if `.lib`/`.a` files are missing.

### Opening in Godot

To test the extension in the Godot editor:
```bash
"D:\Godot_v4.7-stable_win64.exe\Godot_v4.7-stable_win64.exe" --path <project-with-addon>
```

The extension entry point is `timeline_panel_init` in `register_types.cpp`. It is registered with `reloadable = true` in the `.gdextension` file for hot-reload support.

## Architecture

### Class Hierarchy

```
Resource
 └─ TimelineBase (abstract, components/timeline_base.h)
     ├─ TimelineTrack         — a single track (width, background, header icon/style)
     ├─ TimelineIndicator     — playhead base (style, line, label)
     │   └─ TimelineMarker    — named marker at a specific time
     ├─ TimelineTimeRuler     — time ruler header with tick marks
     └─ TimelineConnection    — bezier/linear curve with Points and Handles

Object
 ├─ TimelineTrackKey          — a keyframe (instant or clip/range) with metadata
 └─ TimelineConnectionPoint   — a point on a TimelineConnection curve

Control
 ├─ TimelinePanelBase         — the core timeline panel widget (~1750 lines)
 │   ├─ VTimelinePanel        — vertical orientation (thin wrapper)
 │   └─ HTimelinePanel        — horizontal orientation (thin wrapper)
 └─ TimelineConnectionEditor  — standalone bezier curve editor with ruler/playhead

StyleBox
 └─ StyleBoxArrow             — custom StyleBox that draws an arrow callout
```

### Source File Organization

`src/timeline_panel_base.cpp` is split across multiple implementation files by feature area:

| File | Responsibility |
|------|---------------|
| `timeline_panel_base.cpp` | Core — constructor, notifications, property bindings, `_gui_input` dispatch |
| `timeline_panel_base_draw_vertical.cpp` | Vertical layout drawing (tracks, keys, grid) |
| `timeline_panel_base_horizontal.cpp` | Horizontal layout drawing |
| `timeline_panel_base_keys.cpp` | Key CRUD, selection, drag, clip-edge editing |
| `timeline_panel_base_style.cpp` | Style cache management, style getters |
| `timeline_panel_base_geometry.cpp` | Position/time/beat/frame conversion math, track cache |
| `timeline_panel_base_interaction.cpp` | Mouse/key input handling, selection rect, scrolling |
| `timeline_panel_base_scroll.cpp` | Scroll bar logic, auto-scroll during drag |
| `timeline_panel_base_minimap.cpp` | Minimap drawing and interaction |
| `timeline_panel_base_properties.cpp` | Property getter/setter implementations |

### Key Design Patterns

- **Virtual methods**: `TimelinePanelBase` exposes GDVIRTUAL hooks:
  - `_should_handle_selection_rect(rect, keys, mouse_button)` — override to customize selection behavior
  - `_handle_selection_rect(rect, keys, mouse_button)` — custom selection handling
  - `_is_key_overlap_allowed(key_a, key_b)` — override overlap policy
- **Style fallback chain**: Styles are resolved per-key → per-track → panel-level fallback → built-in defaults (stored in `style_cache` with `_fallback` variants)
- **Internal `_no_signal` setters**: `TimelineTrackKey` has `set_time_no_signal`, `set_selected_no_signal`, etc. for programmatic changes that shouldn't emit `changed` signals — use these during drag operations to avoid feedback loops
- **Track cache**: `_rebuild_track_cache()` builds `CachedTrack` structs used for layout computation; must be called whenever tracks/keys change
- **Enum aliases**: `VTimelinePanel`/`HTimelinePanel` re-export base class enums via `using` declarations so GDScript can reference e.g. `VTimelinePanel.TIME` without going through the base

### GDScript Property Guidelines

When working with `.tscn` scene files that use this addon:
- Static properties (position, size, colors, themes) must be set directly in the `.tscn` file — never in `_ready()` or `_init()`
- Only runtime-dynamic values should be set via script
- Use scene tree paths (`$Path/To/Node`) rather than direct node variable references
- Load resources by UID path, not filesystem path
- Use strong typing for all variables and function signatures
- Do not shadow member-scope variables with parameters or locals
- Connect signals through the scene editor, not via script `connect()` calls

### Theme System

GDExtension cannot use `BIND_THEME_ITEM`, so all theme values are read manually via `get_theme_color_or()`, `get_theme_constant_or()`, `get_theme_stylebox_or()` helpers (defined in `src/theme_helpers.h`). Two type variations are registered:

- `"TimelinePanel"` — for `TimelinePanelBase` (`VTimelinePanel`, `HTimelinePanel`)
- `"TimelineConnectionEditor"` — for `TimelineConnectionEditor`

Each control calls `set_theme_type_variation()` in its constructor. Style resolution follows a fallback chain: per-resource override → panel property → theme → hardcoded default. Property setters call `add_theme_*_override()` so inspector-set values win over theme resources. Handles `NOTIFICATION_THEME_CHANGED` to queue redraws. See `addons/timeline_panel/docs/timeline_panel_theme_variations.md` for the full contract.

### Code Style (C++)

- K&R brace style, pointers/references left-aligned (`int* p`, `const String& s`)
- LF line endings, UTF-8 without BOM
- Avoid C-style casts; use `static_cast`/`reinterpret_cast`
- Include guards use `#ifndef`/`#define` pattern, not `#pragma once`
