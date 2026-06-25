# TimelinePanel Theme Variation Contract

`TimelinePanelBase` (and its subclasses `VTimelinePanel`, `HTimelinePanel`) and `TimelineConnectionEditor` are GDExtension `Control` nodes with self-drawn canvas items. Because GDExtension cannot register Godot's internal `BIND_THEME_ITEM` metadata, the public styling contract is based on stable `theme_type_variation` names and manually-read theme items.

**All visual styling is exclusively controlled via `Theme` resources and per-resource overrides.** There are no inspector properties for style values — the old `Style overrides` inspector group has been removed. Theme changes take effect immediately via `NOTIFICATION_THEME_CHANGED` → `queue_redraw()`.

## Variation Types

Define these type variations in a `Theme` resource when you want editor suggestions and inheritance:

```gdscript
theme.set_type_variation("TimelinePanel", "Control")
theme.set_type_variation("TimelineConnectionEditor", "Control")
```

`VTimelinePanel` and `HTimelinePanel` share the `TimelinePanel` base type since they both inherit from `TimelinePanelBase` and use the same theme contract.

---

## Supported Items — `TimelinePanel`

### Colors

| Item name | Type | Default | Description |
|-----------|------|---------|-------------|
| `background_color` | Color | `Color()` | Panel background fill |
| `separator_color` | Color | `#696969` | Color of track separator lines |
| `beat_line_color` | Color | `#ff7384` | Beat subdivision line color |
| `bar_line_color` | Color | `#5f8fc9` | Bar (measure) line color |

### Constants

| Item name | Type | Default | Description |
|-----------|------|---------|-------------|
| `separator_width` | int | `-1` (auto) | Width of track separator lines in px |
| `bar_line_width` | int | `-1` (auto) | Width of bar lines in px |
| `beat_line_width` | int | `-1` (auto) | Width of beat subdivision lines in px |
| `instant_key_scale` | int | `-1` (use default 0.4) | Scale of instant keys as percentage (40 = 0.4×) |

### StyleBoxes

Items marked with `*` also support per-key overrides on `TimelineTrackKey` resources. Per-key overrides take priority over all theme settings.

| Item name | Per-key? | Default fallback | Description |
|-----------|----------|------------------|-------------|
| `instant_key_normal` | Yes | White rounded pill | StyleBox for normal instant (zero-length) keys |
| `instant_key_selected` | Yes | White outlined pill | StyleBox for selected instant keys |
| `clip_key_normal` | Yes | Solid white rect | StyleBox for normal clip (range) keys |
| `clip_key_selected` | Yes | White outlined rect | StyleBox for selected clip keys |
| `selection_rect` | No | Semi-transparent white rect with border | StyleBox for drag-selection rectangle |
| `key_release_preview` | No | Red semi-transparent | StyleBox preview when key would overlap on release |
| `key_allowed_overlap_preview` | No | Green semi-transparent | StyleBox preview when overlap is permitted |

### Fallback Chain

1. **Per-key override** (`TimelineTrackKey` property) — highest priority
2. **Theme value** (from `Theme` resource, type `TimelinePanel`)
3. **Hardcoded default** (built-in `StyleBoxFlat` / member value) — lowest priority

---

## Supported Items — `TimelineConnectionEditor`

### Colors

| Item name | Type | Default | Description |
|-----------|------|---------|-------------|
| `ruler_background_color` | Color | `(0.13, 0.13, 0.13, 0.92)` | Background of the time/value ruler |
| `ruler_tick_color` | Color | `(1, 1, 1, 0.42)` | Color of ruler tick marks |
| `ruler_text_color` | Color | `(1, 1, 1, 0.72)` | Color of ruler label text |
| `ruler_major_grid_color` | Color | `(1, 1, 1, 0.16)` | Color of major grid lines |
| `ruler_minor_grid_color` | Color | `(1, 1, 1, 0.055)` | Color of minor grid lines |
| `handle_line_color` | Color | `(1, 1, 1, 0.44)` | Color of bezier handle segment lines |
| `handle_line_selected_color` | Color | `(1, 0.78, 0.24, 0.95)` | Color of selected bezier handle segments |
| `zero_tick_color` | Color | `(1, 0.78, 0.24, 0.95)` | Color of the zero (origin) tick highlight |

### Constants

