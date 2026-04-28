#ifndef V_TIMELINE_PANEL_H
#define V_TIMELINE_PANEL_H

#include "components/timeline_track_key.h"

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/h_scroll_bar.hpp>
#include <godot_cpp/classes/v_scroll_bar.hpp>
#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/core/gdvirtual.gen.inc>
#include <cstdint>
#include <unordered_set>
#include <vector>

namespace godot {
	class TimelineIndicator;
	class TimelineMarker;
	class TimelineTimeRuler;
	class TimelineTrack;
	class TimelineTrackKey;

	class VTimelinePanel : public Control {
		GDCLASS(VTimelinePanel, Control)

	public:
		enum CountingUnit {
			TIME,
			FRAME,
			BEAT
		};

		enum TimeFormat {
			HH_MM_SS,
			MM_SS_MS,
			SEC
		};

		enum BeatFormat {
			BEAT_BAR
		};

		enum BarNumberDirection {
			BAR_NUMBER_TOP_DOWN,
			BAR_NUMBER_BOTTOM_UP
		};

		enum ScrollMode {
			SCROLL_MODE_DISABLED = 0,
			SCROLL_MODE_AUTO,
			SCROLL_MODE_SHOW_ALWAYS,
			SCROLL_MODE_SHOW_NEVER,
			SCROLL_MODE_RESERVE,
		};

		enum ScrollHintMode {
			SCROLL_HINT_MODE_DISABLED,
			SCROLL_HINT_MODE_ALL,
			SCROLL_HINT_MODE_TOP_AND_LEFT,
			SCROLL_HINT_MODE_BOTTOM_AND_RIGHT,
		};

	private:
		enum ClipKeyEditEdge {
			CLIP_KEY_EDIT_EDGE_NONE,
			CLIP_KEY_EDIT_EDGE_HEAD,
			CLIP_KEY_EDIT_EDGE_TAIL,
		};

		Color background_color;
		Color separator_color = Color("#696969");
		float separator_width = -1.0f;
		bool enable_row_alternate_color = false;
		Color row_alternate_color;

		float header_width = 0.0f;
		float header_height = 16.0f;
		double duration = 0.0;
		double current_time = 0.0;
		float scale = 32.0f;
		CountingUnit counting_unit = TIME;

		Ref<TimelineIndicator> playhead;
		TypedArray<TimelineMarker> markers;
		Ref<TimelineTimeRuler> time_ruler;
		TypedArray<TimelineTrack> tracks;

		float hscroll_value = 0.0f;
		float vscroll_value = 0.0f;
		float content_height = 0.0f;
		bool updating_scroll = false;

		void _update_scroll_bar();
		void _h_scroll_changed(double p_value);
		void _v_scroll_changed(double p_value);

		TimeFormat time_format = MM_SS_MS;
		bool show_milliseconds = false;

		int fps = 24;
		bool show_subdivision = true;

		struct BPM {
			float time;
			int bpms;

			BPM(float p_time, int p_bpm) : time(p_time), bpms(p_bpm) {}
		};

		Dictionary bpms;
		int beats_per_bar = 4;
		Color beat_line_color = Color("#ff7384");
		float beat_line_width = -1.0f;
		Color bar_line_color = Color("#5f8fc9");
		float bar_line_width = -1.0f;
		BarNumberDirection bar_number_direction = BAR_NUMBER_TOP_DOWN;
		BeatFormat beat_format = BEAT_BAR;

		HScrollBar* hscroll = nullptr;
		VScrollBar* vscroll = nullptr;
		float content_width = 0.0f;
		ScrollMode horizontal_scroll_mode = SCROLL_MODE_AUTO;
		ScrollMode vertical_scroll_mode = SCROLL_MODE_AUTO;
		int deadzone = 0;
		bool draw_minimap = true;
		int minimap_width = 80;
		bool minimap_dragging = false;
		bool minimap_dragging_viewport = false;
		bool minimap_key_cache_dirty = true;
		double minimap_drag_scroll_origin = 0.0;
		float minimap_drag_y_origin = 0.0f;

