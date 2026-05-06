#include "timeline_track_key.h"

namespace godot {
	void TimelineTrackKey::_bind_methods() {
		ClassDB::bind_method(D_METHOD("set_selected", "selected"), &TimelineTrackKey::set_selected);
		ClassDB::bind_method(D_METHOD("set_selected_no_signal", "selected"), &TimelineTrackKey::set_selected_no_signal);
		ClassDB::bind_method(D_METHOD("is_selected"), &TimelineTrackKey::is_selected);
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "selected"), "set_selected", "is_selected");

		ClassDB::bind_method(D_METHOD("set_disabled", "disabled"), &TimelineTrackKey::set_disabled);
		ClassDB::bind_method(D_METHOD("set_disabled_no_signal", "disabled"), &TimelineTrackKey::set_disabled_no_signal);
		ClassDB::bind_method(D_METHOD("is_disabled"), &TimelineTrackKey::is_disabled);
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "disabled"), "set_disabled", "is_disabled");

		ClassDB::bind_method(D_METHOD("set_time", "time"), &TimelineTrackKey::set_time);
		ClassDB::bind_method(D_METHOD("get_time"), &TimelineTrackKey::get_time);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "time"), "set_time", "get_time");

		ClassDB::bind_method(D_METHOD("set_length", "length"), &TimelineTrackKey::set_length);
		ClassDB::bind_method(D_METHOD("get_length"), &TimelineTrackKey::get_length);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "length"), "set_length", "get_length");

		ClassDB::bind_method(D_METHOD("set_text", "text"), &TimelineTrackKey::set_text);
		ClassDB::bind_method(D_METHOD("get_text"), &TimelineTrackKey::get_text);
		ADD_PROPERTY(PropertyInfo(Variant::STRING, "text"), "set_text", "get_text");

		ClassDB::bind_method(D_METHOD("set_icon", "icon"), &TimelineTrackKey::set_icon);
		ClassDB::bind_method(D_METHOD("get_icon"), &TimelineTrackKey::get_icon);
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "icon", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_icon", "get_icon");

		ClassDB::bind_method(D_METHOD("set_metadata", "meta"), &TimelineTrackKey::set_metadata);
		ClassDB::bind_method(D_METHOD("get_metadata"), &TimelineTrackKey::get_metadata);

		ClassDB::bind_method(D_METHOD("set_allowed_track_indices", "track_indices"), &TimelineTrackKey::set_allowed_track_indices);
		ClassDB::bind_method(D_METHOD("get_allowed_track_indices"), &TimelineTrackKey::get_allowed_track_indices);
		ADD_PROPERTY(PropertyInfo(Variant::PACKED_INT32_ARRAY, "allowed_track_indices"), "set_allowed_track_indices", "get_allowed_track_indices");

		ClassDB::bind_method(D_METHOD("is_instant"), &TimelineTrackKey::is_instant);

		ADD_SUBGROUP("Constants", "");
		ClassDB::bind_method(D_METHOD("set_icon_max_width", "width"), &TimelineTrackKey::set_icon_max_width);
		ClassDB::bind_method(D_METHOD("get_icon_max_width"), &TimelineTrackKey::get_icon_max_width);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "icon_max_width"), "set_icon_max_width", "get_icon_max_width");

		ClassDB::bind_method(D_METHOD("set_instant_key_scale", "scale"), &TimelineTrackKey::set_instant_key_scale);
		ClassDB::bind_method(D_METHOD("get_instant_key_scale"), &TimelineTrackKey::get_instant_key_scale);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "instant_key_scale"), "set_instant_key_scale", "get_instant_key_scale");

		ADD_SUBGROUP("Styles", "");
		ClassDB::bind_method(D_METHOD("set_instant_key_normal_style", "style"), &TimelineTrackKey::set_instant_key_normal_style);
		ClassDB::bind_method(D_METHOD("get_instant_key_normal_style"), &TimelineTrackKey::get_instant_key_normal_style);
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "instant_key_normal", PROPERTY_HINT_RESOURCE_TYPE, "StyleBox"), "set_instant_key_normal_style", "get_instant_key_normal_style");
		
		ClassDB::bind_method(D_METHOD("set_instant_key_selected_style", "style"), &TimelineTrackKey::set_instant_key_selected_style);
		ClassDB::bind_method(D_METHOD("get_instant_key_selected_style"), &TimelineTrackKey::get_instant_key_selected_style);
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "instant_key_selected", PROPERTY_HINT_RESOURCE_TYPE, "StyleBox"), "set_instant_key_selected_style", "get_instant_key_selected_style");

		ClassDB::bind_method(D_METHOD("set_clip_key_normal_style", "style"), &TimelineTrackKey::set_clip_key_normal_style);
		ClassDB::bind_method(D_METHOD("get_clip_key_normal_style"), &TimelineTrackKey::get_clip_key_normal_style);
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "clip_key_normal", PROPERTY_HINT_RESOURCE_TYPE, "StyleBox"), "set_clip_key_normal_style", "get_clip_key_normal_style");

		ClassDB::bind_method(D_METHOD("set_clip_key_selected_style", "style"), &TimelineTrackKey::set_clip_key_selected_style);
		ClassDB::bind_method(D_METHOD("get_clip_key_selected_style"), &TimelineTrackKey::get_clip_key_selected_style);
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "clip_key_selected", PROPERTY_HINT_RESOURCE_TYPE, "StyleBox"), "set_clip_key_selected_style", "get_clip_key_selected_style");

		ADD_SIGNAL(MethodInfo("changed"));
	}

	void TimelineTrackKey::set_selected(const bool p_selected) {
		selected = p_selected;
		emit_signal("changed");
	}

	void TimelineTrackKey::set_selected_no_signal(bool p_selected) {
		selected = p_selected;
	}

	bool TimelineTrackKey::is_selected() const {
		return selected;
	}

	void TimelineTrackKey::set_disabled(const bool p_disabled) {
		disabled = p_disabled;
		emit_signal("changed");
	}

	void TimelineTrackKey::set_disabled_no_signal(bool p_disabled) {
		disabled = p_disabled;
	}

	bool TimelineTrackKey::is_disabled() const {
		return disabled;
	}

	void TimelineTrackKey::set_time(const double p_time) {
		time = p_time;
		emit_signal("changed");
	}

	void TimelineTrackKey::set_time_no_signal(double p_time) {
		time = p_time;
	}

	double TimelineTrackKey::get_time() const {
		return time;
	}

	void TimelineTrackKey::set_length(const double p_length) {
		length = p_length;
		emit_signal("changed");
	}

	void TimelineTrackKey::set_length_no_signal(double p_length) {
		length = p_length;
	}

	double TimelineTrackKey::get_length() const {
		return length;
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

	void TimelineTrackKey::set_instant_key_normal_style(const Ref<StyleBox>& p_style) {
		instant_key_normal_style = p_style;
		emit_signal("changed");
	}

	Ref<StyleBox> TimelineTrackKey::get_instant_key_normal_style() const {
		return instant_key_normal_style;
	}

	void TimelineTrackKey::set_instant_key_selected_style(Ref<StyleBox> p_style) {
		instant_key_selected_style = p_style;
		emit_signal("changed");
	}

	Ref<StyleBox> TimelineTrackKey::get_instant_key_selected_style() const {
		return instant_key_selected_style;
	}

	void TimelineTrackKey::set_clip_key_normal_style(const Ref<StyleBox>& p_style) {
		clip_key_normal_style = p_style;
		emit_signal("changed");
	}

	Ref<StyleBox> TimelineTrackKey::get_clip_key_normal_style() const {
		return clip_key_normal_style;
	}

	void TimelineTrackKey::set_clip_key_selected_style(Ref<StyleBox> p_style) {
		clip_key_selected_style = p_style;
		emit_signal("changed");
	}

	Ref<StyleBox> TimelineTrackKey::get_clip_key_selected_style() const {
		return clip_key_selected_style;
	}

	void TimelineTrackKey::set_metadata(const Variant& p_meta) {
		meta = p_meta;
	}

	Variant TimelineTrackKey::get_metadata() const {
		return meta;
	}

	void TimelineTrackKey::set_allowed_track_indices(const PackedInt32Array& p_track_indices) {
		allowed_track_indices = p_track_indices;
		emit_signal("changed");
	}

	PackedInt32Array TimelineTrackKey::get_allowed_track_indices() const {
		return allowed_track_indices;
	}

	bool TimelineTrackKey::can_move_to_track(int p_track_index) const {
		if (allowed_track_indices.is_empty()) {
			return true;
		}
		for (int i = 0; i < allowed_track_indices.size(); i++) {
			if (allowed_track_indices[i] == p_track_index) {
				return true;
			}
		}
		return false;
	}

	bool TimelineTrackKey::is_instant() const {
		return length <= 0.0;
	}
}
