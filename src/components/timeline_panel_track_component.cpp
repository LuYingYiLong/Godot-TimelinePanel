#include "timeline_panel_track_component.h"

namespace godot {
	void TimelinePanelTrackComponent::_bind_methods() {
		ClassDB::bind_method(D_METHOD("set_width", "width"), &TimelinePanelTrackComponent::set_width);
		ClassDB::bind_method(D_METHOD("get_width"), &TimelinePanelTrackComponent::get_width);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "width"), "set_width", "get_width");

		ClassDB::bind_method(D_METHOD("set_header_icon", "header_icon"), &TimelinePanelTrackComponent::set_header_icon);
		ClassDB::bind_method(D_METHOD("get_header_icon"), &TimelinePanelTrackComponent::get_header_icon);
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "header_icon", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_header_icon", "get_header_icon");

		ClassDB::bind_method(D_METHOD("set_header_background", "style"), &TimelinePanelTrackComponent::set_header_background);
		ClassDB::bind_method(D_METHOD("get_header_background"), &TimelinePanelTrackComponent::get_header_background);
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "header_background", PROPERTY_HINT_RESOURCE_TYPE, "StyleBox"), "set_header_background", "get_header_background");
	}

	void TimelinePanelTrackComponent::_on_resource_changed() {
		emit_changed();
	}

	void TimelinePanelTrackComponent::set_width(const float p_width) {
		width = p_width;
		emit_changed();
	}

	float TimelinePanelTrackComponent::get_width() const {
		return width;
	}

	void TimelinePanelTrackComponent::set_header_icon(Ref<Texture2D> p_header_icon) {
		header_icon = p_header_icon;
		emit_changed();
	}

	Ref<Texture2D> TimelinePanelTrackComponent::get_header_icon() const {
		return header_icon;
	}

	void TimelinePanelTrackComponent::set_header_background(const Ref<StyleBox> p_style) {
		header_background = p_style;
		if (header_background.is_valid()) {
			header_background->connect("changed", callable_mp(this, &TimelinePanelTrackComponent::_on_resource_changed));
		}
		emit_changed();
	}

	Ref<StyleBox> TimelinePanelTrackComponent::get_header_background() const {
		return header_background;
	}

	void TimelinePanelTrackComponent::set_tooltip_text(const String& p_tooltip) {
		tooltip_text = p_tooltip;
		emit_changed();
	}

	String TimelinePanelTrackComponent::get_tooltip_text() const {
		return tooltip_text;
	}
}