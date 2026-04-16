#include "timeline_time_ruler.h"

namespace godot {
	void TimelineTimeRuler::_bind_methods() {
		ClassDB::bind_method(D_METHOD("set_width", "width"), &TimelineTimeRuler::set_width);
		ClassDB::bind_method(D_METHOD("get_width"), &TimelineTimeRuler::get_width);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "width"), "set_width", "get_width");

		ClassDB::bind_method(D_METHOD("set_header_icon", "header_icon"), &TimelineTimeRuler::set_header_icon);
		ClassDB::bind_method(D_METHOD("get_header_icon"), &TimelineTimeRuler::get_header_icon);
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "header_icon", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_header_icon", "get_header_icon");

		ClassDB::bind_method(D_METHOD("set_header_background", "style"), &TimelineTimeRuler::set_header_background);
		ClassDB::bind_method(D_METHOD("get_header_background"), &TimelineTimeRuler::get_header_background);
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "header_background", PROPERTY_HINT_RESOURCE_TYPE, "StyleBox"), "set_header_background", "get_header_background");

		ClassDB::bind_method(D_METHOD("set_major_tick_height", "major_tick_height"), &TimelineTimeRuler::set_major_tick_height);
		ClassDB::bind_method(D_METHOD("get_major_tick_height"), &TimelineTimeRuler::get_major_tick_height);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "major_tick_height"), "set_major_tick_height", "get_major_tick_height");

		ClassDB::bind_method(D_METHOD("set_major_tick_width", "major_tick_width"), &TimelineTimeRuler::set_major_tick_width);
		ClassDB::bind_method(D_METHOD("get_major_tick_width"), &TimelineTimeRuler::get_major_tick_width);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "major_tick_width"), "set_major_tick_width", "get_major_tick_width");

		ClassDB::bind_method(D_METHOD("set_minjor_tick_height", "minjor_tick_height"), &TimelineTimeRuler::set_minjor_tick_height);
		ClassDB::bind_method(D_METHOD("get_minjor_tick_height"), &TimelineTimeRuler::get_minjor_tick_height);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "minjor_tick_height"), "set_minjor_tick_height", "get_minjor_tick_height");

		ClassDB::bind_method(D_METHOD("set_minjor_tick_width", "minjor_tick_width"), &TimelineTimeRuler::set_minjor_tick_width);
		ClassDB::bind_method(D_METHOD("get_minjor_tick_width"), &TimelineTimeRuler::get_minjor_tick_width);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "minjor_tick_width"), "set_minjor_tick_width", "get_minjor_tick_width");

		ClassDB::bind_method(D_METHOD("set_tick_color", "tick_color"), &TimelineTimeRuler::set_tick_color);
		ClassDB::bind_method(D_METHOD("get_tick_color"), &TimelineTimeRuler::get_tick_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "tick_color"), "set_tick_color", "get_tick_color");

		ClassDB::bind_method(D_METHOD("set_show_minor_ticks", "show_minor_ticks"), &TimelineTimeRuler::set_show_minor_ticks);
		ClassDB::bind_method(D_METHOD("get_show_minor_ticks"), &TimelineTimeRuler::get_show_minor_ticks);
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "show_minor_ticks"), "set_show_minor_ticks", "get_show_minor_ticks");

		ADD_GROUP("Playhead", "playhead_");
		ClassDB::bind_method(D_METHOD("set_playhead_color", "color"), &TimelineTimeRuler::set_playhead_color);
		ClassDB::bind_method(D_METHOD("get_playhead_color"), &TimelineTimeRuler::get_playhead_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "playhead_color"), "set_playhead_color", "get_playhead_color");

		ClassDB::bind_method(D_METHOD("set_playhead_text_color", "color"), &TimelineTimeRuler::set_playhead_text_color);
		ClassDB::bind_method(D_METHOD("get_playhead_text_color"), &TimelineTimeRuler::get_playhead_text_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "playhead_text_color"), "set_playhead_text_color", "get_playhead_text_color");

		ClassDB::bind_method(D_METHOD("set_playhead_line_width", "width"), &TimelineTimeRuler::set_playhead_line_width);
		ClassDB::bind_method(D_METHOD("get_playhead_line_width"), &TimelineTimeRuler::get_playhead_line_width);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "playhead_line_width"), "set_playhead_line_width", "get_playhead_line_width");

		ClassDB::bind_method(D_METHOD("set_tooltip_text", "text"), &TimelineTimeRuler::set_tooltip_text);
		ClassDB::bind_method(D_METHOD("get_tooltip_text"), &TimelineTimeRuler::get_tooltip_text);
		ADD_PROPERTY(PropertyInfo(Variant::STRING, "tooltip_text", PROPERTY_HINT_MULTILINE_TEXT), "set_tooltip_text", "get_tooltip_text");
	}

	void TimelineTimeRuler::_on_resource_changed() {
		emit_changed();
	}

	void TimelineTimeRuler::set_width(const float p_width) {
		width = p_width;
		emit_changed();
	}

	float TimelineTimeRuler::get_width() const {
		return width;
	}

	void TimelineTimeRuler::set_header_icon(Ref<Texture2D> p_header_icon) {
		header_icon = p_header_icon;
		emit_changed();
	}

	Ref<Texture2D> TimelineTimeRuler::get_header_icon() const {
		return header_icon;
	}

	void TimelineTimeRuler::set_header_background(const Ref<StyleBox> p_style) {
		header_background = p_style;
		if (header_background.is_valid()) {
			header_background->connect("changed", callable_mp(this, &TimelineTimeRuler::_on_resource_changed));
		}
		emit_changed();
	}

	Ref<StyleBox> TimelineTimeRuler::get_header_background() const {
		return header_background;
	}

	void TimelineTimeRuler::set_major_tick_height(const float p_height) {
		major_tick_height = p_height;
		emit_changed();
	}

	float TimelineTimeRuler::get_major_tick_height() const {
		return major_tick_height;
	}

	void TimelineTimeRuler::set_major_tick_width(const float p_width) {
		major_tick_width = p_width;
		emit_changed();
	}

	float TimelineTimeRuler::get_major_tick_width() const {
		return major_tick_width;
	}

	void TimelineTimeRuler::set_minjor_tick_height(const float p_height) {
		minor_tick_height = p_height;
		emit_changed();
	}

	float TimelineTimeRuler::get_minjor_tick_height() const {
		return minor_tick_height;
	}

	void TimelineTimeRuler::set_minjor_tick_width(const float p_width) {
		minor_tick_width = p_width;
		emit_changed();
	}

	float TimelineTimeRuler::get_minjor_tick_width() const {
		return minor_tick_width;
	}

	void TimelineTimeRuler::set_tick_color(const Color& p_color) {
		tick_color = p_color;
		emit_changed();
	}

	Color TimelineTimeRuler::get_tick_color() const {
		return tick_color;
	}

	void TimelineTimeRuler::set_show_minor_ticks(const bool p_show) {
		show_minor_ticks = p_show;
		emit_changed();
	}

	bool TimelineTimeRuler::get_show_minor_ticks() const {
		return show_minor_ticks;
	}

	void TimelineTimeRuler::set_playhead_color(const Color& p_color) {
		playhead_color = p_color;
		emit_changed();
	}

	Color TimelineTimeRuler::get_playhead_color() const {
		return playhead_color;
	}

	void TimelineTimeRuler::set_playhead_text_color(const Color& p_color) {
		playhead_text_color = p_color;
		emit_changed();
	}

	Color TimelineTimeRuler::get_playhead_text_color() const {
		return playhead_text_color;
	}

	void TimelineTimeRuler::set_playhead_line_width(const float p_width) {
		playhead_line_width = p_width;
		emit_changed();
	}

	float TimelineTimeRuler::get_playhead_line_width() const {
		return playhead_line_width;
	}

	void TimelineTimeRuler::set_tooltip_text(const String& p_text) {
		tooltip_text = p_text;
	}

	String TimelineTimeRuler::get_tooltip_text() const {
		return tooltip_text;
	}
}