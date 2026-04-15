#include "timeline_panel_track.h"

namespace godot {
	void TimelinePanelTrack::_bind_methods() {
		ClassDB::bind_method(D_METHOD("set_width", "width"), &TimelinePanelTrack::set_width);
		ClassDB::bind_method(D_METHOD("get_width"), &TimelinePanelTrack::get_width);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "width"), "set_width", "get_width");

		ClassDB::bind_method(D_METHOD("set_header_icon", "header_icon"), &TimelinePanelTrack::set_header_icon);
		ClassDB::bind_method(D_METHOD("get_header_icon"), &TimelinePanelTrack::get_header_icon);
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "header_icon", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_header_icon", "get_header_icon");

		ClassDB::bind_method(D_METHOD("set_header_background", "style"), &TimelinePanelTrack::set_header_background);
		ClassDB::bind_method(D_METHOD("get_header_background"), &TimelinePanelTrack::get_header_background);
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "header_background", PROPERTY_HINT_RESOURCE_TYPE, "StyleBox"), "set_header_background", "get_header_background");
	
		ClassDB::bind_method(D_METHOD("set_tooltip_text", "text"), &TimelinePanelTrack::set_tooltip_text);
		ClassDB::bind_method(D_METHOD("get_tooltip_text"), &TimelinePanelTrack::get_tooltip_text);
		ADD_PROPERTY(PropertyInfo(Variant::STRING, "tooltip_text", PROPERTY_HINT_MULTILINE_TEXT), "set_tooltip_text", "get_tooltip_text");
	}

	void TimelinePanelTrack::_on_resource_changed() {
		emit_changed();
	}

	void TimelinePanelTrack::set_width(const float p_width) {
		width = p_width;
		emit_changed();
	}

	float TimelinePanelTrack::get_width() const {
		return width;
	}

	void TimelinePanelTrack::set_header_icon(Ref<Texture2D> p_header_icon) {
		header_icon = p_header_icon;
		emit_changed();
	}

	Ref<Texture2D> TimelinePanelTrack::get_header_icon() const {
		return header_icon;
	}

	void TimelinePanelTrack::set_header_background(const Ref<StyleBox> p_style) {
		header_background = p_style;
		if (header_background.is_valid()) {
			header_background->connect("changed", callable_mp(this, &TimelinePanelTrack::_on_resource_changed));
		}
		emit_changed();
	}

	Ref<StyleBox> TimelinePanelTrack::get_header_background() const {
		return header_background;
	}

	void TimelinePanelTrack::set_tooltip_text(const String& p_tooltip) {
		tooltip_text = p_tooltip;
		emit_changed();
	}

	String TimelinePanelTrack::get_tooltip_text() const {
		return tooltip_text;
	}
}