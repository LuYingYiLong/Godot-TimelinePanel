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

namespace godot {
	void TimelinePanelBase::_on_resource_changed() {
		_sync_track_cache_geometry();
		_refresh_track_key_metrics();
		queue_redraw();
		update_minimum_size();
	}


	void TimelinePanelBase::_rebuild_track_cache() {
		_track_cache.clear();
		_track_cache.reserve(tracks.size());
		_mark_minimap_key_cache_dirty();

		float current_x = 0.0f;
		if (panel_orientation == PANEL_ORIENTATION_VERTICAL && time_ruler.is_valid()) {
			current_x += time_ruler->get_width();
		}

		for (int i = 0; i < tracks.size(); i++) {
			Ref<TimelineTrack> track = tracks[i];
			const float track_span = panel_orientation == PANEL_ORIENTATION_HORIZONTAL ? _get_horizontal_track_height() : (track.is_valid() ? track->get_width() : 0.0f);
			if (track.is_null()) {
				// 保持索引对齐，放入空槽
				CachedTrack ct;
				ct.x_offset = current_x;
				ct.width = track_span;
				_track_cache.push_back(std::move(ct));
				current_x += track_span;
				continue;
			}

			CachedTrack ct;
			ct.x_offset = current_x;
			ct.width = track_span;
			_track_cache.push_back(std::move(ct));
			current_x += track_span;
		}
	}


	void TimelinePanelBase::_sync_track_cache_geometry() {
		if (_track_cache.size() != static_cast<size_t>(tracks.size())) {
			_rebuild_track_cache();
			return;
		}

		float current_x = 0.0f;
		if (panel_orientation == PANEL_ORIENTATION_VERTICAL && time_ruler.is_valid()) {
			current_x += time_ruler->get_width();
		}

		for (int i = 0; i < tracks.size(); i++) {
			CachedTrack &ct = _track_cache[i];
			Ref<TimelineTrack> track = tracks[i];
			ct.x_offset = current_x;
			ct.width = panel_orientation == PANEL_ORIENTATION_HORIZONTAL ? _get_horizontal_track_height() : (track.is_valid() ? track->get_width() : 0.0f);
			current_x += ct.width;
		}
	}


	void TimelinePanelBase::_update_content_height() {
		content_height = panel_orientation == PANEL_ORIENTATION_HORIZONTAL ? _calculate_track_span() : _calculate_grid_height();
		_update_scroll_bar();
	}


	void TimelinePanelBase::set_background_color(const Color& p_background_color) {
		background_color = p_background_color;
		queue_redraw();
	}


	Color TimelinePanelBase::get_background_color() const {
		return background_color;
	}


	void TimelinePanelBase::set_separator_color(const Color& p_separator_color) {
		separator_color = p_separator_color;
		queue_redraw();
	}


	Color TimelinePanelBase::get_separator_color() const {
		return separator_color;
	}


	void TimelinePanelBase::set_separator_width(const float p_width) {
		separator_width = p_width;
		queue_redraw();
	}


	float TimelinePanelBase::get_separator_width() const {
		return separator_width;
	}


	void TimelinePanelBase::set_header_height(const float p_height) {
		header_height = p_height;
		queue_redraw();
		update_minimum_size();
	}


	float TimelinePanelBase::get_header_height() const {
		return header_height;
	}


	void TimelinePanelBase::set_header_resize_enabled(bool p_enabled) {
		header_resize_enabled = p_enabled;
		if (!header_resize_enabled && header_resizing) {
			_finish_header_resize_drag();
		}
		queue_redraw();
	}


	bool TimelinePanelBase::get_header_resize_enabled() const {
		return header_resize_enabled;
	}


	void TimelinePanelBase::set_duration(const double p_duration) {
		duration = p_duration;
		_calculate_beat_total();
		_calculate_row_total();
		_build_time_to_beat_map();
		_build_beat_to_time_map();
		_update_content_height();
		queue_redraw();
		update_minimum_size();
	}


	double TimelinePanelBase::get_duration() const {
		return duration;
	}


	void TimelinePanelBase::set_current_time(const double p_current_time) {
		current_time = p_current_time;
		queue_redraw();
	}


	double TimelinePanelBase::get_current_time() const {
		return current_time;
	}


