#include "timeline_panel_time_ruler_component.h"

namespace godot {
	void TimelinePanelTimeRulerComponent::_bind_methods() {
		ClassDB::bind_method(D_METHOD("set_width", "width"), &TimelinePanelTimeRulerComponent::set_width);
		ClassDB::bind_method(D_METHOD("get_width"), &TimelinePanelTimeRulerComponent::get_width);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "width"), "set_width", "get_width");

		ClassDB::bind_method(D_METHOD("set_header_icon", "header_icon"), &TimelinePanelTimeRulerComponent::set_header_icon);
		ClassDB::bind_method(D_METHOD("get_header_icon"), &TimelinePanelTimeRulerComponent::get_header_icon);
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "header_icon", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_header_icon", "get_header_icon");

		ClassDB::bind_method(D_METHOD("set_header_color", "header_color"), &TimelinePanelTimeRulerComponent::set_header_color);
		ClassDB::bind_method(D_METHOD("get_header_color"), &TimelinePanelTimeRulerComponent::get_header_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "header_color"), "set_header_color", "get_header_color");
	}

	void TimelinePanelTimeRulerComponent::set_width(const float p_width) {
		width = p_width;
	}

	float TimelinePanelTimeRulerComponent::get_width() const {
		return width;
	}

	void TimelinePanelTimeRulerComponent::set_header_icon(Ref<Texture2D> p_header_icon) {
		header_icon = p_header_icon;
	}

	Ref<Texture2D> TimelinePanelTimeRulerComponent::get_header_icon() const {
		return header_icon;
	}

	void TimelinePanelTimeRulerComponent::set_header_color(const Color& p_color) {
		header_color = p_color;
	}

	Color TimelinePanelTimeRulerComponent::get_header_color() const {
		return header_color;
	}
}