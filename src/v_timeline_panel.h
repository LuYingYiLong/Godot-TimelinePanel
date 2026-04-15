#ifndef V_TIMELINE_PANEL_H
#define V_TIMELINE_PANEL_H

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/h_scroll_bar.hpp>
#include <godot_cpp/classes/v_scroll_bar.hpp>

namespace godot {
	class TimelinePanelIndicator;
	class TimelinePanelMarker;
	class TimelinePanelTimeRuler;
	class TimelinePanelTrack;

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

		Ref<TimelinePanelIndicator> playhead;
		TypedArray<TimelinePanelMarker> markers;
		Ref<TimelinePanelTimeRuler> time_ruler;
		TypedArray<TimelinePanelTrack> tracks;

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
		float beat_line_width = 1.0f;
		Color bar_line_color = Color("#5f8fc9");
		float bar_line_width = -1.0f;
		BarNumberDirection bar_number_direction = BAR_NUMBER_TOP_DOWN;
		
		HScrollBar* hscroll = nullptr;
		VScrollBar* vscroll = nullptr;
		float content_width = 0.0f;
		ScrollMode horizontal_scroll_mode = SCROLL_MODE_AUTO;
		ScrollMode vertical_scroll_mode = SCROLL_MODE_AUTO;
		int deadzone = 0;

		Vector2 drag_speed;
		Vector2 drag_accum;
		Vector2 drag_from;
		Vector2 last_drag_accum;
		float time_since_motion = 0.0f;
		bool drag_touching = false;
		bool drag_touching_deaccel = false;
		bool beyond_deadzone = false;
		bool scroll_on_drag_hover = false;

		void _scroll(ScrollBar* p_scroll, double p_amount);
		void _scroll_to(ScrollBar* p_scroll, double p_pos);
		void _cancel_drag();

		void _draw_header(const Point2& pos, const float width, Ref<StyleBox> header_bg, Ref<Texture2D> header_icon);
		void _draw_time_ruler_ticks(float p_header_width);
		void _draw_indicator(
			const double time,
			const PackedVector2Array& points,
			const PackedColorArray& colors,
			const String& text,
			const Ref<Font> font,
			const Vector2& font_pos,
			const int64_t font_size,
			const Color& font_color,
			const bool show_line,
			const float line_width,
			const Color& line_color
		);
		void _draw_grid_beat(float p_header_width);
		void _draw_grid_frame(float p_header_width);
		void _draw_grid_time(float p_header_width);
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
		float _time_to_y(double p_time) const;
		double _y_to_time(float p_y) const;
		double _time_to_beat(double p_time) const;
		double _beat_to_time(double p_beat) const;
		float _beat_to_y(double p_beat) const;
		double _y_to_beat(float p_y) const;
		float _frame_to_y(int64_t p_frame) const;
		int64_t _y_to_frame(float p_y) const;

		void _on_resource_changed();

	protected:
		static void _bind_methods();
		void _notification(int p_what);
		void _validate_property(PropertyInfo& p_property) const;

	public:
		VTimelinePanel();

		virtual Vector2 _get_minimum_size() const override;
		virtual void _gui_input(const Ref<InputEvent>& p_gui_input) override;
		virtual String _get_tooltip(const Vector2& p_at_position) const override;

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

		void set_playhead(Ref<TimelinePanelIndicator> p_playhead);
		Ref<TimelinePanelIndicator> get_playhead() const;

		void set_markers(const TypedArray<TimelinePanelMarker>& p_markers);
		TypedArray<TimelinePanelMarker> get_markers() const;

		void set_time_ruler(Ref<TimelinePanelTimeRuler> p_time_ruler);
		Ref<TimelinePanelTimeRuler> get_time_ruler() const;

		void set_tracks(const TypedArray<TimelinePanelTrack>& p_tracks);
		TypedArray<TimelinePanelTrack> get_tracks() const;

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
	};
}

VARIANT_ENUM_CAST(VTimelinePanel::CountingUnit);
VARIANT_ENUM_CAST(VTimelinePanel::TimeFormat);
VARIANT_ENUM_CAST(VTimelinePanel::BarNumberDirection);
VARIANT_ENUM_CAST(VTimelinePanel::ScrollMode);

#endif // !V_TIMELINE_PANEL_H