	void TimelinePanelBase::set_scale(const float p_scale) {
		scale = p_scale;
		_update_content_height();
		queue_redraw();
		update_minimum_size();
	}


	float TimelinePanelBase::get_scale() const {
		return scale;
	}


	void TimelinePanelBase::set_counting_unit(CountingUnit p_unit) {
		counting_unit = p_unit;
		_update_content_height();
		notify_property_list_changed();
		queue_redraw();
		update_minimum_size();
	}


	TimelinePanelBase::CountingUnit TimelinePanelBase::get_counting_unit() const {
		return counting_unit;
	}


	void TimelinePanelBase::set_time_format(TimeFormat p_time_format) {
		time_format = p_time_format;
		queue_redraw();
	}


	TimelinePanelBase::TimeFormat TimelinePanelBase::get_time_format() const {
		return time_format;
	}


	void TimelinePanelBase::set_show_milliseconds(const bool p_show_milliseconds) {
		show_milliseconds = p_show_milliseconds;
		queue_redraw();
	}


	bool TimelinePanelBase::get_show_milliseconds() const {
		return show_milliseconds;
	}


	void TimelinePanelBase::set_fps(const int p_fps) {
		fps = p_fps;
		_update_content_height();
		queue_redraw();
		update_minimum_size();
	}


	int TimelinePanelBase::get_fps() const {
		return fps;
	}


	void TimelinePanelBase::set_show_subdivision(const bool p_show_subdivision) {
		show_subdivision = p_show_subdivision;
		queue_redraw();
	}


	bool TimelinePanelBase::get_show_subdivision() const {
		return show_subdivision;
	}


	void TimelinePanelBase::set_bpms(const Dictionary& p_bpms) {
		bpms = p_bpms;
		_calculate_beat_total();
		_calculate_row_total();
		_build_time_to_beat_map();
		_build_beat_to_time_map();
		_update_content_height();
		queue_redraw();
		update_minimum_size();
	}


	Dictionary TimelinePanelBase::get_bpms() const {
		return bpms;
	}


	void TimelinePanelBase::set_beat_per_bar(const int p_beats_per_bar) {
		beats_per_bar = p_beats_per_bar;
		_calculate_row_total();
		_update_content_height();
		queue_redraw();
		update_minimum_size();
	}


	int TimelinePanelBase::get_beat_per_bar() const {
		return beats_per_bar;
	}


	void TimelinePanelBase::set_bar_line_color(const Color& p_color) {
		bar_line_color = p_color;
		queue_redraw();
	}


	Color TimelinePanelBase::get_bar_line_color() const {
		return bar_line_color;
	}


	void TimelinePanelBase::set_bar_line_width(const float p_width) {
		bar_line_width = p_width;
		queue_redraw();
	}


	float TimelinePanelBase::get_bar_line_width() const {
		return bar_line_width;
	}


	void TimelinePanelBase::set_beat_line_color(const Color& p_color) {
		beat_line_color = p_color;
		queue_redraw();
	}


	Color TimelinePanelBase::get_beat_line_color() const {
		return beat_line_color;
	}


	void TimelinePanelBase::set_beat_line_width(const float p_width) {
		beat_line_width = p_width;
		queue_redraw();
	}


	float TimelinePanelBase::get_beat_line_width() const {
		return beat_line_width;
	}


	void TimelinePanelBase::set_bar_number_direction(BarNumberDirection p_direction) {
		bar_number_direction = p_direction;
		queue_redraw();
	}


	TimelinePanelBase::BarNumberDirection TimelinePanelBase::get_bar_number_direction() const {
		return bar_number_direction;
	}


	void TimelinePanelBase::set_beat_format(BeatFormat p_format) {
		beat_format = p_format;
		queue_redraw();
	}


	TimelinePanelBase::BeatFormat TimelinePanelBase::get_beat_format() const {
		return beat_format;
	}


	void TimelinePanelBase::set_playhead(Ref<TimelineIndicator> p_playhead) {
		playhead = p_playhead;
		if (playhead.is_valid()) {
			playhead->connect("changed", callable_mp(this, &TimelinePanelBase::_on_resource_changed));
		}
		queue_redraw();
	}


