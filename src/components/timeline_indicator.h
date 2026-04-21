#ifndef TIMELINE_INDICATOR_H
#define TIMELINE_INDICATOR_H

#include "timeline_base.h"

#include <godot_cpp/classes/font.hpp>
#include <godot_cpp/classes/style_box.hpp>
#include <godot_cpp/classes/text_line.hpp>
#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/core/gdvirtual.gen.inc>

namespace godot {
	class TimelineIndicator : public TimelineBase {
		GDCLASS(TimelineIndicator, TimelineBase)

	private:
		Ref<StyleBox> style;
		int64_t font_size = 12;
		float font_offset = 0.0f;
		Color font_color;
		bool show_line = true;
		Color line_color = Color(1.0f, 1.0f, 1.0f);
		float line_width = 2.0f;

		Ref<TextLine> text_line;

	protected:
		static void _bind_methods();

		GDVIRTUAL5(_draw, RID, Rect2, String, double, bool)

	public:
		TimelineIndicator();

		virtual void _draw(
			const RID& p_to_canvas_item,
			const Rect2& p_header_rect,
			const String& p_text,
			const double line_length,
			const bool vertical
		);
		void draw(
			const RID& p_to_canvas_item,
			const Rect2& p_header_rect,
			const String& p_text,
			const double line_length,
			const bool vertical
		);

		void set_style(Ref<StyleBox> p_style);
		Ref<StyleBox> get_style() const;

		void set_font_size(const int64_t p_font_size);
		int64_t get_font_size() const;

		void set_font_offset(const float p_font_offset);
		float get_font_offset() const;

		void set_font_color(const Color& p_color);
		Color get_font_color() const;

		void set_show_line(const bool p_show_line);
		bool is_show_line() const;

		void set_line_color(const Color& p_color);
		Color get_line_color() const;

		void set_line_width(const float p_line_width);
		float get_line_width() const;
	};
}

#endif // !TIMELINE_INDICATOR_H
