#include "timeline_panel_base.h"

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

namespace {
	constexpr float INSTANT_KEY_BASE_SIZE = 32.0f;
}

namespace godot {
	void TimelinePanelBase::_bind_methods() {
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

		ClassDB::bind_method(D_METHOD("create_key", "track_index", "time", "length", "snap"), &TimelinePanelBase::create_key, DEFVAL(0.0f), DEFVAL(false));
		ClassDB::bind_method(D_METHOD("remove_key", "track_index", "key_index"), &TimelinePanelBase::remove_key);
		ClassDB::bind_method(D_METHOD("clear_track_keys", "track_index"), &TimelinePanelBase::clear_track_keys);
		ClassDB::bind_method(D_METHOD("clear_all_keys"), &TimelinePanelBase::clear_all_keys);
		ClassDB::bind_method(D_METHOD("get_key_count"), &TimelinePanelBase::get_key_count);
		ClassDB::bind_method(D_METHOD("get_key", "track_index", "key_index"), &TimelinePanelBase::get_key);

		ClassDB::bind_method(D_METHOD("get_time_from_position", "position"), &TimelinePanelBase::get_time_from_position);
		ClassDB::bind_method(D_METHOD("get_frame_from_position", "position"), &TimelinePanelBase::get_frame_from_position);
		ClassDB::bind_method(D_METHOD("get_beat_from_position", "position"), &TimelinePanelBase::get_beat_from_position);
		ClassDB::bind_method(D_METHOD("get_position_from_time", "time"), &TimelinePanelBase::get_position_from_time);
		ClassDB::bind_method(D_METHOD("get_position_from_frame", "frame"), &TimelinePanelBase::get_position_from_frame);
		ClassDB::bind_method(D_METHOD("get_position_from_beat", "beat"), &TimelinePanelBase::get_position_from_beat);

		ClassDB::bind_method(D_METHOD("get_h_scroll_bar"), &TimelinePanelBase::get_h_scroll_bar);
		ClassDB::bind_method(D_METHOD("get_v_scroll_bar"), &TimelinePanelBase::get_v_scroll_bar);

		ClassDB::bind_method(D_METHOD("set_background_color", "background_color"), &TimelinePanelBase::set_background_color);
		ClassDB::bind_method(D_METHOD("get_background_color"), &TimelinePanelBase::get_background_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "background_color"), "set_background_color", "get_background_color");

