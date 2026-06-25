#include "timeline_panel_base.h"

#include "components/timeline_indicator.h"
#include "components/timeline_marker.h"
#include "components/timeline_time_ruler.h"
#include "components/timeline_track.h"
#include "components/timeline_track_item.h"
#include "components/timeline_track_key.h"

#include <algorithm>
#include <unordered_set>
#include <godot_cpp/classes/font.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/input_event_mouse_motion.hpp>
#include <godot_cpp/classes/input_event_pan_gesture.hpp>
#include <godot_cpp/classes/style_box_flat.hpp>
#include <godot_cpp/classes/theme_db.hpp>

#include "theme_helpers.h"

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
			CachedTrack ct;
			ct.x_offset = current_x;
			ct.width = tracks[i].height;
			_track_cache.push_back(std::move(ct));
			current_x += ct.width;
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
			ct.x_offset = current_x;
			ct.width = tracks[i].height;
			current_x += ct.width;
		}
	}


	void TimelinePanelBase::_update_content_height() {
		content_height = panel_orientation == PANEL_ORIENTATION_HORIZONTAL ? _calculate_track_span() : _calculate_grid_height();
		_update_scroll_bar();
	}


	TimelinePanelBase::TrackData *TimelinePanelBase::_get_track_data(TimelineTrackItem *p_item, uint64_t p_track_id) {
		if (p_item == nullptr || p_track_id == 0) {
			return nullptr;
		}
		for (TrackData &track : tracks) {
			if (track.item == p_item && track.id == p_track_id) {
				return &track;
			}
		}
		return nullptr;
	}


	const TimelinePanelBase::TrackData *TimelinePanelBase::_get_track_data(const TimelineTrackItem *p_item, uint64_t p_track_id) const {
		if (p_item == nullptr || p_track_id == 0) {
			return nullptr;
		}
		for (const TrackData &track : tracks) {
			if (track.item == p_item && track.id == p_track_id) {
				return &track;
			}
		}
		return nullptr;
	}


	TimelinePanelBase::TrackHeaderColumn &TimelinePanelBase::_ensure_track_column(TrackData &p_track, int p_column) {
		const int column = MAX(p_column, 0);
		if (static_cast<int>(p_track.columns.size()) <= column) {
			p_track.columns.resize(column + 1);
		}
		return p_track.columns[column];
	}


	const TimelinePanelBase::TrackHeaderColumn *TimelinePanelBase::_get_track_column(const TrackData &p_track, int p_column) const {
		if (p_column < 0 || p_column >= static_cast<int>(p_track.columns.size())) {
			return nullptr;
		}
		return &p_track.columns[p_column];
	}


	float TimelinePanelBase::_get_header_column_width(int p_column, float p_total_width) const {
		const int column_count = MAX(header_column_count, 1);
		if (p_column < 0 || p_column >= column_count) {
			return 0.0f;
		}

		if (p_column == 0) {
			float fixed_width = 0.0f;
			for (int column = 1; column < column_count; column++) {
				fixed_width += _get_header_column_width(column, p_total_width);
			}
			return MAX(p_total_width - fixed_width, 0.0f);
		}

		if (p_column < static_cast<int>(header_column_widths.size())) {
			return MAX(header_column_widths[p_column], 0.0f);
		}
		return 24.0f;
	}


	Rect2 TimelinePanelBase::_get_header_column_rect(const Rect2 &p_header_rect, int p_column) const {
		float x = p_header_rect.position.x;
		for (int column = 0; column < p_column; column++) {
			x += _get_header_column_width(column, p_header_rect.size.x);
		}
		return Rect2(Vector2(x, p_header_rect.position.y), Vector2(_get_header_column_width(p_column, p_header_rect.size.x), p_header_rect.size.y));
	}


	Rect2 TimelinePanelBase::_get_track_header_button_rect(const Rect2 &p_cell_rect, int p_button_draw_index) const {
		const float gap = 2.0f;
		const float side_margin = 3.0f;
		const float size = CLAMP(p_cell_rect.size.y - 6.0f, 8.0f, 20.0f);
		const float x = p_cell_rect.position.x + p_cell_rect.size.x - side_margin - size - static_cast<float>(p_button_draw_index) * (size + gap);
		const float y = p_cell_rect.position.y + (p_cell_rect.size.y - size) * 0.5f;
		return Rect2(Vector2(x, y), Vector2(size, size));
	}


	int TimelinePanelBase::_get_track_header_column_at_position(int p_track_index, const Vector2 &p_position) const {
		if (p_track_index < 0 || p_track_index >= static_cast<int>(_track_cache.size())) {
			return -1;
		}

		const CachedTrack &ct = _track_cache[p_track_index];
		if (ct.width <= 0.0f) {
			return -1;
		}

		Rect2 header_rect;
		if (panel_orientation == PANEL_ORIENTATION_HORIZONTAL) {
			header_rect = Rect2(Vector2(0.0f, _get_horizontal_ruler_height() + ct.x_offset - vscroll_value), Vector2(_get_horizontal_track_header_width(), ct.width));
		}
		else {
			header_rect = Rect2(Vector2(ct.x_offset - hscroll_value, 0.0f), Vector2(ct.width, header_height));
		}

		if (!header_rect.has_point(p_position)) {
			return -1;
		}

		const int column_count = panel_orientation == PANEL_ORIENTATION_HORIZONTAL ? MAX(header_column_count, 1) : 1;
		for (int column = 0; column < column_count; column++) {
			if (_get_header_column_rect(header_rect, column).has_point(p_position)) {
				return column;
			}
		}
		return 0;
	}


	bool TimelinePanelBase::_find_track_header_button_at_position(const Vector2 &p_position, int &r_track_index, int &r_column, int &r_button_index) const {
		r_track_index = -1;
		r_column = -1;
		r_button_index = -1;

		for (int track_index = 0; track_index < static_cast<int>(_track_cache.size()) && track_index < static_cast<int>(tracks.size()); track_index++) {
			const CachedTrack &ct = _track_cache[track_index];
			if (ct.width <= 0.0f) {
				continue;
			}

			Rect2 header_rect;
			if (panel_orientation == PANEL_ORIENTATION_HORIZONTAL) {
				header_rect = Rect2(Vector2(0.0f, _get_horizontal_ruler_height() + ct.x_offset - vscroll_value), Vector2(_get_horizontal_track_header_width(), ct.width));
			}
			else {
				header_rect = Rect2(Vector2(ct.x_offset - hscroll_value, 0.0f), Vector2(ct.width, header_height));
			}
			if (!header_rect.has_point(p_position)) {
				continue;
			}

			const TrackData &track = tracks[track_index];
			const int column_count = panel_orientation == PANEL_ORIENTATION_HORIZONTAL ? MAX(header_column_count, 1) : 1;
			for (int column = 0; column < column_count; column++) {
				const TrackHeaderColumn *header_column = _get_track_column(track, column);
				if (header_column == nullptr || header_column->buttons.empty()) {
					continue;
				}

				const Rect2 column_rect = _get_header_column_rect(header_rect, column);
				if (!column_rect.has_point(p_position)) {
					continue;
				}

				for (int button_index = static_cast<int>(header_column->buttons.size()) - 1; button_index >= 0; button_index--) {
					if (_get_track_header_button_rect(column_rect, button_index).has_point(p_position)) {
						r_track_index = track_index;
						r_column = column;
						r_button_index = button_index;
						return true;
					}
				}
			}
		}

		return false;
	}


	TimelineTrackItem *TimelinePanelBase::create_track(int p_index) {
		const int insert_index = p_index < 0 ? static_cast<int>(tracks.size()) : CLAMP(p_index, 0, static_cast<int>(tracks.size()));
		TimelineTrackItem *item = memnew(TimelineTrackItem);
		TrackData track;
		track.id = next_track_id++;
		track.item = item;
		track.columns.resize(1);
		item->_bind_owner(this, track.id);
		tracks.insert(tracks.begin() + insert_index, track);
		CachedTrack cached_track;
		_track_cache.insert(_track_cache.begin() + insert_index, cached_track);
		_sync_track_cache_geometry();
		_mark_minimap_key_cache_dirty();
		_update_content_height();
		queue_redraw();
		update_minimum_size();
		return item;
	}


	void TimelinePanelBase::remove_track(const Variant &p_track) {
		int track_index = -1;
		if (p_track.get_type() == Variant::INT) {
			track_index = p_track;
		}
		else if (p_track.get_type() == Variant::OBJECT) {
			TimelineTrackItem *item = Object::cast_to<TimelineTrackItem>(p_track);
			track_index = get_track_index(item);
		}

		if (track_index < 0 || track_index >= static_cast<int>(tracks.size())) {
			return;
		}

		if (track_index < static_cast<int>(_track_cache.size())) {
			for (TimelineTrackKey *key : _track_cache[track_index].keys) {
				if (key) {
					memdelete(key);
				}
			}
		}

		TimelineTrackItem *item = tracks[track_index].item;
		if (item != nullptr) {
			item->_unbind_owner();
			memdelete(item);
		}
		tracks.erase(tracks.begin() + track_index);
		if (track_index < static_cast<int>(_track_cache.size())) {
			_track_cache.erase(_track_cache.begin() + track_index);
		}
		_clear_key_release_preview();
		_sync_track_cache_geometry();
		_mark_minimap_key_cache_dirty();
		_update_content_height();
		queue_redraw();
		update_minimum_size();
	}


	void TimelinePanelBase::clear_tracks() {
		clear_all_keys();
		for (TrackData &track : tracks) {
			if (track.item != nullptr) {
				track.item->_unbind_owner();
				memdelete(track.item);
				track.item = nullptr;
			}
		}
		tracks.clear();
		_track_cache.clear();
		_mark_minimap_key_cache_dirty();
		_update_content_height();
		queue_redraw();
		update_minimum_size();
	}


	TimelineTrackItem *TimelinePanelBase::get_track(int p_index) const {
		if (p_index < 0 || p_index >= static_cast<int>(tracks.size())) {
			return nullptr;
		}
		return tracks[p_index].item;
	}


	int TimelinePanelBase::get_track_count() const {
		return static_cast<int>(tracks.size());
	}


	int TimelinePanelBase::get_track_index(TimelineTrackItem *p_track) const {
		if (p_track == nullptr) {
			return -1;
		}
		for (int index = 0; index < static_cast<int>(tracks.size()); index++) {
			if (tracks[index].item == p_track) {
				return index;
			}
		}
		return -1;
	}


	void TimelinePanelBase::set_header_column_count(int p_count) {
		header_column_count = MAX(p_count, 1);
		if (static_cast<int>(header_column_widths.size()) < header_column_count) {
			header_column_widths.resize(header_column_count, 24.0f);
			header_column_widths[0] = 0.0f;
		}
		queue_redraw();
	}


	int TimelinePanelBase::get_header_column_count() const {
		return header_column_count;
	}


	void TimelinePanelBase::set_header_column_width(int p_column, float p_width) {
		if (p_column < 0) {
			return;
		}
		if (p_column >= header_column_count) {
			set_header_column_count(p_column + 1);
		}
		if (p_column >= static_cast<int>(header_column_widths.size())) {
			header_column_widths.resize(p_column + 1, 24.0f);
		}
		header_column_widths[p_column] = p_column == 0 ? 0.0f : MAX(p_width, 0.0f);
		queue_redraw();
	}


	float TimelinePanelBase::get_header_column_width(int p_column) const {
		if (p_column < 0 || p_column >= static_cast<int>(header_column_widths.size())) {
			return p_column == 0 ? 0.0f : 24.0f;
		}
		return header_column_widths[p_column];
	}


	bool TimelinePanelBase::_is_track_item_valid(const TimelineTrackItem *p_item, uint64_t p_track_id) const {
		return _get_track_data(p_item, p_track_id) != nullptr;
	}


	void TimelinePanelBase::_track_item_set_height(TimelineTrackItem *p_item, uint64_t p_track_id, float p_height) {
		TrackData *track = _get_track_data(p_item, p_track_id);
		if (track == nullptr) {
			return;
		}
		track->height = MAX(p_height, 0.0f);
		_sync_track_cache_geometry();
		_update_content_height();
		queue_redraw();
		update_minimum_size();
	}


	float TimelinePanelBase::_track_item_get_height(const TimelineTrackItem *p_item, uint64_t p_track_id) const {
		const TrackData *track = _get_track_data(p_item, p_track_id);
		return track != nullptr ? track->height : 0.0f;
	}


	void TimelinePanelBase::_track_item_set_background(TimelineTrackItem *p_item, uint64_t p_track_id, const Color &p_background) {
		TrackData *track = _get_track_data(p_item, p_track_id);
		if (track == nullptr) {
			return;
		}
		track->background = p_background;
		queue_redraw();
	}


	Color TimelinePanelBase::_track_item_get_background(const TimelineTrackItem *p_item, uint64_t p_track_id) const {
		const TrackData *track = _get_track_data(p_item, p_track_id);
		return track != nullptr ? track->background : Color();
	}


	void TimelinePanelBase::_track_item_set_header_background(TimelineTrackItem *p_item, uint64_t p_track_id, const Ref<StyleBox> &p_style) {
		TrackData *track = _get_track_data(p_item, p_track_id);
		if (track == nullptr) {
			return;
		}
		if (track->header_background.is_valid() && track->header_background->is_connected("changed", callable_mp(this, &TimelinePanelBase::_on_resource_changed))) {
			track->header_background->disconnect("changed", callable_mp(this, &TimelinePanelBase::_on_resource_changed));
		}
		track->header_background = p_style;
		if (track->header_background.is_valid() && !track->header_background->is_connected("changed", callable_mp(this, &TimelinePanelBase::_on_resource_changed))) {
			track->header_background->connect("changed", callable_mp(this, &TimelinePanelBase::_on_resource_changed));
		}
		queue_redraw();
	}


	Ref<StyleBox> TimelinePanelBase::_track_item_get_header_background(const TimelineTrackItem *p_item, uint64_t p_track_id) const {
		const TrackData *track = _get_track_data(p_item, p_track_id);
		return track != nullptr ? track->header_background : Ref<StyleBox>();
	}


	void TimelinePanelBase::_track_item_set_header_background_fill_track(TimelineTrackItem *p_item, uint64_t p_track_id, bool p_enabled) {
		TrackData *track = _get_track_data(p_item, p_track_id);
		if (track == nullptr) {
			return;
		}
		track->header_background_fill_track = p_enabled;
		queue_redraw();
	}


	bool TimelinePanelBase::_track_item_get_header_background_fill_track(const TimelineTrackItem *p_item, uint64_t p_track_id) const {
		const TrackData *track = _get_track_data(p_item, p_track_id);
		return track != nullptr && track->header_background_fill_track;
	}


	void TimelinePanelBase::_track_item_set_header_indent(TimelineTrackItem *p_item, uint64_t p_track_id, float p_indent) {
		TrackData *track = _get_track_data(p_item, p_track_id);
		if (track == nullptr) {
			return;
		}
		track->header_indent = MAX(p_indent, 0.0f);
		queue_redraw();
	}


	float TimelinePanelBase::_track_item_get_header_indent(const TimelineTrackItem *p_item, uint64_t p_track_id) const {
		const TrackData *track = _get_track_data(p_item, p_track_id);
		return track != nullptr ? track->header_indent : 0.0f;
	}


	void TimelinePanelBase::_track_item_set_text(TimelineTrackItem *p_item, uint64_t p_track_id, int p_column, const String &p_text) {
		TrackData *track = _get_track_data(p_item, p_track_id);
		if (track == nullptr) {
			return;
		}
		_ensure_track_column(*track, p_column).text = p_text;
		queue_redraw();
	}


	String TimelinePanelBase::_track_item_get_text(const TimelineTrackItem *p_item, uint64_t p_track_id, int p_column) const {
		const TrackData *track = _get_track_data(p_item, p_track_id);
		if (track == nullptr) {
			return String();
		}
		const TrackHeaderColumn *column = _get_track_column(*track, p_column);
		return column != nullptr ? column->text : String();
	}


	void TimelinePanelBase::_track_item_set_icon(TimelineTrackItem *p_item, uint64_t p_track_id, int p_column, Ref<Texture2D> p_icon) {
		TrackData *track = _get_track_data(p_item, p_track_id);
		if (track == nullptr) {
			return;
		}
		_ensure_track_column(*track, p_column).icon = p_icon;
		queue_redraw();
	}


	Ref<Texture2D> TimelinePanelBase::_track_item_get_icon(const TimelineTrackItem *p_item, uint64_t p_track_id, int p_column) const {
		const TrackData *track = _get_track_data(p_item, p_track_id);
		if (track == nullptr) {
			return Ref<Texture2D>();
		}
		const TrackHeaderColumn *column = _get_track_column(*track, p_column);
		return column != nullptr ? column->icon : Ref<Texture2D>();
	}


	void TimelinePanelBase::_track_item_set_tooltip_text(TimelineTrackItem *p_item, uint64_t p_track_id, int p_column, const String &p_text) {
		TrackData *track = _get_track_data(p_item, p_track_id);
		if (track == nullptr) {
			return;
		}
		_ensure_track_column(*track, p_column).tooltip_text = p_text;
	}


	String TimelinePanelBase::_track_item_get_tooltip_text(const TimelineTrackItem *p_item, uint64_t p_track_id, int p_column) const {
		const TrackData *track = _get_track_data(p_item, p_track_id);
		if (track == nullptr) {
			return String();
		}
		const TrackHeaderColumn *column = _get_track_column(*track, p_column);
		return column != nullptr ? column->tooltip_text : String();
	}


	int TimelinePanelBase::_track_item_add_button(TimelineTrackItem *p_item, uint64_t p_track_id, int p_column, Ref<Texture2D> p_icon, int p_id, bool p_disabled, const String &p_tooltip) {
		TrackData *track = _get_track_data(p_item, p_track_id);
		if (track == nullptr) {
			return -1;
		}
		TrackHeaderButton button;
		button.icon = p_icon;
		button.id = p_id;
		button.disabled = p_disabled;
		button.tooltip_text = p_tooltip;
		TrackHeaderColumn &column = _ensure_track_column(*track, p_column);
		column.buttons.push_back(button);
		queue_redraw();
		return static_cast<int>(column.buttons.size()) - 1;
	}


	void TimelinePanelBase::_track_item_erase_button(TimelineTrackItem *p_item, uint64_t p_track_id, int p_column, int p_button_index) {
		TrackData *track = _get_track_data(p_item, p_track_id);
		if (track == nullptr) {
			return;
		}
		TrackHeaderColumn *column = &_ensure_track_column(*track, p_column);
		if (p_button_index < 0 || p_button_index >= static_cast<int>(column->buttons.size())) {
			return;
		}
		column->buttons.erase(column->buttons.begin() + p_button_index);
		queue_redraw();
	}


	void TimelinePanelBase::_track_item_clear_buttons(TimelineTrackItem *p_item, uint64_t p_track_id, int p_column) {
		TrackData *track = _get_track_data(p_item, p_track_id);
		if (track == nullptr) {
			return;
		}
		_ensure_track_column(*track, p_column).buttons.clear();
		queue_redraw();
	}


	void TimelinePanelBase::_track_item_set_button_disabled(TimelineTrackItem *p_item, uint64_t p_track_id, int p_column, int p_button_index, bool p_disabled) {
		TrackData *track = _get_track_data(p_item, p_track_id);
		if (track == nullptr) {
			return;
		}
		TrackHeaderColumn &column = _ensure_track_column(*track, p_column);
		if (p_button_index < 0 || p_button_index >= static_cast<int>(column.buttons.size())) {
			return;
		}
		column.buttons[p_button_index].disabled = p_disabled;
		queue_redraw();
	}


	bool TimelinePanelBase::_track_item_is_button_disabled(const TimelineTrackItem *p_item, uint64_t p_track_id, int p_column, int p_button_index) const {
		const TrackData *track = _get_track_data(p_item, p_track_id);
		if (track == nullptr) {
			return false;
		}
		const TrackHeaderColumn *column = _get_track_column(*track, p_column);
		if (column == nullptr || p_button_index < 0 || p_button_index >= static_cast<int>(column->buttons.size())) {
			return false;
		}
		return column->buttons[p_button_index].disabled;
	}


	int TimelinePanelBase::_track_item_get_button_count(const TimelineTrackItem *p_item, uint64_t p_track_id, int p_column) const {
		const TrackData *track = _get_track_data(p_item, p_track_id);
		if (track == nullptr) {
			return 0;
		}
		const TrackHeaderColumn *column = _get_track_column(*track, p_column);
		return column != nullptr ? static_cast<int>(column->buttons.size()) : 0;
	}


	int TimelinePanelBase::_track_item_get_button_id(const TimelineTrackItem *p_item, uint64_t p_track_id, int p_column, int p_button_index) const {
		const TrackData *track = _get_track_data(p_item, p_track_id);
		if (track == nullptr) {
			return -1;
		}
		const TrackHeaderColumn *column = _get_track_column(*track, p_column);
		if (column == nullptr || p_button_index < 0 || p_button_index >= static_cast<int>(column->buttons.size())) {
			return -1;
		}
		return column->buttons[p_button_index].id;
	}


	String TimelinePanelBase::_track_item_get_button_tooltip_text(const TimelineTrackItem *p_item, uint64_t p_track_id, int p_column, int p_button_index) const {
		const TrackData *track = _get_track_data(p_item, p_track_id);
		if (track == nullptr) {
			return String();
		}
		const TrackHeaderColumn *column = _get_track_column(*track, p_column);
		if (column == nullptr || p_button_index < 0 || p_button_index >= static_cast<int>(column->buttons.size())) {
			return String();
		}
		return column->buttons[p_button_index].tooltip_text;
	}












	void TimelinePanelBase::set_header_width(const float p_width) {
		header_height = p_width;
		queue_redraw();
		update_minimum_size();
	}


	float TimelinePanelBase::get_header_width() const {
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
		clear_tracks();
		for (int i = 0; i < p_tracks.size(); i++) {
			Ref<TimelineTrack> old_track = p_tracks[i];
			TimelineTrackItem *track = create_track();
			if (old_track.is_null() || track == nullptr) {
				continue;
			}
			track->set_width(old_track->get_width());
			track->set_background(old_track->get_background());
			track->set_header_background(old_track->get_header_background());
			track->set_header_background_fill_track(old_track->get_header_background_fill_track());
			track->set_header_indent(old_track->get_header_indent());
			track->set_icon(0, old_track->get_header_icon());
			track->set_text(0, old_track->get_text());
			track->set_tooltip_text(0, old_track->get_tooltip_text());
		}
		queue_redraw();
		update_minimum_size();
	}


	TypedArray<TimelineTrack> TimelinePanelBase::get_tracks() const {
		TypedArray<TimelineTrack> result;
		for (const TrackData &track : tracks) {
			Ref<TimelineTrack> old_track;
			old_track.instantiate();
			old_track->set_width(track.height);
			old_track->set_background(track.background);
			old_track->set_header_background(track.header_background);
			old_track->set_header_background_fill_track(track.header_background_fill_track);
			old_track->set_header_indent(track.header_indent);
			const TrackHeaderColumn *column = _get_track_column(track, 0);
			if (column != nullptr) {
				old_track->set_header_icon(column->icon);
				old_track->set_text(column->text);
				old_track->set_tooltip_text(column->tooltip_text);
			}
			result.append(old_track);
		}
		return result;
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


	void TimelinePanelBase::set_key_snap_step(double p_step) {
		key_snap_step = p_step < 0.0 ? 0.0 : p_step;
	}


	double TimelinePanelBase::get_key_snap_step() const {
		return key_snap_step;
	}


	void TimelinePanelBase::set_key_alt_duplicate_drag_enabled(bool p_enabled) {
		key_alt_duplicate_drag_enabled = p_enabled;
	}


	bool TimelinePanelBase::get_key_alt_duplicate_drag_enabled() const {
		return key_alt_duplicate_drag_enabled;
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


	void TimelinePanelBase::set_clip_key_edge_snap_enabled(bool p_enabled) {
		clip_key_edge_snap_enabled = p_enabled;
	}


	bool TimelinePanelBase::get_clip_key_edge_snap_enabled() const {
		return clip_key_edge_snap_enabled;
	}


	bool TimelinePanelBase::is_clip_key_edge_dragging() const {
		return clip_key_edge_dragging;
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

	Color TimelinePanelBase::_get_background_color() const {
		return get_theme_color_or(const_cast<TimelinePanelBase*>(this), "TimelinePanel", "background_color", background_color);
	}

	Color TimelinePanelBase::_get_separator_color() const {
		return get_theme_color_or(const_cast<TimelinePanelBase*>(this), "TimelinePanel", "separator_color", separator_color);
	}

	float TimelinePanelBase::_get_separator_width() const {
		return static_cast<float>(get_theme_constant_or(const_cast<TimelinePanelBase*>(this), "TimelinePanel", "separator_width", static_cast<int32_t>(separator_width)));
	}

	Color TimelinePanelBase::_get_bar_line_color() const {
		return get_theme_color_or(const_cast<TimelinePanelBase*>(this), "TimelinePanel", "bar_line_color", bar_line_color);
	}

	float TimelinePanelBase::_get_bar_line_width() const {
		return static_cast<float>(get_theme_constant_or(const_cast<TimelinePanelBase*>(this), "TimelinePanel", "bar_line_width", static_cast<int32_t>(bar_line_width)));
	}

	Color TimelinePanelBase::_get_beat_line_color() const {
		return get_theme_color_or(const_cast<TimelinePanelBase*>(this), "TimelinePanel", "beat_line_color", beat_line_color);
	}

	float TimelinePanelBase::_get_beat_line_width() const {
		return static_cast<float>(get_theme_constant_or(const_cast<TimelinePanelBase*>(this), "TimelinePanel", "beat_line_width", static_cast<int32_t>(beat_line_width)));
	}

}
