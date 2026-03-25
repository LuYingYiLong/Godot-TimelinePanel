#ifndef TIMELINE_PANEL_TIME_RULER_COMPONENT_H
#define TIMELINE_PANEL_TIME_RULER_COMPONENT_H

#include "timeline_panel_base_component.h"
#include <godot_cpp/classes/texture2d.hpp>

namespace godot {
	class TimelinePanelTimeRulerComponent : public TimelinePanelBaseComponent {
		GDCLASS(TimelinePanelTimeRulerComponent, TimelinePanelBaseComponent)

	private:
		float width = 64.0f;
		Color header_color = Color("#ff7931");
		Ref<Texture2D> header_icon;

	protected:
		static void _bind_methods();

	public:
		void set_width(const float p_width);
		float get_width() const;

		void set_header_icon(Ref<Texture2D> p_header_icon);
		Ref<Texture2D> get_header_icon() const;

		void set_header_color(const Color& p_color);
		Color get_header_color() const;
	};
}

#endif // !TIMELINE_PANEL_TIME_RULER_COMPONENT_H
