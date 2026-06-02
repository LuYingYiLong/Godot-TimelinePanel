#ifndef TIMELINE_CONNECTION_EDITOR_H
#define TIMELINE_CONNECTION_EDITOR_H

#include "components/timeline_connection.h"
#include "components/timeline_indicator.h"
#include "components/timeline_marker.h"

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/h_scroll_bar.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/style_box.hpp>
#include <godot_cpp/classes/v_scroll_bar.hpp>
#include <godot_cpp/core/gdvirtual.gen.inc>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include <vector>

namespace godot {
	class TimelineConnectionEditor : public Control {
		GDCLASS(TimelineConnectionEditor, Control)

	public:
		enum CountingUnit {
			TIME,
			FRAME,
			BEAT,
		};

	private:
		enum DragTarget {
			DRAG_TARGET_NONE,
			DRAG_TARGET_POINT,
			DRAG_TARGET_IN_HANDLE,
			DRAG_TARGET_OUT_HANDLE,
		};

		TypedArray<TimelineConnection> connections;
		Vector2 content_offset = Vector2();
		Vector2 content_scale = Vector2(1.0f, 1.0f);
		Vector2 scroll_offset = Vector2();
		double current_time = 0.0;
		CountingUnit counting_unit = TIME;
		int fps = 24;
		bool key_snap_enabled = false;
		bool center_short_vertical_range = false;
		int beat_per_bar = 4;
		struct BPM {
			double time = 0.0;
			double beat = 0.0;
			double bpm = 120.0;
			bool has_beat = false;

			BPM() = default;
			BPM(double p_time, double p_beat, double p_bpm, bool p_has_beat = true) :
					time(p_time), beat(p_beat), bpm(p_bpm), has_beat(p_has_beat) {}
		};
		Dictionary bpms;
		Ref<TimelineIndicator> playhead;
		TypedArray<TimelineMarker> markers;
		bool playhead_drag_enabled = true;
		bool playhead_dragging = false;
		float scroll_step = 64.0f;
		bool scroll_enabled = true;
		bool updating_scroll = false;
		bool middle_mouse_panning = false;
		Vector2 middle_mouse_pan_last_position;
		bool selecting = false;
		bool selection_additive = false;
		Vector2 select_start;
		Vector2 select_end;
		bool edit_enabled = true;
		float handle_radius = 4.0f;
		bool ruler_enabled = true;
		Vector2 ruler_size = Vector2(48.0f, 22.0f);
		float ruler_min_tick_spacing = 56.0f;
		int ruler_font_size = 11;
		bool highlight_zero_tick = true;
		Color ruler_background_color = Color(0.13f, 0.13f, 0.13f, 0.92f);
		Color ruler_tick_color = Color(1.0f, 1.0f, 1.0f, 0.42f);
		Color ruler_text_color = Color(1.0f, 1.0f, 1.0f, 0.72f);
		Color ruler_major_grid_color = Color(1.0f, 1.0f, 1.0f, 0.16f);
		Color ruler_minor_grid_color = Color(1.0f, 1.0f, 1.0f, 0.055f);
		bool range_limited = false;
		Vector2 range_min = Vector2();
		Vector2 range_max = Vector2(1024.0f, 1024.0f);

		struct StyleCache {
			Ref<StyleBox> key_normal;
			Ref<StyleBox> key_normal_fallback;
			Ref<StyleBox> key_selected;
			Ref<StyleBox> key_selected_fallback;
			Ref<StyleBox> handle_normal;
			Ref<StyleBox> handle_normal_fallback;
			Ref<StyleBox> handle_selected;
			Ref<StyleBox> handle_selected_fallback;
			Ref<StyleBox> selection_rect;
			Ref<StyleBox> selection_rect_fallback;
			float key_scale = 1.0f;
			float handle_scale = 1.0f;
			float handle_line_width = 1.0f;
			Color handle_line_color = Color(1.0f, 1.0f, 1.0f, 0.44f);
			Color handle_line_selected_color = Color(1.0f, 0.78f, 0.24f, 0.95f);
			Color zero_tick_color = Color(1.0f, 0.78f, 0.24f, 0.95f);
			float bezier_line_width = 2.0f;
		} style_cache;

		Ref<TimelineConnection> dragged_connection;
		HScrollBar* hscroll = nullptr;
		VScrollBar* vscroll = nullptr;
		DragTarget drag_target = DRAG_TARGET_NONE;
		int drag_point_index = -1;
		Vector2 drag_content_offset;

