#include "timeline_track_key.h"

namespace godot {
	void TimelineTrackKey::_bind_methods() {
		ClassDB::bind_method(D_METHOD("set_selected", "selected"), &TimelineTrackKey::set_selected);
		ClassDB::bind_method(D_METHOD("is_selected"), &TimelineTrackKey::is_selected);
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "selected"), "set_selected", "is_selected");

		ClassDB::bind_method(D_METHOD("set_disabled", "disabled"), &TimelineTrackKey::set_disabled);
		ClassDB::bind_method(D_METHOD("is_disabled"), &TimelineTrackKey::is_disabled);
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "disabled"), "set_disabled", "is_disabled");

		ClassDB::bind_method(D_METHOD("set_time", "time"), &TimelineTrackKey::set_time);
		ClassDB::bind_method(D_METHOD("get_time"), &TimelineTrackKey::get_time);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "time"), "set_time", "get_time");

		ClassDB::bind_method(D_METHOD("set_length", "length"), &TimelineTrackKey::set_length);
		ClassDB::bind_method(D_METHOD("get_length"), &TimelineTrackKey::get_length);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "length"), "set_length", "get_length");

		ClassDB::bind_method(D_METHOD("set_color", "color"), &TimelineTrackKey::set_color);
		ClassDB::bind_method(D_METHOD("get_color"), &TimelineTrackKey::get_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "color"), "set_color", "get_color");

		ClassDB::bind_method(D_METHOD("set_text", "text"), &TimelineTrackKey::set_text);
		ClassDB::bind_method(D_METHOD("get_text"), &TimelineTrackKey::get_text);
		ADD_PROPERTY(PropertyInfo(Variant::STRING, "text"), "set_text", "get_text");

		ClassDB::bind_method(D_METHOD("set_icon", "icon"), &TimelineTrackKey::set_icon);
		ClassDB::bind_method(D_METHOD("get_icon"), &TimelineTrackKey::get_icon);
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "icon", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_icon", "get_icon");

		ClassDB::bind_method(D_METHOD("set_metadata", "meta"), &TimelineTrackKey::set_metadata);
		ClassDB::bind_method(D_METHOD("get_metadata"), &TimelineTrackKey::get_metadata);

		ClassDB::bind_method(D_METHOD("is_instant"), &TimelineTrackKey::is_instant);

		ADD_SUBGROUP("Constants", "");
		ClassDB::bind_method(D_METHOD("set_icon_max_width", "width"), &TimelineTrackKey::set_icon_max_width);
		ClassDB::bind_method(D_METHOD("get_icon_max_width"), &TimelineTrackKey::get_icon_max_width);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "icon_max_width"), "set_icon_max_width", "get_icon_max_width");

		ClassDB::bind_method(D_METHOD("set_instant_key_scale", "scale"), &TimelineTrackKey::set_instant_key_scale);
		ClassDB::bind_method(D_METHOD("get_instant_key_scale"), &TimelineTrackKey::get_instant_key_scale);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "instant_key_scale"), "set_instant_key_scale", "get_instant_key_scale");

		ADD_SUBGROUP("Styles", "");
		ClassDB::bind_method(D_METHOD("set_instant_key_style", "style"), &TimelineTrackKey::set_instant_key_style);
		ClassDB::bind_method(D_METHOD("get_instant_key_style"), &TimelineTrackKey::get_instant_key_style);
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "instant_key_style", PROPERTY_HINT_RESOURCE_TYPE, "StyleBox"), "set_instant_key_style", "get_instant_key_style");

		ClassDB::bind_method(D_METHOD("set_clip_style", "style"), &TimelineTrackKey::set_clip_style);
		ClassDB::bind_method(D_METHOD("get_clip_style"), &TimelineTrackKey::get_clip_style);
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "clip_style", PROPERTY_HINT_RESOURCE_TYPE, "StyleBox"), "set_clip_style", "get_clip_style");

		ADD_SIGNAL(MethodInfo("changed"));
	}

	void TimelineTrackKey::set_selected(const bool p_selected) {
		selected = p_selected;
		emit_signal("changed");
	}

	bool TimelineTrackKey::is_selected() const {
		return selected;
	}

	void TimelineTrackKey::set_disabled(const bool p_disabled) {
		disabled = p_disabled;
		emit_signal("changed");
	}

	bool TimelineTrackKey::is_disabled() const {
		return disabled;
	}

	void TimelineTrackKey::set_time(const double p_time) {
		time = p_time;
		emit_signal("changed");
	}

	double TimelineTrackKey::get_time() const {
		return time;
	}

	void TimelineTrackKey::set_length(const double p_length) {
		length = p_length;
		emit_signal("changed");
	}

	double TimelineTrackKey::get_length() const {
		return length;
	}

	void TimelineTrackKey::set_color(const Color& p_color) {
		color = p_color;
		emit_signal("changed");
	}

	Color TimelineTrackKey::get_color() const {
		return color;
	}

	void TimelineTrackKey::set_text(const String& p_text) {
		text = p_text;
		emit_signal("changed");
	}

	String TimelineTrackKey::get_text() const {
		return text;
	}

	void TimelineTrackKey::set_icon(const Ref<Texture2D>& p_icon) {
		icon = p_icon;
		emit_signal("changed");
	}

	Ref<Texture2D> TimelineTrackKey::get_icon() const {
		return icon;
	}

	void TimelineTrackKey::set_icon_max_width(const float p_width) {
		icon_max_width = p_width;
		emit_signal("changed");
	}

	float TimelineTrackKey::get_icon_max_width() const {
		return icon_max_width;
	}

	void TimelineTrackKey::set_instant_key_scale(const float p_scale) {
		instant_key_scale = p_scale;
		emit_signal("changed");
	}

	float TimelineTrackKey::get_instant_key_scale() const {
		return instant_key_scale;
	}

	void TimelineTrackKey::set_instant_key_style(const Ref<StyleBox>& p_style) {
		instant_key_style = p_style;
		emit_signal("changed");
	}

	Ref<StyleBox> TimelineTrackKey::get_instant_key_style() const {
		return instant_key_style;
	}

	void TimelineTrackKey::set_clip_style(const Ref<StyleBox>& p_style) {
		clip_style = p_style;
		emit_signal("changed");
	}

	Ref<StyleBox> TimelineTrackKey::get_clip_style() const {
		return clip_style;
	}

	void TimelineTrackKey::set_metadata(const Variant& p_meta) {
		meta = p_meta;
	}

	Variant TimelineTrackKey::get_metadata() const {
		return meta;
	}

	bool TimelineTrackKey::is_instant() const {
		return length <= 0.0;
	}
}