		Vector2 drag_speed;
		Vector2 drag_accum;
		Vector2 drag_from;
		Vector2 last_drag_accum;
		float time_since_motion = 0.0f;
		bool drag_touching = false;
		bool drag_touching_deaccel = false;
		bool beyond_deadzone = false;
		bool scroll_on_drag_hover = false;

		bool selecting = false;
		bool select_pending = false;
		bool right_selecting = false;
		bool key_dragging = false;
		bool key_drag_moved = false;
		bool allow_key_cross_track_move = true;
		bool key_snap_enabled = true;
		bool clip_key_edge_edit_enabled = true;
		bool allow_unselected_key_edit = true;
		bool allow_right_mouse_selection = false;
		bool clip_key_edge_dragging = false;
		bool clip_key_edge_drag_moved = false;
		float long_press_time = 0.4f;
		float select_timer = 0.0f;
		Vector2 select_start;
		Vector2 select_end;
		Vector2 right_select_start;
		Vector2 right_select_end;
		double key_drag_start_value = 0.0;
		int key_drag_anchor_track = -1;
		TimelineTrackKey *clip_key_edge_drag_key = nullptr;
		ClipKeyEditEdge clip_key_edge_drag_edge = CLIP_KEY_EDIT_EDGE_NONE;
		double clip_key_edge_drag_head_time = 0.0;
		double clip_key_edge_drag_tail_time = 0.0;

		struct DraggedKey {
			TimelineTrackKey* key = nullptr;
			int original_track_index = -1;
			int current_track_index = -1;
			double original_time = 0.0;
		};
		std::vector<DraggedKey> dragged_keys;

		struct MinimapTrackCache {
			std::vector<uint8_t> key_rows;
			std::vector<uint8_t> selected_key_rows;
		};
		std::vector<MinimapTrackCache> minimap_key_cache;
		int minimap_key_cache_height = 0;

		struct ResizedClipKey {
			TimelineTrackKey *key = nullptr;
			double original_head_time = 0.0;
			double original_tail_time = 0.0;
		};
		std::vector<ResizedClipKey> resized_clip_keys;
		std::unordered_set<const TimelineTrackKey *> key_release_preview_keys;

		bool playhead_dragging = false;

		struct CachedTrack;

		void _collect_selected_keys();
		Rect2 _make_selection_rect(const Vector2 &p_start, const Vector2 &p_end) const;
		TypedArray<TimelineTrackKey> _get_keys_in_rect(const Rect2 &p_rect) const;
		bool _try_handle_selection_rect(const Rect2 &p_rect, const TypedArray<TimelineTrackKey> &p_keys, int p_mouse_button);
		void _finish_right_mouse_selection();
		bool _find_selected_key_at_position(const Vector2& p_position, int& r_track_index, TimelineTrackKey*& r_key) const;
		int _get_track_index_at_x(float p_x) const;
		int _get_track_header_index_at_x(float p_x) const;
		void _select_track_keys(int p_track_index);
		void _update_selection_auto_scroll(double p_delta);
		void _stop_internal_process_if_idle();
		double _position_to_key_value(double p_y) const;
		double _get_playhead_drag_time(double p_y) const;
		bool _find_clip_key_edge_at_position(const Vector2 &p_position, int &r_track_index, TimelineTrackKey *&r_key, ClipKeyEditEdge &r_edge) const;
		void _update_clip_key_edge_cursor(const Vector2 &p_position);
		void _begin_clip_key_edge_drag(TimelineTrackKey *p_key, ClipKeyEditEdge p_edge);
		void _update_clip_key_edge_drag(const Vector2 &p_position);
		void _finish_clip_key_edge_drag();
		void _begin_key_drag(int p_track_index, TimelineTrackKey* p_key, const Vector2& p_position);
		void _update_key_drag(const Vector2& p_position);
		void _finish_key_drag();
		void _move_key_to_track(TimelineTrackKey* p_key, int p_from_track, int p_to_track);
		double _snap_key_time(double p_time) const;
		bool _keys_overlap(const TimelineTrackKey* p_a, const TimelineTrackKey* p_b) const;
		bool _has_key_overlap_in_track(const CachedTrack &p_track, const TimelineTrackKey *p_key) const;
		std::vector<TimelineTrackKey *> _get_moved_key_overlaps(const std::vector<TimelineTrackKey *> &p_moved_keys) const;
		void _update_key_release_preview(const std::vector<TimelineTrackKey *> &p_moved_keys);
		void _clear_key_release_preview();
		bool _is_key_release_previewed(const TimelineTrackKey *p_key) const;
		void _destroy_moved_key_overlaps(const std::vector<TimelineTrackKey*>& p_moved_keys);

