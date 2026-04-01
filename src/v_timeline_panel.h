#ifndef V_TIMELINE_PANEL_H
#define V_TIMELINE_PANEL_H

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/v_scroll_bar.hpp>

namespace godot {
	class TimelinePanelPlayheadComponent;
	class TimelinePanelTimeRulerComponent;
	class TimelinePanelTrackComponent;

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

		Ref<TimelinePanelPlayheadComponent> playhead_component;
		Ref<TimelinePanelTimeRulerComponent> time_ruler_component;
		TypedArray<TimelinePanelTrackComponent> track_components;

		VScrollBar* vscroll = nullptr;
		float scroll_value = 0.0f;
		float content_height = 0.0f;
		bool updating_scroll = false;

		void _update_scroll_bar();
		void _scroll_changed(double p_value);

		// 使用时间作为计数单位
		TimeFormat time_format = MM_SS_MS;
		bool show_milliseconds = false;

		// 使用帧作为计数单位
		int fps = 24;
		bool show_subdivision = true;

		// 使用节拍作为计数单位
		int bpm = 120;
		int beats_per_bar = 4;
		Color bar_line_color = Color("#ff7384");
		float bar_line_width = 1.0f;
		Color beat_line_color = Color("#5f8fc9");
		float beat_line_width = -1.0f;
		BarNumberDirection bar_number_direction = BAR_NUMBER_TOP_DOWN;
		
		void _draw_header(const Point2& pos, const float width, const Color& header_color, Ref<Texture2D> header_icon);
		void _draw_time_ruler_ticks(float p_header_width);
		void _draw_playhead(
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
		
		// 时间/拍/帧 与像素位置的转换
		float _time_to_y(double p_time) const;
		double _y_to_time(float p_y) const;
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

		void set_bpm(const int p_bpm);
		int get_bpm() const;

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

		void set_playhead_component(Ref<TimelinePanelPlayheadComponent> p_playhead_component);
		Ref<TimelinePanelPlayheadComponent> get_playhead_component() const;

		void set_time_ruler_component(Ref<TimelinePanelTimeRulerComponent> p_time_ruler_component);
		Ref<TimelinePanelTimeRulerComponent> get_time_ruler_component() const;

		void set_track_components(const TypedArray<TimelinePanelTrackComponent>& p_track_components);
		TypedArray<TimelinePanelTrackComponent> get_track_components() const;
	};
}

VARIANT_ENUM_CAST(VTimelinePanel::CountingUnit);
VARIANT_ENUM_CAST(VTimelinePanel::TimeFormat);
VARIANT_ENUM_CAST(VTimelinePanel::BarNumberDirection);

#endif // !V_TIMELINE_PANEL_H

