#include "v_timeline_panel.h"

#include "components/timeline_indicator.h"
#include "components/timeline_marker.h"
#include "components/timeline_time_ruler.h"
#include "components/timeline_track.h"
#include "components/timeline_track_key.h"

#include <algorithm>
#include <unordered_set>
#include <godot_cpp/classes/font.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/input_event_mouse_motion.hpp>
#include <godot_cpp/classes/input_event_pan_gesture.hpp>
#include <godot_cpp/classes/style_box_flat.hpp>
#include <godot_cpp/classes/theme_db.hpp>

namespace godot {
	void VTimelinePanel::_bind_methods() {
		BIND_ENUM_CONSTANT(TIME);
		BIND_ENUM_CONSTANT(FRAME);
		BIND_ENUM_CONSTANT(BEAT);

		BIND_ENUM_CONSTANT(HH_MM_SS);
		BIND_ENUM_CONSTANT(MM_SS_MS);
		BIND_ENUM_CONSTANT(SEC);

		BIND_ENUM_CONSTANT(BEAT_BAR);

		BIND_ENUM_CONSTANT(BAR_NUMBER_TOP_DOWN);
		BIND_ENUM_CONSTANT(BAR_NUMBER_BOTTOM_UP);

		BIND_ENUM_CONSTANT(SCROLL_MODE_DISABLED);
		BIND_ENUM_CONSTANT(SCROLL_MODE_AUTO);
		BIND_ENUM_CONSTANT(SCROLL_MODE_SHOW_ALWAYS);
		BIND_ENUM_CONSTANT(SCROLL_MODE_SHOW_NEVER);
		BIND_ENUM_CONSTANT(SCROLL_MODE_RESERVE);

		ClassDB::bind_method(D_METHOD("create_key", "track_index", "time", "length", "snap"), &VTimelinePanel::create_key, DEFVAL(0.0f), DEFVAL(false));
		ClassDB::bind_method(D_METHOD("remove_key", "track_index", "key_index"), &VTimelinePanel::remove_key);
		ClassDB::bind_method(D_METHOD("clear_track_keys", "track_index"), &VTimelinePanel::clear_track_keys);
		ClassDB::bind_method(D_METHOD("clear_all_keys"), &VTimelinePanel::clear_all_keys);
		ClassDB::bind_method(D_METHOD("get_key_count"), &VTimelinePanel::get_key_count);
		ClassDB::bind_method(D_METHOD("get_key", "track_index", "key_index"), &VTimelinePanel::get_key);

		ClassDB::bind_method(D_METHOD("get_time_from_position", "position"), &VTimelinePanel::get_time_from_position);
		ClassDB::bind_method(D_METHOD("get_frame_from_position", "position"), &VTimelinePanel::get_frame_from_position);
		ClassDB::bind_method(D_METHOD("get_beat_from_position", "position"), &VTimelinePanel::get_beat_from_position);
		ClassDB::bind_method(D_METHOD("get_position_from_time", "time"), &VTimelinePanel::get_position_from_time);
		ClassDB::bind_method(D_METHOD("get_position_from_frame", "frame"), &VTimelinePanel::get_position_from_frame);
		ClassDB::bind_method(D_METHOD("get_position_from_beat", "beat"), &VTimelinePanel::get_position_from_beat);

		ClassDB::bind_method(D_METHOD("set_background_color", "background_color"), &VTimelinePanel::set_background_color);
		ClassDB::bind_method(D_METHOD("get_background_color"), &VTimelinePanel::get_background_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "background_color"), "set_background_color", "get_background_color");

