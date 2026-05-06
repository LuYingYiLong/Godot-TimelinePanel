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
	void TimelinePanelBase::_move_key_to_track(TimelineTrackKey* p_key, int p_from_track, int p_to_track) {
		if (!p_key || p_to_track < 0 || p_to_track >= static_cast<int>(_track_cache.size()) || p_from_track == p_to_track) {
			return;
		}
		if (!_can_move_key_to_track(p_key, p_to_track)) {
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


	bool TimelinePanelBase::_can_move_key_to_track(const TimelineTrackKey* p_key, int p_track_index) const {
		return p_key != nullptr && p_key->can_move_to_track(p_track_index);
	}


	double TimelinePanelBase::_snap_key_time(double p_time) const {
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


	bool TimelinePanelBase::_keys_overlap(const TimelineTrackKey* p_a, const TimelineTrackKey* p_b) const {
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


	bool TimelinePanelBase::_has_key_overlap_in_track(const CachedTrack &p_track, const TimelineTrackKey *p_key) const {
		if (!p_key || p_track.keys.empty()) {
			return false;
		}

		const double epsilon = 0.000001;
		const double key_start = p_key->get_time();
		const double key_length = MAX(p_key->get_length(), 0.0);
		const double key_end = key_start + key_length;
		const double search_start = key_start - MAX(p_track.max_key_length, 0.0) - epsilon;
		const double search_end = key_end + epsilon;

		auto it = std::lower_bound(p_track.keys.begin(), p_track.keys.end(), search_start,
			[](TimelineTrackKey *p_other_key, double p_time) {
				return p_other_key && p_other_key->get_time() < p_time;
			});

		for (; it != p_track.keys.end(); ++it) {
			TimelineTrackKey *other_key = *it;
			if (!other_key) continue;

			if (other_key->get_time() > search_end) {
				break;
			}
			if (other_key->is_disabled()) continue;
			if (_keys_overlap(p_key, other_key)) {
				return true;
			}
		}

		return false;
	}


	std::vector<TimelineTrackKey *> TimelinePanelBase::_get_moved_key_overlaps(const std::vector<TimelineTrackKey *> &p_moved_keys) const {
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


	void TimelinePanelBase::_update_key_release_preview(const std::vector<TimelineTrackKey *> &p_moved_keys) {
		key_release_preview_keys.clear();
		for (TimelineTrackKey *destroy_key : _get_moved_key_overlaps(p_moved_keys)) {
			key_release_preview_keys.insert(destroy_key);
		}
	}


	void TimelinePanelBase::_clear_key_release_preview() {
		key_release_preview_keys.clear();
	}


	bool TimelinePanelBase::_is_key_release_previewed(const TimelineTrackKey *p_key) const {
		return p_key && key_release_preview_keys.find(p_key) != key_release_preview_keys.end();
	}


	void TimelinePanelBase::_destroy_moved_key_overlaps(const std::vector<TimelineTrackKey*>& p_moved_keys) {
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


	TimelineTrackKey *TimelinePanelBase::create_key(int p_track_index, double p_time, double p_length, bool p_snap) {
		ERR_FAIL_INDEX_V(p_track_index, static_cast<int>(_track_cache.size()), nullptr);

		auto &ct = _track_cache[p_track_index];
		auto &keys = ct.keys;
		const double target_time = p_snap ? _snap_key_time(p_time) : p_time;
		TimelineTrackKey *key = memnew(TimelineTrackKey);
		key->set_time(target_time);
		key->set_length(p_length);

		if (_has_key_overlap_in_track(ct, key)) {
			memdelete(key);
			return nullptr;
		}

		key->connect("changed", callable_mp(this, &TimelinePanelBase::_on_resource_changed));

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


	void TimelinePanelBase::remove_key(int p_track_index, int p_key_index) {
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


	void TimelinePanelBase::clear_track_keys(int p_track_index) {
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


	void TimelinePanelBase::clear_all_keys() {
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


	int TimelinePanelBase::get_key_count(int p_track_index) const {
		ERR_FAIL_INDEX_V(p_track_index, static_cast<int>(_track_cache.size()), 0);
		return static_cast<int>(_track_cache[p_track_index].keys.size());
	}


	TimelineTrackKey* TimelinePanelBase::get_key(int p_track_index, int p_key_index) const {
		ERR_FAIL_INDEX_V(p_track_index, static_cast<int>(_track_cache.size()), nullptr);
		const auto& keys = _track_cache[p_track_index].keys;
		ERR_FAIL_INDEX_V(p_key_index, static_cast<int>(keys.size()), nullptr);
		return keys[p_key_index];
	}


	TypedArray<TimelineTrackKey> TimelinePanelBase::find_keys(int p_track_index, double p_start_time, double p_end_time) const {
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


}
