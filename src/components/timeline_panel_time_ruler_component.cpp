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

		ClassDB::bind_method(D_METHOD("set_major_tick_height", "major_tick_height"), &TimelinePanelTimeRulerComponent::set_major_tick_height);
		ClassDB::bind_method(D_METHOD("get_major_tick_height"), &TimelinePanelTimeRulerComponent::get_major_tick_height);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "major_tick_height"), "set_major_tick_height", "get_major_tick_height");

		ClassDB::bind_method(D_METHOD("set_major_tick_width", "major_tick_width"), &TimelinePanelTimeRulerComponent::set_major_tick_width);
		ClassDB::bind_method(D_METHOD("get_major_tick_width"), &TimelinePanelTimeRulerComponent::get_major_tick_width);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "major_tick_width"), "set_major_tick_width", "get_major_tick_width");

		ClassDB::bind_method(D_METHOD("set_minjor_tick_height", "minjor_tick_height"), &TimelinePanelTimeRulerComponent::set_minjor_tick_height);
		ClassDB::bind_method(D_METHOD("get_minjor_tick_height"), &TimelinePanelTimeRulerComponent::get_minjor_tick_height);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "minjor_tick_height"), "set_minjor_tick_height", "get_minjor_tick_height");

		ClassDB::bind_method(D_METHOD("set_minjor_tick_width", "minjor_tick_width"), &TimelinePanelTimeRulerComponent::set_minjor_tick_width);
		ClassDB::bind_method(D_METHOD("get_minjor_tick_width"), &TimelinePanelTimeRulerComponent::get_minjor_tick_width);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "minjor_tick_width"), "set_minjor_tick_width", "get_minjor_tick_width");

		ClassDB::bind_method(D_METHOD("set_tick_color", "tick_color"), &TimelinePanelTimeRulerComponent::set_tick_color);
		ClassDB::bind_method(D_METHOD("get_tick_color"), &TimelinePanelTimeRulerComponent::get_tick_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "tick_color"), "set_tick_color", "get_tick_color");

		ClassDB::bind_method(D_METHOD("set_show_minor_ticks", "show_minor_ticks"), &TimelinePanelTimeRulerComponent::set_show_minor_ticks);
		ClassDB::bind_method(D_METHOD("get_show_minor_ticks"), &TimelinePanelTimeRulerComponent::get_show_minor_ticks);
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "show_minor_ticks"), "set_show_minor_ticks", "get_show_minor_ticks");

		ADD_GROUP("Playhead", "playhead_");
		ClassDB::bind_method(D_METHOD("set_playhead_color", "color"), &TimelinePanelTimeRulerComponent::set_playhead_color);
		ClassDB::bind_method(D_METHOD("get_playhead_color"), &TimelinePanelTimeRulerComponent::get_playhead_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "playhead_color"), "set_playhead_color", "get_playhead_color");

		ClassDB::bind_method(D_METHOD("set_playhead_text_color", "color"), &TimelinePanelTimeRulerComponent::set_playhead_text_color);
		ClassDB::bind_method(D_METHOD("get_playhead_text_color"), &TimelinePanelTimeRulerComponent::get_playhead_text_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "playhead_text_color"), "set_playhead_text_color", "get_playhead_text_color");

		ClassDB::bind_method(D_METHOD("set_playhead_line_width", "width"), &TimelinePanelTimeRulerComponent::set_playhead_line_width);
		ClassDB::bind_method(D_METHOD("get_playhead_line_width"), &TimelinePanelTimeRulerComponent::get_playhead_line_width);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "playhead_line_width"), "set_playhead_line_width", "get_playhead_line_width");
	}

	void TimelinePanelTimeRulerComponent::set_width(const float p_width) {
		width = p_width;
		emit_changed();
	}

	float TimelinePanelTimeRulerComponent::get_width() const {
		return width;
	}

	void TimelinePanelTimeRulerComponent::set_header_icon(Ref<Texture2D> p_header_icon) {
		header_icon = p_header_icon;
		emit_changed();
	}

	Ref<Texture2D> TimelinePanelTimeRulerComponent::get_header_icon() const {
		return header_icon;
	}

	void TimelinePanelTimeRulerComponent::set_header_color(const Color& p_color) {
		header_color = p_color;
		emit_changed();
	}

	Color TimelinePanelTimeRulerComponent::get_header_color() const {
		return header_color;
	}

	void TimelinePanelTimeRulerComponent::set_major_tick_height(const float p_height) {
		major_tick_height = p_height;
		emit_changed();
	}

	float TimelinePanelTimeRulerComponent::get_major_tick_height() const {
		return major_tick_height;
	}

	void TimelinePanelTimeRulerComponent::set_major_tick_width(const float p_width) {
		major_tick_width = p_width;
		emit_changed();
	}

	float TimelinePanelTimeRulerComponent::get_major_tick_width() const {
		return major_tick_width;
	}

	void TimelinePanelTimeRulerComponent::set_minjor_tick_height(const float p_height) {
		minor_tick_height = p_height;
		emit_changed();
	}

	float TimelinePanelTimeRulerComponent::get_minjor_tick_height() const {
		return minor_tick_height;
	}

	void TimelinePanelTimeRulerComponent::set_minjor_tick_width(const float p_width) {
		minor_tick_width = p_width;
		emit_changed();
	}

	float TimelinePanelTimeRulerComponent::get_minjor_tick_width() const {
		return minor_tick_width;
	}

	void TimelinePanelTimeRulerComponent::set_tick_color(const Color& p_color) {
		tick_color = p_color;
		emit_changed();
	}

	Color TimelinePanelTimeRulerComponent::get_tick_color() const {
		return tick_color;
	}

	void TimelinePanelTimeRulerComponent::set_show_minor_ticks(const bool p_show) {
		show_minor_ticks = p_show;
		emit_changed();
	}

	bool TimelinePanelTimeRulerComponent::get_show_minor_ticks() const {
		return show_minor_ticks;
	}

	void TimelinePanelTimeRulerComponent::set_playhead_color(const Color& p_color) {
		playhead_color = p_color;
		emit_changed();
	}

	Color TimelinePanelTimeRulerComponent::get_playhead_color() const {
		return playhead_color;
	}

	void TimelinePanelTimeRulerComponent::set_playhead_text_color(const Color& p_color) {
		playhead_text_color = p_color;
		emit_changed();
	}

	Color TimelinePanelTimeRulerComponent::get_playhead_text_color() const {
		return playhead_text_color;
	}

	void TimelinePanelTimeRulerComponent::set_playhead_line_width(const float p_width) {
		playhead_line_width = p_width;
		emit_changed();
	}

	float TimelinePanelTimeRulerComponent::get_playhead_line_width() const {
		return playhead_line_width;
	}
}