#include "timeline_panel_time_ruler.h"

namespace godot {
	void TimelinePanelTimeRuler::_bind_methods() {
		ClassDB::bind_method(D_METHOD("set_width", "width"), &TimelinePanelTimeRuler::set_width);
		ClassDB::bind_method(D_METHOD("get_width"), &TimelinePanelTimeRuler::get_width);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "width"), "set_width", "get_width");

		ClassDB::bind_method(D_METHOD("set_header_icon", "header_icon"), &TimelinePanelTimeRuler::set_header_icon);
		ClassDB::bind_method(D_METHOD("get_header_icon"), &TimelinePanelTimeRuler::get_header_icon);
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "header_icon", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_header_icon", "get_header_icon");

		ClassDB::bind_method(D_METHOD("set_header_background", "style"), &TimelinePanelTimeRuler::set_header_background);
		ClassDB::bind_method(D_METHOD("get_header_background"), &TimelinePanelTimeRuler::get_header_background);
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "header_background", PROPERTY_HINT_RESOURCE_TYPE, "StyleBox"), "set_header_background", "get_header_background");

		ClassDB::bind_method(D_METHOD("set_major_tick_height", "major_tick_height"), &TimelinePanelTimeRuler::set_major_tick_height);
		ClassDB::bind_method(D_METHOD("get_major_tick_height"), &TimelinePanelTimeRuler::get_major_tick_height);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "major_tick_height"), "set_major_tick_height", "get_major_tick_height");

		ClassDB::bind_method(D_METHOD("set_major_tick_width", "major_tick_width"), &TimelinePanelTimeRuler::set_major_tick_width);
		ClassDB::bind_method(D_METHOD("get_major_tick_width"), &TimelinePanelTimeRuler::get_major_tick_width);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "major_tick_width"), "set_major_tick_width", "get_major_tick_width");

		ClassDB::bind_method(D_METHOD("set_minjor_tick_height", "minjor_tick_height"), &TimelinePanelTimeRuler::set_minjor_tick_height);
		ClassDB::bind_method(D_METHOD("get_minjor_tick_height"), &TimelinePanelTimeRuler::get_minjor_tick_height);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "minjor_tick_height"), "set_minjor_tick_height", "get_minjor_tick_height");

		ClassDB::bind_method(D_METHOD("set_minjor_tick_width", "minjor_tick_width"), &TimelinePanelTimeRuler::set_minjor_tick_width);
		ClassDB::bind_method(D_METHOD("get_minjor_tick_width"), &TimelinePanelTimeRuler::get_minjor_tick_width);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "minjor_tick_width"), "set_minjor_tick_width", "get_minjor_tick_width");

		ClassDB::bind_method(D_METHOD("set_tick_color", "tick_color"), &TimelinePanelTimeRuler::set_tick_color);
		ClassDB::bind_method(D_METHOD("get_tick_color"), &TimelinePanelTimeRuler::get_tick_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "tick_color"), "set_tick_color", "get_tick_color");

		ClassDB::bind_method(D_METHOD("set_show_minor_ticks", "show_minor_ticks"), &TimelinePanelTimeRuler::set_show_minor_ticks);
		ClassDB::bind_method(D_METHOD("get_show_minor_ticks"), &TimelinePanelTimeRuler::get_show_minor_ticks);
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "show_minor_ticks"), "set_show_minor_ticks", "get_show_minor_ticks");

		ADD_GROUP("Playhead", "playhead_");
		ClassDB::bind_method(D_METHOD("set_playhead_color", "color"), &TimelinePanelTimeRuler::set_playhead_color);
		ClassDB::bind_method(D_METHOD("get_playhead_color"), &TimelinePanelTimeRuler::get_playhead_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "playhead_color"), "set_playhead_color", "get_playhead_color");

		ClassDB::bind_method(D_METHOD("set_playhead_text_color", "color"), &TimelinePanelTimeRuler::set_playhead_text_color);
		ClassDB::bind_method(D_METHOD("get_playhead_text_color"), &TimelinePanelTimeRuler::get_playhead_text_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "playhead_text_color"), "set_playhead_text_color", "get_playhead_text_color");

		ClassDB::bind_method(D_METHOD("set_playhead_line_width", "width"), &TimelinePanelTimeRuler::set_playhead_line_width);
		ClassDB::bind_method(D_METHOD("get_playhead_line_width"), &TimelinePanelTimeRuler::get_playhead_line_width);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "playhead_line_width"), "set_playhead_line_width", "get_playhead_line_width");

		ClassDB::bind_method(D_METHOD("set_tooltip_text", "text"), &TimelinePanelTimeRuler::set_tooltip_text);
		ClassDB::bind_method(D_METHOD("get_tooltip_text"), &TimelinePanelTimeRuler::get_tooltip_text);
		ADD_PROPERTY(PropertyInfo(Variant::STRING, "tooltip_text", PROPERTY_HINT_MULTILINE_TEXT), "set_tooltip_text", "get_tooltip_text");
	}

	void TimelinePanelTimeRuler::_on_resource_changed() {
		emit_changed();
	}

	void TimelinePanelTimeRuler::set_width(const float p_width) {
		width = p_width;
		emit_changed();
	}

	float TimelinePanelTimeRuler::get_width() const {
		return width;
	}

	void TimelinePanelTimeRuler::set_header_icon(Ref<Texture2D> p_header_icon) {
		header_icon = p_header_icon;
		emit_changed();
	}

	Ref<Texture2D> TimelinePanelTimeRuler::get_header_icon() const {
		return header_icon;
	}

	void TimelinePanelTimeRuler::set_header_background(const Ref<StyleBox> p_style) {
		header_background = p_style;
		if (header_background.is_valid()) {
			header_background->connect("changed", callable_mp(this, &TimelinePanelTimeRuler::_on_resource_changed));
		}
		emit_changed();
	}

	Ref<StyleBox> TimelinePanelTimeRuler::get_header_background() const {
		return header_background;
	}

	void TimelinePanelTimeRuler::set_major_tick_height(const float p_height) {
		major_tick_height = p_height;
		emit_changed();
	}

	float TimelinePanelTimeRuler::get_major_tick_height() const {
		return major_tick_height;
	}

	void TimelinePanelTimeRuler::set_major_tick_width(const float p_width) {
		major_tick_width = p_width;
		emit_changed();
	}

	float TimelinePanelTimeRuler::get_major_tick_width() const {
		return major_tick_width;
	}

	void TimelinePanelTimeRuler::set_minjor_tick_height(const float p_height) {
		minor_tick_height = p_height;
		emit_changed();
	}

	float TimelinePanelTimeRuler::get_minjor_tick_height() const {
		return minor_tick_height;
	}

	void TimelinePanelTimeRuler::set_minjor_tick_width(const float p_width) {
		minor_tick_width = p_width;
		emit_changed();
	}

	float TimelinePanelTimeRuler::get_minjor_tick_width() const {
		return minor_tick_width;
	}

	void TimelinePanelTimeRuler::set_tick_color(const Color& p_color) {
		tick_color = p_color;
		emit_changed();
	}

	Color TimelinePanelTimeRuler::get_tick_color() const {
		return tick_color;
	}

	void TimelinePanelTimeRuler::set_show_minor_ticks(const bool p_show) {
		show_minor_ticks = p_show;
		emit_changed();
	}

	bool TimelinePanelTimeRuler::get_show_minor_ticks() const {
		return show_minor_ticks;
	}

	void TimelinePanelTimeRuler::set_playhead_color(const Color& p_color) {
		playhead_color = p_color;
		emit_changed();
	}

	Color TimelinePanelTimeRuler::get_playhead_color() const {
		return playhead_color;
	}

	void TimelinePanelTimeRuler::set_playhead_text_color(const Color& p_color) {
		playhead_text_color = p_color;
		emit_changed();
	}

	Color TimelinePanelTimeRuler::get_playhead_text_color() const {
		return playhead_text_color;
	}

	void TimelinePanelTimeRuler::set_playhead_line_width(const float p_width) {
		playhead_line_width = p_width;
		emit_changed();
	}

	float TimelinePanelTimeRuler::get_playhead_line_width() const {
		return playhead_line_width;
	}

	void TimelinePanelTimeRuler::set_tooltip_text(const String& p_text) {
		tooltip_text = p_text;
	}

	String TimelinePanelTimeRuler::get_tooltip_text() const {
		return tooltip_text;
	}
}