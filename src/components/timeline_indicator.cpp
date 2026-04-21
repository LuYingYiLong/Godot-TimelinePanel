#include "timeline_indicator.h"

#include <godot_cpp/classes/theme_db.hpp>
#include <godot_cpp/classes/rendering_server.hpp>

namespace godot {
	void TimelineIndicator::_bind_methods() {
		GDVIRTUAL_BIND(_draw, "to_canvas_item", "header_rect", "text", "line_length", "vertical");

		ClassDB::bind_method(D_METHOD("set_style", "style"), &TimelineIndicator::set_style);
		ClassDB::bind_method(D_METHOD("get_style"), &TimelineIndicator::get_style);
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "style", PROPERTY_HINT_RESOURCE_TYPE, "StyleBox"), "set_style", "get_style");

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

		ClassDB::bind_method(D_METHOD("set_line_color", "color"), &TimelineIndicator::set_line_color);
		ClassDB::bind_method(D_METHOD("get_line_color"), &TimelineIndicator::get_line_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "line_color"), "set_line_color", "get_line_color");

		ClassDB::bind_method(D_METHOD("set_line_width", "line_width"), &TimelineIndicator::set_line_width);
		ClassDB::bind_method(D_METHOD("get_line_width"), &TimelineIndicator::get_line_width);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "line_width"), "set_line_width", "get_line_width");
	}

	TimelineIndicator::TimelineIndicator() {
		text_line.instantiate();
	}

	void TimelineIndicator::_draw(
		const RID& p_to_canvas_item,
		const Rect2& p_header_rect,
		const String& p_text,
		const double line_length,
		const bool vertical
	) {
		if (GDVIRTUAL_CALL(_draw, p_to_canvas_item, p_header_rect, p_text, line_length, vertical)) {
			return;
		}

		Point2 pos = Vector2(0.0f, p_header_rect.position.y + p_header_rect.size.y / 2.0);
		if (show_line) {
			RenderingServer::get_singleton()->canvas_item_add_line(
				p_to_canvas_item,
				pos,
				Point2(vertical ? pos.x : pos.x + line_length, vertical ? pos.y + line_length : pos.y),
				line_color,
				line_width
			);
		}

		if (style.is_valid()) {
			style->draw(p_to_canvas_item, p_header_rect);
		}

		text_line->clear();
		text_line->add_string(p_text, ThemeDB::get_singleton()->get_fallback_font(), font_size);
		text_line->draw(p_to_canvas_item, Vector2(4.0f, pos.y - (font_size / 2.0f) + font_offset), font_color);
	}

	void TimelineIndicator::draw(
		const RID& p_to_canvas_item,
		const Rect2& p_header_rect,
		const String& p_text,
		const double line_length,
		const bool vertical
	) {
		_draw(p_to_canvas_item, p_header_rect, p_text, line_length, vertical);
	}

	void TimelineIndicator::set_style(Ref<StyleBox> p_style) {
		style = p_style;
		emit_changed();
	}

	Ref<StyleBox> TimelineIndicator::get_style() const {
		return style;
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

	void TimelineIndicator::set_line_color(const Color& p_color) {
		line_color = p_color;
	}

	Color TimelineIndicator::get_line_color() const {
		return line_color;
	}

	void TimelineIndicator::set_line_width(const float p_line_width) {
		line_width = p_line_width;
		emit_changed();
	}

	float TimelineIndicator::get_line_width() const {
		return line_width;
	}
}