		void _scroll(ScrollBar* p_scroll, double p_amount);
		void _scroll_to(ScrollBar* p_scroll, double p_pos);
		void _cancel_drag();

		void _draw_header(const Point2& pos, const float width, Ref<StyleBox> header_bg, Ref<Texture2D> header_icon);
		void _draw_time_ruler_ticks(float p_header_width);
		void _draw_grid_beat(float p_header_width);
		void _draw_grid_frame(float p_header_width);
		void _draw_grid_time(float p_header_width);
		bool _is_minimap_visible() const;
		Rect2 _get_minimap_rect() const;
		Rect2 _get_minimap_viewport_rect() const;
		double _indicator_time_to_content_y(double p_time) const;
		double _content_y_to_minimap_y(const Rect2 &p_rect, double p_content_y) const;
		void _mark_minimap_key_cache_dirty();
		void _rebuild_minimap_key_cache(int p_height);
		void _draw_minimap();
		bool _begin_minimap_drag(const Vector2 &p_position);
		void _update_minimap_drag(const Vector2 &p_position);
		void _finish_minimap_drag();
		void _scroll_minimap_to_position(float p_y);
		float _calculate_header_width() const;
		float _calculate_grid_height() const;

		Array beat_map;
		Array time_map;
		double _beat_total = 0;
		int _row_total = 0;

		void _build_time_to_beat_map();
		void _build_beat_to_time_map();
		void _calculate_beat_total();
		void _calculate_row_total();
		double _time_to_y(double p_time) const;
		double _y_to_time(double p_y) const;
		double _time_to_beat(double p_time) const;
		double _beat_to_time(double p_beat) const;
		double _beat_to_y(double p_beat) const;
		double _y_to_beat(double p_y) const;
		double _frame_to_y(int64_t p_frame) const;
		int64_t _y_to_frame(double p_y) const;

		void _on_resource_changed();
		struct CachedTrack {
			float x_offset = 0.0f;
			float width = 0.0f;
			std::vector<TimelineTrackKey*> keys;
			double max_key_length = 0.0;
			float max_instant_key_scale = 0.4f;
		};
		std::vector<CachedTrack> _track_cache;
		void _rebuild_track_cache();
		void _refresh_track_key_metrics();
		void _get_visible_key_time_range(float p_y_margin, double& r_start, double& r_end) const;
		double _key_to_y(const TimelineTrackKey* p_key) const;
		double _key_end_to_y(const TimelineTrackKey* p_key) const;
		float _get_instant_key_scale(const TimelineTrackKey* p_key) const;
		Rect2 _get_instant_key_rect(const CachedTrack& p_track, const TimelineTrackKey* p_key, double p_y) const;
		Rect2 _get_clip_key_rect(const CachedTrack& p_track, double p_y, double p_y_end) const;
		Ref<StyleBox> _get_instant_key_normal_style(const TimelineTrackKey *p_key) const;
		Ref<StyleBox> _get_instant_key_selected_style(const TimelineTrackKey *p_key) const;
		Ref<StyleBox> _get_clip_key_normal_style(const TimelineTrackKey *p_key) const;
		Ref<StyleBox> _get_clip_key_selected_style(const TimelineTrackKey *p_key) const;
		Ref<StyleBox> _get_key_release_preview_style() const;
		String _format_indicator_time(double p_time) const;

