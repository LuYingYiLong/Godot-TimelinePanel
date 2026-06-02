#include "timeline_track.h"

namespace godot {
	void TimelineTrack::_bind_methods() {
		ClassDB::bind_method(D_METHOD("set_width", "width"), &TimelineTrack::set_width);
		ClassDB::bind_method(D_METHOD("get_width"), &TimelineTrack::get_width);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "width"), "set_width", "get_width");

		ClassDB::bind_method(D_METHOD("set_background", "background"), &TimelineTrack::set_background);
		ClassDB::bind_method(D_METHOD("get_background"), &TimelineTrack::get_background);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "background"), "set_background", "get_background");

		ClassDB::bind_method(D_METHOD("set_header_icon", "header_icon"), &TimelineTrack::set_header_icon);
		ClassDB::bind_method(D_METHOD("get_header_icon"), &TimelineTrack::get_header_icon);
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "header_icon", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_header_icon", "get_header_icon");

		ClassDB::bind_method(D_METHOD("set_header_background", "style"), &TimelineTrack::set_header_background);
		ClassDB::bind_method(D_METHOD("get_header_background"), &TimelineTrack::get_header_background);
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "header_background", PROPERTY_HINT_RESOURCE_TYPE, "StyleBox"), "set_header_background", "get_header_background");

		ClassDB::bind_method(D_METHOD("set_header_background_fill_track", "enabled"), &TimelineTrack::set_header_background_fill_track);
		ClassDB::bind_method(D_METHOD("get_header_background_fill_track"), &TimelineTrack::get_header_background_fill_track);
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "header_background_fill_track"), "set_header_background_fill_track", "get_header_background_fill_track");

		ClassDB::bind_method(D_METHOD("set_header_indent", "indent"), &TimelineTrack::set_header_indent);
		ClassDB::bind_method(D_METHOD("get_header_indent"), &TimelineTrack::get_header_indent);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "header_indent"), "set_header_indent", "get_header_indent");

		ClassDB::bind_method(D_METHOD("set_text", "text"), &TimelineTrack::set_text);
		ClassDB::bind_method(D_METHOD("get_text"), &TimelineTrack::get_text);
		ADD_PROPERTY(PropertyInfo(Variant::STRING, "text"), "set_text", "get_text");
	
		ClassDB::bind_method(D_METHOD("set_tooltip_text", "text"), &TimelineTrack::set_tooltip_text);
		ClassDB::bind_method(D_METHOD("get_tooltip_text"), &TimelineTrack::get_tooltip_text);
		ADD_PROPERTY(PropertyInfo(Variant::STRING, "tooltip_text", PROPERTY_HINT_MULTILINE_TEXT), "set_tooltip_text", "get_tooltip_text");
	}

	void TimelineTrack::_on_resource_changed() {
		emit_changed();
	}

	void TimelineTrack::set_width(const float p_width) {
		width = p_width;
		emit_changed();
	}

	float TimelineTrack::get_width() const {
		return width;
	}

	void TimelineTrack::set_background(const Color &p_background) {
		background = p_background;
		emit_changed();
	}

	Color TimelineTrack::get_background() const {
		return background;
	}

	void TimelineTrack::set_header_icon(Ref<Texture2D> p_header_icon) {
		header_icon = p_header_icon;
		emit_changed();
	}

	Ref<Texture2D> TimelineTrack::get_header_icon() const {
		return header_icon;
	}

	void TimelineTrack::set_header_background(const Ref<StyleBox> p_style) {
		header_background = p_style;
		if (header_background.is_valid()) {
			header_background->connect("changed", callable_mp(this, &TimelineTrack::_on_resource_changed));
		}
		emit_changed();
	}

	Ref<StyleBox> TimelineTrack::get_header_background() const {
		return header_background;
	}

	void TimelineTrack::set_header_background_fill_track(bool p_enabled) {
		header_background_fill_track = p_enabled;
		emit_changed();
	}

	bool TimelineTrack::get_header_background_fill_track() const {
		return header_background_fill_track;
	}

	void TimelineTrack::set_header_indent(const float p_indent) {
		header_indent = p_indent < 0.0f ? 0.0f : p_indent;
		emit_changed();
	}

	float TimelineTrack::get_header_indent() const {
		return header_indent;
	}

	void TimelineTrack::set_text(const String &p_text) {
		text = p_text;
		emit_changed();
	}

	String TimelineTrack::get_text() const {
		return text;
	}

	void TimelineTrack::set_tooltip_text(const String &p_tooltip) {
		tooltip_text = p_tooltip;
		emit_changed();
	}

	String TimelineTrack::get_tooltip_text() const {
		return tooltip_text;
	}
}
