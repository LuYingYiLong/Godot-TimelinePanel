#include "timeline_connection_editor.h"

#include <algorithm>
#include <cmath>
#include <godot_cpp/classes/font.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/input_event_mouse_motion.hpp>
#include <godot_cpp/classes/style_box_flat.hpp>
#include <godot_cpp/classes/theme_db.hpp>
#include <godot_cpp/core/class_db.hpp>

namespace {
	godot::Vector2 sample_cubic_bezier(const godot::Vector2 &p_start, const godot::Vector2 &p_control_a, const godot::Vector2 &p_control_b, const godot::Vector2 &p_end, float p_t) {
		const float inv_t = 1.0f - p_t;
		return p_start * (inv_t * inv_t * inv_t) +
			p_control_a * (3.0f * inv_t * inv_t * p_t) +
			p_control_b * (3.0f * inv_t * p_t * p_t) +
			p_end * (p_t * p_t * p_t);
	}

	double get_nice_ruler_step(double p_raw_step) {
		if (p_raw_step <= 0.0 || !std::isfinite(p_raw_step)) {
			return 1.0;
		}

		const double exponent = std::floor(std::log10(p_raw_step));
		const double base = std::pow(10.0, exponent);
		const double normalized = p_raw_step / base;
		double multiplier = 10.0;
		if (normalized <= 1.0) {
			multiplier = 1.0;
		}
		else if (normalized <= 2.0) {
			multiplier = 2.0;
		}
		else if (normalized <= 5.0) {
			multiplier = 5.0;
		}

		return multiplier * base;
	}

	godot::String format_ruler_value(double p_value) {
		if (std::abs(p_value) < 0.0000001) {
			p_value = 0.0;
		}

		const double abs_value = std::abs(p_value);
		if (abs_value >= 100.0 || std::abs(p_value - std::round(p_value)) < 0.0001) {
			return godot::String::num(p_value, 0);
		}
		if (abs_value >= 10.0) {
			return godot::String::num(p_value, 1);
		}
		if (abs_value >= 1.0) {
			return godot::String::num(p_value, 2);
		}
		return godot::String::num(p_value, 3);
	}
}

