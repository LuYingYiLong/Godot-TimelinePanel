#ifndef TIMELINE_INDICATOR_H
#define TIMELINE_INDICATOR_H

#include "timeline_base.h"

#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/core/gdvirtual.gen.inc>
#include <godot_cpp/classes/font.hpp>

namespace godot {
	class TimelineIndicator : public TimelineBase {
		GDCLASS(TimelineIndicator, TimelineBase)

	private:
		Color color = Color(1, 1, 1);
		int64_t font_size = 12;
		float font_offset = 0.0f;
		Color font_color;
		bool show_line = true;
		float line_width = 2.0f;

	protected:
		static void _bind_methods();

		GDVIRTUAL2RC(PackedVector2Array, _get_points, double, double)
		GDVIRTUAL1RC(PackedColorArray, _get_colors, double)
		GDVIRTUAL3RC(String, _get_text, int, double, double)
		GDVIRTUAL1RC(Ref<Font>, _get_font, double)
		GDVIRTUAL1RC(Vector2, _get_font_pos, double)
		GDVIRTUAL1RC(int64_t, _get_font_size, double)
		GDVIRTUAL1RC(Color, _get_font_color, double)
		GDVIRTUAL1RC(bool, _can_show_line, double)
		GDVIRTUAL1RC(float, _get_line_width, double)
		GDVIRTUAL1RC(Color, _get_line_color, double)

	public:
		virtual PackedVector2Array _get_points(const double current_position, const double current_width) const;
		virtual PackedColorArray _get_colors(const double current_position) const;
		virtual String _get_text(const int counting_unit, const double current_value, const double current_position) const;
		virtual Ref<Font> _get_font(const double current_position) const;
		virtual Vector2 _get_font_pos(const double current_position) const;
		virtual int64_t _get_font_size(const double current_position) const;
		virtual Color _get_font_color(const double current_position) const;
		virtual bool _can_show_line(const double current_position) const;
		virtual float _get_line_width(const double current_position) const;
		virtual Color _get_line_color(const double current_position) const;

		void set_color(const Color& p_color);
		Color get_color() const;

		void set_font_size(const int64_t p_font_size);
		int64_t get_font_size() const;

		void set_font_offset(const float p_font_offset);
		float get_font_offset() const;

		void set_font_color(const Color& p_color);
		Color get_font_color() const;

		void set_show_line(const bool p_show_line);
		bool is_show_line() const;

		void set_line_width(const float p_line_width);
		float get_line_width() const;
	};
}

#endif // !TIMELINE_INDICATOR_H
