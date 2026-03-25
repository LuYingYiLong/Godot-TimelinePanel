#ifndef TIMELINE_PANEL_TRACK_COMPONENT_H
#define TIMELINE_PANEL_TRACK_COMPONENT_H

#include "timeline_panel_base_component.h"

#include <godot_cpp/classes/texture2d.hpp>

namespace godot {
	class TimelinePanelTrackComponent : public TimelinePanelBaseComponent {
		GDCLASS(TimelinePanelTrackComponent, TimelinePanelBaseComponent)

	private:
		float width = 32.0f;
		Ref<Texture2D> header_icon;
		Color header_color;
		String tooltip_text;

	protected:
		static void _bind_methods();

	public:
		void set_width(const float p_width);
		float get_width() const;

		void set_header_icon(Ref<Texture2D> p_header_icon);
		Ref<Texture2D> get_header_icon() const;

		void set_header_color(const Color& p_color);
		Color get_header_color() const;

		void set_tooltip_text(const String& p_tooltip);
		String get_tooltip_text() const;
	};
}

#endif // !TIMELINE_PANEL_TRACK_COMPONENT_H
