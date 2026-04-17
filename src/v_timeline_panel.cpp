#include "v_timeline_panel.h"

#include "components/timeline_indicator.h"
#include "components/timeline_marker.h"
#include "components/timeline_time_ruler.h"
#include "components/timeline_track.h"
#include "components/timeline_track_key.h"

#include <godot_cpp/classes/font.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/input_event_mouse_motion.hpp>
#include <godot_cpp/classes/input_event_pan_gesture.hpp>
#include <godot_cpp/classes/style_box_flat.hpp>
#include <godot_cpp/classes/theme_db.hpp>

#include <algorithm>

namespace godot {
	void VTimelinePanel::_bind_methods() {
		BIND_ENUM_CONSTANT(TIME);
		BIND_ENUM_CONSTANT(FRAME);
		BIND_ENUM_CONSTANT(BEAT);

		BIND_ENUM_CONSTANT(HH_MM_SS);
		BIND_ENUM_CONSTANT(MM_SS_MS);
		BIND_ENUM_CONSTANT(SEC);

		BIND_ENUM_CONSTANT(BAR_NUMBER_TOP_DOWN);
		BIND_ENUM_CONSTANT(BAR_NUMBER_BOTTOM_UP);

		BIND_ENUM_CONSTANT(SCROLL_MODE_DISABLED);
		BIND_ENUM_CONSTANT(SCROLL_MODE_AUTO);
		BIND_ENUM_CONSTANT(SCROLL_MODE_SHOW_ALWAYS);
		BIND_ENUM_CONSTANT(SCROLL_MODE_SHOW_NEVER);
		BIND_ENUM_CONSTANT(SCROLL_MODE_RESERVE);

		ClassDB::bind_method(D_METHOD("create_key", "track_index", "time", "length"), &VTimelinePanel::create_key, DEFVAL(0.0f));
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

		ADD_GROUP("Beat", "beat_");
		ClassDB::bind_method(D_METHOD("set_bpms", "bpms"), &VTimelinePanel::set_bpms);
		ClassDB::bind_method(D_METHOD("get_bpms"), &VTimelinePanel::get_bpms);
		ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "beat_bpms", PROPERTY_HINT_DICTIONARY_TYPE, "float;int"), "set_bpms", "get_bpms");

		ClassDB::bind_method(D_METHOD("set_beat_per_bar", "num"), &VTimelinePanel::set_beat_per_bar);
		ClassDB::bind_method(D_METHOD("get_beat_per_bar"), &VTimelinePanel::get_beat_per_bar);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "beat_per_bar"), "set_beat_per_bar", "get_beat_per_bar");

		ClassDB::bind_method(D_METHOD("set_beat_line_color", "color"), &VTimelinePanel::set_beat_line_color);
		ClassDB::bind_method(D_METHOD("get_beat_line_color"), &VTimelinePanel::get_beat_line_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "beat_beat_line_color"), "set_beat_line_color", "get_beat_line_color");

		ClassDB::bind_method(D_METHOD("set_beat_line_width", "width"), &VTimelinePanel::set_beat_line_width);
		ClassDB::bind_method(D_METHOD("get_beat_line_width"), &VTimelinePanel::get_beat_line_width);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "beat_beat_line_width"), "set_beat_line_width", "get_beat_line_width");

		ClassDB::bind_method(D_METHOD("set_bar_line_color", "color"), &VTimelinePanel::set_bar_line_color);
		ClassDB::bind_method(D_METHOD("get_bar_line_color"), &VTimelinePanel::get_bar_line_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "beat_bar_line_color"), "set_bar_line_color", "get_bar_line_color");

		ClassDB::bind_method(D_METHOD("set_bar_line_width", "width"), &VTimelinePanel::set_bar_line_width);
		ClassDB::bind_method(D_METHOD("get_bar_line_width"), &VTimelinePanel::get_bar_line_width);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "beat_bar_line_width"), "set_bar_line_width", "get_bar_line_width");

		ClassDB::bind_method(D_METHOD("set_bar_number_direction", "direction"), &VTimelinePanel::set_bar_number_direction);
		ClassDB::bind_method(D_METHOD("get_bar_number_direction"), &VTimelinePanel::get_bar_number_direction);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "beat_bar_number_direction", PROPERTY_HINT_ENUM, "Top Down,Bottom Up"), "set_bar_number_direction", "get_bar_number_direction");
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

		ADD_SIGNAL(MethodInfo("scroll_started"));
		ADD_SIGNAL(MethodInfo("scroll_ended"));
	}

	void VTimelinePanel::_notification(int p_what) {
		switch (p_what) {
		case NOTIFICATION_RESIZED: {
			_update_scroll_bar();
		} break;

		case NOTIFICATION_INTERNAL_PROCESS: {
			if (select_pending) {
				select_timer += get_process_delta_time();
				if (select_timer >= long_press_time && !beyond_deadzone) {
					select_pending = false;
					selecting = true;
					// 退出滚动状态，防止后续 motion 被当成滚动
					drag_touching = false;
					beyond_deadzone = false;
					queue_redraw();
				}
			}
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
			for (size_t i = 0; i < _track_cache.size(); i++) {
				const auto& ct = _track_cache[i];
				if (ct.width <= 0.0f) continue;

				for (TimelineTrackKey* key : ct.keys) {
					if (!key || key->is_disabled()) continue;

					float y = 0.0f;
					float y_end = 0.0f;
					switch (counting_unit) {
						case FRAME: {
							y = _frame_to_y(static_cast<int64_t>(key->get_time()));
							y_end = _frame_to_y(static_cast<int64_t>(key->get_time() + key->get_length()));
						} break;
						case BEAT:
						case TIME:
						default: {
							y = _time_to_y(key->get_time());
							y_end = _time_to_y(key->get_time() + key->get_length());
						} break;
					}

					if (key->is_instant()) {
						float key_scale = 0.4f;
						if (style_cache.instant_key_scale != 0.4f) {
							key_scale = style_cache.instant_key_scale;
						}
						else {
							key_scale = key->get_instant_key_scale();
						}
						float pos_x = ct.x_offset - hscroll_value;
						float pos_y = y - ct.width * 0.5f;
						float width = ct.width * key_scale;
						Rect2 key_rect(pos_x + width * 0.5f, pos_y + width * 0.5f, width, width);
						Ref<StyleBox> style;
						if (key->get_instant_key_normal_style().is_valid()) {
							style = key->get_instant_key_normal_style();
						}
						else if (style_cache.instant_key_normal.is_valid()) {
							style = style_cache.instant_key_normal;
						}
						else {
							style = style_cache.instant_key_normal_fallback;
						}
						if (style.is_valid()) {
							draw_style_box(style, key_rect);
						}
						if (key->is_selected()) {
							if (key->get_instant_key_selected_style().is_valid()) {
								style = key->get_instant_key_selected_style();
							}
							else if (style_cache.instant_key_selected.is_valid()) {
								style = style_cache.instant_key_selected;
							}
							else {
								style = style_cache.instant_key_selected_fallback;
							}
							if (style.is_valid()) {
								draw_style_box(style, key_rect);
							}
						}
					}
					else {
						// 防止矩形尺寸出现负数
						Rect2 bar_rect;
						if (y_end < y) {
							bar_rect = Rect2(ct.x_offset - hscroll_value, y_end, ct.width, y - y_end);
						} else {
							bar_rect = Rect2(ct.x_offset - hscroll_value, y, ct.width, y_end - y);
						}
						Ref<StyleBox> style;
						if (key->get_clip_key_normal_style().is_valid()) {
							style = key->get_clip_key_normal_style();
						}
						else if (style_cache.clip_key_normal.is_valid()) {
							style = style_cache.clip_key_normal;
						}
						else {
							style = style_cache.clip_key_normal_fallback;
						}
						if (style.is_valid()) {
							draw_style_box(style, bar_rect);
						}
						if (key->is_selected()) {
							if (key->get_clip_key_selected_style().is_valid()) {
								style = key->get_clip_key_selected_style();
							}
							else if (style_cache.clip_key_selected.is_valid()) {
								style = style_cache.clip_key_selected;
							}
							else {
								style = style_cache.clip_key_selected_fallback;
							}
							if (style.is_valid()) {
								draw_style_box(style, bar_rect);
							}
						}
					}
				}
			}
			// 绘制指示器
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
				const PackedVector2Array points = indicator->_get_points(current_position, current_width);
				const PackedColorArray colors = indicator->_get_colors(current_position);
				const String text = indicator->_get_text(static_cast<int>(counting_unit), time, current_position);
				const Ref<Font> font = indicator->_get_font(current_position);
				const Vector2 font_pos = indicator->_get_font_pos(current_position);
				const int64_t font_size = indicator->_get_font_size(current_position);
				const Color font_color = indicator->_get_font_color(current_position);
				const bool show_line = indicator->_can_show_line(current_position);
				const float line_width = indicator->_get_line_width(current_position);
				const Color line_color = indicator->_get_line_color(current_position);
				_draw_indicator(time, points, colors, text, font, font_pos, font_size, font_color, show_line, line_width, line_color);
			}

			if (selecting) {
				Rect2 sel_rect;
				sel_rect.position.x = Math::min(select_start.x, select_end.x);
				sel_rect.position.y = Math::min(select_start.y, select_end.y);
				sel_rect.size.x = Math::abs(select_end.x - select_start.x);
				sel_rect.size.y = Math::abs(select_end.y - select_start.y);

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

		add_child(vscroll, false, INTERNAL_MODE_FRONT);
	}

	VTimelinePanel::~VTimelinePanel() {
		clear_all_keys();
	}

	void VTimelinePanel::_collect_selected_keys() {
		// 先清空所有已选状态
		for (auto& ct : _track_cache) {
			for (TimelineTrackKey* key : ct.keys) {
				if (key) {
					key->set_selected_no_signal(false);
				}
			}
		}

		// 构造选择矩形
		Rect2 sel_rect;
		sel_rect.position.x = Math::min(select_start.x, select_end.x);
		sel_rect.position.y = Math::min(select_start.y, select_end.y);
		sel_rect.size.x = Math::abs(select_end.x - select_start.x);
		sel_rect.size.y = Math::abs(select_end.y - select_start.y);

		// 接近点击时，扩大为 4x4 像素点选区域
		if (sel_rect.size.x < 2.0f && sel_rect.size.y < 2.0f) {
			sel_rect = Rect2(select_start - Vector2(2, 2), Vector2(4, 4));
		}

		// 遍历所有键，判断是否与选择矩形相交
		for (size_t i = 0; i < _track_cache.size(); i++) {
			const auto& ct = _track_cache[i];
			if (ct.width <= 0.0f) continue;

			for (TimelineTrackKey* key : ct.keys) {
				if (!key || key->is_disabled()) continue;

				float y = 0.0f;
				float y_end = 0.0f;
				switch (counting_unit) {
					case FRAME: {
						y = _frame_to_y(static_cast<int64_t>(key->get_time()));
						y_end = _frame_to_y(static_cast<int64_t>(key->get_time() + key->get_length()));
					} break;
					case BEAT:
					case TIME:
					default: {
						y = _time_to_y(key->get_time());
						y_end = _time_to_y(key->get_time() + key->get_length());
					} break;
				}

				Rect2 key_rect;
				if (key->is_instant()) {
					float key_scale = 0.4f;
					if (style_cache.instant_key_scale != 0.4f) {
						key_scale = style_cache.instant_key_scale;
					}
					else {
						key_scale = key->get_instant_key_scale();
					}
					float pos_x = ct.x_offset - hscroll_value;
					float pos_y = y - ct.width * 0.5f;
					float width = ct.width * key_scale;
					key_rect = Rect2(pos_x + width * 0.5f, pos_y + width * 0.5f, width, width);
				}
				else {
					if (y_end < y) {
						key_rect = Rect2(ct.x_offset - hscroll_value, y_end, ct.width, y - y_end);
					}
					else {
						key_rect = Rect2(ct.x_offset - hscroll_value, y, ct.width, y_end - y);
					}
				}

				if (sel_rect.intersects(key_rect)) {
					key->set_selected_no_signal(true);
				}
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

	float VTimelinePanel::_time_to_y(double p_time) const {
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

	double VTimelinePanel::_y_to_time(float p_y) const {
		double time;
		if (bar_number_direction == BAR_NUMBER_BOTTOM_UP) {
			time = (header_height + content_height - p_y - vscroll_value) / scale;
		}
		else {
			time = (p_y - header_height + vscroll_value) / scale;
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

	float VTimelinePanel::_beat_to_y(double p_beat) const {
		if (bar_number_direction == BAR_NUMBER_BOTTOM_UP) {
			return header_height + content_height - p_beat * (scale / beats_per_bar) - vscroll_value;
		}
		return header_height + p_beat * (scale / beats_per_bar) - vscroll_value;
	}

	double VTimelinePanel::_y_to_beat(float p_y) const {
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

	float VTimelinePanel::_frame_to_y(int64_t p_frame) const {
		if (bar_number_direction == BAR_NUMBER_BOTTOM_UP) {
			return header_height + content_height - p_frame * scale - vscroll_value;
		}
		return header_height + p_frame * scale - vscroll_value;
	}

	int64_t VTimelinePanel::_y_to_frame(float p_y) const {
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

	void VTimelinePanel::_on_resource_changed() {
		queue_redraw();
		update_minimum_size();
	}

	void VTimelinePanel::_rebuild_track_cache() {
		_track_cache.clear();
		_track_cache.reserve(tracks.size());

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

		bool h_scroll_show = horizontal_scroll_mode == SCROLL_MODE_SHOW_ALWAYS ||
			(horizontal_scroll_mode == SCROLL_MODE_AUTO && content_width > size.x);
		bool v_scroll_show = vertical_scroll_mode == SCROLL_MODE_SHOW_ALWAYS ||
			(vertical_scroll_mode == SCROLL_MODE_AUTO && content_height > size.y - header_height);

		updating_scroll = true;

		if (h_scroll_show) {
			hscroll->set_anchor_and_offset(SIDE_LEFT, ANCHOR_BEGIN, 0);
			hscroll->set_anchor_and_offset(SIDE_RIGHT, ANCHOR_END, v_scroll_show ? -vmin.x : 0);
			hscroll->set_anchor_and_offset(SIDE_TOP, ANCHOR_END, -hmin.y);
			hscroll->set_anchor_and_offset(SIDE_BOTTOM, ANCHOR_END, 0);
			hscroll->set_max(content_width);
			hscroll->set_page(v_scroll_show ? size.x - vmin.x : size.x);
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

	void VTimelinePanel::_draw_indicator(
		const double time,
		const PackedVector2Array& points,
		const PackedColorArray& colors,
		const String& text,
		const Ref<Font> font,
		const Vector2& font_pos,
		const int64_t font_size,
		const Color& font_color,
		const bool show_line,
		const float line_width,
		const Color& line_color
	) {
		draw_polygon(points, colors);
		if (show_line) {
			double line_position;
			switch (counting_unit) {
			case FRAME: {
				int64_t frame = static_cast<int64_t>(time * fps);
				line_position = get_position_from_frame(frame);
				break;
			}
			case BEAT: {
				line_position = _time_to_y(time);
				break;
			}
			case TIME:
			default:
				line_position = get_position_from_time(time);
				break;
			}
			draw_line(Point2(0.0f, line_position), Point2(header_width, line_position), line_color, line_width);
		}
		draw_string(font, font_pos, text, HORIZONTAL_ALIGNMENT_LEFT, -1, font_size, font_color);
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

			bool is_touchscreen_available = DisplayServer::get_singleton()->is_touchscreen_available();
			if (mb->get_button_index() == MouseButton::MOUSE_BUTTON_LEFT) {
				if (!is_touchscreen_available) {
					// 只有桌面端左键才直接触发框选
					if (mb->is_pressed() && mb->get_position().y > header_height) {
						selecting = true;
						select_start = mb->get_position();
						select_end = select_start;
						queue_redraw();
					}
					else {
						if (selecting) {
							selecting = false;
							_collect_selected_keys();
							queue_redraw();
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
					if (selecting) {
						selecting = false;
						_collect_selected_keys();
						queue_redraw();
					}
					else if (select_pending) {
						select_pending = false;
						if (drag_touching) {
							_cancel_drag();
						}
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

	TimelineTrackKey* VTimelinePanel::create_key(int p_track_index, double p_time, double p_length) {
		ERR_FAIL_INDEX_V(p_track_index, static_cast<int>(_track_cache.size()), nullptr);

		TimelineTrackKey* key = memnew(TimelineTrackKey);
		key->set_time(p_time);
		key->set_length(p_length);
		key->connect("changed", callable_mp(this, &VTimelinePanel::_on_resource_changed));

		// 有序插入
		auto& ct = _track_cache[p_track_index];
		auto& keys = ct.keys;
		auto it = std::lower_bound(keys.begin(), keys.end(), p_time,
			[](TimelineTrackKey* k, double time) { return k->get_time() < time; });
		keys.insert(it, key);

		if (p_length > ct.max_key_length) {
			ct.max_key_length = p_length;
		}

		queue_redraw();
		return key;
	}

	void VTimelinePanel::remove_key(int p_track_index, int p_key_index) {
		ERR_FAIL_INDEX(p_track_index, static_cast<int>(_track_cache.size()));
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
		queue_redraw();
	}

	void VTimelinePanel::clear_track_keys(int p_track_index) {
		ERR_FAIL_INDEX(p_track_index, static_cast<int>(_track_cache.size()));
		auto& keys = _track_cache[p_track_index].keys;
		for (TimelineTrackKey* key : keys) {
			if (key) {
				memdelete(key);
			}
		}
		keys.clear();
		_track_cache[p_track_index].max_key_length = 0.0;
		queue_redraw();
	}

	void VTimelinePanel::clear_all_keys() {
		for (auto& ct : _track_cache) {
			for (TimelineTrackKey* key : ct.keys) {
				if (key) {
					memdelete(key);
				}
			}
			ct.keys.clear();
			ct.max_key_length = 0.0;
		}
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
		return _y_to_time(static_cast<float>(p_position));
	}

	double VTimelinePanel::get_frame_from_position(const double p_position) const {
		return static_cast<double>(_y_to_frame(static_cast<float>(p_position)));
	}

	double VTimelinePanel::get_beat_from_position(const double p_position) const {
		return _y_to_beat(static_cast<float>(p_position));
	}

	double VTimelinePanel::get_position_from_time(double p_time) const {
		return static_cast<double>(_time_to_y(p_time));
	}

	double VTimelinePanel::get_position_from_frame(int64_t p_frame) const {
		return static_cast<double>(_frame_to_y(p_frame));
	}

	double VTimelinePanel::get_position_from_beat(double p_beat) const {
		return static_cast<double>(_beat_to_y(p_beat));
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

	void VTimelinePanel::set_icon_max_width(const float p_width) {
		style_cache.icon_max_width = p_width;
		queue_redraw();
	}

	float VTimelinePanel::get_icon_max_width() const {
		return style_cache.icon_max_width;
	}

	void VTimelinePanel::set_instant_key_scale(const float p_scale) {
		style_cache.instant_key_scale = p_scale;
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
}