namespace godot {
	void TimelineConnectionEditor::_bind_methods() {
		BIND_ENUM_CONSTANT(TIME);
		BIND_ENUM_CONSTANT(FRAME);
		BIND_ENUM_CONSTANT(BEAT);

		ClassDB::bind_method(D_METHOD("create_connection", "from_position", "to_position"), &TimelineConnectionEditor::create_connection, DEFVAL(Vector2()), DEFVAL(Vector2(160.0f, 80.0f)));
		ClassDB::bind_method(D_METHOD("add_connection", "connection"), &TimelineConnectionEditor::add_connection);
		ClassDB::bind_method(D_METHOD("remove_connection", "connection"), &TimelineConnectionEditor::remove_connection);
		ClassDB::bind_method(D_METHOD("remove_connection_at", "index"), &TimelineConnectionEditor::remove_connection_at);
		ClassDB::bind_method(D_METHOD("clear_connections"), &TimelineConnectionEditor::clear_connections);
		ClassDB::bind_method(D_METHOD("get_connections"), &TimelineConnectionEditor::get_connections);
		ClassDB::bind_method(D_METHOD("get_connection", "index"), &TimelineConnectionEditor::get_connection);
		ClassDB::bind_method(D_METHOD("get_connection_count"), &TimelineConnectionEditor::get_connection_count);
		ClassDB::bind_method(D_METHOD("get_connection_index", "connection"), &TimelineConnectionEditor::get_connection_index);
		ClassDB::bind_method(D_METHOD("get_selected_points"), &TimelineConnectionEditor::get_selected_points);

		ClassDB::bind_method(D_METHOD("get_h_scroll_bar"), &TimelineConnectionEditor::get_h_scroll_bar);
		ClassDB::bind_method(D_METHOD("get_v_scroll_bar"), &TimelineConnectionEditor::get_v_scroll_bar);

		ClassDB::bind_method(D_METHOD("set_content_offset", "offset"), &TimelineConnectionEditor::set_content_offset);
		ClassDB::bind_method(D_METHOD("get_content_offset"), &TimelineConnectionEditor::get_content_offset);
		ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "content_offset"), "set_content_offset", "get_content_offset");

		ClassDB::bind_method(D_METHOD("set_content_scale", "scale"), &TimelineConnectionEditor::set_content_scale);
		ClassDB::bind_method(D_METHOD("get_content_scale"), &TimelineConnectionEditor::get_content_scale);
		ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "content_scale"), "set_content_scale", "get_content_scale");

		ClassDB::bind_method(D_METHOD("set_current_time", "time"), &TimelineConnectionEditor::set_current_time);
		ClassDB::bind_method(D_METHOD("get_current_time"), &TimelineConnectionEditor::get_current_time);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "current_time", PROPERTY_HINT_RANGE, "0,99999,0.01,or_greater,suffix:s"), "set_current_time", "get_current_time");

		ClassDB::bind_method(D_METHOD("set_counting_unit", "unit"), &TimelineConnectionEditor::set_counting_unit);
		ClassDB::bind_method(D_METHOD("get_counting_unit"), &TimelineConnectionEditor::get_counting_unit);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "counting_unit", PROPERTY_HINT_ENUM, "Time,Frame,Beat"), "set_counting_unit", "get_counting_unit");

		ADD_GROUP("Components", "component_");
		ClassDB::bind_method(D_METHOD("set_playhead", "playhead"), &TimelineConnectionEditor::set_playhead);
		ClassDB::bind_method(D_METHOD("get_playhead"), &TimelineConnectionEditor::get_playhead);
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "component_playhead", PROPERTY_HINT_RESOURCE_TYPE, "TimelineIndicator"), "set_playhead", "get_playhead");
		ADD_GROUP("", "");

		ClassDB::bind_method(D_METHOD("set_playhead_drag_enabled", "enabled"), &TimelineConnectionEditor::set_playhead_drag_enabled);
		ClassDB::bind_method(D_METHOD("is_playhead_drag_enabled"), &TimelineConnectionEditor::is_playhead_drag_enabled);
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "playhead_drag_enabled"), "set_playhead_drag_enabled", "is_playhead_drag_enabled");

		ClassDB::bind_method(D_METHOD("get_time_from_position", "position"), &TimelineConnectionEditor::get_time_from_position);
		ClassDB::bind_method(D_METHOD("get_frame_from_position", "position"), &TimelineConnectionEditor::get_frame_from_position);
		ClassDB::bind_method(D_METHOD("get_beat_from_position", "position"), &TimelineConnectionEditor::get_beat_from_position);
		ClassDB::bind_method(D_METHOD("get_position_from_time", "time"), &TimelineConnectionEditor::get_position_from_time);
		ClassDB::bind_method(D_METHOD("get_position_from_frame", "frame"), &TimelineConnectionEditor::get_position_from_frame);
		ClassDB::bind_method(D_METHOD("get_position_from_beat", "beat"), &TimelineConnectionEditor::get_position_from_beat);

		ADD_GROUP("Frame", "frame_");
		ClassDB::bind_method(D_METHOD("set_fps", "fps"), &TimelineConnectionEditor::set_fps);
		ClassDB::bind_method(D_METHOD("get_fps"), &TimelineConnectionEditor::get_fps);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "frame_fps", PROPERTY_HINT_RANGE, "1,240,1,or_greater"), "set_fps", "get_fps");
		ADD_GROUP("", "");

		ADD_GROUP("Beat", "");
		ClassDB::bind_method(D_METHOD("set_bpms", "bpms"), &TimelineConnectionEditor::set_bpms);
		ClassDB::bind_method(D_METHOD("get_bpms"), &TimelineConnectionEditor::get_bpms);
		ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "bpms", PROPERTY_HINT_DICTIONARY_TYPE, "float;int"), "set_bpms", "get_bpms");
		ADD_GROUP("", "");

		ClassDB::bind_method(D_METHOD("set_edit_enabled", "enabled"), &TimelineConnectionEditor::set_edit_enabled);
		ClassDB::bind_method(D_METHOD("is_edit_enabled"), &TimelineConnectionEditor::is_edit_enabled);
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "edit_enabled"), "set_edit_enabled", "is_edit_enabled");

		ClassDB::bind_method(D_METHOD("set_handle_radius", "radius"), &TimelineConnectionEditor::set_handle_radius);
		ClassDB::bind_method(D_METHOD("get_handle_radius"), &TimelineConnectionEditor::get_handle_radius);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "handle_radius", PROPERTY_HINT_RANGE, "0,64,0.1,or_greater,suffix:px"), "set_handle_radius", "get_handle_radius");

		ADD_GROUP("Scroll", "scroll_");
		ClassDB::bind_method(D_METHOD("set_scroll_offset", "offset"), &TimelineConnectionEditor::set_scroll_offset);
		ClassDB::bind_method(D_METHOD("get_scroll_offset"), &TimelineConnectionEditor::get_scroll_offset);
		ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "scroll_offset"), "set_scroll_offset", "get_scroll_offset");

		ClassDB::bind_method(D_METHOD("set_scroll_step", "step"), &TimelineConnectionEditor::set_scroll_step);
		ClassDB::bind_method(D_METHOD("get_scroll_step"), &TimelineConnectionEditor::get_scroll_step);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "scroll_step", PROPERTY_HINT_RANGE, "0,4096,0.01,or_greater"), "set_scroll_step", "get_scroll_step");

		ClassDB::bind_method(D_METHOD("set_scroll_enabled", "enabled"), &TimelineConnectionEditor::set_scroll_enabled);
		ClassDB::bind_method(D_METHOD("is_scroll_enabled"), &TimelineConnectionEditor::is_scroll_enabled);
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "scroll_enabled"), "set_scroll_enabled", "is_scroll_enabled");

		ADD_GROUP("Range", "range_");
		ClassDB::bind_method(D_METHOD("set_range_limited", "limited"), &TimelineConnectionEditor::set_range_limited);
		ClassDB::bind_method(D_METHOD("is_range_limited"), &TimelineConnectionEditor::is_range_limited);
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "range_limited", PROPERTY_HINT_GROUP_ENABLE), "set_range_limited", "is_range_limited");

		ClassDB::bind_method(D_METHOD("set_range_min", "min"), &TimelineConnectionEditor::set_range_min);
		ClassDB::bind_method(D_METHOD("get_range_min"), &TimelineConnectionEditor::get_range_min);

		ClassDB::bind_method(D_METHOD("set_range_max", "max"), &TimelineConnectionEditor::set_range_max);
		ClassDB::bind_method(D_METHOD("get_range_max"), &TimelineConnectionEditor::get_range_max);

		ClassDB::bind_method(D_METHOD("set_range_start_time", "time"), &TimelineConnectionEditor::set_range_start_time);
		ClassDB::bind_method(D_METHOD("get_range_start_time"), &TimelineConnectionEditor::get_range_start_time);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "range_start_time", PROPERTY_HINT_RANGE, "0,99999,0.01,or_greater,suffix:s"), "set_range_start_time", "get_range_start_time");

		ClassDB::bind_method(D_METHOD("set_range_end_time", "time"), &TimelineConnectionEditor::set_range_end_time);
		ClassDB::bind_method(D_METHOD("get_range_end_time"), &TimelineConnectionEditor::get_range_end_time);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "range_end_time", PROPERTY_HINT_RANGE, "0,99999,0.01,or_greater,suffix:s"), "set_range_end_time", "get_range_end_time");

		ClassDB::bind_method(D_METHOD("set_range_min_y", "y"), &TimelineConnectionEditor::set_range_min_y);
		ClassDB::bind_method(D_METHOD("get_range_min_y"), &TimelineConnectionEditor::get_range_min_y);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "range_min_y"), "set_range_min_y", "get_range_min_y");

		ClassDB::bind_method(D_METHOD("set_range_max_y", "y"), &TimelineConnectionEditor::set_range_max_y);
		ClassDB::bind_method(D_METHOD("get_range_max_y"), &TimelineConnectionEditor::get_range_max_y);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "range_max_y"), "set_range_max_y", "get_range_max_y");

		ADD_GROUP("Ruler", "ruler_");
		ClassDB::bind_method(D_METHOD("set_ruler_enabled", "enabled"), &TimelineConnectionEditor::set_ruler_enabled);
		ClassDB::bind_method(D_METHOD("is_ruler_enabled"), &TimelineConnectionEditor::is_ruler_enabled);
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "ruler_enabled", PROPERTY_HINT_GROUP_ENABLE), "set_ruler_enabled", "is_ruler_enabled");

		ClassDB::bind_method(D_METHOD("set_ruler_size", "size"), &TimelineConnectionEditor::set_ruler_size);
		ClassDB::bind_method(D_METHOD("get_ruler_size"), &TimelineConnectionEditor::get_ruler_size);
		ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "ruler_size"), "set_ruler_size", "get_ruler_size");

		ClassDB::bind_method(D_METHOD("set_ruler_min_tick_spacing", "spacing"), &TimelineConnectionEditor::set_ruler_min_tick_spacing);
		ClassDB::bind_method(D_METHOD("get_ruler_min_tick_spacing"), &TimelineConnectionEditor::get_ruler_min_tick_spacing);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "ruler_min_tick_spacing", PROPERTY_HINT_RANGE, "12,512,1,or_greater,suffix:px"), "set_ruler_min_tick_spacing", "get_ruler_min_tick_spacing");

		ClassDB::bind_method(D_METHOD("set_ruler_font_size", "size"), &TimelineConnectionEditor::set_ruler_font_size);
		ClassDB::bind_method(D_METHOD("get_ruler_font_size"), &TimelineConnectionEditor::get_ruler_font_size);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "ruler_font_size", PROPERTY_HINT_RANGE, "6,48,1,or_greater,suffix:px"), "set_ruler_font_size", "get_ruler_font_size");

		ClassDB::bind_method(D_METHOD("set_ruler_background_color", "color"), &TimelineConnectionEditor::set_ruler_background_color);
		ClassDB::bind_method(D_METHOD("get_ruler_background_color"), &TimelineConnectionEditor::get_ruler_background_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "ruler_background_color"), "set_ruler_background_color", "get_ruler_background_color");

		ClassDB::bind_method(D_METHOD("set_ruler_tick_color", "color"), &TimelineConnectionEditor::set_ruler_tick_color);
		ClassDB::bind_method(D_METHOD("get_ruler_tick_color"), &TimelineConnectionEditor::get_ruler_tick_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "ruler_tick_color"), "set_ruler_tick_color", "get_ruler_tick_color");

		ClassDB::bind_method(D_METHOD("set_ruler_text_color", "color"), &TimelineConnectionEditor::set_ruler_text_color);
		ClassDB::bind_method(D_METHOD("get_ruler_text_color"), &TimelineConnectionEditor::get_ruler_text_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "ruler_text_color"), "set_ruler_text_color", "get_ruler_text_color");

		ClassDB::bind_method(D_METHOD("set_ruler_major_grid_color", "color"), &TimelineConnectionEditor::set_ruler_major_grid_color);
		ClassDB::bind_method(D_METHOD("get_ruler_major_grid_color"), &TimelineConnectionEditor::get_ruler_major_grid_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "ruler_major_grid_color"), "set_ruler_major_grid_color", "get_ruler_major_grid_color");

		ClassDB::bind_method(D_METHOD("set_ruler_minor_grid_color", "color"), &TimelineConnectionEditor::set_ruler_minor_grid_color);
		ClassDB::bind_method(D_METHOD("get_ruler_minor_grid_color"), &TimelineConnectionEditor::get_ruler_minor_grid_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "ruler_minor_grid_color"), "set_ruler_minor_grid_color", "get_ruler_minor_grid_color");

		ADD_GROUP("Style Overrides", "");
		ADD_SUBGROUP("Constants", "");
		ClassDB::bind_method(D_METHOD("set_key_scale", "scale"), &TimelineConnectionEditor::set_key_scale);
		ClassDB::bind_method(D_METHOD("get_key_scale"), &TimelineConnectionEditor::get_key_scale);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "key_scale", PROPERTY_HINT_RANGE, "0,8,0.01,or_greater"), "set_key_scale", "get_key_scale");

		ClassDB::bind_method(D_METHOD("set_handle_scale", "scale"), &TimelineConnectionEditor::set_handle_scale);
		ClassDB::bind_method(D_METHOD("get_handle_scale"), &TimelineConnectionEditor::get_handle_scale);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "handle_scale", PROPERTY_HINT_RANGE, "0,8,0.01,or_greater"), "set_handle_scale", "get_handle_scale");

		ClassDB::bind_method(D_METHOD("set_handle_line_width", "width"), &TimelineConnectionEditor::set_handle_line_width);
		ClassDB::bind_method(D_METHOD("get_handle_line_width"), &TimelineConnectionEditor::get_handle_line_width);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "handle_line_width", PROPERTY_HINT_RANGE, "0,32,0.1,or_greater,suffix:px"), "set_handle_line_width", "get_handle_line_width");

		ClassDB::bind_method(D_METHOD("set_bezier_line_width", "width"), &TimelineConnectionEditor::set_bezier_line_width);
		ClassDB::bind_method(D_METHOD("get_bezier_line_width"), &TimelineConnectionEditor::get_bezier_line_width);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "bezier_line_width", PROPERTY_HINT_RANGE, "0,32,0.1,or_greater,suffix:px"), "set_bezier_line_width", "get_bezier_line_width");

		ADD_SUBGROUP("Colors", "");
		ClassDB::bind_method(D_METHOD("set_handle_line_color", "color"), &TimelineConnectionEditor::set_handle_line_color);
		ClassDB::bind_method(D_METHOD("get_handle_line_color"), &TimelineConnectionEditor::get_handle_line_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "handle_line_color"), "set_handle_line_color", "get_handle_line_color");

		ClassDB::bind_method(D_METHOD("set_handle_line_selected_color", "color"), &TimelineConnectionEditor::set_handle_line_selected_color);
		ClassDB::bind_method(D_METHOD("get_handle_line_selected_color"), &TimelineConnectionEditor::get_handle_line_selected_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "handle_line_selected_color"), "set_handle_line_selected_color", "get_handle_line_selected_color");

		ADD_SUBGROUP("Styles", "");
		ClassDB::bind_method(D_METHOD("set_key_normal_style", "style"), &TimelineConnectionEditor::set_key_normal_style);
		ClassDB::bind_method(D_METHOD("get_key_normal_style"), &TimelineConnectionEditor::get_key_normal_style);
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "key_normal", PROPERTY_HINT_RESOURCE_TYPE, "StyleBox"), "set_key_normal_style", "get_key_normal_style");

		ClassDB::bind_method(D_METHOD("set_key_selected_style", "style"), &TimelineConnectionEditor::set_key_selected_style);
		ClassDB::bind_method(D_METHOD("get_key_selected_style"), &TimelineConnectionEditor::get_key_selected_style);
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "key_selected", PROPERTY_HINT_RESOURCE_TYPE, "StyleBox"), "set_key_selected_style", "get_key_selected_style");

		ClassDB::bind_method(D_METHOD("set_handle_style", "style"), &TimelineConnectionEditor::set_handle_style);
		ClassDB::bind_method(D_METHOD("get_handle_style"), &TimelineConnectionEditor::get_handle_style);
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "handle", PROPERTY_HINT_RESOURCE_TYPE, "StyleBox"), "set_handle_style", "get_handle_style");
		ADD_GROUP("", "");

		GDVIRTUAL_BIND(_draw_connection, "to_canvas_item", "connection", "from_position", "to_position");

		ADD_SIGNAL(MethodInfo("current_time_changed", PropertyInfo(Variant::FLOAT, "time")));
		ADD_SIGNAL(MethodInfo("playhead_drag_started", PropertyInfo(Variant::FLOAT, "time")));
		ADD_SIGNAL(MethodInfo("playhead_dragged", PropertyInfo(Variant::FLOAT, "time")));
		ADD_SIGNAL(MethodInfo("playhead_drag_ended", PropertyInfo(Variant::FLOAT, "time")));
		ADD_SIGNAL(MethodInfo("selection_changed", PropertyInfo(Variant::ARRAY, "points", PROPERTY_HINT_ARRAY_TYPE, "TimelineConnectionPoint")));
		ADD_SIGNAL(MethodInfo("connection_point_selected",
				PropertyInfo(Variant::OBJECT, "connection", PROPERTY_HINT_RESOURCE_TYPE, "TimelineConnection"),
				PropertyInfo(Variant::OBJECT, "point"),
				PropertyInfo(Variant::INT, "point_index")));
		ADD_SIGNAL(MethodInfo("connection_point_drag_started",
				PropertyInfo(Variant::OBJECT, "connection", PROPERTY_HINT_RESOURCE_TYPE, "TimelineConnection"),
				PropertyInfo(Variant::OBJECT, "point"),
				PropertyInfo(Variant::INT, "point_index"),
				PropertyInfo(Variant::INT, "target")));
		ADD_SIGNAL(MethodInfo("connection_point_dragged",
				PropertyInfo(Variant::OBJECT, "connection", PROPERTY_HINT_RESOURCE_TYPE, "TimelineConnection"),
				PropertyInfo(Variant::OBJECT, "point"),
				PropertyInfo(Variant::INT, "point_index"),
				PropertyInfo(Variant::INT, "target"),
				PropertyInfo(Variant::VECTOR2, "position")));
		ADD_SIGNAL(MethodInfo("connection_point_drag_ended",
				PropertyInfo(Variant::OBJECT, "connection", PROPERTY_HINT_RESOURCE_TYPE, "TimelineConnection"),
				PropertyInfo(Variant::OBJECT, "point"),
				PropertyInfo(Variant::INT, "point_index"),
				PropertyInfo(Variant::INT, "target")));
	}

	void TimelineConnectionEditor::_validate_property(PropertyInfo &p_property) const {
		const String name = p_property.name;
		switch (counting_unit) {
		case TIME:
			if (name.begins_with("frame_") || name.begins_with("beat_")) {
				p_property.usage = PROPERTY_USAGE_NO_EDITOR;
			}
			break;
		case FRAME:
			if (name.begins_with("beat_")) {
				p_property.usage = PROPERTY_USAGE_NO_EDITOR;
			}
			break;
		case BEAT:
			if (name.begins_with("frame_")) {
				p_property.usage = PROPERTY_USAGE_NO_EDITOR;
			}
			break;
		}
	}

	void TimelineConnectionEditor::_notification(int p_what) {
		switch (p_what) {
		case NOTIFICATION_RESIZED:
			_update_scroll_bars();
			queue_redraw();
			break;
		case NOTIFICATION_DRAW:
			_draw_ruler_guides();
			_draw_connections();
			_draw_ruler_overlays();
			_draw_playhead();
			break;
		}
	}

	Vector2 TimelineConnectionEditor::_get_content_origin() const {
		if (!ruler_enabled) {
			return content_offset;
		}
		return content_offset + Vector2(MAX(ruler_size.x, 0.0f), MAX(ruler_size.y, 0.0f));
	}

	Rect2 TimelineConnectionEditor::_get_content_rect() const {
		const float hscroll_height = hscroll != nullptr && hscroll->is_visible() ? hscroll->get_combined_minimum_size().y : 0.0f;
		const float vscroll_width = vscroll != nullptr && vscroll->is_visible() ? vscroll->get_combined_minimum_size().x : 0.0f;
		if (!ruler_enabled) {
			return Rect2(
				Vector2(),
				Vector2(MAX(get_size().x - vscroll_width, 0.0f), MAX(get_size().y - hscroll_height, 0.0f))
			);
		}

		const float ruler_width = MIN(MAX(ruler_size.x, 0.0f), get_size().x);
		const float ruler_height = MIN(MAX(ruler_size.y, 0.0f), get_size().y);
		return Rect2(
			Vector2(ruler_width, ruler_height),
			Vector2(MAX(get_size().x - ruler_width - vscroll_width, 0.0f), MAX(get_size().y - ruler_height - hscroll_height, 0.0f))
		);
	}

	Rect2 TimelineConnectionEditor::_get_screen_range_rect() const {
		if (!range_limited) {
			return _get_content_rect();
		}

		const float min_x = MIN(range_min.x, range_max.x);
		const float max_x = MAX(range_min.x, range_max.x);
		const float min_y = MIN(range_min.y, range_max.y);
		const float max_y = MAX(range_min.y, range_max.y);
		const Vector2 top_left = _content_to_screen(Vector2(min_x, min_y));
		const Vector2 bottom_right = _content_to_screen(Vector2(max_x, max_y));
		return Rect2(
			Vector2(MIN(top_left.x, bottom_right.x), MIN(top_left.y, bottom_right.y)),
			Vector2(Math::abs(bottom_right.x - top_left.x), Math::abs(bottom_right.y - top_left.y))
		);
	}

	Rect2 TimelineConnectionEditor::_get_draw_clip_rect() const {
		const Rect2 content_rect = _get_content_rect();
		if (!range_limited) {
			return content_rect;
		}
		return content_rect.intersection(_get_screen_range_rect());
	}

	Vector2 TimelineConnectionEditor::_content_to_screen(const Vector2 &p_position) const {
		return _get_content_origin() + Vector2((p_position.x - scroll_offset.x) * content_scale.x, (p_position.y - scroll_offset.y) * content_scale.y);
	}

	Vector2 TimelineConnectionEditor::_content_delta_to_screen(const Vector2 &p_delta) const {
		return Vector2(p_delta.x * content_scale.x, p_delta.y * content_scale.y);
	}

	Vector2 TimelineConnectionEditor::_screen_to_content(const Vector2 &p_position) const {
		const Vector2 origin = _get_content_origin();
		const float scale_x = Math::abs(content_scale.x) < 0.0001f ? 1.0f : content_scale.x;
		const float scale_y = Math::abs(content_scale.y) < 0.0001f ? 1.0f : content_scale.y;
		return Vector2(
			(p_position.x - origin.x) / scale_x + scroll_offset.x,
			(p_position.y - origin.y) / scale_y + scroll_offset.y
		);
	}

	std::vector<TimelineConnectionEditor::BPM> TimelineConnectionEditor::_get_sorted_bpms() const {
		std::vector<BPM> result;
		Array keys = bpms.keys();
		result.reserve(keys.size());

		for (int index = 0; index < keys.size(); ++index) {
			const double time = keys[index];
			const Variant value = bpms.get(time, Variant());
			BPM point;
			point.time = time;

			if (value.get_type() == Variant::DICTIONARY) {
				Dictionary bpm_data = value;
				point.beat = bpm_data.get("beat", 0.0);
				point.bpm = bpm_data.get("bpm", 120.0);
				point.has_beat = true;
			}
			else if (value.get_type() == Variant::FLOAT || value.get_type() == Variant::INT) {
				point.bpm = value;
				point.has_beat = false;
			}
			else {
				point.bpm = 120.0;
				point.has_beat = false;
			}

			if (point.bpm <= 0.0) {
				point.bpm = 120.0;
			}
			result.push_back(point);
		}

		std::sort(result.begin(), result.end(), [](const BPM &p_a, const BPM &p_b) {
			if (p_a.time == p_b.time) {
				return p_a.beat < p_b.beat;
			}
			return p_a.time < p_b.time;
		});

		if (result.empty()) {
			result.push_back(BPM(0.0, 0.0, 120.0));
			return result;
		}

		if (result.front().time > 0.0) {
			result.insert(result.begin(), BPM(0.0, 0.0, result.front().bpm));
		}

		for (int index = 0; index < static_cast<int>(result.size()); ++index) {
			if (result[index].has_beat) {
				continue;
			}
			if (index == 0) {
				result[index].beat = 0.0;
				result[index].has_beat = true;
				continue;
			}

			const BPM &previous = result[index - 1];
			const double sec_delta = MAX(result[index].time - previous.time, 0.0);
			result[index].beat = previous.beat + sec_delta * previous.bpm / 60.0;
			result[index].has_beat = true;
		}

		return result;
	}

	double TimelineConnectionEditor::_time_to_beat(double p_time) const {
		const std::vector<BPM> bpm_points = _get_sorted_bpms();
		if (bpm_points.empty()) {
			return 0.0;
		}

		const BPM *point = &bpm_points.front();
		for (int index = 1; index < static_cast<int>(bpm_points.size()); ++index) {
			const BPM &next = bpm_points[index];
			if (p_time < next.time) {
				break;
			}
			point = &next;
		}

		const double bpm = point->bpm <= 0.0 ? 120.0 : point->bpm;
		return point->beat + (p_time - point->time) * bpm / 60.0;
	}

	double TimelineConnectionEditor::_beat_to_time(double p_beat) const {
		const std::vector<BPM> bpm_points = _get_sorted_bpms();
		if (bpm_points.empty()) {
			return 0.0;
		}

		const BPM *point = &bpm_points.front();
		for (int index = 1; index < static_cast<int>(bpm_points.size()); ++index) {
			const BPM &next = bpm_points[index];
			if (p_beat < next.beat) {
				break;
			}
			point = &next;
		}

		const double bpm = point->bpm <= 0.0 ? 120.0 : point->bpm;
		return point->time + (p_beat - point->beat) * 60.0 / bpm;
	}

	double TimelineConnectionEditor::_time_to_unit(double p_time) const {
		switch (counting_unit) {
		case FRAME:
			return p_time * static_cast<double>(fps);
		case BEAT:
			return _time_to_beat(p_time);
		case TIME:
		default:
			return p_time;
		}
	}

	double TimelineConnectionEditor::_unit_to_time(double p_value) const {
		switch (counting_unit) {
		case FRAME:
			return p_value / static_cast<double>(MAX(fps, 1));
		case BEAT:
			return _beat_to_time(p_value);
		case TIME:
		default:
			return p_value;
		}
	}

	String TimelineConnectionEditor::_format_playhead_time(double p_time) const {
		switch (counting_unit) {
		case FRAME:
			return String::num_int64(static_cast<int64_t>(Math::floor(p_time * static_cast<double>(fps) + 0.000001)));
		case BEAT:
			return format_ruler_value(_time_to_beat(p_time));
		case TIME:
		default:
			return String::num(p_time, 3);
		}
	}

	String TimelineConnectionEditor::_format_ruler_x_value(double p_value) const {
		switch (counting_unit) {
		case FRAME:
			return String::num_int64(static_cast<int64_t>(Math::floor(_time_to_unit(p_value) + 0.000001)));
		case BEAT:
			return format_ruler_value(_time_to_unit(p_value));
		case TIME:
		default:
			return format_ruler_value(p_value);
		}
	}

	Vector2 TimelineConnectionEditor::_clamp_content_position(const Vector2 &p_position) const {
		if (!range_limited) {
			return p_position;
		}

		const float min_x = MIN(range_min.x, range_max.x);
		const float max_x = MAX(range_min.x, range_max.x);
		const float min_y = MIN(range_min.y, range_max.y);
		const float max_y = MAX(range_min.y, range_max.y);
		return Vector2(
			CLAMP(p_position.x, min_x, max_x),
			CLAMP(p_position.y, min_y, max_y)
		);
	}

	Vector2 TimelineConnectionEditor::_clamp_scroll_offset(const Vector2 &p_offset) const {
		if (!range_limited) {
			return p_offset;
		}

		const Rect2 content_rect = _get_content_rect();
		const float scale_x = Math::abs(content_scale.x) < 0.0001f ? 1.0f : Math::abs(content_scale.x);
		const float scale_y = Math::abs(content_scale.y) < 0.0001f ? 1.0f : Math::abs(content_scale.y);
		const float view_width = content_rect.size.x / scale_x;
		const float view_height = content_rect.size.y / scale_y;
		const float min_x = MIN(range_min.x, range_max.x);
		const float max_x = MAX(range_min.x, range_max.x);
		const float min_y = MIN(range_min.y, range_max.y);
		const float max_y = MAX(range_min.y, range_max.y);
		const float max_scroll_x = MAX(min_x, max_x - view_width);
		const float max_scroll_y = MAX(min_y, max_y - view_height);

		return Vector2(
			CLAMP(p_offset.x, min_x, max_scroll_x),
			CLAMP(p_offset.y, min_y, max_scroll_y)
		);
	}

	bool TimelineConnectionEditor::_clip_line_to_rect(Vector2 &r_from, Vector2 &r_to, const Rect2 &p_rect) const {
		if (p_rect.size.x <= 0.0f || p_rect.size.y <= 0.0f) {
			return false;
		}

		const float x_min = p_rect.position.x;
		const float y_min = p_rect.position.y;
		const float x_max = p_rect.position.x + p_rect.size.x;
		const float y_max = p_rect.position.y + p_rect.size.y;
		float t0 = 0.0f;
		float t1 = 1.0f;
		const Vector2 delta = r_to - r_from;

		auto clip_test = [&](float p, float q) {
			if (Math::is_zero_approx(p)) {
				return q >= 0.0f;
			}

			const float r = q / p;
			if (p < 0.0f) {
				if (r > t1) {
					return false;
				}
				if (r > t0) {
					t0 = r;
				}
			}
			else {
				if (r < t0) {
					return false;
				}
				if (r < t1) {
					t1 = r;
				}
			}
			return true;
		};

		if (!clip_test(-delta.x, r_from.x - x_min) ||
				!clip_test(delta.x, x_max - r_from.x) ||
				!clip_test(-delta.y, r_from.y - y_min) ||
				!clip_test(delta.y, y_max - r_from.y)) {
			return false;
		}

		const Vector2 original_from = r_from;
		if (t1 < 1.0f) {
			r_to = original_from + delta * t1;
		}
		if (t0 > 0.0f) {
			r_from = original_from + delta * t0;
		}
		return true;
	}

	void TimelineConnectionEditor::_draw_clipped_line(const Vector2 &p_from, const Vector2 &p_to, const Color &p_color, float p_width, const Rect2 &p_rect) {
		Vector2 clipped_from = p_from;
		Vector2 clipped_to = p_to;
		if (_clip_line_to_rect(clipped_from, clipped_to, p_rect)) {
			draw_line(clipped_from, clipped_to, p_color, p_width);
		}
	}

	void TimelineConnectionEditor::_draw_clipped_circle(const Vector2 &p_position, float p_radius, const Color &p_color, const Rect2 &p_rect) {
		if (p_rect.size.x < p_radius * 2.0f || p_rect.size.y < p_radius * 2.0f) {
			return;
		}

		if (p_rect.grow(-p_radius).has_point(p_position)) {
			draw_circle(p_position, p_radius, p_color);
		}
	}

	void TimelineConnectionEditor::_draw_clipped_style_box(const Vector2 &p_position, float p_size, const Ref<StyleBox> &p_style, const Rect2 &p_rect) {
		if (p_size <= 0.0f || p_style.is_null()) {
			return;
		}

		const Rect2 rect(p_position - Vector2(p_size * 0.5f, p_size * 0.5f), Vector2(p_size, p_size));
		if (p_rect.intersects(rect)) {
			draw_style_box(p_style, rect);
		}
	}

	Ref<StyleBox> TimelineConnectionEditor::_get_key_normal_style() const {
		if (style_cache.key_normal.is_valid()) {
			return style_cache.key_normal;
		}
		return style_cache.key_normal_fallback;
	}

	Ref<StyleBox> TimelineConnectionEditor::_get_key_selected_style() const {
		if (style_cache.key_selected.is_valid()) {
			return style_cache.key_selected;
		}
		return style_cache.key_selected_fallback;
	}

	Ref<StyleBox> TimelineConnectionEditor::_get_handle_style() const {
		if (style_cache.handle.is_valid()) {
			return style_cache.handle;
		}
		return style_cache.handle_fallback;
	}

	float TimelineConnectionEditor::_get_smart_scroll_step(bool p_horizontal) const {
		const Rect2 content_rect = _get_content_rect();
		const float scale_value = p_horizontal ? content_scale.x : content_scale.y;
		const float safe_scale = Math::abs(scale_value) < 0.0001f ? 1.0f : Math::abs(scale_value);
		const float viewport_size = p_horizontal ? content_rect.size.x : content_rect.size.y;
		const float visible_units = viewport_size / safe_scale;
		return CLAMP(visible_units * 0.12f, 0.25f, 256.0f);
	}

	void TimelineConnectionEditor::_scroll(const Vector2 &p_delta) {
		set_scroll_offset(scroll_offset + p_delta);
	}

	void TimelineConnectionEditor::_zoom_at_position(const Vector2 &p_position, float p_factor) {
		if (p_factor <= 0.0f) {
			return;
		}

		const Vector2 content_before = _screen_to_content(p_position);
		const Vector2 origin = _get_content_origin();
		const float sign_x = content_scale.x < 0.0f ? -1.0f : 1.0f;
		const float sign_y = content_scale.y < 0.0f ? -1.0f : 1.0f;
		const float next_scale_x = CLAMP(Math::abs(content_scale.x) * p_factor, 0.01f, 4096.0f) * sign_x;
		const float next_scale_y = CLAMP(Math::abs(content_scale.y) * p_factor, 0.01f, 4096.0f) * sign_y;
		content_scale = Vector2(next_scale_x, next_scale_y);
		scroll_offset = Vector2(
			content_before.x - (p_position.x - origin.x) / next_scale_x,
			content_before.y - (p_position.y - origin.y) / next_scale_y
		);
		_update_scroll_bars();
		queue_redraw();
	}

	void TimelineConnectionEditor::_pan_view(const Vector2 &p_screen_delta) {
		const float scale_x = Math::abs(content_scale.x) < 0.0001f ? 1.0f : content_scale.x;
		const float scale_y = Math::abs(content_scale.y) < 0.0001f ? 1.0f : content_scale.y;
		_scroll(Vector2(-p_screen_delta.x / scale_x, -p_screen_delta.y / scale_y));
	}

	void TimelineConnectionEditor::_update_scroll_bars() {
		if (hscroll == nullptr || vscroll == nullptr) {
			return;
		}

		const Size2 hmin = hscroll->get_combined_minimum_size();
		const Size2 vmin = vscroll->get_combined_minimum_size();
		const float ruler_width = ruler_enabled ? MIN(MAX(ruler_size.x, 0.0f), get_size().x) : 0.0f;
		const float ruler_height = ruler_enabled ? MIN(MAX(ruler_size.y, 0.0f), get_size().y) : 0.0f;
		const float range_width = Math::abs(range_max.x - range_min.x) * Math::abs(content_scale.x);
		const float range_height = Math::abs(range_max.y - range_min.y) * Math::abs(content_scale.y);

		bool h_scroll_show = false;
		bool v_scroll_show = false;
		if (range_limited && scroll_enabled) {
			for (int i = 0; i < 3; i++) {
				const float available_width = MAX(get_size().x - ruler_width - (v_scroll_show ? vmin.x : 0.0f), 0.0f);
				const float available_height = MAX(get_size().y - ruler_height - (h_scroll_show ? hmin.y : 0.0f), 0.0f);
				const bool new_h_scroll_show = range_width > available_width;
				const bool new_v_scroll_show = range_height > available_height;
				if (new_h_scroll_show == h_scroll_show && new_v_scroll_show == v_scroll_show) {
					break;
				}
				h_scroll_show = new_h_scroll_show;
				v_scroll_show = new_v_scroll_show;
			}
		}

		const float available_width = MAX(get_size().x - ruler_width - (v_scroll_show ? vmin.x : 0.0f), 0.0f);
		const float available_height = MAX(get_size().y - ruler_height - (h_scroll_show ? hmin.y : 0.0f), 0.0f);
		const float scale_x = Math::abs(content_scale.x) < 0.0001f ? 1.0f : Math::abs(content_scale.x);
		const float scale_y = Math::abs(content_scale.y) < 0.0001f ? 1.0f : Math::abs(content_scale.y);
		const float page_x = available_width / scale_x;
		const float page_y = available_height / scale_y;
		const float min_x = MIN(range_min.x, range_max.x);
		const float max_x = MAX(range_min.x, range_max.x);
		const float min_y = MIN(range_min.y, range_max.y);
		const float max_y = MAX(range_min.y, range_max.y);

		updating_scroll = true;

		if (h_scroll_show) {
			hscroll->set_anchor_and_offset(SIDE_LEFT, ANCHOR_BEGIN, ruler_width);
			hscroll->set_anchor_and_offset(SIDE_RIGHT, ANCHOR_END, v_scroll_show ? -vmin.x : 0.0f);
			hscroll->set_anchor_and_offset(SIDE_TOP, ANCHOR_END, -hmin.y);
			hscroll->set_anchor_and_offset(SIDE_BOTTOM, ANCHOR_END, 0.0f);
			hscroll->set_min(min_x);
			hscroll->set_max(max_x);
			hscroll->set_page(page_x);
			hscroll->set_custom_step(_get_smart_scroll_step(true));
			hscroll->show();
		}
		else {
			hscroll->hide();
		}

		if (v_scroll_show) {
			vscroll->set_anchor_and_offset(SIDE_LEFT, ANCHOR_END, -vmin.x);
			vscroll->set_anchor_and_offset(SIDE_RIGHT, ANCHOR_END, 0.0f);
			vscroll->set_anchor_and_offset(SIDE_TOP, ANCHOR_BEGIN, ruler_height);
			vscroll->set_anchor_and_offset(SIDE_BOTTOM, ANCHOR_END, h_scroll_show ? -hmin.y : 0.0f);
			vscroll->set_min(min_y);
			vscroll->set_max(max_y);
			vscroll->set_page(page_y);
			vscroll->set_custom_step(_get_smart_scroll_step(false));
			vscroll->show();
		}
		else {
			vscroll->hide();
		}

		scroll_offset = _clamp_scroll_offset(scroll_offset);
		hscroll->set_value_no_signal(h_scroll_show ? scroll_offset.x : (range_limited ? min_x : scroll_offset.x));
		vscroll->set_value_no_signal(v_scroll_show ? scroll_offset.y : (range_limited ? min_y : scroll_offset.y));
		updating_scroll = false;
	}

	void TimelineConnectionEditor::_h_scroll_changed(double p_value) {
		if (updating_scroll) {
			return;
		}

		scroll_offset.x = static_cast<float>(p_value);
		queue_redraw();
	}

	void TimelineConnectionEditor::_v_scroll_changed(double p_value) {
		if (updating_scroll) {
			return;
		}

		scroll_offset.y = static_cast<float>(p_value);
		queue_redraw();
	}

	bool TimelineConnectionEditor::_is_playhead_hit_at_position(const Vector2 &p_position) const {
		if (!playhead_drag_enabled || playhead.is_null()) {
			return false;
		}

		const Rect2 content_rect = _get_content_rect();
		const Rect2 clip_rect = _get_draw_clip_rect();
		if (content_rect.size.x <= 0.0f || content_rect.size.y <= 0.0f || clip_rect.size.x <= 0.0f || clip_rect.size.y <= 0.0f) {
			return false;
		}

		const float x = static_cast<float>(get_position_from_time(current_time));
		const float header_height = ruler_enabled ? MAX(content_rect.position.y, 16.0f) : 16.0f;
		const Rect2 header_rect(Vector2(x - 24.0f, 0.0f), Vector2(48.0f, header_height));
		if (header_rect.has_point(p_position)) {
			return true;
		}

		const float hit_width = 6.0f;
		const Rect2 line_rect(Vector2(x - hit_width, content_rect.position.y), Vector2(hit_width * 2.0f, content_rect.size.y));
		return line_rect.has_point(p_position) && clip_rect.has_point(p_position);
	}

	bool TimelineConnectionEditor::_is_playhead_drag_area_at_position(const Vector2 &p_position) const {
		if (!playhead_drag_enabled) {
			return false;
		}

		if (_is_playhead_hit_at_position(p_position)) {
			return true;
		}

		if (!ruler_enabled) {
			return false;
		}

		const Rect2 content_rect = _get_content_rect();
		if (content_rect.size.x <= 0.0f || content_rect.position.y <= 0.0f) {
			return false;
		}

		const Rect2 time_ruler_rect(Vector2(content_rect.position.x, 0.0f), Vector2(content_rect.size.x, content_rect.position.y));
		return time_ruler_rect.has_point(p_position);
	}

	double TimelineConnectionEditor::_get_time_at_position(const Vector2 &p_position) const {
		double time = _screen_to_content(p_position).x;
		if (range_limited) {
			const double min_time = MIN(range_min.x, range_max.x);
			const double max_time = MAX(range_min.x, range_max.x);
			time = CLAMP(time, min_time, max_time);
		}
		return time;
	}

	TypedArray<TimelineConnectionPoint> TimelineConnectionEditor::_get_selected_points() const {
		TypedArray<TimelineConnectionPoint> selected_points;
		for (int i = 0; i < connections.size(); i++) {
			Ref<TimelineConnection> connection = connections[i];
			if (connection.is_null()) {
				continue;
			}

			const int point_count = connection->get_point_count();
			for (int point_index = 0; point_index < point_count; point_index++) {
				TimelineConnectionPoint* point = connection->get_point(point_index);
				if (point != nullptr && point->is_selected()) {
					selected_points.append(point);
				}
			}
		}
		return selected_points;
	}

	bool TimelineConnectionEditor::_clear_point_selection() {
		bool changed = false;
		for (int i = 0; i < connections.size(); i++) {
			Ref<TimelineConnection> connection = connections[i];
			if (connection.is_null()) {
				continue;
			}

			const int point_count = connection->get_point_count();
			for (int point_index = 0; point_index < point_count; point_index++) {
				TimelineConnectionPoint* point = connection->get_point(point_index);
				if (point != nullptr && point->is_selected()) {
					point->set_selected_no_signal(false);
					changed = true;
				}
			}
		}
		return changed;
	}

	void TimelineConnectionEditor::_select_point(const Ref<TimelineConnection> &p_connection, int p_point_index, bool p_additive, bool p_toggle) {
		if (p_connection.is_null() || p_point_index < 0 || p_point_index >= p_connection->get_point_count()) {
			return;
		}

		bool changed = false;
		TimelineConnectionPoint* point = p_connection->get_point(p_point_index);
		if (!p_additive) {
			changed = _clear_point_selection();
		}

		if (point != nullptr) {
			const bool selected = point->is_selected();
			const bool next_selected = p_toggle ? !selected : true;
			if (selected != next_selected) {
				point->set_selected_no_signal(next_selected);
				changed = true;
			}
			if (next_selected) {
				emit_signal("connection_point_selected", p_connection, point, p_point_index);
			}
		}

		if (changed) {
			_emit_selection_changed();
			queue_redraw();
		}
	}

	void TimelineConnectionEditor::_emit_selection_changed() {
		emit_signal("selection_changed", _get_selected_points());
	}

	bool TimelineConnectionEditor::_find_edit_target_at_position(const Vector2 &p_position, Ref<TimelineConnection> &r_connection, DragTarget &r_target, int &r_point_index, Vector2 &r_content_offset) const {
		r_connection = Ref<TimelineConnection>();
		r_target = DRAG_TARGET_NONE;
		r_point_index = -1;
		r_content_offset = Vector2();

		if (!edit_enabled || !_get_draw_clip_rect().has_point(p_position)) {
			return false;
		}

		const Vector2 mouse_content = _screen_to_content(p_position);
		const float max_target_scale = MAX(MAX(style_cache.key_scale, style_cache.handle_scale), 1.0f);
		const float hit_radius = MAX(handle_radius * max_target_scale * 2.25f, 8.0f);
		float best_distance_sq = hit_radius * hit_radius;

		auto try_target = [&](const Ref<TimelineConnection> &p_connection, DragTarget p_target, int p_point_index, const Vector2 &p_screen_position, const Vector2 &p_content_position) {
			const float distance_sq = p_position.distance_squared_to(p_screen_position);
			if (distance_sq > best_distance_sq) {
				return;
			}

			best_distance_sq = distance_sq;
			r_connection = p_connection;
			r_target = p_target;
			r_point_index = p_point_index;
			r_content_offset = p_content_position - mouse_content;
		};

		for (int i = connections.size() - 1; i >= 0; i--) {
			Ref<TimelineConnection> connection = connections[i];
			if (connection.is_null() || connection->is_disabled() || !connection->is_edit_enabled()) {
				continue;
			}

			const int point_count = connection->get_point_count();
			for (int point_index = point_count - 1; point_index >= 0; point_index--) {
				const Vector2 point_content = connection->get_point_position(point_index);
				if (connection->get_curve_mode() == TimelineConnection::CURVE_BEZIER) {
					if (point_index > 0) {
						const Vector2 in_handle_content = point_content + connection->get_point_in_handle(point_index);
						try_target(connection, DRAG_TARGET_IN_HANDLE, point_index, _content_to_screen(in_handle_content), in_handle_content);
					}
					if (point_index < point_count - 1) {
						const Vector2 out_handle_content = point_content + connection->get_point_out_handle(point_index);
						try_target(connection, DRAG_TARGET_OUT_HANDLE, point_index, _content_to_screen(out_handle_content), out_handle_content);
					}
				}
				try_target(connection, DRAG_TARGET_POINT, point_index, _content_to_screen(point_content), point_content);
			}
		}

		return r_connection.is_valid() && r_target != DRAG_TARGET_NONE;
	}

	void TimelineConnectionEditor::_update_drag(const Vector2 &p_position) {
		if (dragged_connection.is_null() || drag_target == DRAG_TARGET_NONE) {
			return;
		}

		const Vector2 target_content = _clamp_content_position(_screen_to_content(p_position) + drag_content_offset);
		Vector2 signal_position = target_content;
		switch (drag_target) {
		case DRAG_TARGET_POINT:
			dragged_connection->set_point_position(drag_point_index, target_content);
			break;
		case DRAG_TARGET_IN_HANDLE:
			dragged_connection->set_point_in_handle(drag_point_index, target_content - dragged_connection->get_point_position(drag_point_index));
			signal_position = dragged_connection->get_point_position(drag_point_index) + dragged_connection->get_point_in_handle(drag_point_index);
			break;
		case DRAG_TARGET_OUT_HANDLE:
			dragged_connection->set_point_out_handle(drag_point_index, target_content - dragged_connection->get_point_position(drag_point_index));
			signal_position = dragged_connection->get_point_position(drag_point_index) + dragged_connection->get_point_out_handle(drag_point_index);
			break;
		case DRAG_TARGET_NONE:
		default:
			break;
		}

		TimelineConnectionPoint* point = dragged_connection->get_point(drag_point_index);
		if (point != nullptr) {
			emit_signal("connection_point_dragged", dragged_connection, point, drag_point_index, static_cast<int>(drag_target), signal_position);
		}
		queue_redraw();
	}

	void TimelineConnectionEditor::_finish_drag() {
		if (dragged_connection.is_valid() && drag_target != DRAG_TARGET_NONE) {
			TimelineConnectionPoint* point = dragged_connection->get_point(drag_point_index);
			if (point != nullptr) {
				emit_signal("connection_point_drag_ended", dragged_connection, point, drag_point_index, static_cast<int>(drag_target));
			}
		}
		dragged_connection = Ref<TimelineConnection>();
		drag_target = DRAG_TARGET_NONE;
		drag_point_index = -1;
		drag_content_offset = Vector2();
		set_default_cursor_shape(Control::CURSOR_ARROW);
	}

	void TimelineConnectionEditor::_update_hover_cursor(const Vector2 &p_position) {
		if (_is_playhead_drag_area_at_position(p_position)) {
			return;
		}

		Ref<TimelineConnection> connection;
		DragTarget target = DRAG_TARGET_NONE;
		int point_index = -1;
		Vector2 content_offset_found;
		if (_find_edit_target_at_position(p_position, connection, target, point_index, content_offset_found)) {
			set_default_cursor_shape(Control::CURSOR_MOVE);
			return;
		}
		set_default_cursor_shape(Control::CURSOR_ARROW);
	}

	void TimelineConnectionEditor::_draw_ruler_guides() {
		if (!ruler_enabled) {
			return;
		}

		const Rect2 content_rect = _get_draw_clip_rect();
		if (content_rect.size.x <= 0.0f || content_rect.size.y <= 0.0f) {
			return;
		}

		const Vector2 origin = _get_content_origin();
		const double scale_x = std::abs(content_scale.x) < 0.0001 ? 1.0 : content_scale.x;
		const double scale_y = std::abs(content_scale.y) < 0.0001 ? 1.0 : content_scale.y;
		const double min_spacing = MAX(ruler_min_tick_spacing, 12.0f);

		const double visible_x_a = (content_rect.position.x - origin.x) / scale_x + scroll_offset.x;
		const double visible_x_b = (content_rect.position.x + content_rect.size.x - origin.x) / scale_x + scroll_offset.x;
		const double visible_y_a = (content_rect.position.y - origin.y) / scale_y + scroll_offset.y;
		const double visible_y_b = (content_rect.position.y + content_rect.size.y - origin.y) / scale_y + scroll_offset.y;
		const double visible_unit_a = _time_to_unit(visible_x_a);
		const double visible_unit_b = _time_to_unit(visible_x_b);
		const double min_x = std::min(visible_unit_a, visible_unit_b);
		const double max_x = std::max(visible_unit_a, visible_unit_b);
		const double min_y = std::min(visible_y_a, visible_y_b);
		const double max_y = std::max(visible_y_a, visible_y_b);
		const double major_step_x = get_nice_ruler_step(min_spacing * MAX(max_x - min_x, 0.0001) / MAX(static_cast<double>(content_rect.size.x), 1.0));
		const double major_step_y = get_nice_ruler_step(min_spacing / std::abs(scale_y));
		const double minor_step_x = major_step_x / 5.0;
		const double minor_step_y = major_step_y / 5.0;

		for (double value = std::floor(min_x / minor_step_x) * minor_step_x; value <= max_x + minor_step_x; value += minor_step_x) {
			const int major_index = static_cast<int>(std::round(value / major_step_x));
			const bool major = std::abs(value - static_cast<double>(major_index) * major_step_x) <= minor_step_x * 0.1;
			const double time = _unit_to_time(value);
			if (!std::isfinite(time)) {
				continue;
			}
			const float x = static_cast<float>(origin.x + (time - scroll_offset.x) * scale_x);
			if (x < content_rect.position.x || x > content_rect.position.x + content_rect.size.x) {
				continue;
			}
			draw_line(
				Point2(x, content_rect.position.y),
				Point2(x, content_rect.position.y + content_rect.size.y),
				major ? ruler_major_grid_color : ruler_minor_grid_color,
				1.0f
			);
		}

		for (double value = std::floor(min_y / minor_step_y) * minor_step_y; value <= max_y + minor_step_y; value += minor_step_y) {
			const int major_index = static_cast<int>(std::round(value / major_step_y));
			const bool major = std::abs(value - static_cast<double>(major_index) * major_step_y) <= minor_step_y * 0.1;
			const float y = static_cast<float>(origin.y + (value - scroll_offset.y) * scale_y);
			if (y < content_rect.position.y || y > content_rect.position.y + content_rect.size.y) {
				continue;
			}
			draw_line(
				Point2(content_rect.position.x, y),
				Point2(content_rect.position.x + content_rect.size.x, y),
				major ? ruler_major_grid_color : ruler_minor_grid_color,
				1.0f
			);
		}
	}

	void TimelineConnectionEditor::_draw_ruler_overlays() {
		if (!ruler_enabled) {
			return;
		}

		const Rect2 content_rect = _get_content_rect();
		if (content_rect.size.x <= 0.0f || content_rect.size.y <= 0.0f) {
			return;
		}

		const float ruler_width = content_rect.position.x;
		const float ruler_height = content_rect.position.y;
		draw_rect(Rect2(Vector2(), Vector2(ruler_width, ruler_height)), ruler_background_color);
		draw_rect(Rect2(Vector2(ruler_width, 0.0f), Vector2(content_rect.size.x, ruler_height)), ruler_background_color);
		draw_rect(Rect2(Vector2(0.0f, ruler_height), Vector2(ruler_width, content_rect.size.y)), ruler_background_color);
		draw_line(Point2(ruler_width, 0.0f), Point2(ruler_width, get_size().y), ruler_tick_color, 1.0f);
		draw_line(Point2(0.0f, ruler_height), Point2(get_size().x, ruler_height), ruler_tick_color, 1.0f);

		const Rect2 clip_rect = _get_draw_clip_rect();
		if (clip_rect.size.x <= 0.0f || clip_rect.size.y <= 0.0f) {
			return;
		}
		if (range_limited) {
			draw_rect(clip_rect, ruler_tick_color, false, 1.0f);
		}

		const Vector2 origin = _get_content_origin();
		const double scale_x = std::abs(content_scale.x) < 0.0001 ? 1.0 : content_scale.x;
		const double scale_y = std::abs(content_scale.y) < 0.0001 ? 1.0 : content_scale.y;
		const double min_spacing = MAX(ruler_min_tick_spacing, 12.0f);
		const double visible_x_a = (clip_rect.position.x - origin.x) / scale_x + scroll_offset.x;
		const double visible_x_b = (clip_rect.position.x + clip_rect.size.x - origin.x) / scale_x + scroll_offset.x;
		const double visible_y_a = (clip_rect.position.y - origin.y) / scale_y + scroll_offset.y;
		const double visible_y_b = (clip_rect.position.y + clip_rect.size.y - origin.y) / scale_y + scroll_offset.y;
		const double visible_unit_a = _time_to_unit(visible_x_a);
		const double visible_unit_b = _time_to_unit(visible_x_b);
		const double min_x = std::min(visible_unit_a, visible_unit_b);
		const double max_x = std::max(visible_unit_a, visible_unit_b);
		const double min_y = std::min(visible_y_a, visible_y_b);
		const double max_y = std::max(visible_y_a, visible_y_b);
		const double major_step_x = get_nice_ruler_step(min_spacing * MAX(max_x - min_x, 0.0001) / MAX(static_cast<double>(clip_rect.size.x), 1.0));
		const double major_step_y = get_nice_ruler_step(min_spacing / std::abs(scale_y));
		const double minor_step_x = major_step_x / 5.0;
		const double minor_step_y = major_step_y / 5.0;
		const Ref<Font> font = ThemeDB::get_singleton()->get_fallback_font();
		const float major_tick = 8.0f;
		const float minor_tick = 4.0f;

		for (double value = std::floor(min_x / minor_step_x) * minor_step_x; value <= max_x + minor_step_x; value += minor_step_x) {
			const int major_index = static_cast<int>(std::round(value / major_step_x));
			const bool major = std::abs(value - static_cast<double>(major_index) * major_step_x) <= minor_step_x * 0.1;
			const double time = _unit_to_time(value);
			if (!std::isfinite(time)) {
				continue;
			}
			const float x = static_cast<float>(origin.x + (time - scroll_offset.x) * scale_x);
			if (x < clip_rect.position.x || x > clip_rect.position.x + clip_rect.size.x) {
				continue;
			}
			const float tick = major ? major_tick : minor_tick;
			draw_line(Point2(x, ruler_height - tick), Point2(x, ruler_height), ruler_tick_color, 1.0f);
			if (major && font.is_valid()) {
				draw_string(font, Point2(x + 3.0f, ruler_height - 6.0f), _format_ruler_x_value(time), HORIZONTAL_ALIGNMENT_LEFT, -1, ruler_font_size, ruler_text_color);
			}
		}

		for (double value = std::floor(min_y / minor_step_y) * minor_step_y; value <= max_y + minor_step_y; value += minor_step_y) {
			const int major_index = static_cast<int>(std::round(value / major_step_y));
			const bool major = std::abs(value - static_cast<double>(major_index) * major_step_y) <= minor_step_y * 0.1;
			const float y = static_cast<float>(origin.y + (value - scroll_offset.y) * scale_y);
			if (y < clip_rect.position.y || y > clip_rect.position.y + clip_rect.size.y) {
				continue;
			}
			const float tick = major ? major_tick : minor_tick;
			draw_line(Point2(ruler_width - tick, y), Point2(ruler_width, y), ruler_tick_color, 1.0f);
			if (major && font.is_valid()) {
				draw_string(font, Point2(3.0f, y - 3.0f), format_ruler_value(value), HORIZONTAL_ALIGNMENT_LEFT, ruler_width - 6.0f, ruler_font_size, ruler_text_color);
			}
		}
	}

	void TimelineConnectionEditor::_draw_connections() {
		if (connections.is_empty()) {
			return;
		}

		const Rect2 viewport_rect = _get_draw_clip_rect();
		if (viewport_rect.size.x <= 0.0f || viewport_rect.size.y <= 0.0f) {
			return;
		}
		for (int i = 0; i < connections.size(); i++) {
			Ref<TimelineConnection> connection = connections[i];
			const float line_width = MAX(style_cache.bezier_line_width, 0.0f);
			if (connection.is_null() || connection->is_disabled() || line_width <= 0.0f) {
				continue;
			}

			const int point_count = connection->get_point_count();
			if (point_count <= 0) {
				continue;
			}

			const bool bezier = connection->get_curve_mode() == TimelineConnection::CURVE_BEZIER;
			bool has_bounds = false;
			float min_x = 0.0f;
			float min_y = 0.0f;
			float max_x = 0.0f;
			float max_y = 0.0f;

			auto include_bounds = [&](const Vector2 &p_position) {
				if (!has_bounds) {
					min_x = p_position.x;
					min_y = p_position.y;
					max_x = p_position.x;
					max_y = p_position.y;
					has_bounds = true;
					return;
				}
				min_x = std::min(min_x, p_position.x);
				min_y = std::min(min_y, p_position.y);
				max_x = std::max(max_x, p_position.x);
				max_y = std::max(max_y, p_position.y);
			};

			for (int point_index = 0; point_index < point_count; point_index++) {
				const Vector2 point_position = _content_to_screen(connection->get_point_position(point_index));
				include_bounds(point_position);
				if (bezier && point_index > 0) {
					include_bounds(point_position + _content_delta_to_screen(connection->get_point_in_handle(point_index)));
				}
				if (bezier && point_index < point_count - 1) {
					include_bounds(point_position + _content_delta_to_screen(connection->get_point_out_handle(point_index)));
				}
			}

			const Rect2 connection_bounds(Vector2(min_x, min_y), Vector2(max_x - min_x, max_y - min_y));
			if (!viewport_rect.intersects(connection_bounds.grow(96.0f))) {
				continue;
			}

			const Vector2 from_position = _content_to_screen(connection->get_point_position(0));
			const Vector2 to_position = _content_to_screen(connection->get_point_position(point_count - 1));
			bool handled = false;
			if (GDVIRTUAL_CALL(_draw_connection, get_canvas_item(), connection, from_position, to_position, handled) && handled) {
				continue;
			}

			const Color color = connection->get_color();
			if (point_count >= 2) {
				if (!bezier) {
					for (int point_index = 0; point_index < point_count - 1; point_index++) {
						_draw_clipped_line(
							_content_to_screen(connection->get_point_position(point_index)),
							_content_to_screen(connection->get_point_position(point_index + 1)),
							color,
							line_width,
							viewport_rect
						);
					}
				}
				else {
					const int segments = CLAMP(connection->get_curve_segments(), 2, 256);
					for (int point_index = 0; point_index < point_count - 1; point_index++) {
						const Vector2 segment_from = _content_to_screen(connection->get_point_position(point_index));
						const Vector2 segment_to = _content_to_screen(connection->get_point_position(point_index + 1));
						const Vector2 control_a = segment_from + _content_delta_to_screen(connection->get_point_out_handle(point_index));
						const Vector2 control_b = segment_to + _content_delta_to_screen(connection->get_point_in_handle(point_index + 1));
						Vector2 previous = segment_from;
						for (int segment = 1; segment <= segments; segment++) {
							const float t = static_cast<float>(segment) / static_cast<float>(segments);
							const Vector2 current = sample_cubic_bezier(segment_from, control_a, control_b, segment_to, t);
							_draw_clipped_line(previous, current, color, line_width, viewport_rect);
							previous = current;
						}
					}
				}
			}

			if (edit_enabled && connection->is_edit_enabled()) {
				const float handle_line_width = MAX(style_cache.handle_line_width, 0.0f);
				const float base_size = MAX(handle_radius * 2.0f, 8.0f);
				const float key_size = base_size * MAX(style_cache.key_scale, 0.0f);
				const float handle_size = base_size * MAX(style_cache.handle_scale, 0.0f);
				const Ref<StyleBox> key_normal_style = _get_key_normal_style();
				const Ref<StyleBox> key_selected_style = _get_key_selected_style();
				const Ref<StyleBox> handle_style = _get_handle_style();
				for (int point_index = 0; point_index < point_count; point_index++) {
					TimelineConnectionPoint* point = connection->get_point(point_index);
					const Vector2 point_position = _content_to_screen(connection->get_point_position(point_index));
					const bool point_selected = (point != nullptr && point->is_selected()) || (dragged_connection == connection && drag_point_index == point_index && drag_target == DRAG_TARGET_POINT);
					if (bezier && point_index > 0) {
						const Vector2 in_handle_position = point_position + _content_delta_to_screen(connection->get_point_in_handle(point_index));
						const bool handle_selected = (point != nullptr && point->is_selected()) || (dragged_connection == connection && drag_point_index == point_index && drag_target == DRAG_TARGET_IN_HANDLE);
						if (handle_line_width > 0.0f) {
							_draw_clipped_line(point_position, in_handle_position, handle_selected ? style_cache.handle_line_selected_color : style_cache.handle_line_color, handle_line_width, viewport_rect);
						}
						_draw_clipped_style_box(in_handle_position, handle_size, handle_style, viewport_rect);
					}
					if (bezier && point_index < point_count - 1) {
						const Vector2 out_handle_position = point_position + _content_delta_to_screen(connection->get_point_out_handle(point_index));
						const bool handle_selected = (point != nullptr && point->is_selected()) || (dragged_connection == connection && drag_point_index == point_index && drag_target == DRAG_TARGET_OUT_HANDLE);
						if (handle_line_width > 0.0f) {
							_draw_clipped_line(point_position, out_handle_position, handle_selected ? style_cache.handle_line_selected_color : style_cache.handle_line_color, handle_line_width, viewport_rect);
						}
						_draw_clipped_style_box(out_handle_position, handle_size, handle_style, viewport_rect);
					}
					_draw_clipped_style_box(point_position, key_size, point_selected ? key_selected_style : key_normal_style, viewport_rect);
				}
			}
		}
	}

	void TimelineConnectionEditor::_draw_playhead() {
		if (playhead.is_null()) {
			return;
		}

		const Rect2 content_rect = _get_content_rect();
		const Rect2 clip_rect = _get_draw_clip_rect();
		if (content_rect.size.x <= 0.0f || content_rect.size.y <= 0.0f || clip_rect.size.x <= 0.0f || clip_rect.size.y <= 0.0f) {
			return;
		}

		const float x = static_cast<float>(get_position_from_time(current_time));
		if (x < clip_rect.position.x || x > clip_rect.position.x + clip_rect.size.x) {
			return;
		}

		const float header_height = ruler_enabled ? MAX(content_rect.position.y, 16.0f) : 16.0f;
		const float header_width = 48.0f;
		const Rect2 header_rect(Vector2(x - header_width * 0.5f, 0.0f), Vector2(header_width, header_height));
		const double line_length = MAX(content_rect.position.y + content_rect.size.y - (header_rect.position.y + header_rect.size.y * 0.5f), 0.0f);
		playhead->draw(get_canvas_item(), header_rect, _format_playhead_time(current_time), line_length, true);
	}

	void TimelineConnectionEditor::_on_resource_changed() {
		queue_redraw();
	}

	TimelineConnectionEditor::TimelineConnectionEditor() {
		set_mouse_filter(Control::MOUSE_FILTER_PASS);
		set_clip_contents(true);

		hscroll = memnew(HScrollBar);
		hscroll->set_step(0.001);
		hscroll->set_custom_step(scroll_step);
		hscroll->hide();
		hscroll->connect("value_changed", callable_mp(this, &TimelineConnectionEditor::_h_scroll_changed));
		add_child(hscroll, false, INTERNAL_MODE_FRONT);

		vscroll = memnew(VScrollBar);
		vscroll->set_step(0.001);
		vscroll->set_custom_step(scroll_step);
		vscroll->hide();
		vscroll->connect("value_changed", callable_mp(this, &TimelineConnectionEditor::_v_scroll_changed));
		add_child(vscroll, false, INTERNAL_MODE_FRONT);

		Ref<StyleBoxFlat> key_normal;
		key_normal.instantiate();
		key_normal->set_bg_color(Color(1.0f, 1.0f, 1.0f, 0.88f));
		key_normal->set_corner_detail(4);
		key_normal->set_corner_radius_all(512);
		style_cache.key_normal_fallback = key_normal;

		Ref<StyleBoxFlat> key_selected;
		key_selected.instantiate();
		key_selected->set_bg_color(Color(1.0f, 0.78f, 0.24f, 0.95f));
		key_selected->set_border_width_all(2);
		key_selected->set_border_color(Color(1.0f, 1.0f, 1.0f, 0.8f));
		key_selected->set_corner_detail(4);
		key_selected->set_corner_radius_all(512);
		style_cache.key_selected_fallback = key_selected;

		Ref<StyleBoxFlat> handle;
		handle.instantiate();
		handle->set_bg_color(Color(0.45f, 0.72f, 1.0f, 0.9f));
		handle->set_border_width_all(1);
		handle->set_border_color(Color(1.0f, 1.0f, 1.0f, 0.5f));
		handle->set_corner_detail(4);
		handle->set_corner_radius_all(512);
		style_cache.handle_fallback = handle;
	}

	void TimelineConnectionEditor::_gui_input(const Ref<InputEvent> &p_gui_input) {
		Ref<InputEventMouseButton> mouse_button = p_gui_input;
		if (mouse_button.is_valid() && mouse_button->get_button_index() == MouseButton::MOUSE_BUTTON_MIDDLE) {
			if (mouse_button->is_pressed() && scroll_enabled && _get_content_rect().has_point(mouse_button->get_position())) {
				middle_mouse_panning = true;
				set_default_cursor_shape(Control::CURSOR_CROSS);
				accept_event();
			}
			else if (!mouse_button->is_pressed() && middle_mouse_panning) {
				middle_mouse_panning = false;
				set_default_cursor_shape(Control::CURSOR_ARROW);
				accept_event();
			}
			return;
		}

		if (mouse_button.is_valid() && mouse_button->is_pressed() && scroll_enabled && _get_content_rect().has_point(mouse_button->get_position())) {
			const float factor = mouse_button->get_factor() == 0.0f ? 1.0f : mouse_button->get_factor();
			const float horizontal_direction = content_scale.x < 0.0f ? -1.0f : 1.0f;
			const float vertical_direction = content_scale.y < 0.0f ? -1.0f : 1.0f;
			Vector2 scroll_delta;
			bool wheel_handled = false;
			switch (mouse_button->get_button_index()) {
			case MouseButton::MOUSE_BUTTON_WHEEL_UP: {
				if (mouse_button->is_ctrl_pressed()) {
					scroll_delta = Vector2(-_get_smart_scroll_step(true) * factor * horizontal_direction, 0.0f);
				}
				else if (mouse_button->is_shift_pressed()) {
					scroll_delta = Vector2(0.0f, -_get_smart_scroll_step(false) * factor * vertical_direction);
				}
				else {
					_zoom_at_position(mouse_button->get_position(), static_cast<float>(std::pow(1.12f, factor)));
					wheel_handled = true;
				}
				break;
			}
			case MouseButton::MOUSE_BUTTON_WHEEL_DOWN: {
				if (mouse_button->is_ctrl_pressed()) {
					scroll_delta = Vector2(_get_smart_scroll_step(true) * factor * horizontal_direction, 0.0f);
				}
				else if (mouse_button->is_shift_pressed()) {
					scroll_delta = Vector2(0.0f, _get_smart_scroll_step(false) * factor * vertical_direction);
				}
				else {
					_zoom_at_position(mouse_button->get_position(), static_cast<float>(std::pow(1.0f / 1.12f, factor)));
					wheel_handled = true;
				}
				break;
			}
			case MouseButton::MOUSE_BUTTON_WHEEL_LEFT:
				scroll_delta = Vector2(-_get_smart_scroll_step(true) * factor * horizontal_direction, 0.0f);
				break;
			case MouseButton::MOUSE_BUTTON_WHEEL_RIGHT:
				scroll_delta = Vector2(_get_smart_scroll_step(true) * factor * horizontal_direction, 0.0f);
				break;
			default:
				break;
			}

			if (wheel_handled) {
				accept_event();
				return;
			}

			if (scroll_delta != Vector2()) {
				_scroll(scroll_delta);
				accept_event();
				return;
			}
		}

		if (mouse_button.is_valid() && mouse_button->get_button_index() == MouseButton::MOUSE_BUTTON_LEFT) {
			if (mouse_button->is_pressed()) {
				if (_is_playhead_drag_area_at_position(mouse_button->get_position())) {
					playhead_dragging = true;
					set_current_time(_get_time_at_position(mouse_button->get_position()));
					emit_signal("playhead_drag_started", current_time);
					accept_event();
					return;
				}

				Ref<TimelineConnection> connection;
				DragTarget target = DRAG_TARGET_NONE;
				int point_index = -1;
				Vector2 content_offset_found;
				if (_find_edit_target_at_position(mouse_button->get_position(), connection, target, point_index, content_offset_found)) {
					_select_point(connection, point_index, mouse_button->is_shift_pressed(), mouse_button->is_shift_pressed());
					dragged_connection = connection;
					drag_target = target;
					drag_point_index = point_index;
					drag_content_offset = content_offset_found;
					TimelineConnectionPoint* point = dragged_connection->get_point(drag_point_index);
					if (point != nullptr) {
						emit_signal("connection_point_drag_started", dragged_connection, point, drag_point_index, static_cast<int>(drag_target));
					}
					set_default_cursor_shape(Control::CURSOR_MOVE);
					accept_event();
				}
				else if (_get_content_rect().has_point(mouse_button->get_position()) && _clear_point_selection()) {
					_emit_selection_changed();
					queue_redraw();
				}
			}
			else if (playhead_dragging) {
				set_current_time(_get_time_at_position(mouse_button->get_position()));
				playhead_dragging = false;
				set_default_cursor_shape(Control::CURSOR_ARROW);
				emit_signal("playhead_drag_ended", current_time);
				accept_event();
			}
			else if (drag_target != DRAG_TARGET_NONE) {
				_finish_drag();
				accept_event();
			}
			return;
		}

		Ref<InputEventMouseMotion> mouse_motion = p_gui_input;
		if (mouse_motion.is_valid()) {
			if (middle_mouse_panning) {
				_pan_view(mouse_motion->get_relative());
				set_default_cursor_shape(Control::CURSOR_CROSS);
				accept_event();
				return;
			}

			if (playhead_dragging) {
				set_current_time(_get_time_at_position(mouse_motion->get_position()));
				emit_signal("playhead_dragged", current_time);
				accept_event();
				return;
			}

			if (drag_target != DRAG_TARGET_NONE) {
				_update_drag(mouse_motion->get_position());
				set_default_cursor_shape(Control::CURSOR_MOVE);
				accept_event();
				return;
			}
			_update_hover_cursor(mouse_motion->get_position());
		}
	}

	Ref<TimelineConnection> TimelineConnectionEditor::create_connection(const Vector2 &p_from_position, const Vector2 &p_to_position) {
		Ref<TimelineConnection> connection;
		connection.instantiate();
		connection->set_from_position(p_from_position);
		connection->set_to_position(p_to_position);
		add_connection(connection);
		return connection;
	}

	void TimelineConnectionEditor::add_connection(const Ref<TimelineConnection> &p_connection) {
		if (p_connection.is_null() || get_connection_index(p_connection) >= 0) {
			return;
		}

		connections.push_back(p_connection);
		_connect_connection_changed(p_connection);
		queue_redraw();
	}

	void TimelineConnectionEditor::remove_connection(const Ref<TimelineConnection> &p_connection) {
		const int index = get_connection_index(p_connection);
		if (index >= 0) {
			remove_connection_at(index);
		}
	}

	void TimelineConnectionEditor::remove_connection_at(int p_index) {
		if (p_index < 0 || p_index >= connections.size()) {
			return;
		}

		Ref<TimelineConnection> connection = connections[p_index];
		_disconnect_connection_changed(connection);
		if (dragged_connection == connection) {
			_finish_drag();
		}
		connections.remove_at(p_index);
		queue_redraw();
	}

	void TimelineConnectionEditor::clear_connections() {
		for (int i = 0; i < connections.size(); i++) {
			Ref<TimelineConnection> connection = connections[i];
			_disconnect_connection_changed(connection);
		}
		connections.clear();
		_finish_drag();
		queue_redraw();
	}

	TypedArray<TimelineConnection> TimelineConnectionEditor::get_connections() const {
		return connections;
	}

	Ref<TimelineConnection> TimelineConnectionEditor::get_connection(int p_index) const {
		if (p_index < 0 || p_index >= connections.size()) {
			return Ref<TimelineConnection>();
		}
		return connections[p_index];
	}

	int TimelineConnectionEditor::get_connection_count() const {
		return connections.size();
	}

	int TimelineConnectionEditor::get_connection_index(const Ref<TimelineConnection> &p_connection) const {
		if (p_connection.is_null()) {
			return -1;
		}

		for (int i = 0; i < connections.size(); i++) {
			Ref<TimelineConnection> connection = connections[i];
			if (connection == p_connection) {
				return i;
			}
		}
		return -1;
	}

	void TimelineConnectionEditor::_connect_connection_changed(const Ref<TimelineConnection> &p_connection) {
		if (p_connection.is_null()) {
			return;
		}

		const Callable changed_callable = callable_mp(this, &TimelineConnectionEditor::_on_resource_changed);
		if (!p_connection->is_connected("changed", changed_callable)) {
			p_connection->connect("changed", changed_callable);
		}
	}

	void TimelineConnectionEditor::_disconnect_connection_changed(const Ref<TimelineConnection> &p_connection) {
		if (p_connection.is_null()) {
			return;
		}

		const Callable changed_callable = callable_mp(this, &TimelineConnectionEditor::_on_resource_changed);
		if (p_connection->is_connected("changed", changed_callable)) {
			p_connection->disconnect("changed", changed_callable);
		}
	}

	HScrollBar* TimelineConnectionEditor::get_h_scroll_bar() const {
		return hscroll;
	}

	VScrollBar* TimelineConnectionEditor::get_v_scroll_bar() const {
		return vscroll;
	}

	void TimelineConnectionEditor::set_content_offset(const Vector2 &p_offset) {
		content_offset = p_offset;
		queue_redraw();
	}

	Vector2 TimelineConnectionEditor::get_content_offset() const {
		return content_offset;
	}

	void TimelineConnectionEditor::set_content_scale(const Vector2 &p_scale) {
		content_scale = p_scale;
		_update_scroll_bars();
		queue_redraw();
	}

	Vector2 TimelineConnectionEditor::get_content_scale() const {
		return content_scale;
	}

	void TimelineConnectionEditor::set_current_time(double p_time) {
		if (Math::is_equal_approx(current_time, p_time)) {
			return;
		}

		current_time = p_time;
		emit_signal("current_time_changed", current_time);
		queue_redraw();
	}

	double TimelineConnectionEditor::get_current_time() const {
		return current_time;
	}

	void TimelineConnectionEditor::set_counting_unit(CountingUnit p_unit) {
		if (counting_unit == p_unit) {
			return;
		}

		counting_unit = p_unit;
		notify_property_list_changed();
		queue_redraw();
	}

	TimelineConnectionEditor::CountingUnit TimelineConnectionEditor::get_counting_unit() const {
		return counting_unit;
	}

	void TimelineConnectionEditor::set_fps(int p_fps) {
		fps = MAX(p_fps, 1);
		queue_redraw();
	}

	int TimelineConnectionEditor::get_fps() const {
		return fps;
	}

	void TimelineConnectionEditor::set_bpms(const Dictionary &p_bpms) {
		bpms = p_bpms;
		queue_redraw();
	}

	Dictionary TimelineConnectionEditor::get_bpms() const {
		return bpms;
	}

	void TimelineConnectionEditor::set_playhead(Ref<TimelineIndicator> p_playhead) {
		const Callable changed_callable = callable_mp(this, &TimelineConnectionEditor::_on_resource_changed);
		if (playhead.is_valid() && playhead->is_connected("changed", changed_callable)) {
			playhead->disconnect("changed", changed_callable);
		}

		playhead = p_playhead;
		if (playhead.is_valid() && !playhead->is_connected("changed", changed_callable)) {
			playhead->connect("changed", changed_callable);
		}
		queue_redraw();
	}

	Ref<TimelineIndicator> TimelineConnectionEditor::get_playhead() const {
		return playhead;
	}

	void TimelineConnectionEditor::set_playhead_drag_enabled(bool p_enabled) {
		playhead_drag_enabled = p_enabled;
		if (!playhead_drag_enabled) {
			playhead_dragging = false;
		}
		queue_redraw();
	}

	bool TimelineConnectionEditor::is_playhead_drag_enabled() const {
		return playhead_drag_enabled;
	}

	TypedArray<TimelineConnectionPoint> TimelineConnectionEditor::get_selected_points() const {
		return _get_selected_points();
	}

	double TimelineConnectionEditor::get_time_from_position(double p_position) const {
		return _screen_to_content(Vector2(static_cast<float>(p_position), 0.0f)).x;
	}

	double TimelineConnectionEditor::get_frame_from_position(double p_position) const {
		return get_time_from_position(p_position) * static_cast<double>(MAX(fps, 1));
	}

	double TimelineConnectionEditor::get_beat_from_position(double p_position) const {
		return _time_to_beat(get_time_from_position(p_position));
	}

	double TimelineConnectionEditor::get_position_from_time(double p_time) const {
		return _content_to_screen(Vector2(static_cast<float>(p_time), 0.0f)).x;
	}

	double TimelineConnectionEditor::get_position_from_frame(int64_t p_frame) const {
		return get_position_from_time(static_cast<double>(p_frame) / static_cast<double>(MAX(fps, 1)));
	}

	double TimelineConnectionEditor::get_position_from_beat(double p_beat) const {
		return get_position_from_time(_beat_to_time(p_beat));
	}

	void TimelineConnectionEditor::set_scroll_offset(const Vector2 &p_offset) {
		scroll_offset = p_offset;
		_update_scroll_bars();
		queue_redraw();
	}

	Vector2 TimelineConnectionEditor::get_scroll_offset() const {
		return scroll_offset;
	}

	void TimelineConnectionEditor::set_scroll_step(float p_step) {
		scroll_step = MAX(p_step, 0.0f);
		if (hscroll != nullptr) {
			hscroll->set_custom_step(_get_smart_scroll_step(true));
		}
		if (vscroll != nullptr) {
			vscroll->set_custom_step(_get_smart_scroll_step(false));
		}
	}

	float TimelineConnectionEditor::get_scroll_step() const {
		return scroll_step;
	}

	void TimelineConnectionEditor::set_scroll_enabled(bool p_enabled) {
		scroll_enabled = p_enabled;
		_update_scroll_bars();
		queue_redraw();
	}

	bool TimelineConnectionEditor::is_scroll_enabled() const {
		return scroll_enabled;
	}

	void TimelineConnectionEditor::set_edit_enabled(bool p_enabled) {
		edit_enabled = p_enabled;
		queue_redraw();
	}

	bool TimelineConnectionEditor::is_edit_enabled() const {
		return edit_enabled;
	}

	void TimelineConnectionEditor::set_handle_radius(float p_radius) {
		handle_radius = MAX(p_radius, 0.0f);
		queue_redraw();
	}

	float TimelineConnectionEditor::get_handle_radius() const {
		return handle_radius;
	}

	void TimelineConnectionEditor::set_ruler_enabled(bool p_enabled) {
		ruler_enabled = p_enabled;
		_update_scroll_bars();
		queue_redraw();
	}

	bool TimelineConnectionEditor::is_ruler_enabled() const {
		return ruler_enabled;
	}

	void TimelineConnectionEditor::set_ruler_size(const Vector2 &p_size) {
		ruler_size = Vector2(MAX(p_size.x, 0.0f), MAX(p_size.y, 0.0f));
		_update_scroll_bars();
		queue_redraw();
	}

	Vector2 TimelineConnectionEditor::get_ruler_size() const {
		return ruler_size;
	}

	void TimelineConnectionEditor::set_ruler_min_tick_spacing(float p_spacing) {
		ruler_min_tick_spacing = MAX(p_spacing, 1.0f);
		queue_redraw();
	}

	float TimelineConnectionEditor::get_ruler_min_tick_spacing() const {
		return ruler_min_tick_spacing;
	}

	void TimelineConnectionEditor::set_ruler_font_size(int p_size) {
		ruler_font_size = MAX(p_size, 1);
		queue_redraw();
	}

	int TimelineConnectionEditor::get_ruler_font_size() const {
		return ruler_font_size;
	}

	void TimelineConnectionEditor::set_ruler_background_color(const Color &p_color) {
		ruler_background_color = p_color;
		queue_redraw();
	}

	Color TimelineConnectionEditor::get_ruler_background_color() const {
		return ruler_background_color;
	}

	void TimelineConnectionEditor::set_ruler_tick_color(const Color &p_color) {
		ruler_tick_color = p_color;
		queue_redraw();
	}

	Color TimelineConnectionEditor::get_ruler_tick_color() const {
		return ruler_tick_color;
	}

	void TimelineConnectionEditor::set_ruler_text_color(const Color &p_color) {
		ruler_text_color = p_color;
		queue_redraw();
	}

	Color TimelineConnectionEditor::get_ruler_text_color() const {
		return ruler_text_color;
	}

	void TimelineConnectionEditor::set_ruler_major_grid_color(const Color &p_color) {
		ruler_major_grid_color = p_color;
		queue_redraw();
	}

	Color TimelineConnectionEditor::get_ruler_major_grid_color() const {
		return ruler_major_grid_color;
	}

	void TimelineConnectionEditor::set_ruler_minor_grid_color(const Color &p_color) {
		ruler_minor_grid_color = p_color;
		queue_redraw();
	}

	Color TimelineConnectionEditor::get_ruler_minor_grid_color() const {
		return ruler_minor_grid_color;
	}

	void TimelineConnectionEditor::set_range_limited(bool p_limited) {
		range_limited = p_limited;
		_update_scroll_bars();
		queue_redraw();
	}

	bool TimelineConnectionEditor::is_range_limited() const {
		return range_limited;
	}

	void TimelineConnectionEditor::set_range_min(const Vector2 &p_min) {
		range_min = p_min;
		_update_scroll_bars();
		queue_redraw();
	}

	Vector2 TimelineConnectionEditor::get_range_min() const {
		return range_min;
	}

	void TimelineConnectionEditor::set_range_max(const Vector2 &p_max) {
		range_max = p_max;
		_update_scroll_bars();
		queue_redraw();
	}

	Vector2 TimelineConnectionEditor::get_range_max() const {
		return range_max;
	}

	void TimelineConnectionEditor::set_range_start_time(double p_time) {
		range_min.x = static_cast<float>(p_time);
		_update_scroll_bars();
		queue_redraw();
	}

	double TimelineConnectionEditor::get_range_start_time() const {
		return range_min.x;
	}

	void TimelineConnectionEditor::set_range_end_time(double p_time) {
		range_max.x = static_cast<float>(p_time);
		_update_scroll_bars();
		queue_redraw();
	}

	double TimelineConnectionEditor::get_range_end_time() const {
		return range_max.x;
	}

	void TimelineConnectionEditor::set_range_min_y(float p_y) {
		range_min.y = p_y;
		_update_scroll_bars();
		queue_redraw();
	}

	float TimelineConnectionEditor::get_range_min_y() const {
		return range_min.y;
	}

	void TimelineConnectionEditor::set_range_max_y(float p_y) {
		range_max.y = p_y;
		_update_scroll_bars();
		queue_redraw();
	}

	float TimelineConnectionEditor::get_range_max_y() const {
		return range_max.y;
	}

	void TimelineConnectionEditor::set_key_scale(float p_scale) {
		style_cache.key_scale = MAX(p_scale, 0.0f);
		queue_redraw();
	}

	float TimelineConnectionEditor::get_key_scale() const {
		return style_cache.key_scale;
	}

	void TimelineConnectionEditor::set_key_normal_style(Ref<StyleBox> p_style) {
		style_cache.key_normal = p_style;
		queue_redraw();
	}

	Ref<StyleBox> TimelineConnectionEditor::get_key_normal_style() const {
		return style_cache.key_normal;
	}

	void TimelineConnectionEditor::set_key_selected_style(Ref<StyleBox> p_style) {
		style_cache.key_selected = p_style;
		queue_redraw();
	}

	Ref<StyleBox> TimelineConnectionEditor::get_key_selected_style() const {
		return style_cache.key_selected;
	}

	void TimelineConnectionEditor::set_handle_scale(float p_scale) {
		style_cache.handle_scale = MAX(p_scale, 0.0f);
		queue_redraw();
	}

	float TimelineConnectionEditor::get_handle_scale() const {
		return style_cache.handle_scale;
	}

	void TimelineConnectionEditor::set_handle_style(Ref<StyleBox> p_style) {
		style_cache.handle = p_style;
		queue_redraw();
	}

	Ref<StyleBox> TimelineConnectionEditor::get_handle_style() const {
		return style_cache.handle;
	}

	void TimelineConnectionEditor::set_handle_line_width(float p_width) {
		style_cache.handle_line_width = MAX(p_width, 0.0f);
		queue_redraw();
	}

	float TimelineConnectionEditor::get_handle_line_width() const {
		return style_cache.handle_line_width;
	}

	void TimelineConnectionEditor::set_handle_line_color(const Color &p_color) {
		style_cache.handle_line_color = p_color;
		queue_redraw();
	}

	Color TimelineConnectionEditor::get_handle_line_color() const {
		return style_cache.handle_line_color;
	}

	void TimelineConnectionEditor::set_handle_line_selected_color(const Color &p_color) {
		style_cache.handle_line_selected_color = p_color;
		queue_redraw();
	}

	Color TimelineConnectionEditor::get_handle_line_selected_color() const {
		return style_cache.handle_line_selected_color;
	}

	void TimelineConnectionEditor::set_bezier_line_width(float p_width) {
		style_cache.bezier_line_width = MAX(p_width, 0.0f);
		queue_redraw();
	}

	float TimelineConnectionEditor::get_bezier_line_width() const {
		return style_cache.bezier_line_width;
	}
}
