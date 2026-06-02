#ifndef TIMELINE_TRACK_H
#define TIMELINE_TRACK_H

#include "timeline_base.h"
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/style_box.hpp>
#include <godot_cpp/variant/color.hpp>

namespace godot {
	class TimelineTrack : public TimelineBase {
		GDCLASS(TimelineTrack, TimelineBase)

	private:
		float width = 32.0f;
		Color background = Color(0.0f, 0.0f, 0.0f, 0.0f);
		Ref<Texture2D> header_icon;
		Ref<StyleBox> header_background;
		bool header_background_fill_track = false;
		float header_indent = 0.0f;
		String text;
		String tooltip_text;

		void _on_resource_changed();

	protected:
		static void _bind_methods();

	public:
		void set_width(const float p_width);
		float get_width() const;

		void set_background(const Color &p_background);
		Color get_background() const;

		void set_header_icon(Ref<Texture2D> p_header_icon);
		Ref<Texture2D> get_header_icon() const;

		void set_header_background(const Ref<StyleBox> p_style);
		Ref<StyleBox> get_header_background() const;

		void set_header_background_fill_track(bool p_enabled);
		bool get_header_background_fill_track() const;

		void set_header_indent(const float p_indent);
		float get_header_indent() const;

		void set_text(const String &p_text);
		String get_text() const;

		void set_tooltip_text(const String &p_tooltip);
		String get_tooltip_text() const;
	};
}

#endif // !TIMELINE_TRACK_H