| Item name | Type | Default | Description |
|-----------|------|---------|-------------|
| `handle_radius` | int | `-1` (use 4.0) | Radius of handle knobs and hit-test area in px |
| `key_scale` | int | `-1` (use 1.0) | Scale of connection point keys as percentage (100 = 1.0×) |
| `handle_scale` | int | `-1` (use 1.0) | Scale of handle knobs as percentage (100 = 1.0×) |
| `handle_line_width` | int | `-1` (use 1.0) | Width of handle segment lines in px |
| `bezier_line_width` | int | `-1` (use 2.0) | Width of bezier curve lines in px |
| `ruler_font_size` | int | `11` | Font size for ruler label text in px |
| `ruler_min_tick_spacing` | int | `-1` (use 56.0) | Minimum spacing between ruler tick marks in px |

### StyleBoxes

Items marked with `*` also support per-connection overrides on `TimelineConnection` resources.

| Item name | Per-connection? | Default fallback | Description |
|-----------|-----------------|------------------|-------------|
| `key_normal` | Yes | White rounded pill | StyleBox for normal connection point keys |
| `key_selected` | Yes | Gold rounded pill with border | StyleBox for selected point keys |
| `handle_normal` | Yes | Blue rounded pill | StyleBox for normal bezier handle knobs |
| `handle_selected` | Yes | Gold rounded pill | StyleBox for selected bezier handle knobs |
| `selection_rect` | No | Semi-transparent white rect with border | StyleBox for drag-selection rectangle |

### Fallback Chain

1. **Per-connection override** (`TimelineConnection` property) — highest priority
2. **Theme value** (from `Theme` resource, type `TimelineConnectionEditor`)
3. **Hardcoded default** (built-in `StyleBoxFlat` / member value) — lowest priority

---

## Example

```gdscript
var theme: Theme = Theme.new()

theme.set_type_variation("TimelinePanel", "Control")
theme.set_type_variation("TimelineConnectionEditor", "Control")

# --- TimelinePanel ---
theme.set_color("background_color", "TimelinePanel", Color(0.12, 0.12, 0.14))
theme.set_color("separator_color", "TimelinePanel", Color(0.25, 0.25, 0.30))
theme.set_color("beat_line_color", "TimelinePanel", Color(1.0, 0.45, 0.52))
theme.set_color("bar_line_color", "TimelinePanel", Color(0.37, 0.56, 0.79))
theme.set_constant("separator_width", "TimelinePanel", 1)
theme.set_constant("bar_line_width", "TimelinePanel", 2)
theme.set_constant("instant_key_scale", "TimelinePanel", 50)  # 0.5x

var instant_normal: StyleBoxFlat = StyleBoxFlat.new()
instant_normal.bg_color = Color(0.95, 0.95, 0.95)
instant_normal.set_corner_radius_all(512)
theme.set_stylebox("instant_key_normal", "TimelinePanel", instant_normal)

var instant_selected: StyleBoxFlat = StyleBoxFlat.new()
instant_selected.bg_color = Color(0.99, 0.72, 0.18)
instant_selected.set_corner_radius_all(512)
theme.set_stylebox("instant_key_selected", "TimelinePanel", instant_selected)

# --- TimelineConnectionEditor ---
theme.set_color("ruler_background_color", "TimelineConnectionEditor", Color(0.10, 0.10, 0.12, 0.95))
theme.set_color("handle_line_color", "TimelineConnectionEditor", Color(0.50, 0.70, 1.0, 0.55))
theme.set_constant("bezier_line_width", "TimelineConnectionEditor", 3)
theme.set_constant("ruler_font_size", "TimelineConnectionEditor", 12)

var ce_key_normal: StyleBoxFlat = StyleBoxFlat.new()
ce_key_normal.bg_color = Color(1.0, 1.0, 1.0, 0.85)
ce_key_normal.set_corner_radius_all(512)
theme.set_stylebox("key_normal", "TimelineConnectionEditor", ce_key_normal)

var ce_key_selected: StyleBoxFlat = StyleBoxFlat.new()
ce_key_selected.bg_color = Color(1.0, 0.78, 0.24, 0.95)
ce_key_selected.set_corner_radius_all(512)
theme.set_stylebox("key_selected", "TimelineConnectionEditor", ce_key_selected)

# Assign to controls
timeline_panel.theme = theme
connection_editor.theme = theme
```

The names above are part of the public styling contract. Theme item names are stable; avoid depending on internal fallback construction details as they may change.