		Vector2 _get_content_origin() const;
		float _get_vertical_range_center_offset(const Vector2 &p_base_origin) const;
		Rect2 _get_content_rect() const;
		Rect2 _get_screen_range_rect() const;
		Rect2 _get_draw_clip_rect() const;
		Vector2 _content_to_screen(const Vector2 &p_position) const;
		Vector2 _screen_to_content(const Vector2 &p_position) const;
		std::vector<BPM> _get_sorted_bpms() const;
		double _time_to_beat(double p_time) const;
		double _beat_to_time(double p_beat) const;
		double _time_to_unit(double p_time) const;
		double _unit_to_time(double p_value) const;
		double _snap_time(double p_time) const;
		String _format_playhead_time(double p_time) const;
		String _format_ruler_x_value(double p_value) const;
		Vector2 _clamp_content_position(const Vector2 &p_position) const;
		Vector2 _clamp_scroll_offset(const Vector2 &p_offset) const;
		bool _clip_line_to_rect(Vector2 &r_from, Vector2 &r_to, const Rect2 &p_rect) const;
		void _draw_clipped_line(const Vector2 &p_from, const Vector2 &p_to, const Color &p_color, float p_width, const Rect2 &p_rect);
		void _draw_clipped_circle(const Vector2 &p_position, float p_radius, const Color &p_color, const Rect2 &p_rect);
		void _draw_clipped_style_box(const Vector2 &p_position, float p_size, const Ref<StyleBox> &p_style, const Rect2 &p_rect);
		Rect2 _make_selection_rect(const Vector2 &p_start, const Vector2 &p_end) const;
		bool _collect_selected_points(const Rect2 &p_rect, bool p_additive);
		Ref<StyleBox> _get_key_normal_style() const;
		Ref<StyleBox> _get_key_selected_style() const;
		Ref<StyleBox> _get_handle_normal_style() const;
		Ref<StyleBox> _get_handle_selected_style() const;
		Ref<StyleBox> _get_selection_rect_style() const;
		float _get_smart_scroll_step(bool p_horizontal) const;
		void _scroll(const Vector2 &p_delta);
		void _zoom_at_position(const Vector2 &p_position, float p_factor);
		void _pan_view(const Vector2 &p_screen_delta);
		bool _wrap_middle_mouse_pan_position(Vector2 &r_position);
		void _update_selection_auto_scroll(double p_delta);
		void _update_scroll_bars();
		void _h_scroll_changed(double p_value);
		void _v_scroll_changed(double p_value);
		bool _is_playhead_hit_at_position(const Vector2 &p_position) const;
		bool _is_playhead_drag_area_at_position(const Vector2 &p_position) const;
		double _get_time_at_position(const Vector2 &p_position) const;
		TypedArray<TimelineConnectionPoint> _get_selected_points() const;
		Dictionary _get_point_metadata(const Ref<TimelineConnection> &p_connection, int p_point_index) const;
		bool _is_segment_bezier(const Ref<TimelineConnection> &p_connection, int p_point_index) const;
		bool _is_in_handle_visible(const Ref<TimelineConnection> &p_connection, int p_point_index) const;
		bool _is_out_handle_visible(const Ref<TimelineConnection> &p_connection, int p_point_index) const;
		bool _get_segment_samples(const Ref<TimelineConnection> &p_connection, int p_point_index, std::vector<Vector2> &r_samples) const;
		bool _clear_point_selection();
		void _select_point(const Ref<TimelineConnection> &p_connection, int p_point_index, bool p_additive, bool p_toggle);
		void _emit_selection_changed();
		bool _find_edit_target_at_position(const Vector2 &p_position, Ref<TimelineConnection> &r_connection, DragTarget &r_target, int &r_point_index, Vector2 &r_content_offset) const;
		void _update_drag(const Vector2 &p_position);
		void _finish_drag();
		void _update_hover_cursor(const Vector2 &p_position);
		void _draw_ruler_guides();
		void _draw_ruler_overlays();
		void _draw_connections();
		void _draw_selection_rect();
		void _draw_playhead(bool p_draw_line);
		void _draw_marker(const Ref<TimelineMarker>& p_marker, bool p_draw_line);
		void _draw_indicators(bool p_draw_line);
		void _connect_connection_changed(const Ref<TimelineConnection> &p_connection);
		void _disconnect_connection_changed(const Ref<TimelineConnection> &p_connection);
		void _on_resource_changed();

	protected:
		static void _bind_methods();
		void _validate_property(PropertyInfo &p_property) const;
		void _notification(int p_what);

		GDVIRTUAL4RC(bool, _draw_connection, RID, Ref<TimelineConnection>, Vector2, Vector2)

	public:
		TimelineConnectionEditor();
		virtual void _gui_input(const Ref<InputEvent> &p_gui_input) override;

		HScrollBar* get_h_scroll_bar() const;
		VScrollBar* get_v_scroll_bar() const;

		Ref<TimelineConnection> create_connection(const Vector2 &p_from_position = Vector2(), const Vector2 &p_to_position = Vector2(160.0f, 80.0f));
		void add_connection(const Ref<TimelineConnection> &p_connection);
		void remove_connection(const Ref<TimelineConnection> &p_connection);
		void remove_connection_at(int p_index);
		void clear_connections();
		TypedArray<TimelineConnection> get_connections() const;
		Ref<TimelineConnection> get_connection(int p_index) const;
		int get_connection_count() const;
		int get_connection_index(const Ref<TimelineConnection> &p_connection) const;