		ClassDB::bind_method(D_METHOD("set_header_height", "header_height"), &VTimelinePanel::set_header_height);
		ClassDB::bind_method(D_METHOD("get_header_height"), &VTimelinePanel::get_header_height);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "header_height"), "set_header_height", "get_header_height");

		ClassDB::bind_method(D_METHOD("set_duration", "duration"), &VTimelinePanel::set_duration);
		ClassDB::bind_method(D_METHOD("get_duration"), &VTimelinePanel::get_duration);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "duration", PROPERTY_HINT_RANGE, "0,99999,0.01,or_greater,suffix:s"), "set_duration", "get_duration");

		ClassDB::bind_method(D_METHOD("set_current_time", "current_time"), &VTimelinePanel::set_current_time);
		ClassDB::bind_method(D_METHOD("get_current_time"), &VTimelinePanel::get_current_time);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "current_time", PROPERTY_HINT_RANGE, "0,99999,0.01,or_greater,suffix:s"), "set_current_time", "get_current_time");

		ClassDB::bind_method(D_METHOD("set_scale", "scale"), &VTimelinePanel::set_scale);
		ClassDB::bind_method(D_METHOD("get_scale"), &VTimelinePanel::get_scale);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "scale", PROPERTY_HINT_RANGE, "1,128,0.1,or_greater"), "set_scale", "get_scale");

		ClassDB::bind_method(D_METHOD("set_counting_unit", "unit"), &VTimelinePanel::set_counting_unit);
		ClassDB::bind_method(D_METHOD("get_counting_unit"), &VTimelinePanel::get_counting_unit);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "counting_unit", PROPERTY_HINT_ENUM, "Time,Frame,Beat"), "set_counting_unit", "get_counting_unit");

		ADD_GROUP("Time", "time_");
		ClassDB::bind_method(D_METHOD("set_time_format", "format"), &VTimelinePanel::set_time_format);
		ClassDB::bind_method(D_METHOD("get_time_format"), &VTimelinePanel::get_time_format);
		ADD_PROPERTY(PropertyInfo(Variant::STRING, "time_format"), "set_time_format", "get_time_format");

		ClassDB::bind_method(D_METHOD("set_show_milliseconds", "show_milliseconds"), &VTimelinePanel::set_show_milliseconds);
		ClassDB::bind_method(D_METHOD("get_show_milliseconds"), &VTimelinePanel::get_show_milliseconds);
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "time_show_milliseconds"), "set_show_milliseconds", "get_show_milliseconds");

		ADD_GROUP("Frame", "frame_");
		ClassDB::bind_method(D_METHOD("set_fps", "fps"), &VTimelinePanel::set_fps);
		ClassDB::bind_method(D_METHOD("get_fps"), &VTimelinePanel::get_fps);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "frame_fps"), "set_fps", "get_fps");

		ClassDB::bind_method(D_METHOD("set_show_subdivision", "show_subdivision"), &VTimelinePanel::set_show_subdivision);
		ClassDB::bind_method(D_METHOD("get_show_subdivision"), &VTimelinePanel::get_show_subdivision);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "frame_show_subdivision"), "set_show_subdivision", "get_show_subdivision");

		ADD_GROUP("Beat", "");
		ClassDB::bind_method(D_METHOD("set_bpms", "bpms"), &VTimelinePanel::set_bpms);
		ClassDB::bind_method(D_METHOD("get_bpms"), &VTimelinePanel::get_bpms);
		ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "bpms", PROPERTY_HINT_DICTIONARY_TYPE, "float;int"), "set_bpms", "get_bpms");

		ClassDB::bind_method(D_METHOD("set_beat_per_bar", "num"), &VTimelinePanel::set_beat_per_bar);
		ClassDB::bind_method(D_METHOD("get_beat_per_bar"), &VTimelinePanel::get_beat_per_bar);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "beat_per_bar"), "set_beat_per_bar", "get_beat_per_bar");

		ClassDB::bind_method(D_METHOD("set_beat_line_color", "color"), &VTimelinePanel::set_beat_line_color);
		ClassDB::bind_method(D_METHOD("get_beat_line_color"), &VTimelinePanel::get_beat_line_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "beat_line_color"), "set_beat_line_color", "get_beat_line_color");

		ClassDB::bind_method(D_METHOD("set_beat_line_width", "width"), &VTimelinePanel::set_beat_line_width);
		ClassDB::bind_method(D_METHOD("get_beat_line_width"), &VTimelinePanel::get_beat_line_width);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "beat_line_width"), "set_beat_line_width", "get_beat_line_width");

		ClassDB::bind_method(D_METHOD("set_bar_line_color", "color"), &VTimelinePanel::set_bar_line_color);
		ClassDB::bind_method(D_METHOD("get_bar_line_color"), &VTimelinePanel::get_bar_line_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "bar_line_color"), "set_bar_line_color", "get_bar_line_color");

		ClassDB::bind_method(D_METHOD("set_bar_line_width", "width"), &VTimelinePanel::set_bar_line_width);
		ClassDB::bind_method(D_METHOD("get_bar_line_width"), &VTimelinePanel::get_bar_line_width);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "bar_line_width"), "set_bar_line_width", "get_bar_line_width");

		ClassDB::bind_method(D_METHOD("set_bar_number_direction", "direction"), &VTimelinePanel::set_bar_number_direction);
		ClassDB::bind_method(D_METHOD("get_bar_number_direction"), &VTimelinePanel::get_bar_number_direction);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "bar_number_direction", PROPERTY_HINT_ENUM, "Top Down,Bottom Up"), "set_bar_number_direction", "get_bar_number_direction");

		ClassDB::bind_method(D_METHOD("set_beat_format", "format"), &VTimelinePanel::set_beat_format);
		ClassDB::bind_method(D_METHOD("get_beat_format"), &VTimelinePanel::get_beat_format);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "beat_format", PROPERTY_HINT_ENUM, "Beat Bar"), "set_beat_format", "get_beat_format");
		ADD_GROUP("", "");

		ADD_GROUP("Separator", "separator_");
		ClassDB::bind_method(D_METHOD("set_separator_color", "separator_color"), &VTimelinePanel::set_separator_color);
		ClassDB::bind_method(D_METHOD("get_separator_color"), &VTimelinePanel::get_separator_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "separator_color"), "set_separator_color", "get_separator_color");

		ClassDB::bind_method(D_METHOD("set_separator_width", "separator_width"), &VTimelinePanel::set_separator_width);
		ClassDB::bind_method(D_METHOD("get_separator_width"), &VTimelinePanel::get_separator_width);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "separator_width"), "set_separator_width", "get_separator_width");
		ADD_GROUP("", "");

		ADD_GROUP("Components", "component_");
		ClassDB::bind_method(D_METHOD("set_playhead", "playhead"), &VTimelinePanel::set_playhead);
		ClassDB::bind_method(D_METHOD("get_playhead"), &VTimelinePanel::get_playhead);
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "component_playhead", PROPERTY_HINT_RESOURCE_TYPE, "TimelineIndicator"), "set_playhead", "get_playhead");

		ClassDB::bind_method(D_METHOD("set_markers", "markers"), &VTimelinePanel::set_markers);
		ClassDB::bind_method(D_METHOD("get_markers"), &VTimelinePanel::get_markers);
		ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "component_markers", PROPERTY_HINT_ARRAY_TYPE, "TimelineMarker"), "set_markers", "get_markers");

		ADD_GROUP("Components", "component_");
		ClassDB::bind_method(D_METHOD("set_time_ruler", "time_ruler"), &VTimelinePanel::set_time_ruler);
		ClassDB::bind_method(D_METHOD("get_time_ruler"), &VTimelinePanel::get_time_ruler);
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "component_time_ruler", PROPERTY_HINT_RESOURCE_TYPE, "TimelineTimeRuler"), "set_time_ruler", "get_time_ruler");

		ClassDB::bind_method(D_METHOD("set_tracks", "tracks"), &VTimelinePanel::set_tracks);
		ClassDB::bind_method(D_METHOD("get_tracks"), &VTimelinePanel::get_tracks);
		ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "component_track", PROPERTY_HINT_ARRAY_TYPE, "TimelineTrack", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_EDITOR_INSTANTIATE_OBJECT), "set_tracks", "get_tracks");

		ADD_GROUP("Scrollbar", "");
		ClassDB::bind_method(D_METHOD("set_h_scroll", "value"), &VTimelinePanel::set_h_scroll);
		ClassDB::bind_method(D_METHOD("get_h_scroll"), &VTimelinePanel::get_h_scroll);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "scroll_horizontal", PROPERTY_HINT_NONE, "suffix:px"), "set_h_scroll", "get_h_scroll");

		ClassDB::bind_method(D_METHOD("set_v_scroll", "value"), &VTimelinePanel::set_v_scroll);
		ClassDB::bind_method(D_METHOD("get_v_scroll"), &VTimelinePanel::get_v_scroll);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "scroll_vertical", PROPERTY_HINT_NONE, "suffix:px"), "set_v_scroll", "get_v_scroll");

		ClassDB::bind_method(D_METHOD("set_horizontal_custom_step", "value"), &VTimelinePanel::set_horizontal_custom_step);
		ClassDB::bind_method(D_METHOD("get_horizontal_custom_step"), &VTimelinePanel::get_horizontal_custom_step);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "scroll_horizontal_custom_step", PROPERTY_HINT_RANGE, "-1,4096,suffix:px"), "set_horizontal_custom_step", "get_horizontal_custom_step");

		ClassDB::bind_method(D_METHOD("set_vertical_custom_step", "value"), &VTimelinePanel::set_vertical_custom_step);
		ClassDB::bind_method(D_METHOD("get_vertical_custom_step"), &VTimelinePanel::get_vertical_custom_step);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "scroll_vertical_custom_step", PROPERTY_HINT_RANGE, "-1,4096,suffix:px"), "set_vertical_custom_step", "get_vertical_custom_step");

		ClassDB::bind_method(D_METHOD("set_horizontal_scroll_mode", "enable"), &VTimelinePanel::set_horizontal_scroll_mode);
		ClassDB::bind_method(D_METHOD("get_horizontal_scroll_mode"), &VTimelinePanel::get_horizontal_scroll_mode);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "horizontal_scroll_mode", PROPERTY_HINT_ENUM, "Disabled,Auto,Always Show,Never Show,Reserve"), "set_horizontal_scroll_mode", "get_horizontal_scroll_mode");

		ClassDB::bind_method(D_METHOD("set_vertical_scroll_mode", "enable"), &VTimelinePanel::set_vertical_scroll_mode);
		ClassDB::bind_method(D_METHOD("get_vertical_scroll_mode"), &VTimelinePanel::get_vertical_scroll_mode);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "vertical_scroll_mode", PROPERTY_HINT_ENUM, "Disabled,Auto,Always Show,Never Show,Reserve"), "set_vertical_scroll_mode", "get_vertical_scroll_mode");

		ClassDB::bind_method(D_METHOD("set_deadzone", "deadzone"), &VTimelinePanel::set_deadzone);
		ClassDB::bind_method(D_METHOD("get_deadzone"), &VTimelinePanel::get_deadzone);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "scroll_deadzone"), "set_deadzone", "get_deadzone");
		ADD_GROUP("", "");

		ADD_GROUP("Minimap", "minimap_");
		ClassDB::bind_method(D_METHOD("set_draw_minimap", "enabled"), &VTimelinePanel::set_draw_minimap);
		ClassDB::bind_method(D_METHOD("is_drawing_minimap"), &VTimelinePanel::is_drawing_minimap);
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "minimap_draw", PROPERTY_HINT_GROUP_ENABLE), "set_draw_minimap", "is_drawing_minimap");

		ClassDB::bind_method(D_METHOD("set_minimap_width", "width"), &VTimelinePanel::set_minimap_width);
		ClassDB::bind_method(D_METHOD("get_minimap_width"), &VTimelinePanel::get_minimap_width);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "minimap_width", PROPERTY_HINT_RANGE, "0,512,1,or_greater,suffix:px"), "set_minimap_width", "get_minimap_width");
		ADD_GROUP("", "");

		ADD_GROUP("Key Editing", "");
		ClassDB::bind_method(D_METHOD("set_allow_key_cross_track_move", "enabled"), &VTimelinePanel::set_allow_key_cross_track_move);
		ClassDB::bind_method(D_METHOD("get_allow_key_cross_track_move"), &VTimelinePanel::get_allow_key_cross_track_move);
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "allow_key_cross_track_move"), "set_allow_key_cross_track_move", "get_allow_key_cross_track_move");
		ClassDB::bind_method(D_METHOD("set_key_snap_enabled", "enabled"), &VTimelinePanel::set_key_snap_enabled);
		ClassDB::bind_method(D_METHOD("get_key_snap_enabled"), &VTimelinePanel::get_key_snap_enabled);
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "key_snap_enabled"), "set_key_snap_enabled", "get_key_snap_enabled");
		ClassDB::bind_method(D_METHOD("set_clip_key_edge_edit_enabled", "enabled"), &VTimelinePanel::set_clip_key_edge_edit_enabled);
		ClassDB::bind_method(D_METHOD("get_clip_key_edge_edit_enabled"), &VTimelinePanel::get_clip_key_edge_edit_enabled);
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "clip_key_edge_edit_enabled"), "set_clip_key_edge_edit_enabled", "get_clip_key_edge_edit_enabled");
		ClassDB::bind_method(D_METHOD("set_allow_unselected_key_edit", "enabled"), &VTimelinePanel::set_allow_unselected_key_edit);
		ClassDB::bind_method(D_METHOD("get_allow_unselected_key_edit"), &VTimelinePanel::get_allow_unselected_key_edit);
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "allow_unselected_key_edit"), "set_allow_unselected_key_edit", "get_allow_unselected_key_edit");
		ClassDB::bind_method(D_METHOD("set_allow_right_mouse_selection", "enabled"), &VTimelinePanel::set_allow_right_mouse_selection);
		ClassDB::bind_method(D_METHOD("get_allow_right_mouse_selection"), &VTimelinePanel::get_allow_right_mouse_selection);
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "allow_right_mouse_selection"), "set_allow_right_mouse_selection", "get_allow_right_mouse_selection");
		ADD_GROUP("", "");

		GDVIRTUAL_BIND(_should_handle_selection_rect, "rect", "keys", "mouse_button");
		GDVIRTUAL_BIND(_handle_selection_rect, "rect", "keys", "mouse_button");

		ADD_GROUP("Style overrides", "");
		ADD_SUBGROUP("Constants", "");
		ClassDB::bind_method(D_METHOD("set_icon_max_width", "width"), &VTimelinePanel::set_icon_max_width);
		ClassDB::bind_method(D_METHOD("get_icon_max_width"), &VTimelinePanel::get_icon_max_width);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "icon_max_width"), "set_icon_max_width", "get_icon_max_width");

		ClassDB::bind_method(D_METHOD("set_instant_key_scale", "scale"), &VTimelinePanel::set_instant_key_scale);
		ClassDB::bind_method(D_METHOD("get_instant_key_scale"), &VTimelinePanel::get_instant_key_scale);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "instant_key_scale"), "set_instant_key_scale", "get_instant_key_scale");

		ADD_SUBGROUP("Styles", "");
		ClassDB::bind_method(D_METHOD("set_instant_key_normal_style", "style"), &VTimelinePanel::set_instant_key_normal_style);
		ClassDB::bind_method(D_METHOD("get_instant_key_normal_style"), &VTimelinePanel::get_instant_key_normal_style);
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "instant_key_normal", PROPERTY_HINT_RESOURCE_TYPE, "StyleBox"), "set_instant_key_normal_style", "get_instant_key_normal_style");

		ClassDB::bind_method(D_METHOD("set_instant_key_selected_style", "style"), &VTimelinePanel::set_instant_key_selected_style);
		ClassDB::bind_method(D_METHOD("get_instant_key_selected_style"), &VTimelinePanel::get_instant_key_selected_style);
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "instant_key_selected", PROPERTY_HINT_RESOURCE_TYPE, "StyleBox"), "set_instant_key_selected_style", "get_instant_key_selected_style");

		ClassDB::bind_method(D_METHOD("set_clip_key_normal_style", "style"), &VTimelinePanel::set_clip_key_normal_style);
		ClassDB::bind_method(D_METHOD("get_clip_key_normal_style"), &VTimelinePanel::get_clip_key_normal_style);
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "clip_key_normal", PROPERTY_HINT_RESOURCE_TYPE, "StyleBox"), "set_clip_key_normal_style", "get_clip_key_normal_style");

		ClassDB::bind_method(D_METHOD("set_selection_rect_style", "style"), &VTimelinePanel::set_selection_rect_style);
		ClassDB::bind_method(D_METHOD("get_selection_rect_style"), &VTimelinePanel::get_selection_rect_style);
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "selection_rect", PROPERTY_HINT_RESOURCE_TYPE, "StyleBox"), "set_selection_rect_style", "get_selection_rect_style");

		ClassDB::bind_method(D_METHOD("set_clip_key_selected_style", "style"), &VTimelinePanel::set_clip_key_selected_style);
		ClassDB::bind_method(D_METHOD("get_clip_key_selected_style"), &VTimelinePanel::get_clip_key_selected_style);
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "clip_key_selected", PROPERTY_HINT_RESOURCE_TYPE, "StyleBox"), "set_clip_key_selected_style", "get_clip_key_selected_style");

		ClassDB::bind_method(D_METHOD("set_key_release_preview_style", "style"), &VTimelinePanel::set_key_release_preview_style);
		ClassDB::bind_method(D_METHOD("get_key_release_preview_style"), &VTimelinePanel::get_key_release_preview_style);
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "key_release_preview", PROPERTY_HINT_RESOURCE_TYPE, "StyleBox"), "set_key_release_preview_style", "get_key_release_preview_style");

		ADD_SIGNAL(MethodInfo("scroll_started"));
		ADD_SIGNAL(MethodInfo("scroll_ended"));
		ADD_SIGNAL(MethodInfo("right_mouse_selection_finished",
			PropertyInfo(Variant::RECT2, "rect"),
			PropertyInfo(Variant::ARRAY, "keys", PROPERTY_HINT_ARRAY_TYPE, "TimelineTrackKey")));
		ADD_SIGNAL(MethodInfo("time_ruler_right_clicked",
			PropertyInfo(Variant::FLOAT, "time"),
			PropertyInfo(Variant::VECTOR2, "position")));
	}

	void VTimelinePanel::_notification(int p_what) {
		switch (p_what) {
		case NOTIFICATION_MOUSE_EXIT:
		case NOTIFICATION_MOUSE_EXIT_SELF: {
			if (!clip_key_edge_dragging) {
				set_default_cursor_shape(Control::CURSOR_ARROW);
			}
		} break;

		case NOTIFICATION_RESIZED: {
			_update_scroll_bar();
		} break;

		case NOTIFICATION_INTERNAL_PROCESS: {
			const double delta = get_process_delta_time();
			if (select_pending) {
				select_timer += delta;
				if (select_timer >= long_press_time && !beyond_deadzone) {
					select_pending = false;
					selecting = true;
					// 退出滚动状态，防止后续 motion 被当成滚动
					drag_touching = false;
					beyond_deadzone = false;
					queue_redraw();
				}
			}
			_update_selection_auto_scroll(delta);
			_stop_internal_process_if_idle();
		} break;

		case NOTIFICATION_DRAW: {
			draw_rect(Rect2(Vector2(0, 0), get_size()), background_color);

			// 先计算 header_width
			header_width = _calculate_header_width();
			content_width = header_width;

			// 计算内容高度并更新滚动条
			content_height = _calculate_grid_height();
			_update_scroll_bar();

			// 绘制网格
			switch (counting_unit) {
			case BEAT:
				_draw_grid_beat(header_width);
				break;
			case FRAME:
				_draw_grid_frame(header_width);
				break;
			case TIME:
			default:
				_draw_grid_time(header_width);
				break;
			}

			// 绘制时间尺刻度
			_draw_time_ruler_ticks(header_width);

			// 绘制分隔线
			Vector2 start_pos;
			if (time_ruler.is_valid()) {
				float width = time_ruler->get_width();
				draw_line(Point2(start_pos.x + width, header_height), Point2(start_pos.x + width, get_size().y), separator_color, separator_width);
				start_pos.x += width;
			}
			for (int64_t i = 0; i < tracks.size(); i++) {
				Ref<TimelineTrack> track = tracks[i];
				if (track.is_null()) continue;

				float width = track->get_width();
				draw_line(Point2(start_pos.x + width, header_height), Point2(start_pos.x + width, get_size().y), separator_color, separator_width);
				start_pos.x += width;
			}

			// 绘制轨道键
			Rect2 key_cull_rect(Vector2(0.0f, header_height), Vector2(get_size().x, MAX(get_size().y - header_height, 0.0f)));
			double visible_start = 0.0;
			double visible_end = 0.0;
			_get_visible_key_time_range(16.0f, visible_start, visible_end);
			Ref<StyleBox> key_release_preview_style = _get_key_release_preview_style();
			std::vector<Rect2> key_release_preview_rects;
			struct DeferredStyleDraw {
				Rect2 rect;
				Ref<StyleBox> style;
			};
			std::vector<DeferredStyleDraw> selected_key_overlays;

			for (size_t i = 0; i < _track_cache.size(); i++) {
				const auto &ct = _track_cache[i];
				if (ct.width <= 0.0f) continue;

				float track_x = ct.x_offset - hscroll_value;
				if (track_x + ct.width < 0.0f || track_x > get_size().x) continue;

				float key_margin = MAX(16.0f, ct.width * MAX(ct.max_instant_key_scale, 0.0f) * 0.5f + 16.0f);
				const float instant_lod_bucket_size = CLAMP(ct.width * MAX(ct.max_instant_key_scale, 0.0f) * 0.2f, 2.0f, 8.0f);
				double padded_start = 0.0;
				double padded_end = 0.0;
				_get_visible_key_time_range(key_margin, padded_start, padded_end);

				const double search_start = MIN(padded_start, visible_start - MAX(ct.max_key_length, 0.0));
				const double search_end = padded_end;
				auto key_it = std::lower_bound(ct.keys.begin(), ct.keys.end(), search_start,
					[](TimelineTrackKey* p_key, double p_time) {
						return p_key && p_key->get_time() < p_time;
					});
				Rect2 track_cull_rect = key_cull_rect.grow(key_margin);

				struct InstantKeyBucket {
					bool active = false;
					int index = 0;
					int count = 0;
					Rect2 rect;
					Ref<StyleBox> style;
				};
				InstantKeyBucket instant_key_bucket;
				InstantKeyBucket selected_instant_key_bucket;
				auto append_instant_key_bucket = [&](InstantKeyBucket &p_bucket, const Rect2 &p_rect, const Ref<StyleBox> &p_style, int p_bucket_index) {
					if (p_bucket.active && p_bucket.index != p_bucket_index) {
						if (p_bucket.style.is_valid()) {
							draw_style_box(p_bucket.style, p_bucket.rect);
						}
						p_bucket.active = false;
						p_bucket.count = 0;
					}
					if (!p_bucket.active) {
						p_bucket.active = true;
						p_bucket.index = p_bucket_index;
						p_bucket.count = 1;
						p_bucket.rect = p_rect;
						p_bucket.style = p_style;
					}
					else {
						p_bucket.count++;
						p_bucket.rect = p_bucket.rect.merge(p_rect);
						if (!p_bucket.style.is_valid()) {
							p_bucket.style = p_style;
						}
					}
				};
				auto flush_instant_key_bucket = [&]() {
					if (!instant_key_bucket.active) {
						return;
					}
					if (instant_key_bucket.style.is_valid()) {
						draw_style_box(instant_key_bucket.style, instant_key_bucket.rect);
					}
					instant_key_bucket.active = false;
					instant_key_bucket.count = 0;
				};
				auto flush_selected_instant_key_bucket = [&]() {
					if (!selected_instant_key_bucket.active) {
						return;
					}
					if (selected_instant_key_bucket.style.is_valid()) {
						selected_key_overlays.push_back({ selected_instant_key_bucket.rect, selected_instant_key_bucket.style });
					}
					selected_instant_key_bucket.active = false;
					selected_instant_key_bucket.count = 0;
				};

				for (; key_it != ct.keys.end(); ++key_it) {
					TimelineTrackKey *key = *key_it;
					if (!key || key->is_disabled()) continue;

					const double key_start = key->get_time();
					if (key_start > search_end) {
						break;
					}

					if (key->is_instant()) {
						Rect2 key_rect = _get_instant_key_rect(ct, key, _key_to_y(key));
						if (!track_cull_rect.intersects(key_rect)) continue;

						const bool release_previewed = _is_key_release_previewed(key);
						if (!release_previewed) {
							const int bucket_index = static_cast<int>(Math::floor((key_rect.get_center().y - header_height) / instant_lod_bucket_size));
							Ref<StyleBox> style = _get_instant_key_normal_style(key);
							append_instant_key_bucket(instant_key_bucket, key_rect, style, bucket_index);
							if (key->is_selected()) {
								style = _get_instant_key_selected_style(key);
								if (selected_instant_key_bucket.active && selected_instant_key_bucket.index != bucket_index) {
									flush_selected_instant_key_bucket();
								}
								append_instant_key_bucket(selected_instant_key_bucket, key_rect, style, bucket_index);
							}
							continue;
						}

						flush_instant_key_bucket();
						Ref<StyleBox> style = _get_instant_key_normal_style(key);
						if (style.is_valid()) {
							draw_style_box(style, key_rect);
						}
						if (key->is_selected()) {
							style = _get_instant_key_selected_style(key);
							if (style.is_valid()) {
								selected_key_overlays.push_back({ key_rect, style });
							}
						}
						if (release_previewed) {
							key_release_preview_rects.push_back(key_rect);
						}
					}
					else {
						flush_instant_key_bucket();
						// 防止矩形尺寸出现负数
						if (key_start > visible_end) continue;

						const double key_end = key_start + key->get_length();
						if (key_end < visible_start) continue;

						Rect2 bar_rect = _get_clip_key_rect(ct, _key_to_y(key), _key_end_to_y(key));
						if (!track_cull_rect.intersects(bar_rect)) continue;

						Ref<StyleBox> style = _get_clip_key_normal_style(key);
						if (style.is_valid()) {
							draw_style_box(style, bar_rect);
						}
						if (key->is_selected()) {
							style = _get_clip_key_selected_style(key);
							if (style.is_valid()) {
								selected_key_overlays.push_back({ bar_rect, style });
							}
						}
						if (_is_key_release_previewed(key)) {
							key_release_preview_rects.push_back(bar_rect);
						}
					}
				}
				flush_instant_key_bucket();
				flush_selected_instant_key_bucket();
			}

			for (const DeferredStyleDraw &overlay : selected_key_overlays) {
				if (overlay.style.is_valid()) {
					draw_style_box(overlay.style, overlay.rect);
				}
			}

			if (key_release_preview_style.is_valid()) {
				for (const Rect2 &preview_rect : key_release_preview_rects) {
					draw_style_box(key_release_preview_style, preview_rect);
				}
			}

			// 绘制指示器
			if (time_ruler.is_valid()) {
				TypedArray<TimelineIndicator> all_indicators;
				for (int i = 0; i < markers.size(); i++) {
					Ref<TimelineIndicator> indicator = markers[i];
					if (indicator.is_valid()) {
						all_indicators.append(markers[i]);
					}
				}
				if (playhead.is_valid()) {
					all_indicators.append(playhead);
				}

				for (int i = 0; i < all_indicators.size(); i++) {
					float current_width = 0.0f;
					if (time_ruler.is_valid()) {
						current_width += time_ruler->get_width();
					}

					Ref<TimelineIndicator> indicator = all_indicators[i];
					double current_position = 0.0;
					double time = current_time;
					Ref<TimelineMarker> marker = indicator;
					if (marker.is_valid()) {
						time = marker->get_time();
					}

					switch (counting_unit) {
					case FRAME: {
						int64_t frame = static_cast<int64_t>(time * fps);
						current_position = get_position_from_frame(frame);
						break;
					}
					case BEAT: {
						current_position = _time_to_y(time);
						break;
					}
					case TIME:
					default:
						current_position = get_position_from_time(time);
						break;
					}
					Rect2 header_rect = Rect2(Vector2(0.0f, current_position - 8.0f), Vector2(time_ruler->get_width(), 16.0f));
					indicator->draw(get_canvas_item(), header_rect, _format_indicator_time(time), header_width, false);
				}
			}

			if (selecting || right_selecting) {
				Rect2 sel_rect = selecting ? _make_selection_rect(select_start, select_end) : _make_selection_rect(right_select_start, right_select_end);

				Ref<StyleBox> style = get_selection_rect_style();
				if (style.is_valid()) {
					draw_style_box(style, sel_rect);
				}
				else {
					draw_rect(sel_rect, Color(1.0f, 1.0f, 1.0f, 0.3f));
					draw_rect(sel_rect, Color(1.0f, 1.0f, 1.0f), false);
				}
			}

			// 最后绘制 header
			start_pos = Vector2(0, 0);
			if (time_ruler.is_valid()) {
				float width = time_ruler->get_width();
				Ref<StyleBox> header_background = time_ruler->get_header_background();
				Ref<Texture2D> header_icon = time_ruler->get_header_icon();
				_draw_header(start_pos, width, header_background, header_icon);
				start_pos.x += width;
			}
			for (int64_t i = 0; i < tracks.size(); i++) {
				Ref<TimelineTrack> track = tracks[i];
				if (track.is_null()) continue;

				float width = track->get_width();
				Ref<StyleBox> header_background = track->get_header_background();
				Ref<Texture2D> header_icon = track->get_header_icon();
				_draw_header(start_pos, width, header_background, header_icon);
				start_pos.x += width;
			}

			draw_line(Point2(start_pos.x, header_height), Point2(start_pos.x, get_size().y), separator_color, separator_width);
			_draw_minimap();
		} break;
		}
	}

	void VTimelinePanel::_validate_property(PropertyInfo& p_property) const {
		String name = p_property.name;

		switch (counting_unit) {
		case TIME: {
			if (name.begins_with("frame_") || name.begins_with("beat_")) {
				p_property.usage = PROPERTY_USAGE_NO_EDITOR;
			}
		} break;

		case FRAME: {
			if (name.begins_with("time_") || name.begins_with("beat_")) {
				p_property.usage = PROPERTY_USAGE_NO_EDITOR;
			}
		} break;

		case BEAT: {
			if (name.begins_with("time_") || name.begins_with("frame_")) {
				p_property.usage = PROPERTY_USAGE_NO_EDITOR;
			}
		} break;
		}
	}

	VTimelinePanel::VTimelinePanel() {
		set_clip_contents(true);

		hscroll = memnew(HScrollBar);
		hscroll->set_step(0.001);
		hscroll->set_anchor_and_offset(SIDE_LEFT, ANCHOR_BEGIN, 0);
		hscroll->set_anchor_and_offset(SIDE_RIGHT, ANCHOR_END, 0);
		hscroll->set_anchor_and_offset(SIDE_TOP, ANCHOR_END, 0);
		hscroll->set_anchor_and_offset(SIDE_BOTTOM, ANCHOR_END, 0);
		hscroll->hide();
		hscroll->connect("value_changed", callable_mp(this, &VTimelinePanel::_h_scroll_changed));
		add_child(hscroll, false, INTERNAL_MODE_FRONT);

		vscroll = memnew(VScrollBar);
		vscroll->set_step(0.001);
		vscroll->set_anchor_and_offset(SIDE_LEFT, ANCHOR_END, 0);
		vscroll->set_anchor_and_offset(SIDE_RIGHT, ANCHOR_END, 0);
		vscroll->set_anchor_and_offset(SIDE_TOP, ANCHOR_BEGIN, 0);
		vscroll->set_anchor_and_offset(SIDE_BOTTOM, ANCHOR_END, 0);
		vscroll->hide();
		vscroll->connect("value_changed", callable_mp(this, &VTimelinePanel::_v_scroll_changed));

		Ref<StyleBoxFlat> instant_key_normal;
		instant_key_normal.instantiate();
		instant_key_normal->set_bg_color(Color(1.0f, 1.0f, 1.0f));
		instant_key_normal->set_corner_detail(1);
		instant_key_normal->set_corner_radius_all(512);
		style_cache.instant_key_normal_fallback = instant_key_normal;

		Ref<StyleBoxFlat> instant_key_selected;
		instant_key_selected.instantiate();
		instant_key_selected->set_draw_center(false);
		instant_key_selected->set_border_width_all(4);
		instant_key_selected->set_border_color(Color(1.0f, 1.0f, 1.0f, 0.3f));
		instant_key_selected->set_expand_margin_all(4);
		instant_key_selected->set_corner_detail(1);
		instant_key_selected->set_corner_radius_all(512);
		style_cache.instant_key_selected_fallback = instant_key_selected;

		Ref<StyleBoxFlat> clip_key_normal;
		clip_key_normal.instantiate();
		clip_key_normal->set_bg_color(Color(1.0f, 1.0f, 1.0f));
		style_cache.clip_key_normal_fallback = clip_key_normal;

		Ref<StyleBoxFlat> clip_key_selected;
		clip_key_selected.instantiate();
		clip_key_selected->set_draw_center(false);
		clip_key_selected->set_border_width_all(4);
		clip_key_selected->set_border_color(Color(1.0f, 1.0f, 1.0f, 0.3f));
		clip_key_selected->set_expand_margin_all(4);
		style_cache.clip_key_selected_fallback = clip_key_selected;

		Ref<StyleBoxFlat> key_release_preview;
		key_release_preview.instantiate();
		key_release_preview->set_bg_color(Color(1.0f, 0.12f, 0.06f, 0.35f));
		key_release_preview->set_border_width_all(2);
		key_release_preview->set_border_color(Color(1.0f, 0.12f, 0.06f, 0.9f));
		key_release_preview->set_corner_detail(4);
		key_release_preview->set_corner_radius_all(4);
		style_cache.key_release_preview_fallback = key_release_preview;

		add_child(vscroll, false, INTERNAL_MODE_FRONT);
	}

	VTimelinePanel::~VTimelinePanel() {
		clear_all_keys();
	}

	void VTimelinePanel::_collect_selected_keys() {
		const Rect2 sel_rect = _make_selection_rect(select_start, select_end);
		const TypedArray<TimelineTrackKey> selected_keys = _get_keys_in_rect(sel_rect);
		if (_try_handle_selection_rect(sel_rect, selected_keys, static_cast<int>(MouseButton::MOUSE_BUTTON_LEFT))) {
			return;
		}

		for (auto& ct : _track_cache) {
			for (TimelineTrackKey* key : ct.keys) {
				if (key) {
					key->set_selected_no_signal(false);
				}
			}
		}

		for (int i = 0; i < selected_keys.size(); i++) {
			TimelineTrackKey *key = VariantCaster<TimelineTrackKey *>::cast(selected_keys[i]);
			if (key) {
				key->set_selected_no_signal(true);
			}
		}
		_mark_minimap_key_cache_dirty();
	}

	Rect2 VTimelinePanel::_make_selection_rect(const Vector2 &p_start, const Vector2 &p_end) const {
		Rect2 sel_rect;
		sel_rect.position.x = Math::min(p_start.x, p_end.x);
		sel_rect.position.y = Math::min(p_start.y, p_end.y);
		sel_rect.size.x = Math::abs(p_end.x - p_start.x);
		sel_rect.size.y = Math::abs(p_end.y - p_start.y);

		if (sel_rect.size.x < 2.0f && sel_rect.size.y < 2.0f) {
			sel_rect = Rect2(p_start - Vector2(2, 2), Vector2(4, 4));
		}
		return sel_rect;
	}

	TypedArray<TimelineTrackKey> VTimelinePanel::_get_keys_in_rect(const Rect2 &p_rect) const {
		TypedArray<TimelineTrackKey> keys;
		for (const CachedTrack &ct : _track_cache) {
			if (ct.width <= 0.0f) continue;

			for (TimelineTrackKey *key : ct.keys) {
				if (!key || key->is_disabled()) continue;

				Rect2 key_rect;
				if (key->is_instant()) {
					key_rect = _get_instant_key_rect(ct, key, _key_to_y(key));
				}
				else {
					key_rect = _get_clip_key_rect(ct, _key_to_y(key), _key_end_to_y(key));
				}

				if (p_rect.intersects(key_rect)) {
					keys.append(key);
				}
			}
		}
		return keys;
	}

	bool VTimelinePanel::_try_handle_selection_rect(const Rect2 &p_rect, const TypedArray<TimelineTrackKey> &p_keys, int p_mouse_button) {
		bool should_handle = false;
		if (GDVIRTUAL_CALL(_should_handle_selection_rect, p_rect, p_keys, p_mouse_button, should_handle) && should_handle) {
			GDVIRTUAL_CALL(_handle_selection_rect, p_rect, p_keys, p_mouse_button);
			return true;
		}
		return false;
	}

	void VTimelinePanel::_finish_right_mouse_selection() {
		if (!right_selecting) {
			return;
		}

		right_selecting = false;
		const Rect2 selection_rect = _make_selection_rect(right_select_start, right_select_end);
		const TypedArray<TimelineTrackKey> selected_keys = _get_keys_in_rect(selection_rect);
		if (!_try_handle_selection_rect(selection_rect, selected_keys, static_cast<int>(MouseButton::MOUSE_BUTTON_RIGHT))) {
			emit_signal("right_mouse_selection_finished", selection_rect, selected_keys);
		}
		queue_redraw();
		_stop_internal_process_if_idle();
	}

	bool VTimelinePanel::_find_selected_key_at_position(const Vector2& p_position, int& r_track_index, TimelineTrackKey*& r_key) const {
		r_track_index = -1;
		r_key = nullptr;

		for (int i = static_cast<int>(_track_cache.size()) - 1; i >= 0; i--) {
			const CachedTrack& ct = _track_cache[i];
			if (ct.width <= 0.0f) continue;

			for (auto it = ct.keys.rbegin(); it != ct.keys.rend(); ++it) {
				TimelineTrackKey* key = *it;
				if (!key || key->is_disabled()) continue;
				if (!allow_unselected_key_edit && !key->is_selected()) continue;

				Rect2 key_rect;
				if (key->is_instant()) {
					key_rect = _get_instant_key_rect(ct, key, _key_to_y(key));
				}
				else {
					key_rect = _get_clip_key_rect(ct, _key_to_y(key), _key_end_to_y(key));
				}

				if (key_rect.has_point(p_position)) {
					r_track_index = i;
					r_key = key;
					return true;
				}
			}
		}

		return false;
	}

	int VTimelinePanel::_get_track_index_at_x(float p_x) const {
		int last_valid = -1;

		for (int i = 0; i < static_cast<int>(_track_cache.size()); i++) {
			const CachedTrack& ct = _track_cache[i];
			if (ct.width <= 0.0f) continue;

			const float left = ct.x_offset - hscroll_value;
			const float right = left + ct.width;
			if (p_x >= left && p_x <= right) {
				return i;
			}
			if (p_x < left) {
				return last_valid >= 0 ? last_valid : i;
			}
			last_valid = i;
		}

		return last_valid;
	}

	int VTimelinePanel::_get_track_header_index_at_x(float p_x) const {
		float x = 0.0f;
		if (time_ruler.is_valid()) {
			x += time_ruler->get_width();
		}

		for (int i = 0; i < static_cast<int>(_track_cache.size()); i++) {
			const CachedTrack &ct = _track_cache[i];
			if (ct.width <= 0.0f) continue;

			const float left = x;
			const float right = left + ct.width;
			if (p_x >= left && p_x <= right) {
				return i;
			}
			x = right;
		}

		return -1;
	}

	void VTimelinePanel::_select_track_keys(int p_track_index) {
		if (p_track_index < 0 || p_track_index >= static_cast<int>(_track_cache.size())) {
			return;
		}

		for (CachedTrack &ct : _track_cache) {
			for (TimelineTrackKey *key : ct.keys) {
				if (key) {
					key->set_selected_no_signal(false);
				}
			}
		}

		for (TimelineTrackKey *key : _track_cache[p_track_index].keys) {
			if (key && !key->is_disabled()) {
				key->set_selected_no_signal(true);
			}
		}

		_mark_minimap_key_cache_dirty();
		queue_redraw();
	}

	void VTimelinePanel::_update_selection_auto_scroll(double p_delta) {
		if ((!selecting && !right_selecting) || vscroll == nullptr || !vscroll->is_visible() || vertical_scroll_mode == SCROLL_MODE_DISABLED) {
			return;
		}

		Vector2 *selection_start = right_selecting ? &right_select_start : &select_start;
		const Vector2 *selection_end = right_selecting ? &right_select_end : &select_end;
		const float hscroll_height = hscroll != nullptr && hscroll->is_visible() ? hscroll->get_combined_minimum_size().y : 0.0f;
		const float top = header_height;
		const float bottom = get_size().y - hscroll_height;
		if (bottom <= top) {
			return;
		}

		const float margin = CLAMP((bottom - top) * 0.12f, 24.0f, 64.0f);
		const float max_speed = 720.0f;
		float speed = 0.0f;
		if (selection_end->y < top + margin) {
			speed = -CLAMP((top + margin - selection_end->y) / margin, 0.0f, 1.0f) * max_speed;
		}
		else if (selection_end->y > bottom - margin) {
			speed = CLAMP((selection_end->y - (bottom - margin)) / margin, 0.0f, 1.0f) * max_speed;
		}

		if (speed == 0.0f) {
			return;
		}

		const double before = vscroll->get_value();
		_scroll(vscroll, speed * p_delta);
		const double after = vscroll->get_value();
		if (Math::is_equal_approx(before, after)) {
			return;
		}

		selection_start->y -= static_cast<float>(after - before);
		queue_redraw();
	}

	void VTimelinePanel::_stop_internal_process_if_idle() {
		if (!select_pending && !selecting && !right_selecting && !drag_touching && !drag_touching_deaccel) {
			set_process_internal(false);
		}
	}

	double VTimelinePanel::_position_to_key_value(double p_y) const {
		if (counting_unit == FRAME) {
			return static_cast<double>(_y_to_frame(p_y));
		}
		return _y_to_time(p_y);
	}

	double VTimelinePanel::_get_playhead_drag_time(double p_y) const {
		double time = get_time_from_position(p_y);
		if (key_snap_enabled) {
			if (counting_unit == FRAME) {
				const int safe_fps = MAX(fps, 1);
				time = static_cast<double>(Math::floor(time * safe_fps + 0.5)) / safe_fps;
			}
			else {
				time = _snap_key_time(time);
			}
		}
		return CLAMP(time, 0.0, duration);
	}

	bool VTimelinePanel::_find_clip_key_edge_at_position(const Vector2 &p_position, int &r_track_index, TimelineTrackKey *&r_key, ClipKeyEditEdge &r_edge) const {
		r_track_index = -1;
		r_key = nullptr;
		r_edge = CLIP_KEY_EDIT_EDGE_NONE;

		if (!clip_key_edge_edit_enabled || p_position.y <= header_height) {
			return false;
		}

		const float edge_margin = 6.0f;
		for (int i = static_cast<int>(_track_cache.size()) - 1; i >= 0; i--) {
			const CachedTrack &ct = _track_cache[i];
			if (ct.width <= 0.0f) continue;

			for (auto it = ct.keys.rbegin(); it != ct.keys.rend(); ++it) {
				TimelineTrackKey *key = *it;
				if (!key || key->is_disabled() || key->is_instant()) continue;
				if (!allow_unselected_key_edit && !key->is_selected()) continue;

				const double head_y = _key_to_y(key);
				const double tail_y = _key_end_to_y(key);
				Rect2 key_rect = _get_clip_key_rect(ct, head_y, tail_y);
				if (p_position.x < key_rect.position.x || p_position.x > key_rect.position.x + key_rect.size.x) continue;

				const float head_distance = Math::abs(p_position.y - static_cast<float>(head_y));
				const float tail_distance = Math::abs(p_position.y - static_cast<float>(tail_y));
				const float nearest_distance = MIN(head_distance, tail_distance);
				if (nearest_distance > edge_margin) continue;

				r_track_index = i;
				r_key = key;
				r_edge = head_distance <= tail_distance ? CLIP_KEY_EDIT_EDGE_HEAD : CLIP_KEY_EDIT_EDGE_TAIL;
				return true;
			}
		}

		return false;
	}

	void VTimelinePanel::_update_clip_key_edge_cursor(const Vector2 &p_position) {
		if (clip_key_edge_dragging) {
			set_default_cursor_shape(Control::CURSOR_VSPLIT);
			return;
		}

		int hit_track_index = -1;
		TimelineTrackKey *hit_key = nullptr;
		ClipKeyEditEdge hit_edge = CLIP_KEY_EDIT_EDGE_NONE;
		if (_find_clip_key_edge_at_position(p_position, hit_track_index, hit_key, hit_edge)) {
			set_default_cursor_shape(Control::CURSOR_VSPLIT);
		}
		else {
			set_default_cursor_shape(Control::CURSOR_ARROW);
		}
	}

	void VTimelinePanel::_begin_clip_key_edge_drag(TimelineTrackKey *p_key, ClipKeyEditEdge p_edge) {
		if (!p_key || p_key->is_instant() || p_edge == CLIP_KEY_EDIT_EDGE_NONE) {
			return;
		}
		if (!allow_unselected_key_edit && !p_key->is_selected()) {
			return;
		}

		_cancel_drag();
		selecting = false;
		select_pending = false;
		_clear_key_release_preview();
		if (!p_key->is_selected()) {
			for (CachedTrack &ct : _track_cache) {
				for (TimelineTrackKey *key : ct.keys) {
					if (key) {
						key->set_selected_no_signal(false);
					}
				}
			}
			p_key->set_selected_no_signal(true);
			_mark_minimap_key_cache_dirty();
			queue_redraw();
		}
		clip_key_edge_dragging = true;
		clip_key_edge_drag_moved = false;
		clip_key_edge_drag_key = p_key;
		clip_key_edge_drag_edge = p_edge;
		clip_key_edge_drag_head_time = p_key->get_time();
		clip_key_edge_drag_tail_time = p_key->get_time() + p_key->get_length();
		resized_clip_keys.clear();

		auto add_resized_key = [&](TimelineTrackKey *p_add_key) {
			if (!p_add_key || p_add_key->is_disabled() || p_add_key->is_instant() || !p_add_key->is_selected()) {
				return;
			}

			ResizedClipKey resized_key;
			resized_key.key = p_add_key;
			resized_key.original_head_time = p_add_key->get_time();
			resized_key.original_tail_time = p_add_key->get_time() + p_add_key->get_length();
			resized_clip_keys.push_back(resized_key);
		};

		add_resized_key(p_key);
		for (CachedTrack &ct : _track_cache) {
			for (TimelineTrackKey *key : ct.keys) {
				if (key == p_key) continue;
				add_resized_key(key);
			}
		}

		set_default_cursor_shape(Control::CURSOR_VSPLIT);
	}

	void VTimelinePanel::_update_clip_key_edge_drag(const Vector2 &p_position) {
		if (!clip_key_edge_dragging || !clip_key_edge_drag_key || resized_clip_keys.empty()) {
			return;
		}

		double dragged_time = _position_to_key_value(p_position.y);
		if (key_snap_enabled) {
			dragged_time = _snap_key_time(dragged_time);
		}

		const double anchor_time = clip_key_edge_drag_edge == CLIP_KEY_EDIT_EDGE_HEAD ? clip_key_edge_drag_head_time : clip_key_edge_drag_tail_time;
		const double edge_delta = dragged_time - anchor_time;
		bool changed = false;

		for (ResizedClipKey &resized_key : resized_clip_keys) {
			if (!resized_key.key) continue;

			const double fixed_time = clip_key_edge_drag_edge == CLIP_KEY_EDIT_EDGE_HEAD ? resized_key.original_tail_time : resized_key.original_head_time;
			const double dragged_edge_time = clip_key_edge_drag_edge == CLIP_KEY_EDIT_EDGE_HEAD ? resized_key.original_head_time + edge_delta : resized_key.original_tail_time + edge_delta;
			const double new_head_time = MIN(fixed_time, dragged_edge_time);
			const double new_tail_time = MAX(fixed_time, dragged_edge_time);
			const double new_length = new_tail_time - new_head_time;

			if (!Math::is_equal_approx(resized_key.key->get_time(), new_head_time) || !Math::is_equal_approx(resized_key.key->get_length(), new_length)) {
				changed = true;
			}
			resized_key.key->set_time_no_signal(new_head_time);
			resized_key.key->set_length_no_signal(new_length);
		}

		if (!changed) {
			return;
		}

		clip_key_edge_drag_moved = true;
		_refresh_track_key_metrics();
		std::vector<TimelineTrackKey *> resized_keys;
		resized_keys.reserve(resized_clip_keys.size());
		for (const ResizedClipKey &resized_key : resized_clip_keys) {
			if (resized_key.key) {
				resized_keys.push_back(resized_key.key);
			}
		}
		_update_key_release_preview(resized_keys);
		queue_redraw();
	}

	void VTimelinePanel::_finish_clip_key_edge_drag() {
		if (!clip_key_edge_dragging) {
			return;
		}

		std::vector<TimelineTrackKey *> resized_keys;
		resized_keys.reserve(resized_clip_keys.size());
		for (const ResizedClipKey &resized_key : resized_clip_keys) {
			if (resized_key.key) {
				resized_keys.push_back(resized_key.key);
			}
		}
		const bool was_moved = clip_key_edge_drag_moved;
		clip_key_edge_dragging = false;
		clip_key_edge_drag_moved = false;
		clip_key_edge_drag_key = nullptr;
		clip_key_edge_drag_edge = CLIP_KEY_EDIT_EDGE_NONE;
		resized_clip_keys.clear();

		if (was_moved) {
			_destroy_moved_key_overlaps(resized_keys);
		}
		_clear_key_release_preview();
		_refresh_track_key_metrics();
		queue_redraw();
	}

	void VTimelinePanel::_begin_key_drag(int p_track_index, TimelineTrackKey* p_key, const Vector2& p_position) {
		if (!p_key || p_track_index < 0 || p_track_index >= static_cast<int>(_track_cache.size())) {
			return;
		}
		if (!allow_unselected_key_edit && !p_key->is_selected()) {
			return;
		}

		_cancel_drag();
		selecting = false;
		select_pending = false;
		_clear_key_release_preview();
		if (!p_key->is_selected()) {
			for (CachedTrack &ct : _track_cache) {
				for (TimelineTrackKey *key : ct.keys) {
					if (key) {
						key->set_selected_no_signal(false);
					}
				}
			}
			p_key->set_selected_no_signal(true);
			_mark_minimap_key_cache_dirty();
			queue_redraw();
		}
		key_dragging = true;
		key_drag_moved = false;
		key_drag_start_value = _position_to_key_value(p_position.y);
		key_drag_anchor_track = p_track_index;
		dragged_keys.clear();

		for (int i = 0; i < static_cast<int>(_track_cache.size()); i++) {
			CachedTrack& ct = _track_cache[i];
			for (TimelineTrackKey* key : ct.keys) {
				if (!key || key->is_disabled() || !key->is_selected()) continue;

				DraggedKey dragged_key;
				dragged_key.key = key;
				dragged_key.original_track_index = i;
				dragged_key.current_track_index = i;
				dragged_key.original_time = key->get_time();
				dragged_keys.push_back(dragged_key);
			}
		}

		if (dragged_keys.empty()) {
			DraggedKey dragged_key;
			dragged_key.key = p_key;
			dragged_key.original_track_index = p_track_index;
			dragged_key.current_track_index = p_track_index;
			dragged_key.original_time = p_key->get_time();
			dragged_keys.push_back(dragged_key);
		}
	}

	void VTimelinePanel::_update_key_drag(const Vector2& p_position) {
		if (!key_dragging || dragged_keys.empty()) {
			return;
		}

		const double current_value = _position_to_key_value(p_position.y);
		const double value_delta = current_value - key_drag_start_value;
		const int target_anchor_track = allow_key_cross_track_move ? _get_track_index_at_x(p_position.x) : key_drag_anchor_track;
		const int track_delta = target_anchor_track >= 0 && key_drag_anchor_track >= 0 ? target_anchor_track - key_drag_anchor_track : 0;
		const double max_value = counting_unit == FRAME ? duration * fps : duration;

		if (Math::abs(value_delta) <= 0.000001 && track_delta == 0) {
			return;
		}

		bool changed = false;
		for (DraggedKey& dragged_key : dragged_keys) {
			if (!dragged_key.key) continue;

			int target_track = dragged_key.original_track_index;
			if (allow_key_cross_track_move) {
				target_track = CLAMP(dragged_key.original_track_index + track_delta, 0, static_cast<int>(_track_cache.size()) - 1);
			}

			if (target_track != dragged_key.current_track_index) {
				_move_key_to_track(dragged_key.key, dragged_key.current_track_index, target_track);
				dragged_key.current_track_index = target_track;
				changed = true;
			}

			double target_time = dragged_key.original_time + value_delta;
			if (key_snap_enabled) {
				target_time = _snap_key_time(target_time);
			}
			target_time = CLAMP(target_time, 0.0, max_value);
			if (!Math::is_equal_approx(dragged_key.key->get_time(), target_time)) {
				changed = true;
			}
			dragged_key.key->set_time_no_signal(target_time);
		}

		if (!changed) {
			return;
		}
		key_drag_moved = true;
		_refresh_track_key_metrics();
		std::vector<TimelineTrackKey *> moved_keys;
		moved_keys.reserve(dragged_keys.size());
		for (const DraggedKey &dragged_key : dragged_keys) {
			if (dragged_key.key) {
				moved_keys.push_back(dragged_key.key);
			}
		}
		_update_key_release_preview(moved_keys);
		queue_redraw();
	}

	void VTimelinePanel::_finish_key_drag() {
		if (!key_dragging) {
			return;
		}

		std::vector<TimelineTrackKey*> moved_keys;
		moved_keys.reserve(dragged_keys.size());
		for (const DraggedKey& dragged_key : dragged_keys) {
			if (dragged_key.key) {
				moved_keys.push_back(dragged_key.key);
			}
		}

		key_dragging = false;
		key_drag_anchor_track = -1;
		dragged_keys.clear();

		if (key_drag_moved) {
			_destroy_moved_key_overlaps(moved_keys);
		}
		_clear_key_release_preview();
		key_drag_moved = false;
		_refresh_track_key_metrics();
		queue_redraw();
	}

	void VTimelinePanel::_move_key_to_track(TimelineTrackKey* p_key, int p_from_track, int p_to_track) {
		if (!p_key || p_to_track < 0 || p_to_track >= static_cast<int>(_track_cache.size()) || p_from_track == p_to_track) {
			return;
		}

		if (p_from_track < 0 || p_from_track >= static_cast<int>(_track_cache.size())) {
			for (int i = 0; i < static_cast<int>(_track_cache.size()); i++) {
				auto it = std::find(_track_cache[i].keys.begin(), _track_cache[i].keys.end(), p_key);
				if (it != _track_cache[i].keys.end()) {
					p_from_track = i;
					break;
				}
			}
		}
		if (p_from_track < 0 || p_from_track >= static_cast<int>(_track_cache.size())) {
			return;
		}

		std::vector<TimelineTrackKey*>& from_keys = _track_cache[p_from_track].keys;
		auto it = std::find(from_keys.begin(), from_keys.end(), p_key);
		if (it == from_keys.end()) {
			return;
		}

		from_keys.erase(it);
		_track_cache[p_to_track].keys.push_back(p_key);
	}

	double VTimelinePanel::_snap_key_time(double p_time) const {
		switch (counting_unit) {
		case FRAME:
			return Math::floor(p_time + 0.5);
		case BEAT: {
			if (beat_map.is_empty() || time_map.is_empty()) {
				return p_time;
			}
			const int divisions = MAX(beats_per_bar, 1);
			const double beat = _time_to_beat(p_time);
			const double snapped_beat = Math::floor(beat * divisions + 0.5) / divisions;
			return _beat_to_time(snapped_beat);
		}
		case TIME:
		default: {
			double time_interval = 1.0;
			if (scale >= 64.0f) time_interval = 0.1;
			else if (scale >= 32.0f) time_interval = 0.5;
			else if (scale >= 16.0f) time_interval = 1.0;
			else if (scale >= 8.0f) time_interval = 5.0;
			else time_interval = 10.0;

			return Math::floor(p_time / time_interval + 0.5) * time_interval;
		}
		}
	}

	bool VTimelinePanel::_keys_overlap(const TimelineTrackKey* p_a, const TimelineTrackKey* p_b) const {
		if (!p_a || !p_b || p_a == p_b) {
			return false;
		}

		const double epsilon = 0.000001;
		const double a_start = p_a->get_time();
		const double b_start = p_b->get_time();
		const double a_length = MAX(p_a->get_length(), 0.0);
		const double b_length = MAX(p_b->get_length(), 0.0);
		const bool a_instant = a_length <= epsilon;
		const bool b_instant = b_length <= epsilon;

		if (a_instant && b_instant) {
			return Math::abs(a_start - b_start) <= epsilon;
		}

		const double a_end = a_start + a_length;
		const double b_end = b_start + b_length;
		if (a_instant) {
			return a_start >= b_start - epsilon && a_start < b_end - epsilon;
		}
		if (b_instant) {
			return b_start >= a_start - epsilon && b_start < a_end - epsilon;
		}

		return a_start < b_end - epsilon && b_start < a_end - epsilon;
	}

	std::vector<TimelineTrackKey *> VTimelinePanel::_get_moved_key_overlaps(const std::vector<TimelineTrackKey *> &p_moved_keys) const {
		std::vector<TimelineTrackKey *> destroy_keys;
		std::vector<TimelineTrackKey *> kept_moved_keys;
		std::unordered_set<TimelineTrackKey *> moved_key_set;
		std::unordered_set<TimelineTrackKey *> destroy_key_set;

		moved_key_set.reserve(p_moved_keys.size());
		destroy_key_set.reserve(p_moved_keys.size());
		for (TimelineTrackKey *moved_key : p_moved_keys) {
			if (moved_key) {
				moved_key_set.insert(moved_key);
			}
		}

		auto mark_destroy = [&](TimelineTrackKey *p_key) {
			if (destroy_key_set.insert(p_key).second) {
				destroy_keys.push_back(p_key);
			}
		};

		for (TimelineTrackKey *moved_key : p_moved_keys) {
			if (!moved_key) continue;
			if (destroy_key_set.find(moved_key) != destroy_key_set.end()) continue;

			const CachedTrack *moved_track = nullptr;
			for (const CachedTrack &ct : _track_cache) {
				if (std::find(ct.keys.begin(), ct.keys.end(), moved_key) == ct.keys.end()) {
					continue;
				}
				moved_track = &ct;
				break;
			}
			if (moved_track == nullptr) {
				continue;
			}

			bool should_destroy = false;
			for (TimelineTrackKey *other_key : moved_track->keys) {
				if (!other_key || other_key->is_disabled() || other_key == moved_key) continue;
				if (moved_key_set.find(other_key) != moved_key_set.end()) continue;

				if (_keys_overlap(moved_key, other_key)) {
					should_destroy = true;
					break;
				}
			}

			if (!should_destroy) {
				for (TimelineTrackKey *kept_key : kept_moved_keys) {
					if (!kept_key) continue;
					if (destroy_key_set.find(kept_key) != destroy_key_set.end()) continue;
					if (std::find(moved_track->keys.begin(), moved_track->keys.end(), kept_key) == moved_track->keys.end()) continue;

					if (_keys_overlap(moved_key, kept_key)) {
						should_destroy = true;
						break;
					}
				}
			}

			if (should_destroy) {
				mark_destroy(moved_key);
			}
			else {
				kept_moved_keys.push_back(moved_key);
			}
		}

		return destroy_keys;
	}

	void VTimelinePanel::_update_key_release_preview(const std::vector<TimelineTrackKey *> &p_moved_keys) {
		key_release_preview_keys.clear();
		for (TimelineTrackKey *destroy_key : _get_moved_key_overlaps(p_moved_keys)) {
			key_release_preview_keys.insert(destroy_key);
		}
	}

	void VTimelinePanel::_clear_key_release_preview() {
		key_release_preview_keys.clear();
	}

	bool VTimelinePanel::_is_key_release_previewed(const TimelineTrackKey *p_key) const {
		return p_key && key_release_preview_keys.find(p_key) != key_release_preview_keys.end();
	}

	void VTimelinePanel::_destroy_moved_key_overlaps(const std::vector<TimelineTrackKey*>& p_moved_keys) {
		std::vector<TimelineTrackKey *> destroy_keys = _get_moved_key_overlaps(p_moved_keys);

		for (TimelineTrackKey *destroy_key : destroy_keys) {
			for (CachedTrack &ct : _track_cache) {
				auto it = std::find(ct.keys.begin(), ct.keys.end(), destroy_key);
				if (it == ct.keys.end()) continue;

				ct.keys.erase(it);
				memdelete(destroy_key);
				break;
			}
		}
	}

	void VTimelinePanel::_scroll(ScrollBar* p_scroll, double p_amount) {
		_scroll_to(p_scroll, p_scroll->get_value() + p_amount);
	}

	void VTimelinePanel::_scroll_to(ScrollBar* p_scroll, double p_pos) {
		double pre_scroll = p_scroll->get_value();
		p_scroll->set_value(p_pos);
		if (!Math::is_equal_approx(pre_scroll, p_scroll->get_value())) {
			p_scroll->emit_signal("scrolling");
		}
	}

	void VTimelinePanel::_cancel_drag() {
		set_process_internal(false);
		drag_touching_deaccel = false;
		drag_touching = false;
		drag_speed = Vector2();
		drag_accum = Vector2();
		last_drag_accum = Vector2();
		drag_from = Vector2();

		if (beyond_deadzone) {
			emit_signal("scroll_ended");
			propagate_notification(NOTIFICATION_SCROLL_END);
			beyond_deadzone = false;
		}
	}

	void VTimelinePanel::_draw_header(const Point2& pos, const float width, Ref<StyleBox> header_bg, Ref<Texture2D> header_icon) {
		if (header_bg.is_valid()) {
			draw_style_box(header_bg, Rect2(pos, Size2(width, header_height)));
		}

		if (header_icon.is_valid()) {
			Size2 tex_size = header_icon->get_size();

			// 计算缩放比例，选择较小的那个以保持比例并限制在区域内
			Size2 scale_size(width / tex_size.width, header_height / tex_size.height);
			float scale = scale_size.width < scale_size.height ? scale_size.width : scale_size.height;
			Size2 scaled_tex_size = tex_size * scale;

			// 计算居中偏移
			Point2 offset = (Size2(width, header_height) - scaled_tex_size) / 2;
			draw_texture_rect(header_icon, Rect2(pos + offset, scaled_tex_size), false);
		}
	}

	void VTimelinePanel::_build_time_to_beat_map() {
		Array map;
		Array sorted_beats;
		Array keys = bpms.keys();

		for (int index = 0; index < bpms.size(); ++index) {
			double time = keys[index];
			Dictionary current_bpm = bpms.get(time, 120);
			sorted_beats.append(current_bpm.get("beat", 0.0));
		}

		double current_sec = 0.0;
		for (int64_t index = 0; index < sorted_beats.size(); ++index) {
			double time = keys[index];
			Dictionary current_bpm = bpms.get(time, 120);
			double from_beat = sorted_beats[index];
			double to_beat = INFINITY;
			int bpm = current_bpm.get("bpm", 120);

			if (bpm == 0) bpm = 120;

			// 最后一段用剩余秒数反推拍数
			double sec_left = duration - current_sec;

			if (index == sorted_beats.size() - 1) {
				to_beat = _beat_total;
				Dictionary result;
				result["from_sec"] = current_sec;
				result["to_sec"] = duration;
				result["from_beat"] = from_beat;
				result["to_beat"] = to_beat;
				result["bpm"] = bpm;
				map.append(result);
				break;
			}

			// 普通段
			to_beat = sorted_beats[index + 1];
			double beats_in_seg = to_beat - from_beat;
			double sec_in_seg = beats_in_seg * 60.0 / bpm;
			Dictionary result;
			result["from_sec"] = current_sec;
			result["to_sec"] = current_sec + sec_in_seg;
			result["from_beat"] = from_beat;
			result["to_beat"] = to_beat;
			result["bpm"] = bpm;
			map.append(result);
			current_sec += sec_in_seg;
		}
		beat_map = map;
	}

	void VTimelinePanel::_build_beat_to_time_map() {
		Array map;
		Array sorted_beats;
		Array keys = bpms.keys();

		for (int index = 0; index < bpms.size(); ++index) {
			double time = keys[index];
			Dictionary current_bpm = bpms.get(time, 120);
			sorted_beats.append(current_bpm.get("beat", 0.0));
		}

		double current_map_end_beat = _beat_total;
		double current_sec = 0.0;

		for (int64_t index = 0; index < sorted_beats.size(); ++index) {
			double from_beat = sorted_beats[index];
			double time = keys[index];
			Dictionary current_bpm = bpms.get(time, 120);
			int bpm = current_bpm.get("bpm", 120);

			// 最后一段
			if (index == sorted_beats.size() - 1) {
				Dictionary result;
				result["from_sec"] = current_sec;
				result["from_beat"] = from_beat;
				result["to_beat"] = current_map_end_beat;
				result["bpm"] = bpm;
				map.append(result);
				break;
			}

			double to_beat_in_seg = sorted_beats[index + 1];
			double beats_in_seg = to_beat_in_seg - from_beat;
			double sec_in_seg = beats_in_seg * 60.0 / bpm;
			Dictionary result;
			result["from_sec"] = current_sec;
			result["from_beat"] = from_beat;
			result["to_beat"] = to_beat_in_seg;
			result["bpm"] = bpm;
			map.append(result);
			current_sec += sec_in_seg;
		}
		time_map = map;
	}

	void VTimelinePanel::_calculate_beat_total() {
		if (bpms.is_empty()) {
			_beat_total = 0.0;
			return;
		}

		double current_sec = 0.0;
		double current_beat = 0.0;
		Array keys = bpms.keys();

		for (int64_t index = 0; index < bpms.size(); ++index) {
			double time = keys[index];
			Dictionary current_bpm = bpms.get(time, 120);
			int from_beat = current_bpm.get("beat", 0.0);
			int bpm = current_bpm.get("bpm", 120);

			if (bpm == 0) bpm = 120;

			// 用剩余秒数反推拍数
			if (index == bpms.size() - 1) {
				double sec_left = duration - current_sec;

				// 防止精度误差导致负数
				if (sec_left < 0) sec_left = 0;

				double beats_left = sec_left * bpm / 60.0;
				_beat_total = from_beat + beats_left;
				break;
			}

			// 普通段
			double next_time = keys[index + 1];
			Dictionary next_bpm = bpms.get(next_time, 120);
			int to_beat = next_bpm.get("beat", 0.0);
			int beats_seg = to_beat - from_beat;
			if (bpm == 0) bpm = 120.0;

			double sec_seg = beats_seg * 60.0 / bpm;
			current_sec += sec_seg;
			current_beat = to_beat;
		}
	}

	void VTimelinePanel::_calculate_row_total() {
		_row_total = _beat_total * beats_per_bar;
	}

	double VTimelinePanel::_time_to_y(double p_time) const {
		if (counting_unit == BEAT) {
			double row = _time_to_beat(p_time) * beats_per_bar;
			if (bar_number_direction == BAR_NUMBER_BOTTOM_UP) {
				return header_height + content_height - row * (scale / beats_per_bar) - vscroll_value;
			}
			return header_height + row * (scale / beats_per_bar) - vscroll_value;
		}
		else {
			if (bar_number_direction == BAR_NUMBER_BOTTOM_UP) {
				return header_height + content_height - p_time * scale - vscroll_value;
			}
			return header_height + p_time * scale - vscroll_value;
		}
	}

	double VTimelinePanel::_y_to_time(double p_y) const {
		double time;
		if (bar_number_direction == BAR_NUMBER_BOTTOM_UP) {
			if (counting_unit == BEAT) {
				double beat = (header_height + content_height - p_y - vscroll_value) / scale;
				time = _beat_to_time(beat);
			}
			else {
				time = (header_height + content_height - p_y - vscroll_value) / scale;
			}
		}
		else {
			if (counting_unit == BEAT) {
				double beat = (p_y - header_height + vscroll_value) / scale;
				time = _beat_to_time(beat);
			}
			else {
				time = (p_y - header_height + vscroll_value) / scale;
			}
		}
		if (time < 0) time = 0;
		if (time > duration) time = duration;
		return time;
	}

	double VTimelinePanel::_time_to_beat(double time_sec) const {
		Array seg_map = beat_map;

		for (int index = 0; index < seg_map.size(); ++index) {
			Dictionary seg = seg_map[index];
			double from_sec = seg.get("from_sec", 0.0);
			double to_sec = seg.get("to_sec", 0.0);
			double from_beat = seg.get("from_beat", 0.0);
			double to_beat = seg.get("to_beat", 0.0);

			if (time_sec >= from_sec && time_sec < to_sec) {
				double time = (time_sec - from_sec) / (to_sec - from_sec);
				return UtilityFunctions::lerpf(from_beat, to_beat, time);
			}
		}

		// 超尾
		if (seg_map.is_empty()) return 0.0;

		Dictionary last = seg_map[seg_map.size() - 1];
		double to_sec = last.get("to_sec", 0.0);
		double to_beat = last.get("to_beat", 0.0);
		int bpm = last.get("bpm", 0);
		double extra_sec = time_sec - to_sec;
		double extra_beat_cout = extra_sec * bpm / 60.0;
		return to_beat + extra_beat_cout;
	}

	double VTimelinePanel::_beat_to_time(double beat) const {
		Array seg_map = time_map;

		for (int index = 0; index < seg_map.size(); ++index) {
			Dictionary seg = seg_map[index];
			double from_sec = seg.get("from_sec", 0.0);
			double from_beat = seg.get("from_beat", 0.0);
			double to_beat = seg.get("to_beat", 0.0);
			int bpm = seg.get("bpm", 0);

			if (beat < from_beat) continue;
			if (beat >= to_beat) continue;

			double time = (beat - from_beat) / (to_beat - from_beat);
			return UtilityFunctions::lerpf(from_sec, from_sec + (to_beat - from_beat) * 60.0 / bpm, time);
		}

		// 超尾
		if (seg_map.is_empty()) return 0.0;

		Dictionary last = seg_map[seg_map.size() - 1];
		double from_sec = last.get("from_sec", 0.0);
		double from_beat = last.get("from_beat", 0.0);
		int bpm = last.get("bpm", 0);
		double extra_beats = beat - from_beat;
		double extra_sec = extra_beats * 60.0 / bpm;
		return from_sec + extra_sec;
	}

	double VTimelinePanel::_beat_to_y(double p_beat) const {
		if (bar_number_direction == BAR_NUMBER_BOTTOM_UP) {
			return header_height + content_height - p_beat * (scale / beats_per_bar) - vscroll_value;
		}
		return header_height + p_beat * (scale / beats_per_bar) - vscroll_value;
	}

	double VTimelinePanel::_y_to_beat(double p_y) const {
		double beat;
		if (bar_number_direction == BAR_NUMBER_BOTTOM_UP) {
			beat = (header_height + content_height - p_y - vscroll_value) / (scale / beats_per_bar);
		}
		else {
			beat = (p_y - header_height + vscroll_value) / (scale / beats_per_bar);
		}
		// 限制 beat 在有效范围内
		if (beat < 0) beat = 0;
		double total_beats = content_height / (scale / beats_per_bar);
		if (beat > total_beats) beat = total_beats;
		return beat;
	}

	double VTimelinePanel::_frame_to_y(int64_t p_frame) const {
		if (bar_number_direction == BAR_NUMBER_BOTTOM_UP) {
			return header_height + content_height - p_frame * scale - vscroll_value;
		}
		return header_height + p_frame * scale - vscroll_value;
	}

	int64_t VTimelinePanel::_y_to_frame(double p_y) const {
		double frame;
		if (bar_number_direction == BAR_NUMBER_BOTTOM_UP) {
			frame = (header_height + content_height - p_y - vscroll_value) / scale;
		}
		else {
			frame = (p_y - header_height + vscroll_value) / scale;
		}
		if (frame < 0) frame = 0;
		double total_frames = duration * fps;
		if (frame > total_frames) frame = total_frames;
		return static_cast<int64_t>(frame);
	}

	void VTimelinePanel::_refresh_track_key_metrics() {
		_mark_minimap_key_cache_dirty();
		for (CachedTrack& ct : _track_cache) {
			std::sort(ct.keys.begin(), ct.keys.end(), [](TimelineTrackKey* p_a, TimelineTrackKey* p_b) {
				if (p_a == p_b) return false;
				if (p_a == nullptr) return false;
				if (p_b == nullptr) return true;
				return p_a->get_time() < p_b->get_time();
				});

			ct.max_key_length = 0.0;
			ct.max_instant_key_scale = 0.4f;

			for (TimelineTrackKey* key : ct.keys) {
				if (!key) continue;

				if (key->is_instant()) {
					ct.max_instant_key_scale = MAX(ct.max_instant_key_scale, _get_instant_key_scale(key));
				}
				else {
					ct.max_key_length = MAX(ct.max_key_length, key->get_length());
				}
			}
		}
	}

	void VTimelinePanel::_get_visible_key_time_range(float p_y_margin, double& r_start, double& r_end) const {
		const double y_start = header_height - p_y_margin;
		const double y_end = get_size().y + p_y_margin;
		double a = 0.0;
		double b = 0.0;

		switch (counting_unit) {
		case FRAME:
			a = static_cast<double>(_y_to_frame(y_start));
			b = static_cast<double>(_y_to_frame(y_end));
			break;
		case BEAT:
		case TIME:
		default:
			a = _y_to_time(y_start);
			b = _y_to_time(y_end);
			break;
		}

		r_start = MIN(a, b);
		r_end = MAX(a, b);
		if (counting_unit == FRAME) {
			r_start = MAX(0.0, r_start - 1.0);
			r_end += 1.0;
		}
	}

	double VTimelinePanel::_key_to_y(const TimelineTrackKey* p_key) const {
		if (!p_key) return header_height;

		switch (counting_unit) {
		case FRAME:
			return _frame_to_y(static_cast<int64_t>(p_key->get_time()));
		case BEAT:
		case TIME:
		default:
			return _time_to_y(p_key->get_time());
		}
	}

	double VTimelinePanel::_key_end_to_y(const TimelineTrackKey* p_key) const {
		if (!p_key) return header_height;

		switch (counting_unit) {
		case FRAME:
			return _frame_to_y(static_cast<int64_t>(p_key->get_time() + p_key->get_length()));
		case BEAT:
		case TIME:
		default:
			return _time_to_y(p_key->get_time() + p_key->get_length());
		}
	}

	float VTimelinePanel::_get_instant_key_scale(const TimelineTrackKey* p_key) const {
		if (style_cache.instant_key_scale != 0.4f) {
			return style_cache.instant_key_scale;
		}
		return p_key ? p_key->get_instant_key_scale() : 0.4f;
	}

	Rect2 VTimelinePanel::_get_instant_key_rect(const CachedTrack& p_track, const TimelineTrackKey* p_key, double p_y) const {
		float key_size = p_track.width * _get_instant_key_scale(p_key);
		float pos_x = p_track.x_offset - hscroll_value + (p_track.width - key_size) * 0.5f;
		float pos_y = static_cast<float>(p_y) - key_size * 0.5f;
		return Rect2(pos_x, pos_y, key_size, key_size);
	}

	Rect2 VTimelinePanel::_get_clip_key_rect(const CachedTrack& p_track, double p_y, double p_y_end) const {
		float y = static_cast<float>(p_y);
		float y_end = static_cast<float>(p_y_end);
		if (y_end < y) {
			return Rect2(p_track.x_offset - hscroll_value, y_end, p_track.width, y - y_end);
		}
		return Rect2(p_track.x_offset - hscroll_value, y, p_track.width, y_end - y);
	}

	Ref<StyleBox> VTimelinePanel::_get_instant_key_normal_style(const TimelineTrackKey *p_key) const {
		if (p_key && p_key->get_instant_key_normal_style().is_valid()) {
			return p_key->get_instant_key_normal_style();
		}
		if (style_cache.instant_key_normal.is_valid()) {
			return style_cache.instant_key_normal;
		}
		return style_cache.instant_key_normal_fallback;
	}

	Ref<StyleBox> VTimelinePanel::_get_instant_key_selected_style(const TimelineTrackKey *p_key) const {
		if (p_key && p_key->get_instant_key_selected_style().is_valid()) {
			return p_key->get_instant_key_selected_style();
		}
		if (style_cache.instant_key_selected.is_valid()) {
			return style_cache.instant_key_selected;
		}
		return style_cache.instant_key_selected_fallback;
	}

	Ref<StyleBox> VTimelinePanel::_get_clip_key_normal_style(const TimelineTrackKey *p_key) const {
		if (p_key && p_key->get_clip_key_normal_style().is_valid()) {
			return p_key->get_clip_key_normal_style();
		}
		if (style_cache.clip_key_normal.is_valid()) {
			return style_cache.clip_key_normal;
		}
		return style_cache.clip_key_normal_fallback;
	}

	Ref<StyleBox> VTimelinePanel::_get_clip_key_selected_style(const TimelineTrackKey *p_key) const {
		if (p_key && p_key->get_clip_key_selected_style().is_valid()) {
			return p_key->get_clip_key_selected_style();
		}
		if (style_cache.clip_key_selected.is_valid()) {
			return style_cache.clip_key_selected;
		}
		return style_cache.clip_key_selected_fallback;
	}

	Ref<StyleBox> VTimelinePanel::_get_key_release_preview_style() const {
		if (style_cache.key_release_preview.is_valid()) {
			return style_cache.key_release_preview;
		}
		return style_cache.key_release_preview_fallback;
	}

	String VTimelinePanel::_format_indicator_time(double p_time) const {
		switch (counting_unit) {
		case FRAME: {
			const int safe_fps = MAX(fps, 1);
			const int64_t frame = static_cast<int64_t>(p_time * safe_fps);
			return String::num_int64(frame);
		}
		case BEAT: {
			switch (beat_format) {
			case BEAT_BAR:
			default: {
				const int subdivisions = MAX(beats_per_bar, 1);
				double beat = _time_to_beat(p_time);
				if (beat < 0.0) {
					beat = 0.0;
				}

				const double beat_epsilon = 0.0001;
				int64_t whole_beat = static_cast<int64_t>(Math::floor(beat + beat_epsilon));
				int64_t subdivision = static_cast<int64_t>(Math::floor((beat - static_cast<double>(whole_beat)) * subdivisions + beat_epsilon));
				if (subdivision >= subdivisions) {
					whole_beat += subdivision / subdivisions;
					subdivision %= subdivisions;
				}
				if (subdivision < 0) {
					subdivision = 0;
				}

				return String::num_int64(whole_beat) + "+" + String::num_int64(subdivision);
			}
			}
		}
		case TIME:
		default:
			switch (time_format) {
			case HH_MM_SS:
				return String::num_int64(static_cast<int>(p_time) / 3600) + ":" +
					String::num_int64((static_cast<int>(p_time) % 3600) / 60).pad_zeros(2) + ":" +
					String::num_int64(static_cast<int>(p_time) % 60).pad_zeros(2);
			case MM_SS_MS: {
				String text = String::num_int64(static_cast<int>(p_time) / 60) + ":" +
					String::num_int64(static_cast<int>(p_time) % 60).pad_zeros(2);
				if (show_milliseconds) {
					text += "." + String::num_int64(static_cast<int64_t>((p_time - static_cast<int>(p_time)) * 100)).pad_zeros(2);
				}
				return text;
			}
			case SEC:
			default:
				return String::num(p_time, (show_milliseconds ? 2 : 0)) + "s";
			}
		}
	}

	void VTimelinePanel::_on_resource_changed() {
		_refresh_track_key_metrics();
		queue_redraw();
		update_minimum_size();
	}

	void VTimelinePanel::_rebuild_track_cache() {
		_track_cache.clear();
		_track_cache.reserve(tracks.size());
		_mark_minimap_key_cache_dirty();

		float current_x = 0.0f;
		if (time_ruler.is_valid()) {
			current_x += time_ruler->get_width();
		}

		for (int i = 0; i < tracks.size(); i++) {
			Ref<TimelineTrack> track = tracks[i];
			if (track.is_null()) {
				// 保持索引对齐，放入空槽
				CachedTrack ct;
				_track_cache.push_back(std::move(ct));
				continue;
			}

			CachedTrack ct;
			ct.x_offset = current_x;
			ct.width = track->get_width();
			_track_cache.push_back(std::move(ct));
			current_x += track->get_width();
		}
	}

	void VTimelinePanel::_update_scroll_bar() {
		if (hscroll == nullptr || vscroll == nullptr) return;

		Size2 hmin = hscroll->get_combined_minimum_size();
		Size2 vmin = vscroll->get_combined_minimum_size();
		Size2 size = get_size();

		float content_width = _calculate_header_width();

		bool v_scroll_show = vertical_scroll_mode == SCROLL_MODE_SHOW_ALWAYS ||
			(vertical_scroll_mode == SCROLL_MODE_AUTO && content_height > size.y - header_height);
		const float visible_minimap_width = draw_minimap && minimap_width > 0 && v_scroll_show ? static_cast<float>(minimap_width) : 0.0f;
		const float vertical_side_width = v_scroll_show ? vmin.x + visible_minimap_width : 0.0f;
		const float horizontal_available_width = MAX(size.x - vertical_side_width, 0.0f);
		bool h_scroll_show = horizontal_scroll_mode == SCROLL_MODE_SHOW_ALWAYS ||
			(horizontal_scroll_mode == SCROLL_MODE_AUTO && content_width > horizontal_available_width);

		updating_scroll = true;

		if (h_scroll_show) {
			hscroll->set_anchor_and_offset(SIDE_LEFT, ANCHOR_BEGIN, 0);
			hscroll->set_anchor_and_offset(SIDE_RIGHT, ANCHOR_END, -vertical_side_width);
			hscroll->set_anchor_and_offset(SIDE_TOP, ANCHOR_END, -hmin.y);
			hscroll->set_anchor_and_offset(SIDE_BOTTOM, ANCHOR_END, 0);
			hscroll->set_max(content_width);
			hscroll->set_page(horizontal_available_width);
			hscroll->show();
		}
		else {
			hscroll->hide();
			hscroll->set_value(0);
		}

		if (v_scroll_show) {
			vscroll->set_anchor_and_offset(SIDE_LEFT, ANCHOR_END, -vmin.x);
			vscroll->set_anchor_and_offset(SIDE_RIGHT, ANCHOR_END, 0);
			vscroll->set_anchor_and_offset(SIDE_TOP, ANCHOR_BEGIN, header_height);
			vscroll->set_anchor_and_offset(SIDE_BOTTOM, ANCHOR_END, h_scroll_show ? -hmin.y : 0);
			vscroll->set_max(content_height);
			vscroll->set_page(size.y - header_height - (h_scroll_show ? hmin.y : 0));
			vscroll->show();
		}
		else {
			vscroll->hide();
			vscroll->set_value(0);
		}

		updating_scroll = false;
	}

	void VTimelinePanel::_h_scroll_changed(double p_value) {
		if (updating_scroll) return;
		hscroll_value = p_value;
		queue_redraw();
	}

	void VTimelinePanel::_v_scroll_changed(double p_value) {
		if (updating_scroll) return;
		vscroll_value = p_value;
		queue_redraw();
	}

	void VTimelinePanel::_draw_time_ruler_ticks(float p_header_width) {
		if (time_ruler.is_null()) return;

		const float ruler_width = time_ruler->get_width();
		const float major_tick_height = time_ruler->get_major_tick_height();
		const float major_tick_width = time_ruler->get_major_tick_width();
		const float minor_tick_height = time_ruler->get_minjor_tick_height();
		const float minor_tick_width = time_ruler->get_minjor_tick_width();
		const Color tick_color = time_ruler->get_tick_color();
		const float visible_start_y = header_height;
		const float visible_end_y = get_size().y;
		const float margin = 8.0f;
		const Ref<Font> font = ThemeDB::get_singleton()->get_fallback_font();

		switch (counting_unit) {
		case BEAT: {
			// 计算可见范围内的拍
			double start_beat = _y_to_beat(visible_start_y);
			double end_beat = _y_to_beat(visible_end_y);
			int start_beat_i = Math::floor(Math::min(start_beat, end_beat));
			int end_beat_i = Math::ceil(Math::max(start_beat, end_beat));

			// 绘制所有拍线
			for (int beat = start_beat_i; beat <= end_beat_i; beat++) {
				float y = _beat_to_y(beat);
				if (y < visible_start_y || y > visible_end_y) continue;

				// 判断是小节线还是拍线
				bool is_bar_line = (beat % beats_per_bar) == 0;
				float tick_height = is_bar_line ? major_tick_height : minor_tick_height;
				float tick_width = is_bar_line ? major_tick_width : minor_tick_width;

				draw_line(
					Point2(ruler_width - tick_height, y),
					Point2(ruler_width, y),
					tick_color,
					tick_width
				);

				// 小节数字
				if (is_bar_line) {
					int bar = beat / beats_per_bar;
					bool should_draw_number = true;
					if (bar_number_direction == BAR_NUMBER_BOTTOM_UP) {
						if (y > get_size().y - margin) should_draw_number = false;
					}
					else {
						if (y < visible_start_y + margin) should_draw_number = false;
					}

					if (should_draw_number) {
						String text = String::num_int64(bar);
						if (font.is_valid()) {
							draw_string(font, Point2(2.0f, y + 6.0f), text, HORIZONTAL_ALIGNMENT_LEFT, -1, 12, tick_color);
						}
					}
				}
			}
			break;
		}

		case FRAME: {
			// 计算可见范围内的帧
			int64_t start_frame = _y_to_frame(visible_start_y);
			int64_t end_frame = _y_to_frame(visible_end_y);
			// 确保 start_frame <= end_frame (考虑 Bottom Up 模式)
			if (start_frame > end_frame) {
				int64_t temp = start_frame;
				start_frame = end_frame;
				end_frame = temp;
			}

			// 根据缩放决定显示间隔
			int frame_interval = 1;
			if (scale < 10) frame_interval = fps;
			else if (scale < 30) frame_interval = fps / 2;

			// 对齐到间隔边界
			start_frame = (start_frame / frame_interval) * frame_interval;

			for (int64_t frame = start_frame; frame <= end_frame; frame += frame_interval) {
				float y = _frame_to_y(frame);
				if (y < visible_start_y || y > visible_end_y) continue;

				bool is_second = (frame % fps) == 0;
				float tick_width = is_second ? major_tick_height : minor_tick_height;

				draw_line(
					Point2(ruler_width - tick_width, y),
					Point2(ruler_width, y),
					tick_color
				);

				// 秒数标签
				if (is_second) {
					bool should_draw_label = true;
					if (bar_number_direction == BAR_NUMBER_BOTTOM_UP) {
						if (y > get_size().y - margin) should_draw_label = false;
					}
					else {
						if (y < visible_start_y + margin) should_draw_label = false;
					}

					if (should_draw_label) {
						String text = String::num_int64(frame / fps);
						if (font.is_valid()) {
							draw_string(font, Point2(2.0f, y + 6.0f), text + "s", HORIZONTAL_ALIGNMENT_LEFT, -1, 12, Color(1, 1, 1));
						}
					}
				}
			}
			break;
		}

		case TIME:
		default: {
			// 计算可见范围内的时间
			double start_time = _y_to_time(visible_start_y);
			double end_time = _y_to_time(visible_end_y);
			// 确保 start_time <= end_time (考虑 Bottom Up 模式)
			if (start_time > end_time) {
				double temp = start_time;
				start_time = end_time;
				end_time = temp;
			}

			// 根据缩放决定显示间隔
			double time_interval = 1.0;
			if (scale >= 64) time_interval = 0.1;
			else if (scale >= 32) time_interval = 0.5;
			else if (scale >= 16) time_interval = 1.0;
			else if (scale >= 8) time_interval = 5.0;
			else time_interval = 10.0;

			// 对齐到间隔边界
			start_time = Math::floor(start_time / time_interval) * time_interval;

			for (double t = start_time; t <= end_time; t += time_interval) {
				float y = _time_to_y(t);
				if (y < visible_start_y || y > visible_end_y) continue;

				bool is_major = Math::fmod(t, 1.0) < 0.001;
				float tick_width = is_major ? major_tick_height : minor_tick_height;

				draw_line(
					Point2(ruler_width - tick_width, y),
					Point2(ruler_width, y),
					tick_color
				);

				// 时间标签
				if (is_major) {
					bool should_draw_label = true;
					if (bar_number_direction == BAR_NUMBER_BOTTOM_UP) {
						if (y > get_size().y - margin) should_draw_label = false;
					}
					else {
						if (y < visible_start_y + margin) should_draw_label = false;
					}

					if (should_draw_label) {
						String text;
						switch (time_format) {
						case HH_MM_SS:
							text = String::num_int64(static_cast<int>(t) / 3600) + ":" +
								String::num_int64((static_cast<int>(t) % 3600) / 60).pad_zeros(2) + ":" +
								String::num_int64(static_cast<int>(t) % 60).pad_zeros(2);
							break;
						case MM_SS_MS:
							text = String::num_int64(static_cast<int>(t) / 60) + ":" +
								String::num_int64(static_cast<int>(t) % 60).pad_zeros(2);
							if (show_milliseconds) {
								text += "." + String::num_int64(static_cast<int64_t>((t - static_cast<int>(t)) * 100)).pad_zeros(2);
							}
							break;
						case SEC:
						default:
							text = String::num(t, (show_milliseconds ? 2 : 0)) + "s";
							break;
						}
						if (font.is_valid()) {
							draw_string(font, Point2(2.0f, y + 6.0f), text, HORIZONTAL_ALIGNMENT_LEFT, -1, 12, Color(1, 1, 1));
						}
					}
				}
			}
			break;
		}
		}
	}

	void VTimelinePanel::_draw_grid_beat(float p_header_width) {
		float start_pos = 0.0f;
		if (time_ruler.is_valid()) {
			start_pos = time_ruler->get_width();
		}

		float visible_start_y = header_height;
		float visible_end_y = get_size().y;

		// 计算可见范围内的拍
		double start_beat = _y_to_beat(visible_start_y);
		double end_beat = _y_to_beat(visible_end_y);
		int start_beat_i = Math::floor(Math::min(start_beat, end_beat));
		int end_beat_i = Math::ceil(Math::max(start_beat, end_beat));

		// 绘制所有拍线
		for (int beat = start_beat_i; beat <= end_beat_i; beat++) {
			float y = _beat_to_y(beat);
			if (y < visible_start_y || y > visible_end_y) continue;

			// 判断是小节线还是拍线
			bool is_beat_line = (beat % beats_per_bar) == 0;
			Color line_color = is_beat_line ? beat_line_color : bar_line_color;
			float line_w = is_beat_line ? beat_line_width : bar_line_width;

			draw_line(
				Point2(start_pos, y),
				Point2(p_header_width, y),
				line_color,
				line_w
			);
		}
	}

	void VTimelinePanel::_draw_grid_frame(float p_header_width) {
		float start_pos = 0.0f;
		if (time_ruler.is_valid()) {
			start_pos = time_ruler->get_width();
		}

		float visible_start_y = header_height;
		float visible_end_y = get_size().y;

		int64_t start_frame = _y_to_frame(visible_start_y);
		int64_t end_frame = _y_to_frame(visible_end_y);
		// 确保 start_frame <= end_frame (考虑 Bottom Up 模式)
		if (start_frame > end_frame) {
			int64_t temp = start_frame;
			start_frame = end_frame;
			end_frame = temp;
		}

		// 根据缩放决定绘制间隔
		int frame_interval = 1;
		if (scale < 5) frame_interval = fps;
		else if (scale < 15) frame_interval = fps / 2;

		// 绘制帧线
		for (int64_t frame = start_frame; frame <= end_frame; frame += frame_interval) {
			float y = _frame_to_y(frame);
			if (y < visible_start_y || y > visible_end_y) continue;

			bool is_second = (frame % fps) == 0;
			Color line_color = is_second ? bar_line_color : beat_line_color;
			float line_width = is_second ? bar_line_width : beat_line_width;

			draw_line(
				Point2(start_pos, y),
				Point2(p_header_width, y),
				line_color,
				line_width
			);
		}
	}

	void VTimelinePanel::_draw_grid_time(float p_header_width) {
		float start_pos = 0.0f;
		if (time_ruler.is_valid()) {
			start_pos = time_ruler->get_width();
		}

		float visible_start_y = header_height;
		float visible_end_y = get_size().y;

		double start_time = _y_to_time(visible_start_y);
		double end_time = _y_to_time(visible_end_y);
		// 确保 start_time <= end_time (考虑 Bottom Up 模式)
		if (start_time > end_time) {
			double temp = start_time;
			start_time = end_time;
			end_time = temp;
		}

		// 根据缩放决定绘制间隔
		double time_interval = 1.0;
		if (scale >= 64) time_interval = 0.1;
		else if (scale >= 32) time_interval = 0.5;
		else if (scale >= 16) time_interval = 1.0;
		else if (scale >= 8) time_interval = 5.0;
		else time_interval = 10.0;

		// 绘制时间线
		double t = Math::floor(start_time / time_interval) * time_interval;
		for (; t <= end_time; t += time_interval) {
			float y = _time_to_y(t);
			if (y < visible_start_y || y > visible_end_y) continue;

			bool is_major = Math::fmod(t, 1.0) < 0.001;
			Color line_color = is_major ? bar_line_color : beat_line_color;
			float line_width = is_major ? bar_line_width : beat_line_width;

			draw_line(
				Point2(start_pos, y),
				Point2(p_header_width, y),
				line_color,
				line_width
			);
		}
	}

	bool VTimelinePanel::_is_minimap_visible() const {
		return draw_minimap && minimap_width > 0 && vscroll != nullptr && vscroll->is_visible();
	}

	Rect2 VTimelinePanel::_get_minimap_rect() const {
		if (!_is_minimap_visible()) {
			return Rect2();
		}

		const Size2 size = get_size();
		const float hscroll_height = hscroll != nullptr && hscroll->is_visible() ? hscroll->get_combined_minimum_size().y : 0.0f;
		const float vscroll_width = vscroll != nullptr && vscroll->is_visible() ? vscroll->get_combined_minimum_size().x : 0.0f;
		const float width = static_cast<float>(minimap_width);
		const float height = MAX(size.y - header_height - hscroll_height, 0.0f);
		const float x = size.x - vscroll_width - width;
		return Rect2(Vector2(x, header_height), Vector2(width, height));
	}

	Rect2 VTimelinePanel::_get_minimap_viewport_rect() const {
		const Rect2 minimap_rect = _get_minimap_rect();
		if (minimap_rect.size.y <= 0.0f || vscroll == nullptr) {
			return Rect2();
		}

		const double scroll_max = MAX(vscroll->get_max(), 0.0);
		const double scroll_page = MAX(vscroll->get_page(), 0.0);
		if (scroll_max <= 0.0 || scroll_page >= scroll_max) {
			return minimap_rect;
		}

		const float min_height = MIN(12.0f, minimap_rect.size.y);
		const float viewport_height = CLAMP(static_cast<float>(scroll_page / scroll_max) * minimap_rect.size.y, min_height, minimap_rect.size.y);
		const double scroll_range = MAX(scroll_max - scroll_page, 0.0);
		const float minimap_range = MAX(minimap_rect.size.y - viewport_height, 0.0f);
		const float offset = scroll_range > 0.0 ? static_cast<float>(CLAMP(vscroll_value, 0.0, scroll_range) / scroll_range) * minimap_range : 0.0f;
		return Rect2(Vector2(minimap_rect.position.x, minimap_rect.position.y + offset), Vector2(minimap_rect.size.x, viewport_height));
	}

	double VTimelinePanel::_indicator_time_to_content_y(double p_time) const {
		double y = header_height;
		switch (counting_unit) {
		case FRAME: {
			const int safe_fps = MAX(fps, 1);
			y = _frame_to_y(static_cast<int64_t>(p_time * safe_fps));
			break;
		}
		case BEAT:
		case TIME:
		default:
			y = _time_to_y(p_time);
			break;
		}
		return y - header_height + vscroll_value;
	}

	double VTimelinePanel::_content_y_to_minimap_y(const Rect2 &p_rect, double p_content_y) const {
		const double content = MAX(static_cast<double>(content_height), 1.0);
		const double ratio = CLAMP(p_content_y / content, 0.0, 1.0);
		return p_rect.position.y + ratio * p_rect.size.y;
	}

	void VTimelinePanel::_mark_minimap_key_cache_dirty() {
		minimap_key_cache_dirty = true;
	}

	void VTimelinePanel::_rebuild_minimap_key_cache(int p_height) {
		minimap_key_cache_height = MAX(p_height, 0);
		minimap_key_cache.clear();
		minimap_key_cache.resize(_track_cache.size());

		if (minimap_key_cache_height <= 0) {
			minimap_key_cache_dirty = false;
			return;
		}

		for (MinimapTrackCache &cache : minimap_key_cache) {
			cache.key_rows.assign(minimap_key_cache_height, 0);
			cache.selected_key_rows.assign(minimap_key_cache_height, 0);
		}

		const double content = MAX(static_cast<double>(content_height), 1.0);
		auto mark_key_rows = [&](MinimapTrackCache &cache, double p_start_content_y, double p_end_content_y, bool p_selected, bool p_instant) {
			const double min_content_y = CLAMP(MIN(p_start_content_y, p_end_content_y), 0.0, content);
			const double max_content_y = CLAMP(MAX(p_start_content_y, p_end_content_y), 0.0, content);
			int start_row = CLAMP(static_cast<int>(Math::floor(min_content_y / content * minimap_key_cache_height)), 0, minimap_key_cache_height - 1);
			int end_row = CLAMP(static_cast<int>(Math::floor(max_content_y / content * minimap_key_cache_height)), 0, minimap_key_cache_height - 1);
			if (p_instant) {
				end_row = MIN(start_row + 1, minimap_key_cache_height - 1);
			}

			for (int row = start_row; row <= end_row; row++) {
				cache.key_rows[row] = 1;
				if (p_selected) {
					cache.selected_key_rows[row] = 1;
				}
			}
		};

		for (int i = 0; i < static_cast<int>(_track_cache.size()); i++) {
			const CachedTrack &ct = _track_cache[i];
			if (ct.width <= 0.0f) continue;

			MinimapTrackCache &cache = minimap_key_cache[i];
			for (TimelineTrackKey *key : ct.keys) {
				if (!key || key->is_disabled()) continue;

				const double key_start_content_y = _key_to_y(key) - header_height + vscroll_value;
				const double key_end_content_y = key->is_instant() ? key_start_content_y : _key_end_to_y(key) - header_height + vscroll_value;
				mark_key_rows(cache, key_start_content_y, key_end_content_y, key->is_selected(), key->is_instant());
			}
		}

		minimap_key_cache_dirty = false;
	}

	void VTimelinePanel::_draw_minimap() {
		const Rect2 minimap_rect = _get_minimap_rect();
		if (minimap_rect.size.x <= 0.0f || minimap_rect.size.y <= 0.0f) {
			return;
		}

		const int cache_height = MAX(static_cast<int>(Math::ceil(minimap_rect.size.y)), 1);
		if (minimap_key_cache_dirty || minimap_key_cache_height != cache_height || minimap_key_cache.size() != _track_cache.size()) {
			_rebuild_minimap_key_cache(cache_height);
		}

		draw_rect(minimap_rect, Color(0.0f, 0.0f, 0.0f, 0.28f));

		float total_track_width = 0.0f;
		for (const CachedTrack &ct : _track_cache) {
			if (ct.width > 0.0f) {
				total_track_width += ct.width;
			}
		}

		if (total_track_width > 0.0f) {
			float track_offset = 0.0f;
			for (int i = 0; i < static_cast<int>(_track_cache.size()); i++) {
				const CachedTrack &ct = _track_cache[i];
				if (ct.width <= 0.0f) continue;

				const float track_x = minimap_rect.position.x + (track_offset / total_track_width) * minimap_rect.size.x;
				const float track_width = MAX((ct.width / total_track_width) * minimap_rect.size.x, 1.0f);
				draw_line(Point2(track_x, minimap_rect.position.y), Point2(track_x, minimap_rect.position.y + minimap_rect.size.y), Color(1.0f, 1.0f, 1.0f, 0.08f));

				const MinimapTrackCache &cache = minimap_key_cache[i];
				const float row_height = minimap_rect.size.y / static_cast<float>(cache_height);
				const float key_x = track_x + 1.0f;
				const float key_width = MAX(track_width - 2.0f, 1.0f);
				auto draw_cached_rows = [&](const std::vector<uint8_t> &p_rows, const Color &p_color) {
					int run_start = -1;
					for (int row = 0; row <= cache_height; row++) {
						const bool active = row < cache_height && p_rows[row] != 0;
						if (active && run_start < 0) {
							run_start = row;
						}
						else if (!active && run_start >= 0) {
							const float y = minimap_rect.position.y + row_height * run_start;
							const float height = MAX(row_height * (row - run_start), 1.0f);
							draw_rect(Rect2(Vector2(key_x, y), Vector2(key_width, height)), p_color);
							run_start = -1;
						}
					}
				};
				draw_cached_rows(cache.key_rows, Color(1.0f, 1.0f, 1.0f, 0.42f));
				draw_cached_rows(cache.selected_key_rows, Color(1.0f, 1.0f, 1.0f, 0.82f));

				track_offset += ct.width;
			}
		}

		for (int i = 0; i < markers.size(); i++) {
			Ref<TimelineMarker> marker = markers[i];
			if (marker.is_null()) continue;

			Color marker_color = marker->get_line_color();
			marker_color.a = MAX(marker_color.a, 0.75f);
			const float y = static_cast<float>(_content_y_to_minimap_y(minimap_rect, _indicator_time_to_content_y(marker->get_time())));
			draw_line(Point2(minimap_rect.position.x, y), Point2(minimap_rect.position.x + minimap_rect.size.x, y), marker_color, 1.0f);
		}

		if (playhead.is_valid()) {
			Color playhead_color = playhead->get_line_color();
			playhead_color.a = MAX(playhead_color.a, 0.9f);
			const float y = static_cast<float>(_content_y_to_minimap_y(minimap_rect, _indicator_time_to_content_y(current_time)));
			draw_line(Point2(minimap_rect.position.x, y), Point2(minimap_rect.position.x + minimap_rect.size.x, y), playhead_color, 2.0f);
		}

		const Rect2 viewport_rect = _get_minimap_viewport_rect();
		if (viewport_rect.size.y > 0.0f) {
			draw_rect(viewport_rect, Color(1.0f, 1.0f, 1.0f, minimap_dragging ? 0.23f : 0.12f));
			draw_rect(viewport_rect, Color(1.0f, 1.0f, 1.0f, 0.35f), false, 1.0f);
		}
		draw_rect(minimap_rect, Color(1.0f, 1.0f, 1.0f, 0.16f), false, 1.0f);
	}

	bool VTimelinePanel::_begin_minimap_drag(const Vector2 &p_position) {
		const Rect2 minimap_rect = _get_minimap_rect();
		if (!minimap_rect.has_point(p_position)) {
			return false;
		}

		minimap_dragging = true;
		const Rect2 viewport_rect = _get_minimap_viewport_rect();
		minimap_dragging_viewport = viewport_rect.has_point(p_position);
		minimap_drag_scroll_origin = vscroll != nullptr ? vscroll->get_value() : 0.0;
		minimap_drag_y_origin = p_position.y;
		if (!minimap_dragging_viewport) {
			_scroll_minimap_to_position(p_position.y);
		}
		queue_redraw();
		return true;
	}

	void VTimelinePanel::_update_minimap_drag(const Vector2 &p_position) {
		if (!minimap_dragging || vscroll == nullptr) {
			return;
		}

		if (!minimap_dragging_viewport) {
			_scroll_minimap_to_position(p_position.y);
			queue_redraw();
			return;
		}

		const Rect2 minimap_rect = _get_minimap_rect();
		const Rect2 viewport_rect = _get_minimap_viewport_rect();
		const double scroll_range = MAX(vscroll->get_max() - vscroll->get_page(), 0.0);
		const double minimap_range = MAX(static_cast<double>(minimap_rect.size.y - viewport_rect.size.y), 1.0);
		const double scroll_delta = (p_position.y - minimap_drag_y_origin) / minimap_range * scroll_range;
		_scroll_to(vscroll, minimap_drag_scroll_origin + scroll_delta);
		queue_redraw();
	}

	void VTimelinePanel::_finish_minimap_drag() {
		if (!minimap_dragging) {
			return;
		}

		minimap_dragging = false;
		minimap_dragging_viewport = false;
		queue_redraw();
	}

	void VTimelinePanel::_scroll_minimap_to_position(float p_y) {
		if (vscroll == nullptr) {
			return;
		}

		const Rect2 minimap_rect = _get_minimap_rect();
		const Rect2 viewport_rect = _get_minimap_viewport_rect();
		const double scroll_range = MAX(vscroll->get_max() - vscroll->get_page(), 0.0);
		const double minimap_range = MAX(static_cast<double>(minimap_rect.size.y - viewport_rect.size.y), 1.0);
		const double target_y = CLAMP(static_cast<double>(p_y - minimap_rect.position.y - viewport_rect.size.y * 0.5f), 0.0, minimap_range);
		const double target_scroll = target_y / minimap_range * scroll_range;
		_scroll_to(vscroll, target_scroll);
	}

	float VTimelinePanel::_calculate_header_width() const {
		float width = 0.0f;

		if (time_ruler.is_valid()) {
			width += time_ruler->get_width();
		}

		for (int64_t i = 0; i < tracks.size(); i++) {
			Ref<TimelineTrack> track = tracks[i];
			if (track.is_null()) continue;
			width += track->get_width();
		}

		return width;
	}

	float VTimelinePanel::_calculate_grid_height() const {
		switch (counting_unit) {
		case BEAT: {
			double total_rows = _row_total;
			if (total_rows < 1) total_rows = 1;
			return total_rows * (scale / beats_per_bar);
		}
		case FRAME: {
			double total_frames = duration * fps;
			if (total_frames < 1) total_frames = 1;
			return total_frames * scale;
		}
		case TIME:
		default: {
			float time_height = duration * scale;
			if (time_height < scale) time_height = scale;
			return time_height;
		}
		}
	}

	Vector2 VTimelinePanel::_get_minimum_size() const {
		float min_width = 0.0f;
		float min_height = header_height;

		if (horizontal_scroll_mode == SCROLL_MODE_DISABLED) {
			min_width = _calculate_header_width();
			bool v_scroll_show = vertical_scroll_mode == SCROLL_MODE_SHOW_ALWAYS ||
				vertical_scroll_mode == SCROLL_MODE_RESERVE ||
				(vertical_scroll_mode == SCROLL_MODE_AUTO && content_height > get_size().y - header_height);
			if (v_scroll_show && vscroll && vscroll->get_parent()) {
				min_width += vscroll->get_minimum_size().x;
				if (draw_minimap && minimap_width > 0) {
					min_width += minimap_width;
				}
			}
		}

		if (vertical_scroll_mode == SCROLL_MODE_DISABLED) {
			min_height = header_height + content_height;
			bool h_scroll_show = horizontal_scroll_mode == SCROLL_MODE_SHOW_ALWAYS ||
				horizontal_scroll_mode == SCROLL_MODE_RESERVE ||
				(horizontal_scroll_mode == SCROLL_MODE_AUTO && _calculate_header_width() > get_size().x);
			if (h_scroll_show && hscroll && hscroll->get_parent()) {
				min_height += hscroll->get_minimum_size().y;
			}
		}

		return Size2(min_width, min_height);
	}

	void VTimelinePanel::_gui_input(const Ref<InputEvent>& p_gui_input) {
		ERR_FAIL_COND(p_gui_input.is_null());

		double prev_v_scroll = vscroll->get_value();
		double prev_h_scroll = hscroll->get_value();
		bool h_scroll_enabled = horizontal_scroll_mode != SCROLL_MODE_DISABLED;
		bool v_scroll_enabled = vertical_scroll_mode != SCROLL_MODE_DISABLED;

		Ref<InputEventMouseButton> mb = p_gui_input;

		if (mb.is_valid()) {
			if (mb->is_pressed()) {
				bool scroll_value_modified = false;

				bool v_scroll_hidden = !vscroll->is_visible() && vertical_scroll_mode != SCROLL_MODE_SHOW_NEVER;
				if (mb->get_button_index() == MouseButton::MOUSE_BUTTON_WHEEL_UP) {
					// 默认情况下，垂直方向优先。这是一个例外
					if ((h_scroll_enabled && mb->is_shift_pressed()) || v_scroll_hidden) {
						_scroll(hscroll, -hscroll->get_page() / 8 * mb->get_factor());
						scroll_value_modified = true;
					}
					else if (v_scroll_enabled) {
						_scroll(vscroll, -vscroll->get_page() / 8 * mb->get_factor());
						scroll_value_modified = true;
					}
				}
				if (mb->get_button_index() == MouseButton::MOUSE_BUTTON_WHEEL_DOWN) {
					if ((h_scroll_enabled && mb->is_shift_pressed()) || v_scroll_hidden) {
						_scroll(hscroll, hscroll->get_page() / 8 * mb->get_factor());
						scroll_value_modified = true;
					}
					else if (v_scroll_enabled) {
						_scroll(vscroll, vscroll->get_page() / 8 * mb->get_factor());
						scroll_value_modified = true;
					}
				}

				bool h_scroll_hidden = !hscroll->is_visible() && horizontal_scroll_mode != SCROLL_MODE_SHOW_NEVER;
				if (mb->get_button_index() == MouseButton::MOUSE_BUTTON_WHEEL_LEFT) {
					// 默认情况下，水平方向优先。这是一个例外
					if ((v_scroll_enabled && mb->is_shift_pressed()) || h_scroll_hidden) {
						_scroll(vscroll, -vscroll->get_page() / 8 * mb->get_factor());
						scroll_value_modified = true;
					}
					else if (h_scroll_enabled) {
						_scroll(hscroll, -hscroll->get_page() / 8 * mb->get_factor());
						scroll_value_modified = true;
					}
				}
				if (mb->get_button_index() == MouseButton::MOUSE_BUTTON_WHEEL_RIGHT) {
					if ((v_scroll_enabled && mb->is_shift_pressed()) || h_scroll_hidden) {
						_scroll(vscroll, vscroll->get_page() / 8 * mb->get_factor());
						scroll_value_modified = true;
					}
					else if (h_scroll_enabled) {
						_scroll(hscroll, hscroll->get_page() / 8 * mb->get_factor());
						scroll_value_modified = true;
					}
				}

				if (scroll_value_modified && (vscroll->get_value() != prev_v_scroll || hscroll->get_value() != prev_h_scroll)) {
					// 如果滚动发生变化则接受事件
					accept_event();
					return;
				}
			}

			if (mb->get_button_index() == MouseButton::MOUSE_BUTTON_RIGHT) {
				const Vector2 mouse_position = mb->get_position();
				const bool is_time_ruler_position = time_ruler.is_valid() &&
					mouse_position.x >= 0.0f && mouse_position.x <= time_ruler->get_width();

				if (!mb->is_pressed() && right_selecting) {
					_finish_right_mouse_selection();
					accept_event();
					return;
				}

				if (mb->is_pressed() && is_time_ruler_position) {
					emit_signal("time_ruler_right_clicked", get_time_from_position(mouse_position.y), mouse_position);
					accept_event();
					return;
				}

				if (mb->is_pressed() && allow_right_mouse_selection &&
					mouse_position.y > header_height && !_get_minimap_rect().has_point(mouse_position)) {
					right_selecting = true;
					right_select_start = mouse_position;
					right_select_end = right_select_start;
					set_process_internal(true);
					queue_redraw();
					accept_event();
					return;
				}
			}

			if (mb->get_button_index() == MouseButton::MOUSE_BUTTON_LEFT) {
				if (!mb->is_pressed() && minimap_dragging) {
					_finish_minimap_drag();
					accept_event();
					return;
				}

				if (!mb->is_pressed() && clip_key_edge_dragging) {
					_finish_clip_key_edge_drag();
					_update_clip_key_edge_cursor(mb->get_position());
					accept_event();
					return;
				}

				if (!mb->is_pressed() && key_dragging) {
					_finish_key_drag();
					accept_event();
					return;
				}

				const Vector2 mouse_position = mb->get_position();
				const bool is_time_ruler_position = time_ruler.is_valid() &&
					mouse_position.x >= 0.0f && mouse_position.x <= time_ruler->get_width();
				if (mb->is_pressed() && _begin_minimap_drag(mouse_position)) {
					accept_event();
					return;
				}

				if (!mb->is_pressed() && playhead_dragging) {
					playhead_dragging = false;
					accept_event();
					return;
				}

				if (mb->is_pressed() && mouse_position.y >= 0.0f && mouse_position.y <= header_height && !is_time_ruler_position) {
					const int track_index = _get_track_header_index_at_x(mouse_position.x);
					if (track_index >= 0) {
						_select_track_keys(track_index);
						accept_event();
						return;
					}
				}

				if (mb->is_pressed() && is_time_ruler_position) {
					if (drag_touching) {
						_cancel_drag();
					}
					if (selecting || select_pending) {
						selecting = false;
						select_pending = false;
					}
					playhead_dragging = true;
					set_current_time(_get_playhead_drag_time(mouse_position.y));
					accept_event();
					return;
				}

				if (mb->is_pressed() && mb->get_position().y > header_height) {
					int edge_track_index = -1;
					TimelineTrackKey *edge_key = nullptr;
					ClipKeyEditEdge edge = CLIP_KEY_EDIT_EDGE_NONE;
					if (_find_clip_key_edge_at_position(mb->get_position(), edge_track_index, edge_key, edge)) {
						_begin_clip_key_edge_drag(edge_key, edge);
						accept_event();
						return;
					}

					int hit_track_index = -1;
					TimelineTrackKey* hit_key = nullptr;
					if (_find_selected_key_at_position(mb->get_position(), hit_track_index, hit_key)) {
						_begin_key_drag(hit_track_index, hit_key, mb->get_position());
						accept_event();
						return;
					}
				}
			}

			bool is_touchscreen_available = DisplayServer::get_singleton()->is_touchscreen_available();
			if (mb->get_button_index() == MouseButton::MOUSE_BUTTON_LEFT) {
				if (!is_touchscreen_available) {
					if (mb->is_pressed() && time_ruler.is_valid() && mb->get_position().x <= time_ruler->get_width()) {
						// 在时间尺范围内点击，更新 current_time
						playhead_dragging = true;
						queue_redraw();
					}
					else if (mb->is_pressed() && mb->get_position().y > header_height) {
						// 只有桌面端左键才直接触发框选
						selecting = true;
						select_start = mb->get_position();
						select_end = select_start;
						set_process_internal(true);
						queue_redraw();
					}
					else {
						if (playhead_dragging) {
							playhead_dragging = false;
						}
						if (selecting) {
							selecting = false;
							_collect_selected_keys();
							queue_redraw();
							_stop_internal_process_if_idle();
						}
					}
					accept_event();
					return;
				}

				// 触摸屏逻辑
				if (mb->is_pressed() && mb->get_position().y > header_height) {
					if (drag_touching) {
						_cancel_drag();
					}

					select_pending = true;
					select_timer = 0.0f;
					select_start = mb->get_position();
					select_end = select_start;

					drag_speed = Vector2();
					drag_accum = Vector2();
					last_drag_accum = Vector2();
					drag_from = Vector2(prev_h_scroll, prev_v_scroll);
					drag_touching = true;
					drag_touching_deaccel = false;
					beyond_deadzone = false;
					time_since_motion = 0;
					set_process_internal(true);
					time_since_motion = 0;
				}
				else {
					if (playhead_dragging) {
						playhead_dragging = false;
					}
					else if (selecting) {
						selecting = false;
						_collect_selected_keys();
						queue_redraw();
						_stop_internal_process_if_idle();
					}
					else if (select_pending) {
						select_pending = false;
						if (drag_touching) {
							_cancel_drag();
						}
						_stop_internal_process_if_idle();
					}
				}
				accept_event();
				return;
			}

			if (!is_touchscreen_available) {
				return;
			}

			if (mb->get_button_index() != MouseButton::MOUSE_BUTTON_LEFT) {
				return;
			}

			if (mb->is_pressed()) {
				if (mb->is_pressed() && time_ruler.is_valid() && mb->get_position().x <= time_ruler->get_width()) {
					// 在时间尺范围内点击，更新 current_time
					playhead_dragging = true;
					queue_redraw();
				}

				if (drag_touching) {
					_cancel_drag();
				}

				// 先不决定是滚动还是框选
				select_pending = true;
				select_timer = 0.0f;
				select_start = mb->get_position();
				select_end = select_start;

				// 同时预备好滚动所需的初始值
				drag_speed = Vector2();
				drag_accum = Vector2();
				last_drag_accum = Vector2();
				drag_from = Vector2(prev_h_scroll, prev_v_scroll);
				drag_touching = true;
				drag_touching_deaccel = false;
				beyond_deadzone = false;
				time_since_motion = 0;
				set_process_internal(true);
				time_since_motion = 0;
			}
			else {
				if (drag_touching) {
					if (drag_speed == Vector2()) {
						_cancel_drag();
					}
					else {
						drag_touching_deaccel = true;
					}
				}
			}
			return;
		}

		Ref<InputEventMouseMotion> mm = p_gui_input;

		if (mm.is_valid()) {
			if (right_selecting) {
				right_select_end = mm->get_position();
				queue_redraw();
				accept_event();
				return;
			}

			if (minimap_dragging) {
				_update_minimap_drag(mm->get_position());
				accept_event();
				return;
			}

			if (clip_key_edge_dragging) {
				_update_clip_key_edge_drag(mm->get_position());
				accept_event();
				return;
			}

			if (key_dragging) {
				_update_key_drag(mm->get_position());
				accept_event();
				return;
			}

			if (playhead_dragging) {
				double new_time = _get_playhead_drag_time(mm->get_position().y);
				set_current_time(new_time);
				accept_event();
				return;
			}

			if (selecting) {
				select_end = mm->get_position();
				queue_redraw();
				accept_event();
				return;
			}

			if (select_pending) {
				// 如果还没触发长按，但已经超出死区则判定为滚动，取消框选 pending
				if (beyond_deadzone) {
					select_pending = false;
				}
				else {
					select_end = mm->get_position();
					queue_redraw();
				}
			}

			if (!playhead_dragging && !selecting && !select_pending && !drag_touching) {
				_update_clip_key_edge_cursor(mm->get_position());
			}

			if (drag_touching && !drag_touching_deaccel) {
				Vector2 motion = mm->get_relative();
				drag_accum -= motion;

				if (beyond_deadzone || (h_scroll_enabled && Math::abs(drag_accum.x) > deadzone) || (v_scroll_enabled && Math::abs(drag_accum.y) > deadzone)) {
					if (!beyond_deadzone) {
						propagate_notification(NOTIFICATION_SCROLL_BEGIN);
						emit_signal("scroll_started");

						beyond_deadzone = true;
						// 在此重置 drag_accum 可确保达到死区后滚动平滑
						drag_accum = -motion;
					}
					Vector2 diff = drag_from + drag_accum;
					if (h_scroll_enabled) {
						_scroll_to(hscroll, diff.x);
					}
					else {
						drag_accum.x = 0;
					}
					if (v_scroll_enabled) {
						_scroll_to(vscroll, diff.y);
					}
					else {
						drag_accum.y = 0;
					}
					time_since_motion = 0;
				}
			}

			if (vscroll->get_value() != prev_v_scroll || hscroll->get_value() != prev_h_scroll) {
				// 如果滚动发生变化则接受事件
				accept_event();
			}
			return;
		}

		Ref<InputEventPanGesture> pan_gesture = p_gui_input;
		if (pan_gesture.is_valid()) {
			if (h_scroll_enabled) {
				_scroll(hscroll, hscroll->get_page() * pan_gesture->get_delta().x / 8);
			}
			if (v_scroll_enabled) {
				_scroll(vscroll, vscroll->get_page() * pan_gesture->get_delta().y / 8);
			}

			if (vscroll->get_value() != prev_v_scroll || hscroll->get_value() != prev_h_scroll) {
				// 如果滚动发生变化则接受事件
				accept_event();
			}
			return;
		}
	}

	String VTimelinePanel::_get_tooltip(const Vector2& p_at_position) const {
		if (p_at_position.y > header_height) return String();

		Vector2 start_pos;
		if (time_ruler.is_valid()) {
			float width = time_ruler->get_width();
			Rect2 area = Rect2(Vector2(0.0, 0.0), Vector2(width, header_height));
			if (area.has_point(p_at_position)) return time_ruler->get_tooltip_text();
			start_pos.x += width;
		}

		for (int i = 0; i < tracks.size(); i++) {
			Ref<TimelineTrack> track = tracks[i];
			if (track.is_null()) continue;

			float width = track->get_width();
			Rect2 area = Rect2(start_pos, Vector2(width, header_height));
			if (area.has_point(p_at_position)) return track->get_tooltip_text();
			start_pos.x += width;
		}

		return String();
	}

	TimelineTrackKey *VTimelinePanel::create_key(int p_track_index, double p_time, double p_length, bool p_snap) {
		ERR_FAIL_INDEX_V(p_track_index, static_cast<int>(_track_cache.size()), nullptr);

		auto &ct = _track_cache[p_track_index];
		auto &keys = ct.keys;
		const double target_time = p_snap ? _snap_key_time(p_time) : p_time;
		TimelineTrackKey *key = memnew(TimelineTrackKey);
		key->set_time(target_time);
		key->set_length(p_length);

		for (TimelineTrackKey *other_key : keys) {
			if (!other_key || other_key->is_disabled()) continue;
			if (_keys_overlap(key, other_key)) {
				memdelete(key);
				return nullptr;
			}
		}

		key->connect("changed", callable_mp(this, &VTimelinePanel::_on_resource_changed));

		// 有序插入
		auto it = std::lower_bound(keys.begin(), keys.end(), target_time,
			[](TimelineTrackKey *k, double time) { return k->get_time() < time; });
		keys.insert(it, key);

		if (p_length > ct.max_key_length) {
			ct.max_key_length = p_length;
		}
		if (p_length <= 0.0) {
			ct.max_instant_key_scale = MAX(ct.max_instant_key_scale, _get_instant_key_scale(key));
		}

		_mark_minimap_key_cache_dirty();
		queue_redraw();
		return key;
	}

	void VTimelinePanel::remove_key(int p_track_index, int p_key_index) {
		ERR_FAIL_INDEX(p_track_index, static_cast<int>(_track_cache.size()));
		_clear_key_release_preview();
		auto& ct = _track_cache[p_track_index];
		auto& keys = ct.keys;
		ERR_FAIL_INDEX(p_key_index, static_cast<int>(keys.size()));

		if (keys[p_key_index]) {
			if (keys[p_key_index]->get_length() >= ct.max_key_length) {
				ct.max_key_length = 0.0;
			}
			memdelete(keys[p_key_index]);
		}
		keys.erase(keys.begin() + p_key_index);

		if (ct.max_key_length == 0.0 && !keys.empty()) {
			for (TimelineTrackKey* k : keys) {
				if (k && k->get_length() > ct.max_key_length) {
					ct.max_key_length = k->get_length();
				}
			}
		}
		_refresh_track_key_metrics();
		queue_redraw();
	}

	void VTimelinePanel::clear_track_keys(int p_track_index) {
		ERR_FAIL_INDEX(p_track_index, static_cast<int>(_track_cache.size()));
		_clear_key_release_preview();
		auto& keys = _track_cache[p_track_index].keys;
		for (TimelineTrackKey* key : keys) {
			if (key) {
				memdelete(key);
			}
		}
		keys.clear();
		_track_cache[p_track_index].max_key_length = 0.0;
		_track_cache[p_track_index].max_instant_key_scale = 0.4f;
		_mark_minimap_key_cache_dirty();
		queue_redraw();
	}

	void VTimelinePanel::clear_all_keys() {
		_clear_key_release_preview();
		for (auto& ct : _track_cache) {
			for (TimelineTrackKey* key : ct.keys) {
				if (key) {
					memdelete(key);
				}
			}
			ct.keys.clear();
			ct.max_key_length = 0.0;
			ct.max_instant_key_scale = 0.4f;
		}
		_mark_minimap_key_cache_dirty();
		queue_redraw();
	}

	int VTimelinePanel::get_key_count(int p_track_index) const {
		ERR_FAIL_INDEX_V(p_track_index, static_cast<int>(_track_cache.size()), 0);
		return static_cast<int>(_track_cache[p_track_index].keys.size());
	}

	TimelineTrackKey* VTimelinePanel::get_key(int p_track_index, int p_key_index) const {
		ERR_FAIL_INDEX_V(p_track_index, static_cast<int>(_track_cache.size()), nullptr);
		const auto& keys = _track_cache[p_track_index].keys;
		ERR_FAIL_INDEX_V(p_key_index, static_cast<int>(keys.size()), nullptr);
		return keys[p_key_index];
	}

	TypedArray<TimelineTrackKey> VTimelinePanel::find_keys(int p_track_index, double p_start_time, double p_end_time) const {
		ERR_FAIL_INDEX_V(p_track_index, static_cast<int>(_track_cache.size()), TypedArray<TimelineTrackKey>());
		const auto& ct = _track_cache[p_track_index];
		const auto& keys = ct.keys;
		TypedArray<TimelineTrackKey> result;

		if (p_start_time > p_end_time || keys.empty()) {
			return result;
		}

		// 往前回溯 max_key_length，防止漏掉长片段
		double search_start = p_start_time - ct.max_key_length;
		auto it = std::lower_bound(keys.begin(), keys.end(), search_start,
			[](TimelineTrackKey* k, double time) { return k->get_time() < time; });

		for (; it != keys.end(); ++it) {
			TimelineTrackKey* key = *it;
			if (!key || key->is_disabled()) continue;

			double key_start = key->get_time();
			if (key_start > p_end_time) {
				break;
			}

			double key_end = key_start + key->get_length();
			if (key_end >= p_start_time) {
				result.append(key);
			}
		}

		return result;
	}

	double VTimelinePanel::get_time_from_position(const double p_position) const {
		return _y_to_time(p_position);
	}

	double VTimelinePanel::get_frame_from_position(const double p_position) const {
		return _y_to_frame(p_position);
	}

	double VTimelinePanel::get_beat_from_position(const double p_position) const {
		return _y_to_beat(p_position);
	}

	double VTimelinePanel::get_position_from_time(double p_time) const {
		return _time_to_y(p_time);
	}

	double VTimelinePanel::get_position_from_frame(int64_t p_frame) const {
		return _frame_to_y(p_frame);
	}

	double VTimelinePanel::get_position_from_beat(double p_beat) const {
		return _beat_to_y(p_beat);
	}

	void VTimelinePanel::set_background_color(const Color& p_background_color) {
		background_color = p_background_color;
		queue_redraw();
	}

	Color VTimelinePanel::get_background_color() const {
		return background_color;
	}

	void VTimelinePanel::set_separator_color(const Color& p_separator_color) {
		separator_color = p_separator_color;
		queue_redraw();
	}

	Color VTimelinePanel::get_separator_color() const {
		return separator_color;
	}

	void VTimelinePanel::set_separator_width(const float p_width) {
		separator_width = p_width;
		queue_redraw();
	}

	float VTimelinePanel::get_separator_width() const {
		return separator_width;
	}

	void VTimelinePanel::set_header_height(const float p_height) {
		header_height = p_height;
		queue_redraw();
		update_minimum_size();
	}

	float VTimelinePanel::get_header_height() const {
		return header_height;
	}

	void VTimelinePanel::set_duration(const double p_duration) {
		duration = p_duration;
		_calculate_beat_total();
		_calculate_row_total();
		_build_time_to_beat_map();
		_build_beat_to_time_map();
		queue_redraw();
		update_minimum_size();
	}

	double VTimelinePanel::get_duration() const {
		return duration;
	}

	void VTimelinePanel::set_current_time(const double p_current_time) {
		current_time = p_current_time;
		queue_redraw();
	}

	double VTimelinePanel::get_current_time() const {
		return current_time;
	}

	void VTimelinePanel::set_scale(const float p_scale) {
		scale = p_scale;
		queue_redraw();
		update_minimum_size();
	}

	float VTimelinePanel::get_scale() const {
		return scale;
	}

	void VTimelinePanel::set_counting_unit(CountingUnit p_unit) {
		counting_unit = p_unit;
		notify_property_list_changed();
		queue_redraw();
		update_minimum_size();
	}

	VTimelinePanel::CountingUnit VTimelinePanel::get_counting_unit() const {
		return counting_unit;
	}

	void VTimelinePanel::set_time_format(TimeFormat p_time_format) {
		time_format = p_time_format;
		queue_redraw();
	}

	VTimelinePanel::TimeFormat VTimelinePanel::get_time_format() const {
		return time_format;
	}

	void VTimelinePanel::set_show_milliseconds(const bool p_show_milliseconds) {
		show_milliseconds = p_show_milliseconds;
		queue_redraw();
	}

	bool VTimelinePanel::get_show_milliseconds() const {
		return show_milliseconds;
	}

	void VTimelinePanel::set_fps(const int p_fps) {
		fps = p_fps;
		queue_redraw();
		update_minimum_size();
	}

	int VTimelinePanel::get_fps() const {
		return fps;
	}

	void VTimelinePanel::set_show_subdivision(const bool p_show_subdivision) {
		show_subdivision = p_show_subdivision;
		queue_redraw();
	}

	bool VTimelinePanel::get_show_subdivision() const {
		return show_subdivision;
	}

	void VTimelinePanel::set_bpms(const Dictionary& p_bpms) {
		bpms = p_bpms;
		_calculate_beat_total();
		_calculate_row_total();
		_build_time_to_beat_map();
		_build_beat_to_time_map();
		queue_redraw();
		update_minimum_size();
	}

	Dictionary VTimelinePanel::get_bpms() const {
		return bpms;
	}

	void VTimelinePanel::set_beat_per_bar(const int p_beats_per_bar) {
		beats_per_bar = p_beats_per_bar;
		_calculate_row_total();
		queue_redraw();
		update_minimum_size();
	}

	int VTimelinePanel::get_beat_per_bar() const {
		return beats_per_bar;
	}

	void VTimelinePanel::set_bar_line_color(const Color& p_color) {
		bar_line_color = p_color;
		queue_redraw();
	}

	Color VTimelinePanel::get_bar_line_color() const {
		return bar_line_color;
	}

	void VTimelinePanel::set_bar_line_width(const float p_width) {
		bar_line_width = p_width;
		queue_redraw();
	}

	float VTimelinePanel::get_bar_line_width() const {
		return bar_line_width;
	}

	void VTimelinePanel::set_beat_line_color(const Color& p_color) {
		beat_line_color = p_color;
		queue_redraw();
	}

	Color VTimelinePanel::get_beat_line_color() const {
		return beat_line_color;
	}

	void VTimelinePanel::set_beat_line_width(const float p_width) {
		beat_line_width = p_width;
		queue_redraw();
	}

	float VTimelinePanel::get_beat_line_width() const {
		return beat_line_width;
	}

	void VTimelinePanel::set_bar_number_direction(BarNumberDirection p_direction) {
		bar_number_direction = p_direction;
		queue_redraw();
	}

	VTimelinePanel::BarNumberDirection VTimelinePanel::get_bar_number_direction() const {
		return bar_number_direction;
	}

	void VTimelinePanel::set_beat_format(BeatFormat p_format) {
		beat_format = p_format;
		queue_redraw();
	}

	VTimelinePanel::BeatFormat VTimelinePanel::get_beat_format() const {
		return beat_format;
	}

	void VTimelinePanel::set_playhead(Ref<TimelineIndicator> p_playhead) {
		playhead = p_playhead;
		if (playhead.is_valid()) {
			playhead->connect("changed", callable_mp(this, &VTimelinePanel::_on_resource_changed));
		}
		queue_redraw();
	}

	Ref<TimelineIndicator> VTimelinePanel::get_playhead() const {
		return playhead;
	}

	void VTimelinePanel::set_markers(const TypedArray<TimelineMarker>& p_markers) {
		markers = p_markers;
		for (int i = 0; i < markers.size(); i++) {
			Ref<TimelineMarker> marker = markers[i];
			if (marker.is_valid() && !marker->is_connected("changed", callable_mp(this, &VTimelinePanel::_on_resource_changed))) {
				marker->connect("changed", callable_mp(this, &VTimelinePanel::_on_resource_changed));
			}
		}
	}

	TypedArray<TimelineMarker> VTimelinePanel::get_markers() const {
		return markers;
	}

	void VTimelinePanel::set_time_ruler(Ref<TimelineTimeRuler> p_time_ruler) {
		time_ruler = p_time_ruler;
		if (time_ruler.is_valid()) {
			time_ruler->connect("changed", callable_mp(this, &VTimelinePanel::_on_resource_changed));
		}
		queue_redraw();
		update_minimum_size();
	}

	Ref<TimelineTimeRuler> VTimelinePanel::get_time_ruler() const {
		return time_ruler;
	}

	void VTimelinePanel::set_tracks(const TypedArray<TimelineTrack>& p_tracks) {
		clear_all_keys();
		tracks = p_tracks;
		_rebuild_track_cache();
		for (int i = 0; i < tracks.size(); i++) {
			Ref<TimelineTrack> track = tracks[i];
			if (track.is_valid() && !track->is_connected("changed", callable_mp(this, &VTimelinePanel::_on_resource_changed))) {
				track->connect("changed", callable_mp(this, &VTimelinePanel::_on_resource_changed));
			}
		}
		queue_redraw();
		update_minimum_size();
	}

	TypedArray<TimelineTrack> VTimelinePanel::get_tracks() const {
		return tracks;
	}

	void VTimelinePanel::set_h_scroll(int p_pos) {
		hscroll->set_value(p_pos);
	}

	int VTimelinePanel::get_h_scroll() const {
		return hscroll->get_value();
	}

	void VTimelinePanel::set_v_scroll(int p_pos) {
		vscroll->set_value(p_pos);
	}

	int VTimelinePanel::get_v_scroll() const {
		return vscroll->get_value();
	}

	void VTimelinePanel::set_horizontal_custom_step(float p_custom_step) {
		hscroll->set_custom_step(p_custom_step);
	}

	float VTimelinePanel::get_horizontal_custom_step() const {
		return hscroll->get_custom_step();
	}

	void VTimelinePanel::set_vertical_custom_step(float p_custom_step) {
		vscroll->set_custom_step(p_custom_step);
	}

	float VTimelinePanel::get_vertical_custom_step() const {
		return vscroll->get_custom_step();
	}

	void VTimelinePanel::set_horizontal_scroll_mode(ScrollMode p_mode) {
		if (horizontal_scroll_mode == p_mode) {
			return;
		}

		horizontal_scroll_mode = p_mode;
		update_minimum_size();
	}

	VTimelinePanel::ScrollMode VTimelinePanel::get_horizontal_scroll_mode() const {
		return horizontal_scroll_mode;
	}

	void VTimelinePanel::set_vertical_scroll_mode(ScrollMode p_mode) {
		if (vertical_scroll_mode == p_mode) {
			return;
		}

		vertical_scroll_mode = p_mode;
		update_minimum_size();
	}

	VTimelinePanel::ScrollMode VTimelinePanel::get_vertical_scroll_mode() const {
		return vertical_scroll_mode;
	}

	void VTimelinePanel::set_deadzone(int p_deadzone) {
		deadzone = p_deadzone;
	}

	int VTimelinePanel::get_deadzone() const {
		return deadzone;
	}

	void VTimelinePanel::set_draw_minimap(bool p_enabled) {
		if (draw_minimap == p_enabled) {
			return;
		}
		draw_minimap = p_enabled;
		_update_scroll_bar();
		queue_redraw();
		update_minimum_size();
	}

	bool VTimelinePanel::is_drawing_minimap() const {
		return draw_minimap;
	}

	void VTimelinePanel::set_minimap_width(int p_width) {
		const int new_width = MAX(p_width, 0);
		if (minimap_width == new_width) {
			return;
		}
		minimap_width = new_width;
		_update_scroll_bar();
		queue_redraw();
		update_minimum_size();
	}

	int VTimelinePanel::get_minimap_width() const {
		return minimap_width;
	}

	void VTimelinePanel::set_icon_max_width(const float p_width) {
		style_cache.icon_max_width = p_width;
		queue_redraw();
	}

	float VTimelinePanel::get_icon_max_width() const {
		return style_cache.icon_max_width;
	}

	void VTimelinePanel::set_instant_key_scale(const float p_scale) {
		style_cache.instant_key_scale = p_scale;
		_refresh_track_key_metrics();
		queue_redraw();
	}

	float VTimelinePanel::get_instant_key_scale() const {
		return style_cache.instant_key_scale;
	}

	void VTimelinePanel::set_instant_key_normal_style(Ref<StyleBox> p_style) {
		style_cache.instant_key_normal = p_style;
		queue_redraw();
	}

	Ref<StyleBox> VTimelinePanel::get_instant_key_normal_style() const {
		return style_cache.instant_key_normal;
	}

	void VTimelinePanel::set_instant_key_selected_style(Ref<StyleBox> p_style) {
		style_cache.instant_key_selected = p_style;
		queue_redraw();
	}

	Ref<StyleBox> VTimelinePanel::get_instant_key_selected_style() const {
		return style_cache.instant_key_selected;
	}

	void VTimelinePanel::set_clip_key_normal_style(Ref<StyleBox> p_style) {
		style_cache.clip_key_normal = p_style;
		queue_redraw();
	}

	Ref<StyleBox> VTimelinePanel::get_clip_key_normal_style() const {
		return style_cache.clip_key_normal;
	}

	void VTimelinePanel::set_clip_key_selected_style(Ref<StyleBox> p_style) {
		style_cache.clip_key_selected = p_style;
		queue_redraw();
	}

	Ref<StyleBox> VTimelinePanel::get_clip_key_selected_style() const {
		return style_cache.clip_key_selected;
	}

	void VTimelinePanel::set_selection_rect_style(Ref<StyleBox> p_style) {
		style_cache.selection_rect = p_style;
		queue_redraw();
	}

	Ref<StyleBox> VTimelinePanel::get_selection_rect_style() const {
		return style_cache.selection_rect;
	}

	void VTimelinePanel::set_key_release_preview_style(Ref<StyleBox> p_style) {
		style_cache.key_release_preview = p_style;
		queue_redraw();
	}

	Ref<StyleBox> VTimelinePanel::get_key_release_preview_style() const {
		return style_cache.key_release_preview;
	}

	void VTimelinePanel::set_allow_key_cross_track_move(bool p_enabled) {
		allow_key_cross_track_move = p_enabled;
	}

	bool VTimelinePanel::get_allow_key_cross_track_move() const {
		return allow_key_cross_track_move;
	}

	void VTimelinePanel::set_key_snap_enabled(bool p_enabled) {
		key_snap_enabled = p_enabled;
	}

	bool VTimelinePanel::get_key_snap_enabled() const {
		return key_snap_enabled;
	}

	void VTimelinePanel::set_clip_key_edge_edit_enabled(bool p_enabled) {
		clip_key_edge_edit_enabled = p_enabled;
		if (!clip_key_edge_edit_enabled && !clip_key_edge_dragging) {
			set_default_cursor_shape(Control::CURSOR_ARROW);
		}
	}

	bool VTimelinePanel::get_clip_key_edge_edit_enabled() const {
		return clip_key_edge_edit_enabled;
	}

	void VTimelinePanel::set_allow_unselected_key_edit(bool p_enabled) {
		allow_unselected_key_edit = p_enabled;
	}

	bool VTimelinePanel::get_allow_unselected_key_edit() const {
		return allow_unselected_key_edit;
	}

	void VTimelinePanel::set_allow_right_mouse_selection(bool p_enabled) {
		allow_right_mouse_selection = p_enabled;
		if (!allow_right_mouse_selection && right_selecting) {
			right_selecting = false;
			queue_redraw();
			_stop_internal_process_if_idle();
		}
	}

	bool VTimelinePanel::get_allow_right_mouse_selection() const {
		return allow_right_mouse_selection;
	}
}