		ClassDB::bind_method(D_METHOD("set_header_height", "header_height"), &TimelinePanelBase::set_header_height);
		ClassDB::bind_method(D_METHOD("get_header_height"), &TimelinePanelBase::get_header_height);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "header_height"), "set_header_height", "get_header_height");

		ClassDB::bind_method(D_METHOD("set_header_resize_enabled", "enabled"), &TimelinePanelBase::set_header_resize_enabled);
		ClassDB::bind_method(D_METHOD("get_header_resize_enabled"), &TimelinePanelBase::get_header_resize_enabled);
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "header_resize_enabled"), "set_header_resize_enabled", "get_header_resize_enabled");

		ClassDB::bind_method(D_METHOD("set_duration", "duration"), &TimelinePanelBase::set_duration);
		ClassDB::bind_method(D_METHOD("get_duration"), &TimelinePanelBase::get_duration);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "duration", PROPERTY_HINT_RANGE, "0,99999,0.01,or_greater,suffix:s"), "set_duration", "get_duration");

		ClassDB::bind_method(D_METHOD("set_current_time", "current_time"), &TimelinePanelBase::set_current_time);
		ClassDB::bind_method(D_METHOD("get_current_time"), &TimelinePanelBase::get_current_time);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "current_time", PROPERTY_HINT_RANGE, "0,99999,0.01,or_greater,suffix:s"), "set_current_time", "get_current_time");

		ClassDB::bind_method(D_METHOD("set_scale", "scale"), &TimelinePanelBase::set_scale);
		ClassDB::bind_method(D_METHOD("get_scale"), &TimelinePanelBase::get_scale);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "scale", PROPERTY_HINT_RANGE, "1,128,0.1,or_greater"), "set_scale", "get_scale");

		ClassDB::bind_method(D_METHOD("set_counting_unit", "unit"), &TimelinePanelBase::set_counting_unit);
		ClassDB::bind_method(D_METHOD("get_counting_unit"), &TimelinePanelBase::get_counting_unit);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "counting_unit", PROPERTY_HINT_ENUM, "Time,Frame,Beat"), "set_counting_unit", "get_counting_unit");

		ADD_GROUP("Time", "time_");
		ClassDB::bind_method(D_METHOD("set_time_format", "format"), &TimelinePanelBase::set_time_format);
		ClassDB::bind_method(D_METHOD("get_time_format"), &TimelinePanelBase::get_time_format);
		ADD_PROPERTY(PropertyInfo(Variant::STRING, "time_format"), "set_time_format", "get_time_format");

		ClassDB::bind_method(D_METHOD("set_show_milliseconds", "show_milliseconds"), &TimelinePanelBase::set_show_milliseconds);
		ClassDB::bind_method(D_METHOD("get_show_milliseconds"), &TimelinePanelBase::get_show_milliseconds);
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "time_show_milliseconds"), "set_show_milliseconds", "get_show_milliseconds");

		ADD_GROUP("Frame", "frame_");
		ClassDB::bind_method(D_METHOD("set_fps", "fps"), &TimelinePanelBase::set_fps);
		ClassDB::bind_method(D_METHOD("get_fps"), &TimelinePanelBase::get_fps);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "frame_fps"), "set_fps", "get_fps");

		ClassDB::bind_method(D_METHOD("set_show_subdivision", "show_subdivision"), &TimelinePanelBase::set_show_subdivision);
		ClassDB::bind_method(D_METHOD("get_show_subdivision"), &TimelinePanelBase::get_show_subdivision);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "frame_show_subdivision"), "set_show_subdivision", "get_show_subdivision");

		ADD_GROUP("Beat", "");
		ClassDB::bind_method(D_METHOD("set_bpms", "bpms"), &TimelinePanelBase::set_bpms);
		ClassDB::bind_method(D_METHOD("get_bpms"), &TimelinePanelBase::get_bpms);
		ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "bpms"), "set_bpms", "get_bpms");

		ClassDB::bind_method(D_METHOD("set_beat_per_bar", "num"), &TimelinePanelBase::set_beat_per_bar);
		ClassDB::bind_method(D_METHOD("get_beat_per_bar"), &TimelinePanelBase::get_beat_per_bar);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "beat_per_bar"), "set_beat_per_bar", "get_beat_per_bar");

		ClassDB::bind_method(D_METHOD("set_beat_line_color", "color"), &TimelinePanelBase::set_beat_line_color);
		ClassDB::bind_method(D_METHOD("get_beat_line_color"), &TimelinePanelBase::get_beat_line_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "beat_line_color"), "set_beat_line_color", "get_beat_line_color");

		ClassDB::bind_method(D_METHOD("set_beat_line_width", "width"), &TimelinePanelBase::set_beat_line_width);
		ClassDB::bind_method(D_METHOD("get_beat_line_width"), &TimelinePanelBase::get_beat_line_width);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "beat_line_width"), "set_beat_line_width", "get_beat_line_width");

		ClassDB::bind_method(D_METHOD("set_bar_line_color", "color"), &TimelinePanelBase::set_bar_line_color);
		ClassDB::bind_method(D_METHOD("get_bar_line_color"), &TimelinePanelBase::get_bar_line_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "bar_line_color"), "set_bar_line_color", "get_bar_line_color");

		ClassDB::bind_method(D_METHOD("set_bar_line_width", "width"), &TimelinePanelBase::set_bar_line_width);
		ClassDB::bind_method(D_METHOD("get_bar_line_width"), &TimelinePanelBase::get_bar_line_width);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "bar_line_width"), "set_bar_line_width", "get_bar_line_width");

		ClassDB::bind_method(D_METHOD("set_bar_number_direction", "direction"), &TimelinePanelBase::set_bar_number_direction);
		ClassDB::bind_method(D_METHOD("get_bar_number_direction"), &TimelinePanelBase::get_bar_number_direction);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "bar_number_direction", PROPERTY_HINT_ENUM, "Top Down,Bottom Up"), "set_bar_number_direction", "get_bar_number_direction");

		ClassDB::bind_method(D_METHOD("set_beat_format", "format"), &TimelinePanelBase::set_beat_format);
		ClassDB::bind_method(D_METHOD("get_beat_format"), &TimelinePanelBase::get_beat_format);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "beat_format", PROPERTY_HINT_ENUM, "Beat Bar"), "set_beat_format", "get_beat_format");
		ADD_GROUP("", "");

		ADD_GROUP("Separator", "separator_");
		ClassDB::bind_method(D_METHOD("set_separator_color", "separator_color"), &TimelinePanelBase::set_separator_color);
		ClassDB::bind_method(D_METHOD("get_separator_color"), &TimelinePanelBase::get_separator_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "separator_color"), "set_separator_color", "get_separator_color");

		ClassDB::bind_method(D_METHOD("set_separator_width", "separator_width"), &TimelinePanelBase::set_separator_width);
		ClassDB::bind_method(D_METHOD("get_separator_width"), &TimelinePanelBase::get_separator_width);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "separator_width"), "set_separator_width", "get_separator_width");
		ADD_GROUP("", "");

		ADD_GROUP("Components", "component_");
		ClassDB::bind_method(D_METHOD("set_playhead", "playhead"), &TimelinePanelBase::set_playhead);
		ClassDB::bind_method(D_METHOD("get_playhead"), &TimelinePanelBase::get_playhead);
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "component_playhead", PROPERTY_HINT_RESOURCE_TYPE, "TimelineIndicator"), "set_playhead", "get_playhead");

		ClassDB::bind_method(D_METHOD("set_markers", "markers"), &TimelinePanelBase::set_markers);
		ClassDB::bind_method(D_METHOD("get_markers"), &TimelinePanelBase::get_markers);
		ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "component_markers", PROPERTY_HINT_ARRAY_TYPE, "TimelineMarker"), "set_markers", "get_markers");

		ADD_GROUP("Components", "component_");
		ClassDB::bind_method(D_METHOD("set_time_ruler", "time_ruler"), &TimelinePanelBase::set_time_ruler);
		ClassDB::bind_method(D_METHOD("get_time_ruler"), &TimelinePanelBase::get_time_ruler);
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "component_time_ruler", PROPERTY_HINT_RESOURCE_TYPE, "TimelineTimeRuler"), "set_time_ruler", "get_time_ruler");

		ClassDB::bind_method(D_METHOD("set_tracks", "tracks"), &TimelinePanelBase::set_tracks);
		ClassDB::bind_method(D_METHOD("get_tracks"), &TimelinePanelBase::get_tracks);
		ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "component_track", PROPERTY_HINT_ARRAY_TYPE, "TimelineTrack", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_EDITOR_INSTANTIATE_OBJECT), "set_tracks", "get_tracks");

		ADD_GROUP("Scrollbar", "");
		ClassDB::bind_method(D_METHOD("set_h_scroll", "value"), &TimelinePanelBase::set_h_scroll);
		ClassDB::bind_method(D_METHOD("get_h_scroll"), &TimelinePanelBase::get_h_scroll);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "scroll_horizontal", PROPERTY_HINT_NONE, "suffix:px"), "set_h_scroll", "get_h_scroll");

		ClassDB::bind_method(D_METHOD("set_v_scroll", "value"), &TimelinePanelBase::set_v_scroll);
		ClassDB::bind_method(D_METHOD("get_v_scroll"), &TimelinePanelBase::get_v_scroll);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "scroll_vertical", PROPERTY_HINT_NONE, "suffix:px"), "set_v_scroll", "get_v_scroll");

		ClassDB::bind_method(D_METHOD("set_horizontal_custom_step", "value"), &TimelinePanelBase::set_horizontal_custom_step);
		ClassDB::bind_method(D_METHOD("get_horizontal_custom_step"), &TimelinePanelBase::get_horizontal_custom_step);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "scroll_horizontal_custom_step", PROPERTY_HINT_RANGE, "-1,4096,suffix:px"), "set_horizontal_custom_step", "get_horizontal_custom_step");

		ClassDB::bind_method(D_METHOD("set_vertical_custom_step", "value"), &TimelinePanelBase::set_vertical_custom_step);
		ClassDB::bind_method(D_METHOD("get_vertical_custom_step"), &TimelinePanelBase::get_vertical_custom_step);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "scroll_vertical_custom_step", PROPERTY_HINT_RANGE, "-1,4096,suffix:px"), "set_vertical_custom_step", "get_vertical_custom_step");

		ClassDB::bind_method(D_METHOD("set_horizontal_scroll_mode", "enable"), &TimelinePanelBase::set_horizontal_scroll_mode);
		ClassDB::bind_method(D_METHOD("get_horizontal_scroll_mode"), &TimelinePanelBase::get_horizontal_scroll_mode);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "horizontal_scroll_mode", PROPERTY_HINT_ENUM, "Disabled,Auto,Always Show,Never Show,Reserve"), "set_horizontal_scroll_mode", "get_horizontal_scroll_mode");

		ClassDB::bind_method(D_METHOD("set_vertical_scroll_mode", "enable"), &TimelinePanelBase::set_vertical_scroll_mode);
		ClassDB::bind_method(D_METHOD("get_vertical_scroll_mode"), &TimelinePanelBase::get_vertical_scroll_mode);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "vertical_scroll_mode", PROPERTY_HINT_ENUM, "Disabled,Auto,Always Show,Never Show,Reserve"), "set_vertical_scroll_mode", "get_vertical_scroll_mode");

		ClassDB::bind_method(D_METHOD("set_deadzone", "deadzone"), &TimelinePanelBase::set_deadzone);
		ClassDB::bind_method(D_METHOD("get_deadzone"), &TimelinePanelBase::get_deadzone);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "scroll_deadzone"), "set_deadzone", "get_deadzone");

		ClassDB::bind_method(D_METHOD("set_middle_mouse_pan_enabled", "enabled"), &TimelinePanelBase::set_middle_mouse_pan_enabled);
		ClassDB::bind_method(D_METHOD("get_middle_mouse_pan_enabled"), &TimelinePanelBase::get_middle_mouse_pan_enabled);
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "middle_mouse_pan_enabled"), "set_middle_mouse_pan_enabled", "get_middle_mouse_pan_enabled");
		ADD_GROUP("", "");

		ADD_GROUP("Minimap", "minimap_");
		ClassDB::bind_method(D_METHOD("set_draw_minimap", "enabled"), &TimelinePanelBase::set_draw_minimap);
		ClassDB::bind_method(D_METHOD("is_drawing_minimap"), &TimelinePanelBase::is_drawing_minimap);
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "minimap_draw", PROPERTY_HINT_GROUP_ENABLE), "set_draw_minimap", "is_drawing_minimap");

		ClassDB::bind_method(D_METHOD("set_minimap_width", "width"), &TimelinePanelBase::set_minimap_width);
		ClassDB::bind_method(D_METHOD("get_minimap_width"), &TimelinePanelBase::get_minimap_width);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "minimap_width", PROPERTY_HINT_RANGE, "0,512,1,or_greater,suffix:px"), "set_minimap_width", "get_minimap_width");
		ADD_GROUP("", "");

		ADD_GROUP("Key Editing", "");
		ClassDB::bind_method(D_METHOD("set_allow_key_cross_track_move", "enabled"), &TimelinePanelBase::set_allow_key_cross_track_move);
		ClassDB::bind_method(D_METHOD("get_allow_key_cross_track_move"), &TimelinePanelBase::get_allow_key_cross_track_move);
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "allow_key_cross_track_move"), "set_allow_key_cross_track_move", "get_allow_key_cross_track_move");
		ClassDB::bind_method(D_METHOD("set_key_snap_enabled", "enabled"), &TimelinePanelBase::set_key_snap_enabled);
		ClassDB::bind_method(D_METHOD("get_key_snap_enabled"), &TimelinePanelBase::get_key_snap_enabled);
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "key_snap_enabled"), "set_key_snap_enabled", "get_key_snap_enabled");
		ClassDB::bind_method(D_METHOD("set_clip_key_edge_edit_enabled", "enabled"), &TimelinePanelBase::set_clip_key_edge_edit_enabled);
		ClassDB::bind_method(D_METHOD("get_clip_key_edge_edit_enabled"), &TimelinePanelBase::get_clip_key_edge_edit_enabled);
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "clip_key_edge_edit_enabled"), "set_clip_key_edge_edit_enabled", "get_clip_key_edge_edit_enabled");
		ClassDB::bind_method(D_METHOD("set_allow_unselected_key_edit", "enabled"), &TimelinePanelBase::set_allow_unselected_key_edit);
		ClassDB::bind_method(D_METHOD("get_allow_unselected_key_edit"), &TimelinePanelBase::get_allow_unselected_key_edit);
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "allow_unselected_key_edit"), "set_allow_unselected_key_edit", "get_allow_unselected_key_edit");
		ClassDB::bind_method(D_METHOD("set_allow_right_mouse_selection", "enabled"), &TimelinePanelBase::set_allow_right_mouse_selection);
		ClassDB::bind_method(D_METHOD("get_allow_right_mouse_selection"), &TimelinePanelBase::get_allow_right_mouse_selection);
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "allow_right_mouse_selection"), "set_allow_right_mouse_selection", "get_allow_right_mouse_selection");
		ADD_GROUP("", "");

		GDVIRTUAL_BIND(_should_handle_selection_rect, "rect", "keys", "mouse_button");
		GDVIRTUAL_BIND(_handle_selection_rect, "rect", "keys", "mouse_button");

		ADD_GROUP("Style overrides", "");
		ADD_SUBGROUP("Constants", "");
		ClassDB::bind_method(D_METHOD("set_icon_max_width", "width"), &TimelinePanelBase::set_icon_max_width);
		ClassDB::bind_method(D_METHOD("get_icon_max_width"), &TimelinePanelBase::get_icon_max_width);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "icon_max_width"), "set_icon_max_width", "get_icon_max_width");

		ClassDB::bind_method(D_METHOD("set_instant_key_scale", "scale"), &TimelinePanelBase::set_instant_key_scale);
		ClassDB::bind_method(D_METHOD("get_instant_key_scale"), &TimelinePanelBase::get_instant_key_scale);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "instant_key_scale"), "set_instant_key_scale", "get_instant_key_scale");

		ADD_SUBGROUP("Styles", "");
		ClassDB::bind_method(D_METHOD("set_instant_key_normal_style", "style"), &TimelinePanelBase::set_instant_key_normal_style);
		ClassDB::bind_method(D_METHOD("get_instant_key_normal_style"), &TimelinePanelBase::get_instant_key_normal_style);
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "instant_key_normal", PROPERTY_HINT_RESOURCE_TYPE, "StyleBox"), "set_instant_key_normal_style", "get_instant_key_normal_style");

		ClassDB::bind_method(D_METHOD("set_instant_key_selected_style", "style"), &TimelinePanelBase::set_instant_key_selected_style);
		ClassDB::bind_method(D_METHOD("get_instant_key_selected_style"), &TimelinePanelBase::get_instant_key_selected_style);
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "instant_key_selected", PROPERTY_HINT_RESOURCE_TYPE, "StyleBox"), "set_instant_key_selected_style", "get_instant_key_selected_style");

		ClassDB::bind_method(D_METHOD("set_clip_key_normal_style", "style"), &TimelinePanelBase::set_clip_key_normal_style);
		ClassDB::bind_method(D_METHOD("get_clip_key_normal_style"), &TimelinePanelBase::get_clip_key_normal_style);
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "clip_key_normal", PROPERTY_HINT_RESOURCE_TYPE, "StyleBox"), "set_clip_key_normal_style", "get_clip_key_normal_style");

		ClassDB::bind_method(D_METHOD("set_selection_rect_style", "style"), &TimelinePanelBase::set_selection_rect_style);
		ClassDB::bind_method(D_METHOD("get_selection_rect_style"), &TimelinePanelBase::get_selection_rect_style);
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "selection_rect", PROPERTY_HINT_RESOURCE_TYPE, "StyleBox"), "set_selection_rect_style", "get_selection_rect_style");

		ClassDB::bind_method(D_METHOD("set_clip_key_selected_style", "style"), &TimelinePanelBase::set_clip_key_selected_style);
		ClassDB::bind_method(D_METHOD("get_clip_key_selected_style"), &TimelinePanelBase::get_clip_key_selected_style);
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "clip_key_selected", PROPERTY_HINT_RESOURCE_TYPE, "StyleBox"), "set_clip_key_selected_style", "get_clip_key_selected_style");

		ClassDB::bind_method(D_METHOD("set_key_release_preview_style", "style"), &TimelinePanelBase::set_key_release_preview_style);
		ClassDB::bind_method(D_METHOD("get_key_release_preview_style"), &TimelinePanelBase::get_key_release_preview_style);
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

	void TimelinePanelBase::_validate_property(PropertyInfo& p_property) const {
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

	void TimelinePanelBase::_set_panel_orientation(PanelOrientation p_orientation) {
		if (panel_orientation == p_orientation) {
			return;
		}

		panel_orientation = p_orientation;
		queue_redraw();
		update_minimum_size();
	}

	TimelinePanelBase::PanelOrientation TimelinePanelBase::_get_panel_orientation() const {
		return panel_orientation;
	}

	TimelinePanelBase::TimelinePanelBase() {
		set_clip_contents(true);

		hscroll = memnew(HScrollBar);
		hscroll->set_step(0.001);
		hscroll->set_anchor_and_offset(SIDE_LEFT, ANCHOR_BEGIN, 0);
		hscroll->set_anchor_and_offset(SIDE_RIGHT, ANCHOR_END, 0);
		hscroll->set_anchor_and_offset(SIDE_TOP, ANCHOR_END, 0);
		hscroll->set_anchor_and_offset(SIDE_BOTTOM, ANCHOR_END, 0);
		hscroll->hide();
		hscroll->connect("value_changed", callable_mp(this, &TimelinePanelBase::_h_scroll_changed));
		add_child(hscroll, false, INTERNAL_MODE_FRONT);

		vscroll = memnew(VScrollBar);
		vscroll->set_step(0.001);
		vscroll->set_anchor_and_offset(SIDE_LEFT, ANCHOR_END, 0);
		vscroll->set_anchor_and_offset(SIDE_RIGHT, ANCHOR_END, 0);
		vscroll->set_anchor_and_offset(SIDE_TOP, ANCHOR_BEGIN, 0);
		vscroll->set_anchor_and_offset(SIDE_BOTTOM, ANCHOR_END, 0);
		vscroll->hide();
		vscroll->connect("value_changed", callable_mp(this, &TimelinePanelBase::_v_scroll_changed));

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

	TimelinePanelBase::~TimelinePanelBase() {
		clear_all_keys();
	}

}
