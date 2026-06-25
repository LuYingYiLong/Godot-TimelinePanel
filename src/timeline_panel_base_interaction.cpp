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
#include <godot_cpp/variant/dictionary.hpp>

namespace godot {
	void TimelinePanelBase::_collect_selected_keys() {
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


	Rect2 TimelinePanelBase::_make_selection_rect(const Vector2 &p_start, const Vector2 &p_end) const {
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


	TypedArray<TimelineTrackKey> TimelinePanelBase::_get_keys_in_rect(const Rect2 &p_rect) const {
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


	bool TimelinePanelBase::_try_handle_selection_rect(const Rect2 &p_rect, const TypedArray<TimelineTrackKey> &p_keys, int p_mouse_button) {
		bool should_handle = false;
		if (GDVIRTUAL_CALL(_should_handle_selection_rect, p_rect, p_keys, p_mouse_button, should_handle) && should_handle) {
			GDVIRTUAL_CALL(_handle_selection_rect, p_rect, p_keys, p_mouse_button);
			return true;
		}
		return false;
	}


	void TimelinePanelBase::_finish_right_mouse_selection() {
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


	bool TimelinePanelBase::_find_selected_key_at_position(const Vector2& p_position, int& r_track_index, TimelineTrackKey*& r_key) const {
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


	int TimelinePanelBase::_get_track_index_at_x(float p_x) const {
		int last_valid = -1;

		for (int i = 0; i < static_cast<int>(_track_cache.size()); i++) {
			const CachedTrack& ct = _track_cache[i];
			if (ct.width <= 0.0f) continue;

			const float left = panel_orientation == PANEL_ORIENTATION_HORIZONTAL ? _get_horizontal_ruler_height() + ct.x_offset - vscroll_value : ct.x_offset - hscroll_value;
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


	int TimelinePanelBase::_get_track_header_index_at_x(float p_x) const {
		for (int i = 0; i < static_cast<int>(_track_cache.size()); i++) {
			const CachedTrack &ct = _track_cache[i];
			if (ct.width <= 0.0f) continue;

			const float left = panel_orientation == PANEL_ORIENTATION_HORIZONTAL ? _get_horizontal_ruler_height() + ct.x_offset - vscroll_value : ct.x_offset - hscroll_value;
			const float right = left + ct.width;
			if (p_x >= left && p_x <= right) {
				return i;
			}
		}

		return -1;
	}


	void TimelinePanelBase::_select_track_keys(int p_track_index) {
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


	void TimelinePanelBase::_update_selection_auto_scroll(double p_delta) {
		if (!selecting && !right_selecting) {
			return;
		}

		Vector2 *selection_start = right_selecting ? &right_select_start : &select_start;
		const Vector2 *selection_end = right_selecting ? &right_select_end : &select_end;
		const float max_speed = 720.0f;
		bool changed = false;

		if (vscroll != nullptr && vscroll->is_visible() && vertical_scroll_mode != SCROLL_MODE_DISABLED) {
			const float hscroll_height = hscroll != nullptr && hscroll->is_visible() ? hscroll->get_combined_minimum_size().y : 0.0f;
			const float top = panel_orientation == PANEL_ORIENTATION_HORIZONTAL ? _get_horizontal_ruler_height() : header_height;
			const float bottom = get_size().y - hscroll_height;
			if (bottom > top) {
				const float margin = CLAMP((bottom - top) * 0.12f, 24.0f, 64.0f);
				float speed = 0.0f;
				if (selection_end->y < top + margin) {
					speed = -CLAMP((top + margin - selection_end->y) / margin, 0.0f, 1.0f) * max_speed;
				}
				else if (selection_end->y > bottom - margin) {
					speed = CLAMP((selection_end->y - (bottom - margin)) / margin, 0.0f, 1.0f) * max_speed;
				}

				if (speed != 0.0f) {
					const double before = vscroll->get_value();
					_scroll(vscroll, speed * p_delta);
					const double after = vscroll->get_value();
					if (!Math::is_equal_approx(before, after)) {
						selection_start->y -= static_cast<float>(after - before);
						changed = true;
					}
				}
			}
		}

		if (hscroll != nullptr && hscroll->is_visible() && horizontal_scroll_mode != SCROLL_MODE_DISABLED) {
			const float vscroll_width = vscroll != nullptr && vscroll->is_visible() ? vscroll->get_combined_minimum_size().x : 0.0f;
			const float minimap_visible_width = _is_minimap_visible() ? static_cast<float>(minimap_width) : 0.0f;
			const float left = panel_orientation == PANEL_ORIENTATION_HORIZONTAL ? _get_horizontal_track_header_width() : 0.0f;
			const float right = get_size().x - vscroll_width - minimap_visible_width;
			if (right > left) {
				const float margin = CLAMP((right - left) * 0.12f, 24.0f, 64.0f);
				float speed = 0.0f;
				if (selection_end->x < left + margin) {
					speed = -CLAMP((left + margin - selection_end->x) / margin, 0.0f, 1.0f) * max_speed;
				}
				else if (selection_end->x > right - margin) {
					speed = CLAMP((selection_end->x - (right - margin)) / margin, 0.0f, 1.0f) * max_speed;
				}

				if (speed != 0.0f) {
					const double before = hscroll->get_value();
					_scroll(hscroll, speed * p_delta);
					const double after = hscroll->get_value();
					if (!Math::is_equal_approx(before, after)) {
						selection_start->x -= static_cast<float>(after - before);
						changed = true;
					}
				}
			}
		}

		if (changed) {
			queue_redraw();
		}
	}


	void TimelinePanelBase::_update_key_drag_auto_scroll(double p_delta) {
		if (!key_dragging && !clip_key_edge_dragging) {
			return;
		}

		const Vector2 mouse_position = get_local_mouse_position();
		const float max_speed = 720.0f;
		bool changed = false;

		if (vscroll != nullptr && vscroll->is_visible() && vertical_scroll_mode != SCROLL_MODE_DISABLED) {
			const float hscroll_height = hscroll != nullptr && hscroll->is_visible() ? hscroll->get_combined_minimum_size().y : 0.0f;
			const float top = panel_orientation == PANEL_ORIENTATION_HORIZONTAL ? _get_horizontal_ruler_height() : header_height;
			const float bottom = get_size().y - hscroll_height;
			if (bottom > top) {
				const float margin = CLAMP((bottom - top) * 0.12f, 24.0f, 64.0f);
				float speed = 0.0f;
				if (mouse_position.y < top + margin) {
					speed = -CLAMP((top + margin - mouse_position.y) / margin, 0.0f, 1.0f) * max_speed;
				}
				else if (mouse_position.y > bottom - margin) {
					speed = CLAMP((mouse_position.y - (bottom - margin)) / margin, 0.0f, 1.0f) * max_speed;
				}

				if (speed != 0.0f) {
					const double before = vscroll->get_value();
					_scroll(vscroll, speed * p_delta);
					changed = changed || !Math::is_equal_approx(before, vscroll->get_value());
				}
			}
		}

		if (hscroll != nullptr && hscroll->is_visible() && horizontal_scroll_mode != SCROLL_MODE_DISABLED) {
			const float vscroll_width = vscroll != nullptr && vscroll->is_visible() ? vscroll->get_combined_minimum_size().x : 0.0f;
			const float minimap_visible_width = _is_minimap_visible() ? static_cast<float>(minimap_width) : 0.0f;
			const float left = panel_orientation == PANEL_ORIENTATION_HORIZONTAL ? _get_horizontal_track_header_width() : 0.0f;
			const float right = get_size().x - vscroll_width - minimap_visible_width;
			if (right > left) {
				const float margin = CLAMP((right - left) * 0.12f, 24.0f, 64.0f);
				float speed = 0.0f;
				if (mouse_position.x < left + margin) {
					speed = -CLAMP((left + margin - mouse_position.x) / margin, 0.0f, 1.0f) * max_speed;
				}
				else if (mouse_position.x > right - margin) {
					speed = CLAMP((mouse_position.x - (right - margin)) / margin, 0.0f, 1.0f) * max_speed;
				}

				if (speed != 0.0f) {
					const double before = hscroll->get_value();
					_scroll(hscroll, speed * p_delta);
					changed = changed || !Math::is_equal_approx(before, hscroll->get_value());
				}
			}
		}

		if (!changed) {
			return;
		}

		if (clip_key_edge_dragging) {
			_update_clip_key_edge_drag(mouse_position);
		}
		else if (key_dragging) {
			_update_key_drag(mouse_position);
		}
	}


	void TimelinePanelBase::_stop_internal_process_if_idle() {
		if (!select_pending && !selecting && !right_selecting && !drag_touching && !drag_touching_deaccel && !key_dragging && !clip_key_edge_dragging) {
			set_process_internal(false);
		}
	}


	double TimelinePanelBase::_position_to_key_value(double p_y) const {
		if (counting_unit == FRAME) {
			return static_cast<double>(_y_to_frame(p_y));
		}
		return _y_to_time(p_y);
	}


	double TimelinePanelBase::_get_playhead_drag_time(double p_y) const {
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


	bool TimelinePanelBase::_find_header_resize_edge_at_position(const Vector2 &p_position, HeaderResizeTarget &r_target, int &r_track_index) const {
		r_target = HEADER_RESIZE_TARGET_NONE;
		r_track_index = -1;

		if (!header_resize_enabled) {
			return false;
		}

		const float edge_margin = 5.0f;
		const float viewport_width = get_size().x;
		if (p_position.x < -edge_margin || p_position.x > viewport_width + edge_margin) {
			return false;
		}

		float best_distance = edge_margin + 1.0f;
		auto test_edge = [&](float p_edge_x, HeaderResizeTarget p_target, int p_track_index) {
			if (p_edge_x < -edge_margin || p_edge_x > viewport_width + edge_margin) {
				return;
			}

			const float distance = Math::abs(p_position.x - p_edge_x);
			if (distance <= edge_margin && distance < best_distance) {
				best_distance = distance;
				r_target = p_target;
				r_track_index = p_track_index;
			}
		};

		if (panel_orientation == PANEL_ORIENTATION_HORIZONTAL) {
			const float ruler_height = _get_horizontal_ruler_height();
			if (p_position.y <= ruler_height) {
				return false;
			}

			test_edge(_get_horizontal_track_header_width(), HEADER_RESIZE_TARGET_HEADER_WIDTH, -1);
			return r_target != HEADER_RESIZE_TARGET_NONE;
		}

		if (p_position.y < 0.0f || p_position.y > header_height) {
			return false;
		}

		if (time_ruler.is_valid() && time_ruler->get_width() > 0.0f) {
			test_edge(-hscroll_value + time_ruler->get_width(), HEADER_RESIZE_TARGET_TIME_RULER, -1);
		}

		for (int i = 0; i < static_cast<int>(_track_cache.size()) && i < static_cast<int>(tracks.size()); i++) {
			const CachedTrack &ct = _track_cache[i];
			if (ct.width <= 0.0f) continue;

			test_edge(ct.x_offset - hscroll_value + ct.width, HEADER_RESIZE_TARGET_TRACK, i);
		}

		return r_target != HEADER_RESIZE_TARGET_NONE;
	}


	bool TimelinePanelBase::_update_header_resize_cursor(const Vector2 &p_position) {
		if (header_resizing) {
			set_default_cursor_shape(Control::CURSOR_HSIZE);
			return true;
		}

		HeaderResizeTarget target = HEADER_RESIZE_TARGET_NONE;
		int track_index = -1;
		if (_find_header_resize_edge_at_position(p_position, target, track_index)) {
			set_default_cursor_shape(Control::CURSOR_HSIZE);
			return true;
		}

		return false;
	}


	void TimelinePanelBase::_begin_header_resize_drag(HeaderResizeTarget p_target, int p_track_index, const Vector2 &p_position) {
		if (p_target == HEADER_RESIZE_TARGET_NONE) {
			return;
		}

		float target_width = 0.0f;
		switch (p_target) {
		case HEADER_RESIZE_TARGET_TIME_RULER: {
			if (time_ruler.is_null()) {
				return;
			}
			target_width = time_ruler->get_width();
		} break;
		case HEADER_RESIZE_TARGET_TRACK: {
			if (p_track_index < 0 || p_track_index >= static_cast<int>(tracks.size())) {
				return;
			}
			target_width = tracks[p_track_index].height;
		} break;
		case HEADER_RESIZE_TARGET_HEADER_WIDTH: {
			target_width = _get_horizontal_track_header_width();
		} break;
		case HEADER_RESIZE_TARGET_NONE:
		default:
			return;
		}

		_cancel_drag();
		selecting = false;
		select_pending = false;
		right_selecting = false;
		header_resizing = true;
		header_resize_target = p_target;
		header_resize_track_index = p_track_index;
		header_resize_start_mouse_x = p_position.x;
		header_resize_start_width = target_width;
		set_default_cursor_shape(Control::CURSOR_HSIZE);
	}


	void TimelinePanelBase::_update_header_resize_drag(const Vector2 &p_position) {
		if (!header_resizing) {
			return;
		}

		const float min_width = 8.0f;
		const float width = MAX(header_resize_start_width + p_position.x - header_resize_start_mouse_x, min_width);
		bool changed = false;

		switch (header_resize_target) {
		case HEADER_RESIZE_TARGET_TIME_RULER: {
			if (time_ruler.is_valid() && !Math::is_equal_approx(time_ruler->get_width(), width)) {
				time_ruler->set_width(width);
				changed = true;
			}
		} break;
		case HEADER_RESIZE_TARGET_TRACK: {
			if (header_resize_track_index >= 0 && header_resize_track_index < static_cast<int>(tracks.size())) {
				if (!Math::is_equal_approx(tracks[header_resize_track_index].height, width)) {
					tracks[header_resize_track_index].height = width;
					changed = true;
				}
			}
		} break;
		case HEADER_RESIZE_TARGET_HEADER_WIDTH: {
			if (!Math::is_equal_approx(header_height, width)) {
				header_height = width;
				changed = true;
			}
		} break;
		case HEADER_RESIZE_TARGET_NONE:
		default:
			break;
		}

		if (!changed) {
			return;
		}

		_sync_track_cache_geometry();
		_update_scroll_bar();
		update_minimum_size();
		queue_redraw();
	}


	void TimelinePanelBase::_finish_header_resize_drag() {
		if (!header_resizing) {
			return;
		}

		header_resizing = false;
		header_resize_target = HEADER_RESIZE_TARGET_NONE;
		header_resize_track_index = -1;
		header_resize_start_mouse_x = 0.0f;
		header_resize_start_width = 0.0f;
		if (!_update_header_resize_cursor(get_local_mouse_position())) {
			set_default_cursor_shape(Control::CURSOR_ARROW);
		}
	}


	bool TimelinePanelBase::_find_clip_key_edge_at_position(const Vector2 &p_position, int &r_track_index, TimelineTrackKey *&r_key, ClipKeyEditEdge &r_edge) const {
		r_track_index = -1;
		r_key = nullptr;
		r_edge = CLIP_KEY_EDIT_EDGE_NONE;

		const float ruler_height = panel_orientation == PANEL_ORIENTATION_HORIZONTAL ? _get_horizontal_ruler_height() : header_height;
		if (!clip_key_edge_edit_enabled || p_position.y <= ruler_height) {
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
				if (panel_orientation == PANEL_ORIENTATION_HORIZONTAL) {
					if (p_position.y < key_rect.position.y || p_position.y > key_rect.position.y + key_rect.size.y) continue;
				}
				else {
					if (p_position.x < key_rect.position.x || p_position.x > key_rect.position.x + key_rect.size.x) continue;
				}

				const float axis_position = panel_orientation == PANEL_ORIENTATION_HORIZONTAL ? p_position.x : p_position.y;
				const float head_distance = Math::abs(axis_position - static_cast<float>(head_y));
				const float tail_distance = Math::abs(axis_position - static_cast<float>(tail_y));
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


	void TimelinePanelBase::_update_clip_key_edge_cursor(const Vector2 &p_position) {
		const Control::CursorShape edge_cursor = panel_orientation == PANEL_ORIENTATION_HORIZONTAL ? Control::CURSOR_HSPLIT : Control::CURSOR_VSPLIT;
		if (clip_key_edge_dragging) {
			set_default_cursor_shape(edge_cursor);
			return;
		}

		int hit_track_index = -1;
		TimelineTrackKey *hit_key = nullptr;
		ClipKeyEditEdge hit_edge = CLIP_KEY_EDIT_EDGE_NONE;
		if (_find_clip_key_edge_at_position(p_position, hit_track_index, hit_key, hit_edge)) {
			set_default_cursor_shape(edge_cursor);
		}
		else {
			set_default_cursor_shape(Control::CURSOR_ARROW);
		}
	}


	void TimelinePanelBase::_begin_clip_key_edge_drag(TimelineTrackKey *p_key, ClipKeyEditEdge p_edge) {
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

		set_default_cursor_shape(panel_orientation == PANEL_ORIENTATION_HORIZONTAL ? Control::CURSOR_HSPLIT : Control::CURSOR_VSPLIT);
		set_process_internal(true);
	}


	void TimelinePanelBase::_update_clip_key_edge_drag(const Vector2 &p_position) {
		if (!clip_key_edge_dragging || !clip_key_edge_drag_key || resized_clip_keys.empty()) {
			return;
		}

		double dragged_time = _position_to_key_value(panel_orientation == PANEL_ORIENTATION_HORIZONTAL ? p_position.x : p_position.y);
		if (key_snap_enabled || clip_key_edge_snap_enabled) {
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


	void TimelinePanelBase::_finish_clip_key_edge_drag() {
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
		_stop_internal_process_if_idle();
	}


	void TimelinePanelBase::_begin_key_drag(int p_track_index, TimelineTrackKey* p_key, const Vector2& p_position, bool p_duplicate) {
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
		key_drag_duplicating = false;
		key_drag_start_value = _position_to_key_value(panel_orientation == PANEL_ORIENTATION_HORIZONTAL ? p_position.x : p_position.y);
		key_drag_anchor_track = p_track_index;
		dragged_keys.clear();

		if (p_duplicate && key_alt_duplicate_drag_enabled) {
			std::vector<DraggedKey> duplicate_dragged_keys;
			for (int i = 0; i < static_cast<int>(_track_cache.size()); i++) {
				CachedTrack &ct = _track_cache[i];
				const std::vector<TimelineTrackKey *> source_keys = ct.keys;
				for (TimelineTrackKey *key : source_keys) {
					if (!key || key->is_disabled() || !key->is_selected()) continue;

					TimelineTrackKey *duplicate_key = _duplicate_key_for_drag(key, i);
					if (!duplicate_key) continue;

					DraggedKey dragged_key;
					dragged_key.key = duplicate_key;
					dragged_key.original_track_index = i;
					dragged_key.current_track_index = i;
					dragged_key.original_time = duplicate_key->get_time();
					duplicate_dragged_keys.push_back(dragged_key);
				}
			}

			if (!duplicate_dragged_keys.empty()) {
				for (CachedTrack &ct : _track_cache) {
					for (TimelineTrackKey *key : ct.keys) {
						if (key) {
							key->set_selected_no_signal(false);
						}
					}
				}
				for (DraggedKey &dragged_key : duplicate_dragged_keys) {
					if (dragged_key.key) {
						dragged_key.key->set_selected_no_signal(true);
					}
				}
				dragged_keys = duplicate_dragged_keys;
				key_drag_duplicating = true;
				_mark_minimap_key_cache_dirty();
				_refresh_track_key_metrics();
				queue_redraw();
			}
		}

		if (dragged_keys.empty()) {
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
		}

		if (dragged_keys.empty()) {
			DraggedKey dragged_key;
			dragged_key.key = p_key;
			dragged_key.original_track_index = p_track_index;
			dragged_key.current_track_index = p_track_index;
			dragged_key.original_time = p_key->get_time();
			dragged_keys.push_back(dragged_key);
		}
		set_process_internal(true);
	}


	void TimelinePanelBase::_update_key_drag(const Vector2& p_position) {
		if (!key_dragging || dragged_keys.empty()) {
			return;
		}

		const double current_value = _position_to_key_value(panel_orientation == PANEL_ORIENTATION_HORIZONTAL ? p_position.x : p_position.y);
		const double value_delta = current_value - key_drag_start_value;
		const int target_anchor_track = allow_key_cross_track_move ? _get_track_index_at_x(panel_orientation == PANEL_ORIENTATION_HORIZONTAL ? p_position.y : p_position.x) : key_drag_anchor_track;
		int track_delta = target_anchor_track >= 0 && key_drag_anchor_track >= 0 ? target_anchor_track - key_drag_anchor_track : 0;
		if (allow_key_cross_track_move && track_delta != 0) {
			for (const DraggedKey& dragged_key : dragged_keys) {
				if (!dragged_key.key) continue;
				const int target_track = CLAMP(dragged_key.original_track_index + track_delta, 0, static_cast<int>(_track_cache.size()) - 1);
				if (!_can_move_key_to_track(dragged_key.key, target_track)) {
					track_delta = 0;
					break;
				}
			}
		}
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


	void TimelinePanelBase::_finish_key_drag() {
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
		const bool was_duplicating = key_drag_duplicating;
		key_drag_duplicating = false;
		dragged_keys.clear();

		if (was_duplicating && !key_drag_moved) {
			_remove_drag_duplicate_keys(moved_keys);
		}
		else if (key_drag_moved) {
			_destroy_moved_key_overlaps(moved_keys);
		}
		_clear_key_release_preview();
		key_drag_moved = false;
		_refresh_track_key_metrics();
		queue_redraw();
		_stop_internal_process_if_idle();
	}


	void TimelinePanelBase::_cancel_drag() {
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


	void TimelinePanelBase::_begin_middle_mouse_pan(const Vector2 &p_position) {
		if (!middle_mouse_pan_enabled || (panel_orientation == PANEL_ORIENTATION_HORIZONTAL ? (p_position.x <= _get_horizontal_track_header_width() || p_position.y <= _get_horizontal_ruler_height()) : p_position.y <= header_height)) {
			return;
		}

		if (header_resizing || minimap_dragging || clip_key_edge_dragging || key_dragging || playhead_dragging || selecting || right_selecting || select_pending) {
			return;
		}

		_cancel_drag();
		middle_mouse_panning = true;
		middle_mouse_pan_last_position = p_position;
		set_default_cursor_shape(Control::CURSOR_CROSS);
	}


	void TimelinePanelBase::_update_middle_mouse_pan(const Vector2 &p_position) {
		if (!middle_mouse_panning) {
			return;
		}

		Vector2 current_position = p_position;
		const Vector2 motion = current_position - middle_mouse_pan_last_position;
		if (hscroll != nullptr && hscroll->is_visible() && horizontal_scroll_mode != SCROLL_MODE_DISABLED) {
			_scroll(hscroll, -motion.x);
		}
		if (vscroll != nullptr && vscroll->is_visible() && vertical_scroll_mode != SCROLL_MODE_DISABLED) {
			_scroll(vscroll, -motion.y);
		}

		middle_mouse_pan_last_position = current_position;
		_wrap_middle_mouse_pan_position(current_position);
		middle_mouse_pan_last_position = current_position;
	}


	void TimelinePanelBase::_finish_middle_mouse_pan(const Vector2 &p_position) {
		if (!middle_mouse_panning) {
			return;
		}

		middle_mouse_panning = false;
		middle_mouse_pan_last_position = Vector2();
		if (!_update_header_resize_cursor(p_position)) {
			_update_clip_key_edge_cursor(p_position);
		}
	}


	bool TimelinePanelBase::_wrap_middle_mouse_pan_position(Vector2 &r_position) {
		const Size2 size = get_size();
		if (size.x <= 0.0f || size.y <= 0.0f) {
			return false;
		}

		const float margin = 2.0f;
		const float left_wrap = MIN(margin, size.x * 0.5f);
		const float right_wrap = MAX(size.x - margin, size.x * 0.5f);
		const float top_wrap = MIN(margin, size.y * 0.5f);
		const float bottom_wrap = MAX(size.y - margin, size.y * 0.5f);
		Vector2 wrapped_position = r_position;

		if (r_position.x < 0.0f) {
			wrapped_position.x = right_wrap;
		}
		else if (r_position.x > size.x) {
			wrapped_position.x = left_wrap;
		}

		if (r_position.y < 0.0f) {
			wrapped_position.y = bottom_wrap;
		}
		else if (r_position.y > size.y) {
			wrapped_position.y = top_wrap;
		}

		if (wrapped_position == r_position) {
			return false;
		}

		warp_mouse(wrapped_position);
		r_position = wrapped_position;
		return true;
	}


	void TimelinePanelBase::_gui_input(const Ref<InputEvent>& p_gui_input) {
		ERR_FAIL_COND(p_gui_input.is_null());

		_sync_track_cache_geometry();

		double prev_v_scroll = vscroll->get_value();
		double prev_h_scroll = hscroll->get_value();
		bool h_scroll_enabled = horizontal_scroll_mode != SCROLL_MODE_DISABLED;
		bool v_scroll_enabled = vertical_scroll_mode != SCROLL_MODE_DISABLED;
		auto is_time_ruler_position_at = [&](const Vector2 &p_position) {
			if (time_ruler.is_null()) {
				return false;
			}
			if (panel_orientation == PANEL_ORIENTATION_HORIZONTAL) {
				return p_position.y >= 0.0f && p_position.y <= _get_horizontal_ruler_height() && p_position.x >= _get_horizontal_track_header_width();
			}
			const float time_ruler_left = -hscroll_value;
			const float time_ruler_right = time_ruler_left + time_ruler->get_width();
			return p_position.x >= time_ruler_left && p_position.x <= time_ruler_right;
		};
		auto is_track_header_position_at = [&](const Vector2 &p_position) {
			if (panel_orientation == PANEL_ORIENTATION_HORIZONTAL) {
				return p_position.x >= 0.0f && p_position.x <= _get_horizontal_track_header_width() && p_position.y > _get_horizontal_ruler_height();
			}
			return p_position.y >= 0.0f && p_position.y <= header_height && !is_time_ruler_position_at(p_position);
		};
		auto is_content_position_at = [&](const Vector2 &p_position) {
			if (panel_orientation == PANEL_ORIENTATION_HORIZONTAL) {
				return p_position.x > _get_horizontal_track_header_width() && p_position.y > _get_horizontal_ruler_height();
			}
			return p_position.y > header_height;
		};

		Ref<InputEventMouseButton> mb = p_gui_input;

		if (mb.is_valid()) {
			if (mb->is_pressed()) {
				bool scroll_value_modified = false;

				bool v_scroll_hidden = !vscroll->is_visible() && vertical_scroll_mode != SCROLL_MODE_SHOW_NEVER;
				bool h_scroll_hidden = !hscroll->is_visible() && horizontal_scroll_mode != SCROLL_MODE_SHOW_NEVER;
				const bool horizontal_panel = panel_orientation == PANEL_ORIENTATION_HORIZONTAL;
				const bool vertical_wheel_as_horizontal = horizontal_panel && !mb->is_shift_pressed();
				const bool vertical_wheel_as_vertical = !horizontal_panel || mb->is_shift_pressed();
				if (mb->get_button_index() == MouseButton::MOUSE_BUTTON_WHEEL_UP) {
					if ((vertical_wheel_as_horizontal && h_scroll_enabled) || v_scroll_hidden) {
						_scroll(hscroll, -hscroll->get_page() / 8 * mb->get_factor());
						scroll_value_modified = true;
					}
					else if (vertical_wheel_as_vertical && v_scroll_enabled) {
						_scroll(vscroll, -vscroll->get_page() / 8 * mb->get_factor());
						scroll_value_modified = true;
					}
				}
				if (mb->get_button_index() == MouseButton::MOUSE_BUTTON_WHEEL_DOWN) {
					if ((vertical_wheel_as_horizontal && h_scroll_enabled) || v_scroll_hidden) {
						_scroll(hscroll, hscroll->get_page() / 8 * mb->get_factor());
						scroll_value_modified = true;
					}
					else if (vertical_wheel_as_vertical && v_scroll_enabled) {
						_scroll(vscroll, vscroll->get_page() / 8 * mb->get_factor());
						scroll_value_modified = true;
					}
				}

				if (mb->get_button_index() == MouseButton::MOUSE_BUTTON_WHEEL_LEFT) {
					if ((mb->is_shift_pressed() && v_scroll_enabled) || h_scroll_hidden) {
						_scroll(vscroll, -vscroll->get_page() / 8 * mb->get_factor());
						scroll_value_modified = true;
					}
					else if (h_scroll_enabled) {
						_scroll(hscroll, -hscroll->get_page() / 8 * mb->get_factor());
						scroll_value_modified = true;
					}
				}
				if (mb->get_button_index() == MouseButton::MOUSE_BUTTON_WHEEL_RIGHT) {
					if ((mb->is_shift_pressed() && v_scroll_enabled) || h_scroll_hidden) {
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

			if (mb->get_button_index() == MouseButton::MOUSE_BUTTON_MIDDLE) {
				if (!mb->is_pressed() && middle_mouse_panning) {
					_finish_middle_mouse_pan(mb->get_position());
					accept_event();
					return;
				}

				if (mb->is_pressed()) {
					_begin_middle_mouse_pan(mb->get_position());
					if (middle_mouse_panning) {
						accept_event();
						return;
					}
				}
			}

			if (mb->get_button_index() == MouseButton::MOUSE_BUTTON_RIGHT) {
				const Vector2 mouse_position = mb->get_position();
				const bool is_time_ruler_position = is_time_ruler_position_at(mouse_position);

				if (!mb->is_pressed() && right_selecting) {
					_finish_right_mouse_selection();
					accept_event();
					return;
				}

				if (mb->is_pressed() && is_time_ruler_position) {
					emit_signal("time_ruler_right_clicked", get_time_from_position(panel_orientation == PANEL_ORIENTATION_HORIZONTAL ? mouse_position.x : mouse_position.y), mouse_position);
					accept_event();
					return;
				}

				if (mb->is_pressed() && is_track_header_position_at(mouse_position)) {
					int track_index = -1;
					int column = -1;
					int button_index = -1;
					if (_find_track_header_button_at_position(mouse_position, track_index, column, button_index)) {
						const TrackHeaderColumn *header_column = track_index >= 0 && track_index < static_cast<int>(tracks.size()) ? _get_track_column(tracks[track_index], column) : nullptr;
						if (header_column != nullptr && button_index >= 0 && button_index < static_cast<int>(header_column->buttons.size()) && !header_column->buttons[button_index].disabled) {
							emit_signal("track_header_button_clicked", track_index, column, header_column->buttons[button_index].id, button_index, static_cast<int>(MouseButton::MOUSE_BUTTON_RIGHT), mouse_position);
							accept_event();
							return;
						}
					}

					track_index = _get_track_header_index_at_x(panel_orientation == PANEL_ORIENTATION_HORIZONTAL ? mouse_position.y : mouse_position.x);
					if (track_index >= 0) {
						column = _get_track_header_column_at_position(track_index, mouse_position);
						emit_signal("track_header_right_clicked", track_index, column, mouse_position);
						accept_event();
						return;
					}
				}

				if (mb->is_pressed() && allow_right_mouse_selection &&
					is_content_position_at(mouse_position) && !_get_minimap_rect().has_point(mouse_position)) {
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
				if (!mb->is_pressed() && header_resizing) {
					_finish_header_resize_drag();
					accept_event();
					return;
				}

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
				const bool is_time_ruler_position = is_time_ruler_position_at(mouse_position);
				if (mb->is_pressed()) {
					HeaderResizeTarget header_resize_hit = HEADER_RESIZE_TARGET_NONE;
					int header_resize_hit_track = -1;
					if (_find_header_resize_edge_at_position(mouse_position, header_resize_hit, header_resize_hit_track)) {
						_begin_header_resize_drag(header_resize_hit, header_resize_hit_track, mouse_position);
						accept_event();
						return;
					}
				}

				if (mb->is_pressed() && _begin_minimap_drag(mouse_position)) {
					accept_event();
					return;
				}

				if (!mb->is_pressed() && playhead_dragging) {
					playhead_dragging = false;
					accept_event();
					return;
				}

				if (mb->is_pressed() && is_track_header_position_at(mouse_position)) {
					int track_index = -1;
					int column = -1;
					int button_index = -1;
					if (_find_track_header_button_at_position(mouse_position, track_index, column, button_index)) {
						const TrackHeaderColumn *header_column = track_index >= 0 && track_index < static_cast<int>(tracks.size()) ? _get_track_column(tracks[track_index], column) : nullptr;
						if (header_column != nullptr && button_index >= 0 && button_index < static_cast<int>(header_column->buttons.size()) && !header_column->buttons[button_index].disabled) {
							emit_signal("track_header_button_clicked", track_index, column, header_column->buttons[button_index].id, button_index, static_cast<int>(MouseButton::MOUSE_BUTTON_LEFT), mouse_position);
							accept_event();
							return;
						}
					}

					track_index = _get_track_header_index_at_x(panel_orientation == PANEL_ORIENTATION_HORIZONTAL ? mouse_position.y : mouse_position.x);
					if (track_index >= 0) {
						column = _get_track_header_column_at_position(track_index, mouse_position);
						emit_signal("track_header_clicked", track_index, column, mouse_position);
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
					set_current_time(_get_playhead_drag_time(panel_orientation == PANEL_ORIENTATION_HORIZONTAL ? mouse_position.x : mouse_position.y));
					accept_event();
					return;
				}

				if (mb->is_pressed() && is_content_position_at(mb->get_position())) {
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
						_begin_key_drag(hit_track_index, hit_key, mb->get_position(), mb->is_alt_pressed());
						accept_event();
						return;
					}
				}
			}

			bool is_touchscreen_available = DisplayServer::get_singleton()->is_touchscreen_available();
			if (mb->get_button_index() == MouseButton::MOUSE_BUTTON_LEFT) {
				if (!is_touchscreen_available) {
					if (mb->is_pressed() && is_time_ruler_position_at(mb->get_position())) {
						// 在时间尺范围内点击，更新 current_time
						playhead_dragging = true;
						queue_redraw();
					}
					else if (mb->is_pressed() && is_content_position_at(mb->get_position())) {
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
				if (mb->is_pressed() && is_content_position_at(mb->get_position())) {
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
				if (mb->is_pressed() && is_time_ruler_position_at(mb->get_position())) {
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
			if (middle_mouse_panning) {
				_update_middle_mouse_pan(mm->get_position());
				accept_event();
				return;
			}

			if (header_resizing) {
				_update_header_resize_drag(mm->get_position());
				accept_event();
				return;
			}

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
				double new_time = _get_playhead_drag_time(panel_orientation == PANEL_ORIENTATION_HORIZONTAL ? mm->get_position().x : mm->get_position().y);
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
				if (!_update_header_resize_cursor(mm->get_position())) {
					_update_clip_key_edge_cursor(mm->get_position());
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


	String TimelinePanelBase::_get_tooltip(const Vector2& p_at_position) const {
		const float marker_line_width_margin = 3.0f;
		const float horizontal_header_width = _get_horizontal_track_header_width();
		const float horizontal_ruler_height = _get_horizontal_ruler_height();
		const float indicator_left = panel_orientation == PANEL_ORIENTATION_HORIZONTAL ? horizontal_header_width : -hscroll_value;
		const float indicator_right = panel_orientation == PANEL_ORIENTATION_HORIZONTAL ? get_size().x : indicator_left + _calculate_header_width();
		const bool in_indicator_axis = panel_orientation == PANEL_ORIENTATION_HORIZONTAL ? (p_at_position.y >= horizontal_ruler_height && p_at_position.x >= indicator_left && p_at_position.x <= indicator_right) : (p_at_position.x >= indicator_left && p_at_position.x <= indicator_right);
		if (in_indicator_axis) {
			for (int i = markers.size() - 1; i >= 0; i--) {
				Ref<TimelineMarker> marker = markers[i];
				if (marker.is_null()) continue;

				const String annotation = marker->get_annotation();
				if (annotation.is_empty()) continue;

				double marker_y = 0.0;
				switch (counting_unit) {
				case FRAME: {
					const int safe_fps = MAX(fps, 1);
					marker_y = get_position_from_frame(static_cast<int64_t>(marker->get_time() * safe_fps));
					break;
				}
				case BEAT:
					marker_y = _time_to_y(marker->get_time());
					break;
				case TIME:
				default:
					marker_y = get_position_from_time(marker->get_time());
					break;
				}

				const float hit_margin = MAX(8.0f, marker->get_line_width() * 0.5f + marker_line_width_margin);
				const float axis_position = panel_orientation == PANEL_ORIENTATION_HORIZONTAL ? p_at_position.x : p_at_position.y;
				if (Math::abs(axis_position - marker_y) <= hit_margin) {
					return annotation;
				}
			}
		}

		if (panel_orientation == PANEL_ORIENTATION_HORIZONTAL) {
			if (p_at_position.y <= horizontal_ruler_height && p_at_position.x >= horizontal_header_width) {
				return time_ruler.is_valid() ? time_ruler->get_tooltip_text() : String();
			}
			if (p_at_position.x > horizontal_header_width) return String();
		}
		else {
			if (p_at_position.y > header_height) return String();
			if (time_ruler.is_valid()) {
				float width = time_ruler->get_width();
				Rect2 area = Rect2(Vector2(-hscroll_value, 0.0), Vector2(width, header_height));
				if (area.has_point(p_at_position)) return time_ruler->get_tooltip_text();
			}
		}

		int button_track_index = -1;
		int button_column = -1;
		int button_index = -1;
		if (_find_track_header_button_at_position(p_at_position, button_track_index, button_column, button_index)) {
			const TrackHeaderColumn *column = button_track_index >= 0 && button_track_index < static_cast<int>(tracks.size()) ? _get_track_column(tracks[button_track_index], button_column) : nullptr;
			if (column != nullptr && button_index >= 0 && button_index < static_cast<int>(column->buttons.size())) {
				return column->buttons[button_index].tooltip_text;
			}
		}

		for (int i = 0; i < static_cast<int>(tracks.size()) && i < static_cast<int>(_track_cache.size()); i++) {
			const CachedTrack &ct = _track_cache[i];
			if (ct.width <= 0.0f) continue;

			Rect2 area = panel_orientation == PANEL_ORIENTATION_HORIZONTAL ?
				Rect2(Vector2(0.0f, horizontal_ruler_height + ct.x_offset - vscroll_value), Vector2(_get_horizontal_track_header_width(), ct.width)) :
				Rect2(Vector2(ct.x_offset - hscroll_value, 0.0f), Vector2(ct.width, header_height));
			if (area.has_point(p_at_position)) {
				const int column_index = _get_track_header_column_at_position(i, p_at_position);
				const TrackHeaderColumn *column = _get_track_column(tracks[i], column_index >= 0 ? column_index : 0);
				return column != nullptr ? column->tooltip_text : String();
			}
		}

		return String();
	}


}