	Ref<TimelineIndicator> TimelinePanelBase::get_playhead() const {
		return playhead;
	}


	void TimelinePanelBase::set_markers(const TypedArray<TimelineMarker>& p_markers) {
		markers = p_markers;
		for (int i = 0; i < markers.size(); i++) {
			Ref<TimelineMarker> marker = markers[i];
			if (marker.is_valid() && !marker->is_connected("changed", callable_mp(this, &TimelinePanelBase::_on_resource_changed))) {
				marker->connect("changed", callable_mp(this, &TimelinePanelBase::_on_resource_changed));
			}
		}
	}


	TypedArray<TimelineMarker> TimelinePanelBase::get_markers() const {
		return markers;
	}


	void TimelinePanelBase::set_time_ruler(Ref<TimelineTimeRuler> p_time_ruler) {
		time_ruler = p_time_ruler;
		if (time_ruler.is_valid()) {
			time_ruler->connect("changed", callable_mp(this, &TimelinePanelBase::_on_resource_changed));
		}
		queue_redraw();
		update_minimum_size();
	}


	Ref<TimelineTimeRuler> TimelinePanelBase::get_time_ruler() const {
		return time_ruler;
	}


	void TimelinePanelBase::set_tracks(const TypedArray<TimelineTrack>& p_tracks) {
		clear_all_keys();
		tracks = p_tracks;
		_rebuild_track_cache();
		for (int i = 0; i < tracks.size(); i++) {
			Ref<TimelineTrack> track = tracks[i];
			if (track.is_valid() && !track->is_connected("changed", callable_mp(this, &TimelinePanelBase::_on_resource_changed))) {
				track->connect("changed", callable_mp(this, &TimelinePanelBase::_on_resource_changed));
			}
		}
		queue_redraw();
		update_minimum_size();
	}


	TypedArray<TimelineTrack> TimelinePanelBase::get_tracks() const {
		return tracks;
	}


	void TimelinePanelBase::set_middle_mouse_pan_enabled(bool p_enabled) {
		middle_mouse_pan_enabled = p_enabled;
		if (!middle_mouse_pan_enabled && middle_mouse_panning) {
			_finish_middle_mouse_pan(get_local_mouse_position());
		}
	}


	bool TimelinePanelBase::get_middle_mouse_pan_enabled() const {
		return middle_mouse_pan_enabled;
	}


	void TimelinePanelBase::set_icon_max_width(const float p_width) {
		style_cache.icon_max_width = p_width;
		queue_redraw();
	}


	float TimelinePanelBase::get_icon_max_width() const {
		return style_cache.icon_max_width;
	}


	void TimelinePanelBase::set_allow_key_cross_track_move(bool p_enabled) {
		allow_key_cross_track_move = p_enabled;
	}


	bool TimelinePanelBase::get_allow_key_cross_track_move() const {
		return allow_key_cross_track_move;
	}


	void TimelinePanelBase::set_key_snap_enabled(bool p_enabled) {
		key_snap_enabled = p_enabled;
	}


	bool TimelinePanelBase::get_key_snap_enabled() const {
		return key_snap_enabled;
	}


	void TimelinePanelBase::set_clip_key_edge_edit_enabled(bool p_enabled) {
		clip_key_edge_edit_enabled = p_enabled;
		if (!clip_key_edge_edit_enabled && !clip_key_edge_dragging) {
			set_default_cursor_shape(Control::CURSOR_ARROW);
		}
	}


	bool TimelinePanelBase::get_clip_key_edge_edit_enabled() const {
		return clip_key_edge_edit_enabled;
	}


	void TimelinePanelBase::set_allow_unselected_key_edit(bool p_enabled) {
		allow_unselected_key_edit = p_enabled;
	}


	bool TimelinePanelBase::get_allow_unselected_key_edit() const {
		return allow_unselected_key_edit;
	}


	void TimelinePanelBase::set_allow_right_mouse_selection(bool p_enabled) {
		allow_right_mouse_selection = p_enabled;
		if (!allow_right_mouse_selection && right_selecting) {
			right_selecting = false;
			queue_redraw();
			_stop_internal_process_if_idle();
		}
	}


	bool TimelinePanelBase::get_allow_right_mouse_selection() const {
		return allow_right_mouse_selection;
	}

}
