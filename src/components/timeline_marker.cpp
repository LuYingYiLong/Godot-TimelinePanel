#include "timeline_marker.h"

#include <godot_cpp/classes/theme_db.hpp>
#include <godot_cpp/classes/rendering_server.hpp>

namespace godot {
	void TimelineMarker::_bind_methods() {
		ClassDB::bind_method(D_METHOD("set_name", "name"), &TimelineMarker::set_name);
		ClassDB::bind_method(D_METHOD("get_name"), &TimelineMarker::get_name);
		ADD_PROPERTY(PropertyInfo(Variant::STRING, "name"), "set_name", "get_name");

		ClassDB::bind_method(D_METHOD("set_annotation", "annotation"), &TimelineMarker::set_annotation);
		ClassDB::bind_method(D_METHOD("get_annotation"), &TimelineMarker::get_annotation);
		ADD_PROPERTY(PropertyInfo(Variant::STRING, "annotation", PROPERTY_HINT_MULTILINE_TEXT), "set_annotation", "get_annotation");

		ClassDB::bind_method(D_METHOD("set_time", "time"), &TimelineMarker::set_time);
		ClassDB::bind_method(D_METHOD("get_time"), &TimelineMarker::get_time);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "time", PROPERTY_HINT_RANGE, "0,99999,0.01,or_greater,suffix:s"), "set_time", "get_time");
	}

	TimelineMarker::TimelineMarker() {
		text_line.instantiate();
	}

	void TimelineMarker::_draw(
		const RID& p_to_canvas_item,
		const Rect2& p_header_rect,
		const String& p_text,
		const double line_length,
		const bool vertical
	) {
		if (GDVIRTUAL_CALL(_draw, p_to_canvas_item, p_header_rect, p_text, line_length, vertical)) {
			return;
		}

		Point2 pos = vertical ?
			Vector2(p_header_rect.position.x + p_header_rect.size.x / 2.0, p_header_rect.position.y + p_header_rect.size.y / 2.0) :
			Vector2(p_header_rect.position.x, p_header_rect.position.y + p_header_rect.size.y / 2.0);
		if (is_show_line()) {
			RenderingServer::get_singleton()->canvas_item_add_line(
				p_to_canvas_item,
				pos,
				Point2(vertical ? pos.x : pos.x + line_length, vertical ? pos.y + line_length : pos.y),
				get_line_color(),
				get_line_width()
			);
		}

		Ref<StyleBox> style = get_style();
		if (style.is_valid()) {
			style->draw(p_to_canvas_item, p_header_rect);
		}

		text_line->clear();
		text_line->add_string(name, ThemeDB::get_singleton()->get_fallback_font(), get_font_size());
		const float text_x = vertical ? p_header_rect.position.x + (p_header_rect.size.x - text_line->get_size().x) * 0.5f : p_header_rect.position.x + 4.0f;
		text_line->draw(p_to_canvas_item, Vector2(text_x, pos.y - (get_font_size() / 2.0f) + get_font_offset()), get_font_color());
	}

	void TimelineMarker::set_name(const String& p_name) {
		name = p_name;
		emit_changed();
	}

	String TimelineMarker::get_name() const {
		return name;
	}

	void TimelineMarker::set_annotation(const String& p_annotation) {
		annotation = p_annotation;
	}

	String TimelineMarker::get_annotation() const {
		return annotation;
	}

	void TimelineMarker::set_time(const double p_time) {
		time = p_time;
		emit_changed();
	}

	double TimelineMarker::get_time() const {
		return time;
	}
}