		struct StyleCache {
			Ref<StyleBox> instant_key_normal;
			Ref<StyleBox> instant_key_normal_fallback;
			Ref<StyleBox> instant_key_selected;
			Ref<StyleBox> instant_key_selected_fallback;
			Ref<StyleBox> clip_key_normal;
			Ref<StyleBox> clip_key_normal_fallback;
			Ref<StyleBox> clip_key_selected;
			Ref<StyleBox> clip_key_selected_fallback;
			Ref<StyleBox> selection_rect;
			Ref<StyleBox> selection_rect_fallback;
			Ref<StyleBox> key_release_preview;
			Ref<StyleBox> key_release_preview_fallback;

			float icon_max_width = 0.0f;
			float instant_key_scale = 0.4f;
		} style_cache;

	protected:
		static void _bind_methods();
		void _notification(int p_what);
		void _validate_property(PropertyInfo& p_property) const;

		GDVIRTUAL3RC(bool, _should_handle_selection_rect, Rect2, TypedArray<TimelineTrackKey>, int)
		GDVIRTUAL3(_handle_selection_rect, Rect2, TypedArray<TimelineTrackKey>, int)

	public:
		VTimelinePanel();
		~VTimelinePanel();

		virtual Vector2 _get_minimum_size() const override;
		virtual void _gui_input(const Ref<InputEvent>& p_gui_input) override;
		virtual String _get_tooltip(const Vector2& p_at_position) const override;

		TimelineTrackKey *create_key(int p_track_index, double p_time, double p_length = 0.0, bool p_snap = false);
		void remove_key(int p_track_index, int p_key_index);
		void clear_track_keys(int p_track_index);
		void clear_all_keys();
		int get_key_count(int p_track_index) const;
		TimelineTrackKey* get_key(int p_track_index, int p_key_index) const;
		TypedArray<TimelineTrackKey> find_keys(int p_track_index, double p_start_time, double p_end_time) const;

		double get_time_from_position(const double p_position) const;
		double get_frame_from_position(const double p_position) const;
		double get_beat_from_position(const double p_position) const;
		double get_position_from_time(double p_time) const;
		double get_position_from_frame(int64_t p_frame) const;
		double get_position_from_beat(double p_beat) const;

		void set_background_color(const Color& p_background_color);
		Color get_background_color() const;

		void set_separator_color(const Color& p_separator_color);
		Color get_separator_color() const;

		void set_separator_width(const float p_width);
		float get_separator_width() const;

		void set_header_height(const float p_height);
		float get_header_height() const;

		void set_duration(const double p_duration);
		double get_duration() const;

		void set_current_time(const double p_current_time);
		double get_current_time() const;

		void set_scale(const float p_scale);
		float get_scale() const;

		void set_counting_unit(CountingUnit p_unit);
		CountingUnit get_counting_unit() const;

		void set_time_format(TimeFormat p_time_format);
		TimeFormat get_time_format() const;

		void set_show_milliseconds(const bool p_show_milliseconds);
		bool get_show_milliseconds() const;

		void set_fps(const int p_fps);
		int get_fps() const;

		void set_show_subdivision(const bool p_show_subdivision);
		bool get_show_subdivision() const;

		void set_bpms(const Dictionary& p_bpms);
		Dictionary get_bpms() const;

		void set_beat_per_bar(const int p_beats_per_bar);
		int get_beat_per_bar() const;

		void set_bar_line_color(const Color& p_color);
		Color get_bar_line_color() const;

