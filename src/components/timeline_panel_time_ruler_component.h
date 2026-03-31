#ifndef TIMELINE_PANEL_TIME_RULER_COMPONENT_H
#define TIMELINE_PANEL_TIME_RULER_COMPONENT_H

#include "timeline_panel_base_component.h"
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/core/gdvirtual.gen.inc>

namespace godot {
	class TimelinePanelTimeRulerComponent : public TimelinePanelBaseComponent {
		GDCLASS(TimelinePanelTimeRulerComponent, TimelinePanelBaseComponent)

	private:
		float width = 64.0f;
		Color header_color = Color("#ff7931");
		Ref<Texture2D> header_icon;
		float major_tick_height = 20.0f;
		float major_tick_width = -1.0f;
		float minor_tick_height = 10.0f;
		float minor_tick_width = -1.0f;
		Color tick_color = Color("#696969");
		bool show_minor_ticks = true;

		Color playhead_color = Color("#ffffff");
		Color playhead_text_color = Color("#000000");
		float playhead_line_width = 2.0f;

	protected:
		static void _bind_methods();

	public:
		void set_width(const float p_width);
		float get_width() const;

		void set_header_icon(Ref<Texture2D> p_header_icon);
		Ref<Texture2D> get_header_icon() const;

		void set_header_color(const Color& p_color);
		Color get_header_color() const;

		void set_major_tick_height(const float p_height);
		float get_major_tick_height() const;

		void set_major_tick_width(const float p_width);
		float get_major_tick_width() const;

		void set_minjor_tick_height(const float p_height);
		float get_minjor_tick_height() const;

		void set_minjor_tick_width(const float p_width);
		float get_minjor_tick_width() const;

		void set_tick_color(const Color& p_color);
		Color get_tick_color() const;

		void set_show_minor_ticks(const bool p_show);
		bool get_show_minor_ticks() const;

		void set_playhead_color(const Color& p_color);
		Color get_playhead_color() const;

		void set_playhead_text_color(const Color& p_color);
		Color get_playhead_text_color() const;

		void set_playhead_line_width(const float p_width);
		float get_playhead_line_width() const;
	};
}

#endif // !TIMELINE_PANEL_TIME_RULER_COMPONENT_H
