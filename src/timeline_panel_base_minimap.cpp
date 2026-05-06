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
	bool TimelinePanelBase::_is_minimap_visible() const {
		return draw_minimap && minimap_width > 0 && vscroll != nullptr && vscroll->is_visible();
	}


	Rect2 TimelinePanelBase::_get_minimap_rect() const {
		if (!_is_minimap_visible()) {
			return Rect2();
		}

		const Size2 size = get_size();
		const float hscroll_height = hscroll != nullptr && hscroll->is_visible() ? hscroll->get_combined_minimum_size().y : 0.0f;
		const float vscroll_width = vscroll != nullptr && vscroll->is_visible() ? vscroll->get_combined_minimum_size().x : 0.0f;
		const float width = static_cast<float>(minimap_width);
		const float top_header_height = panel_orientation == PANEL_ORIENTATION_HORIZONTAL ? _get_horizontal_ruler_height() : header_height;
		const float height = MAX(size.y - top_header_height - hscroll_height, 0.0f);
		const float x = size.x - vscroll_width - width;
		return Rect2(Vector2(x, top_header_height), Vector2(width, height));
	}


	Rect2 TimelinePanelBase::_get_minimap_viewport_rect() const {
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


	double TimelinePanelBase::_indicator_time_to_content_y(double p_time) const {
		if (panel_orientation == PANEL_ORIENTATION_HORIZONTAL) {
			switch (counting_unit) {
			case FRAME: {
				const int safe_fps = MAX(fps, 1);
				return static_cast<int64_t>(p_time * safe_fps) * scale;
			}
			case BEAT:
				return _time_to_beat(p_time) * scale;
			case TIME:
			default:
				return p_time * scale;
			}
		}

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


	double TimelinePanelBase::_content_y_to_minimap_y(const Rect2 &p_rect, double p_content_y) const {
		const double content = MAX(static_cast<double>(panel_orientation == PANEL_ORIENTATION_HORIZONTAL ? _calculate_grid_height() : content_height), 1.0);
		const double ratio = CLAMP(p_content_y / content, 0.0, 1.0);
		return p_rect.position.y + ratio * p_rect.size.y;
	}


	void TimelinePanelBase::_mark_minimap_key_cache_dirty() {
		minimap_key_cache_dirty = true;
	}


	void TimelinePanelBase::_rebuild_minimap_key_cache(int p_height) {
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

		const double content = MAX(static_cast<double>(panel_orientation == PANEL_ORIENTATION_HORIZONTAL ? _calculate_grid_height() : content_height), 1.0);
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

				const double key_start_content_y = panel_orientation == PANEL_ORIENTATION_HORIZONTAL ? _indicator_time_to_content_y(key->get_time()) : _key_to_y(key) - header_height + vscroll_value;
				const double key_end_content_y = key->is_instant() ? key_start_content_y : (panel_orientation == PANEL_ORIENTATION_HORIZONTAL ? _indicator_time_to_content_y(key->get_time() + key->get_length()) : _key_end_to_y(key) - header_height + vscroll_value);
				mark_key_rows(cache, key_start_content_y, key_end_content_y, key->is_selected(), key->is_instant());
			}
		}

		minimap_key_cache_dirty = false;
	}


	void TimelinePanelBase::_draw_minimap() {
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


	bool TimelinePanelBase::_begin_minimap_drag(const Vector2 &p_position) {
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


	void TimelinePanelBase::_update_minimap_drag(const Vector2 &p_position) {
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


	void TimelinePanelBase::_finish_minimap_drag() {
		if (!minimap_dragging) {
			return;
		}

		minimap_dragging = false;
		minimap_dragging_viewport = false;
		queue_redraw();
	}


	void TimelinePanelBase::_scroll_minimap_to_position(float p_y) {
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


	void TimelinePanelBase::set_draw_minimap(bool p_enabled) {
		if (draw_minimap == p_enabled) {
			return;
		}
		draw_minimap = p_enabled;
		_update_scroll_bar();
		queue_redraw();
		update_minimum_size();
	}


	bool TimelinePanelBase::is_drawing_minimap() const {
		return draw_minimap;
	}


	void TimelinePanelBase::set_minimap_width(int p_width) {
		const int new_width = MAX(p_width, 0);
		if (minimap_width == new_width) {
			return;
		}
		minimap_width = new_width;
		_update_scroll_bar();
		queue_redraw();
		update_minimum_size();
	}


	int TimelinePanelBase::get_minimap_width() const {
		return minimap_width;
	}


}