		void set_bar_line_width(const float p_width);
		float get_bar_line_width() const;

		void set_beat_line_color(const Color& p_color);
		Color get_beat_line_color() const;

		void set_beat_line_width(const float p_width);
		float get_beat_line_width() const;

		void set_bar_number_direction(BarNumberDirection p_direction);
		BarNumberDirection get_bar_number_direction() const;

		void set_beat_format(BeatFormat p_format);
		BeatFormat get_beat_format() const;

		void set_playhead(Ref<TimelineIndicator> p_playhead);
		Ref<TimelineIndicator> get_playhead() const;

		void set_markers(const TypedArray<TimelineMarker>& p_markers);
		TypedArray<TimelineMarker> get_markers() const;

		void set_time_ruler(Ref<TimelineTimeRuler> p_time_ruler);
		Ref<TimelineTimeRuler> get_time_ruler() const;

		void set_tracks(const TypedArray<TimelineTrack>& p_tracks);
		TypedArray<TimelineTrack> get_tracks() const;

		void set_h_scroll(int p_pos);
		int get_h_scroll() const;

		void set_v_scroll(int p_pos);
		int get_v_scroll() const;

		void set_horizontal_custom_step(float p_custom_step);
		float get_horizontal_custom_step() const;

		void set_vertical_custom_step(float p_custom_step);
		float get_vertical_custom_step() const;

		void set_horizontal_scroll_mode(ScrollMode p_mode);
		ScrollMode get_horizontal_scroll_mode() const;

		void set_vertical_scroll_mode(ScrollMode p_mode);
		ScrollMode get_vertical_scroll_mode() const;

		void set_deadzone(int p_deadzone);
		int get_deadzone() const;

		void set_draw_minimap(bool p_enabled);
		bool is_drawing_minimap() const;

		void set_minimap_width(int p_width);
		int get_minimap_width() const;

		void set_icon_max_width(const float p_width);
		float get_icon_max_width() const;

		void set_instant_key_scale(const float p_scale);
		float get_instant_key_scale() const;

		void set_instant_key_normal_style(Ref<StyleBox> p_style);
		Ref<StyleBox> get_instant_key_normal_style() const;

		void set_instant_key_selected_style(Ref<StyleBox> p_style);
		Ref<StyleBox> get_instant_key_selected_style() const;

		void set_clip_key_normal_style(Ref<StyleBox> p_style);
		Ref<StyleBox> get_clip_key_normal_style() const;

		void set_clip_key_selected_style(Ref<StyleBox> p_style);
		Ref<StyleBox> get_clip_key_selected_style() const;

		void set_selection_rect_style(Ref<StyleBox> p_style);
		Ref<StyleBox> get_selection_rect_style() const;

		void set_key_release_preview_style(Ref<StyleBox> p_style);
		Ref<StyleBox> get_key_release_preview_style() const;

		void set_allow_key_cross_track_move(bool p_enabled);
		bool get_allow_key_cross_track_move() const;

		void set_key_snap_enabled(bool p_enabled);
		bool get_key_snap_enabled() const;

		void set_clip_key_edge_edit_enabled(bool p_enabled);
		bool get_clip_key_edge_edit_enabled() const;

		void set_allow_unselected_key_edit(bool p_enabled);
		bool get_allow_unselected_key_edit() const;

		void set_allow_right_mouse_selection(bool p_enabled);
		bool get_allow_right_mouse_selection() const;
	};
}

VARIANT_ENUM_CAST(VTimelinePanel::CountingUnit);
VARIANT_ENUM_CAST(VTimelinePanel::TimeFormat);
VARIANT_ENUM_CAST(VTimelinePanel::BeatFormat);
VARIANT_ENUM_CAST(VTimelinePanel::BarNumberDirection);
VARIANT_ENUM_CAST(VTimelinePanel::ScrollMode);

#endif // !V_TIMELINE_PANEL_H

