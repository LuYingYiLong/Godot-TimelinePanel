#include "timeline_indicator.h"

#include <godot_cpp/classes/theme_db.hpp>

namespace godot {
	void TimelineIndicator::_bind_methods() {
		GDVIRTUAL_BIND(_get_points, "current_position", "current_width");
		GDVIRTUAL_BIND(_get_colors, "current_position");
		GDVIRTUAL_BIND(_get_text, "counting_unit", "current_value", "current_position");
		GDVIRTUAL_BIND(_get_font, "current_position");
		GDVIRTUAL_BIND(_get_font_pos, "current_position");
		GDVIRTUAL_BIND(_get_font_size, "current_position");
		GDVIRTUAL_BIND(_get_font_color, "current_position");
		GDVIRTUAL_BIND(_can_show_line, "current_position");
		GDVIRTUAL_BIND(_get_line_width, "current_position");
		GDVIRTUAL_BIND(_get_line_color, "current_position");

		ClassDB::bind_method(D_METHOD("set_color", "color"), &TimelineIndicator::set_color);
		ClassDB::bind_method(D_METHOD("get_color"), &TimelineIndicator::get_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "color"), "set_color", "get_color");

		ClassDB::bind_method(D_METHOD("set_font_size", "font_size"), &TimelineIndicator::set_font_size);
		ClassDB::bind_method(D_METHOD("get_font_size"), &TimelineIndicator::get_font_size);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "font_size"), "set_font_size", "get_font_size");

		ClassDB::bind_method(D_METHOD("set_font_offset", "font_offset"), &TimelineIndicator::set_font_offset);
		ClassDB::bind_method(D_METHOD("get_font_offset"), &TimelineIndicator::get_font_offset);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "font_offset"), "set_font_offset", "get_font_offset");

		ClassDB::bind_method(D_METHOD("set_font_color", "font_color"), &TimelineIndicator::set_font_color);
		ClassDB::bind_method(D_METHOD("get_font_color"), &TimelineIndicator::get_font_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "font_color"), "set_font_color", "get_font_color");

		ClassDB::bind_method(D_METHOD("set_show_line", "show_line"), &TimelineIndicator::set_show_line);
		ClassDB::bind_method(D_METHOD("is_show_line"), &TimelineIndicator::is_show_line);
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "show_line"), "set_show_line", "is_show_line");

		ClassDB::bind_method(D_METHOD("set_line_width", "line_width"), &TimelineIndicator::set_line_width);
		ClassDB::bind_method(D_METHOD("get_line_width"), &TimelineIndicator::get_line_width);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "line_width"), "set_line_width", "get_line_width");
	}

	PackedVector2Array TimelineIndicator::_get_points(const double current_position, const double current_width) const {
		PackedVector2Array ret;
		if (GDVIRTUAL_CALL(_get_points, current_position, current_width, ret)) {
			return ret;
		}
		ret.append(Vector2(0.0f, current_position + 6.0f));
		ret.append(Vector2(current_width - 6.0f, current_position + 6.0f));
		ret.append(Vector2(current_width, current_position));
		ret.append(Vector2(current_width - 6.0f, current_position - 6.0f));
		ret.append(Vector2(0.0f, current_position - 6.0f));
		return ret;
	}

	PackedColorArray TimelineIndicator::_get_colors(const double current_position) const {
		PackedColorArray ret;
		if (GDVIRTUAL_CALL(_get_colors, current_position, ret)) {
			return ret;
		}
		ret.append(color);
		return ret;
	}

	String TimelineIndicator::_get_text(const int counting_unit, const double current_value, const double current_position) const {
		String ret;
		if (GDVIRTUAL_CALL(_get_text, counting_unit, current_value, current_position, ret)) {
			return ret;
		}
		ret = String::num(current_value);
		return ret;
	}

	Ref<Font> TimelineIndicator::_get_font(const double current_position) const {
		Ref<Font> ret = ThemeDB::get_singleton()->get_fallback_font();
		if (GDVIRTUAL_CALL(_get_font, current_position, ret)) {
			return ret;
		}
		return ret;
	}

	Vector2 TimelineIndicator::_get_font_pos(const double current_position) const {
		Vector2 ret;
		if (GDVIRTUAL_CALL(_get_font_pos, current_position, ret)) {
			return ret;
		}
		ret = Vector2(4.0f, current_position + (font_size / 2.0f) + font_offset);
		return ret;
	}

	int64_t TimelineIndicator::_get_font_size(const double current_position) const {
		int64_t ret;
		if (GDVIRTUAL_CALL(_get_font_size, current_position, ret)) {
			return ret;
		}
		return font_size;
	}

	Color TimelineIndicator::_get_font_color(const double current_position) const {
		Color ret;
		if (GDVIRTUAL_CALL(_get_font_color, current_position, ret)) {
			return ret;
		}
		return font_color;
	}

	bool TimelineIndicator::_can_show_line(const double current_position) const {
		bool ret;
		if (GDVIRTUAL_CALL(_can_show_line, current_position, ret)) {
			return ret;
		}
		return show_line;
	}

	float TimelineIndicator::_get_line_width(const double current_position) const {
		float ret;
		if (GDVIRTUAL_CALL(_get_line_width, current_position, ret)) {
			return ret;
		}
		return line_width;
	}

	Color TimelineIndicator::_get_line_color(const double current_position) const {
		Color ret;
		if (GDVIRTUAL_CALL(_get_line_color, current_position, ret)) {
			return ret;
		}
		return color;
	}

	void TimelineIndicator::set_color(const Color& p_color) {
		color = p_color;
		emit_changed();
	}

	Color TimelineIndicator::get_color() const {
		return color;
	}

	void TimelineIndicator::set_font_size(const int64_t p_font_size) {
		font_size = p_font_size;
		emit_changed();
	}

	int64_t TimelineIndicator::get_font_size() const {
		return font_size;
	}

	void TimelineIndicator::set_font_offset(const float p_font_offset) {
		font_offset = p_font_offset;
		emit_changed();
	}

	float TimelineIndicator::get_font_offset() const {
		return font_offset;
	}

	void TimelineIndicator::set_font_color(const Color& p_color) {
		font_color = p_color;
		emit_changed();
	}

	Color TimelineIndicator::get_font_color() const {
		return font_color;
	}

	void TimelineIndicator::set_show_line(const bool p_show_line) {
		show_line = p_show_line;
		emit_changed();
	}

	bool TimelineIndicator::is_show_line() const {
		return show_line;
	}

	void TimelineIndicator::set_line_width(const float p_line_width) {
		line_width = p_line_width;
		emit_changed();
	}

	float TimelineIndicator::get_line_width() const {
		return line_width;
	}
}