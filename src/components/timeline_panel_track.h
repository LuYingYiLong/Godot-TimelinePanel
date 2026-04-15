#ifndef TIMELINE_PANEL_TRACK_H
#define TIMELINE_PANEL_TRACK_H

#include "timeline_panel_base.h"
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/style_box.hpp>

namespace godot {
	class TimelinePanelTrack : public TimelinePanelBase {
		GDCLASS(TimelinePanelTrack, TimelinePanelBase)

	private:
		float width = 32.0f;
		Ref<Texture2D> header_icon;
		Ref<StyleBox> header_background;
		String tooltip_text;

		void _on_resource_changed();

	protected:
		static void _bind_methods();

	public:
		void set_width(const float p_width);
		float get_width() const;

		void set_header_icon(Ref<Texture2D> p_header_icon);
		Ref<Texture2D> get_header_icon() const;

		void set_header_background(const Ref<StyleBox> p_style);
		Ref<StyleBox> get_header_background() const;

		void set_tooltip_text(const String& p_tooltip);
		String get_tooltip_text() const;
	};
}

#endif // !TIMELINE_PANEL_TRACK_H