		void set_content_offset(const Vector2 &p_offset);
		Vector2 get_content_offset() const;

		void set_content_scale(const Vector2 &p_scale);
		Vector2 get_content_scale() const;

		void set_current_time(double p_time);
		double get_current_time() const;

		void set_counting_unit(CountingUnit p_unit);
		CountingUnit get_counting_unit() const;

		void set_fps(int p_fps);
		int get_fps() const;

		void set_beat_per_bar(int p_num);
		int get_beat_per_bar() const;

		void set_bpms(const Dictionary &p_bpms);
		Dictionary get_bpms() const;

		void set_playhead(Ref<TimelineIndicator> p_playhead);
		Ref<TimelineIndicator> get_playhead() const;

		void set_markers(const TypedArray<TimelineMarker>& p_markers);
		TypedArray<TimelineMarker> get_markers() const;

		void set_playhead_drag_enabled(bool p_enabled);
		bool is_playhead_drag_enabled() const;

		void set_key_snap_enabled(bool p_enabled);
		bool is_key_snap_enabled() const;

		void set_center_short_vertical_range(bool p_enabled);
		bool is_short_vertical_range_centered() const;

		TypedArray<TimelineConnectionPoint> get_selected_points() const;

		double get_time_from_position(double p_position) const;
		double get_frame_from_position(double p_position) const;
		double get_beat_from_position(double p_position) const;
		double get_position_from_time(double p_time) const;
		double get_position_from_frame(int64_t p_frame) const;
		double get_position_from_beat(double p_beat) const;

		void set_scroll_offset(const Vector2 &p_offset);
		Vector2 get_scroll_offset() const;

		void set_scroll_step(float p_step);
		float get_scroll_step() const;

		void set_scroll_enabled(bool p_enabled);
		bool is_scroll_enabled() const;

		void set_edit_enabled(bool p_enabled);
		bool is_edit_enabled() const;

		void set_handle_radius(float p_radius);
		float get_handle_radius() const;

		void set_ruler_enabled(bool p_enabled);
		bool is_ruler_enabled() const;

		void set_ruler_size(const Vector2 &p_size);
		Vector2 get_ruler_size() const;

		void set_ruler_min_tick_spacing(float p_spacing);
		float get_ruler_min_tick_spacing() const;

		void set_ruler_font_size(int p_size);
		int get_ruler_font_size() const;

		void set_highlight_zero_tick(bool p_enabled);
		bool is_highlighting_zero_tick() const;

		void set_ruler_background_color(const Color &p_color);
		Color get_ruler_background_color() const;

		void set_ruler_tick_color(const Color &p_color);
		Color get_ruler_tick_color() const;

		void set_ruler_text_color(const Color &p_color);
		Color get_ruler_text_color() const;

		void set_ruler_major_grid_color(const Color &p_color);
		Color get_ruler_major_grid_color() const;

		void set_ruler_minor_grid_color(const Color &p_color);
		Color get_ruler_minor_grid_color() const;

		void set_range_limited(bool p_limited);
		bool is_range_limited() const;

		void set_range_min(const Vector2 &p_min);
		Vector2 get_range_min() const;

		void set_range_max(const Vector2 &p_max);
		Vector2 get_range_max() const;

		void set_range_start_time(double p_time);
		double get_range_start_time() const;

		void set_range_end_time(double p_time);
		double get_range_end_time() const;

		void set_range_min_y(float p_y);
		float get_range_min_y() const;

		void set_range_max_y(float p_y);
		float get_range_max_y() const;

		void set_key_scale(float p_scale);
		float get_key_scale() const;

		void set_key_normal_style(Ref<StyleBox> p_style);
		Ref<StyleBox> get_key_normal_style() const;

		void set_key_selected_style(Ref<StyleBox> p_style);
		Ref<StyleBox> get_key_selected_style() const;

		void set_handle_scale(float p_scale);
		float get_handle_scale() const;

		void set_handle_normal_style(Ref<StyleBox> p_style);
		Ref<StyleBox> get_handle_normal_style() const;

		void set_handle_selected_style(Ref<StyleBox> p_style);
		Ref<StyleBox> get_handle_selected_style() const;

		void set_selection_rect_style(Ref<StyleBox> p_style);
		Ref<StyleBox> get_selection_rect_style() const;

		void set_handle_style(Ref<StyleBox> p_style);
		Ref<StyleBox> get_handle_style() const;

		void set_handle_line_width(float p_width);
		float get_handle_line_width() const;

		void set_handle_line_color(const Color &p_color);
		Color get_handle_line_color() const;

		void set_handle_line_selected_color(const Color &p_color);
		Color get_handle_line_selected_color() const;

		void set_zero_tick_color(const Color &p_color);
		Color get_zero_tick_color() const;

		void set_bezier_line_width(float p_width);
		float get_bezier_line_width() const;
	};
}

VARIANT_ENUM_CAST(TimelineConnectionEditor::CountingUnit);

#endif // TIMELINE_CONNECTION_EDITOR_H
