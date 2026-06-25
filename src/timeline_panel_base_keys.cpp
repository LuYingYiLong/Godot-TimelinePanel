#include "timeline_panel_base.h"

#include "components/timeline_indicator.h"
#include "components/timeline_marker.h"
#include "components/timeline_time_ruler.h"
#include "components/timeline_track.h"
#include "components/timeline_track_key.h"

#include <algorithm>
#include <unordered_set>
#include <godot_cpp/classes/style_box.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/font.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/input_event_mouse_motion.hpp>
#include <godot_cpp/classes/input_event_pan_gesture.hpp>
#include <godot_cpp/classes/style_box_flat.hpp>
#include <godot_cpp/classes/theme_db.hpp>
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/variant/dictionary.hpp>

namespace godot {
	namespace {
		Ref<Texture2D> get_texture_from_variant(const Variant& p_value) {
			if (p_value.get_type() != Variant::OBJECT) {
				return Ref<Texture2D>();
			}
			Object* object = p_value;
			Texture2D* texture = Object::cast_to<Texture2D>(object);
			if (!texture) {
				return Ref<Texture2D>();
			}
			return Ref<Texture2D>(texture);
		}

		Ref<StyleBox> get_style_box_from_variant(const Variant& p_value) {
			if (p_value.get_type() != Variant::OBJECT) {
				return Ref<StyleBox>();
			}
			Object* object = p_value;
			StyleBox* style_box = Object::cast_to<StyleBox>(object);
			if (!style_box) {
				return Ref<StyleBox>();
			}
			return Ref<StyleBox>(style_box);
		}

		PackedInt32Array get_packed_int32_array_from_variant(const Variant& p_value) {
			if (p_value.get_type() == Variant::PACKED_INT32_ARRAY) {
				return p_value;
			}
			PackedInt32Array result;
			if (p_value.get_type() != Variant::ARRAY) {
				return result;
			}
			Array values = p_value;
			for (int i = 0; i < values.size(); i++) {
				result.append(static_cast<int>(values[i]));
			}
			return result;
		}
	}

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


	TimelineTrackKey *TimelinePanelBase::_duplicate_key_for_drag(TimelineTrackKey *p_key, int p_track_index) {
		if (!p_key || p_track_index < 0 || p_track_index >= static_cast<int>(_track_cache.size())) {
			return nullptr;
		}

		TimelineTrackKey *duplicate_key = memnew(TimelineTrackKey);
		duplicate_key->set_time_no_signal(p_key->get_time());
		duplicate_key->set_length_no_signal(p_key->get_length());
		duplicate_key->set_text(p_key->get_text());
		duplicate_key->set_icon(p_key->get_icon());
		duplicate_key->set_icon_max_width(p_key->get_icon_max_width());
		duplicate_key->set_instant_key_scale(p_key->get_instant_key_scale());
		duplicate_key->set_instant_key_normal_style(p_key->get_instant_key_normal_style());
		duplicate_key->set_instant_key_selected_style(p_key->get_instant_key_selected_style());
		duplicate_key->set_clip_key_normal_style(p_key->get_clip_key_normal_style());
		duplicate_key->set_clip_key_selected_style(p_key->get_clip_key_selected_style());
		duplicate_key->set_allowed_track_indices(p_key->get_allowed_track_indices());
		duplicate_key->set_disabled_no_signal(p_key->is_disabled());
		duplicate_key->set_selected_no_signal(true);

		Variant metadata = p_key->get_metadata();
		if (metadata.get_type() == Variant::DICTIONARY) {
			Dictionary metadata_dictionary = metadata.duplicate(true);
			metadata_dictionary["timeline_panel_copied"] = true;
			duplicate_key->set_metadata(metadata_dictionary);
		}
		else {
			duplicate_key->set_metadata(metadata);
		}

		duplicate_key->connect("changed", callable_mp(this, &TimelinePanelBase::_on_resource_changed));
		_track_cache[p_track_index].keys.push_back(duplicate_key);
		_mark_minimap_key_cache_dirty();
		return duplicate_key;
	}


	void TimelinePanelBase::_remove_drag_duplicate_keys(const std::vector<TimelineTrackKey *> &p_keys) {
		for (TimelineTrackKey *remove_key : p_keys) {
			if (!remove_key) continue;
			for (CachedTrack &ct : _track_cache) {
				auto it = std::find(ct.keys.begin(), ct.keys.end(), remove_key);
				if (it == ct.keys.end()) continue;

				ct.keys.erase(it);
				memdelete(remove_key);
				break;
			}
		}
		_mark_minimap_key_cache_dirty();
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
			if (key_snap_step > 0.0) {
				return Math::floor(p_time / key_snap_step + 0.5) * key_snap_step;
			}
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


	bool TimelinePanelBase::_is_key_overlap_permitted(const TimelineTrackKey* p_a, const TimelineTrackKey* p_b) const {
		bool allowed = false;
		return GDVIRTUAL_CALL(_is_key_overlap_allowed, const_cast<TimelineTrackKey *>(p_a), const_cast<TimelineTrackKey *>(p_b), allowed) && allowed;
	}


	bool TimelinePanelBase::_keys_intersect(const TimelineTrackKey* p_a, const TimelineTrackKey* p_b) const {
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


	bool TimelinePanelBase::_keys_overlap(const TimelineTrackKey* p_a, const TimelineTrackKey* p_b) const {
		return _keys_intersect(p_a, p_b) && !_is_key_overlap_permitted(p_a, p_b);
	}


	bool TimelinePanelBase::_keys_intersect_inclusive(const TimelineTrackKey* p_a, const TimelineTrackKey* p_b) const {
		if (!p_a || !p_b || p_a == p_b) {
			return false;
		}

		const double epsilon = 0.000001;
		const double a_start = p_a->get_time();
		const double b_start = p_b->get_time();
		const double a_end = a_start + MAX(p_a->get_length(), 0.0);
		const double b_end = b_start + MAX(p_b->get_length(), 0.0);
		return a_start <= b_end + epsilon && b_start <= a_end + epsilon;
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
		key_allowed_overlap_preview_keys.clear();
		for (TimelineTrackKey *destroy_key : _get_moved_key_overlaps(p_moved_keys)) {
			key_release_preview_keys.insert(destroy_key);
		}

		std::unordered_set<TimelineTrackKey *> moved_key_set;
		moved_key_set.reserve(p_moved_keys.size());
		for (TimelineTrackKey* moved_key : p_moved_keys) {
			if (moved_key) {
				moved_key_set.insert(moved_key);
			}
		}

		for (TimelineTrackKey* moved_key : p_moved_keys) {
			if (!moved_key || key_release_preview_keys.find(moved_key) != key_release_preview_keys.end()) {
				continue;
			}
			bool has_allowed_overlap = false;
			const double epsilon = 0.000001;
			const double moved_start = moved_key->get_time();
			const double moved_end = moved_start + MAX(moved_key->get_length(), 0.0);
			for (const CachedTrack& track : _track_cache) {
				const double search_start = moved_start - MAX(track.max_key_length, 0.0) - epsilon;
				const double search_end = moved_end + epsilon;
				auto other_it = std::lower_bound(track.keys.begin(), track.keys.end(), search_start,
					[](TimelineTrackKey* p_other_key, double p_time) {
						return p_other_key && p_other_key->get_time() < p_time;
					});
				for (; other_it != track.keys.end(); ++other_it) {
					TimelineTrackKey* other_key = *other_it;
					if (!other_key || other_key->is_disabled() || other_key == moved_key) {
						continue;
					}
					if (other_key->get_time() > search_end) {
						break;
					}
					if (moved_key_set.find(other_key) != moved_key_set.end()) {
						continue;
					}
					if (_keys_intersect_inclusive(moved_key, other_key) && _is_key_overlap_permitted(moved_key, other_key)) {
						has_allowed_overlap = true;
						break;
					}
				}
				if (has_allowed_overlap) {
					break;
				}
			}
			if (has_allowed_overlap) {
				key_allowed_overlap_preview_keys.insert(moved_key);
			}
		}
	}


	void TimelinePanelBase::_clear_key_release_preview() {
		key_release_preview_keys.clear();
		key_allowed_overlap_preview_keys.clear();
	}


	bool TimelinePanelBase::_is_key_release_previewed(const TimelineTrackKey *p_key) const {
		return p_key && key_release_preview_keys.find(p_key) != key_release_preview_keys.end();
	}


	bool TimelinePanelBase::_is_key_allowed_overlap_previewed(const TimelineTrackKey* p_key) const {
		return p_key && key_allowed_overlap_preview_keys.find(p_key) != key_allowed_overlap_preview_keys.end();
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


	TimelineTrackKey* TimelinePanelBase::create_key(int p_track_index, double p_time, double p_length, bool p_snap, const Variant& p_metadata) {
		ERR_FAIL_INDEX_V(p_track_index, static_cast<int>(_track_cache.size()), nullptr);

		auto &ct = _track_cache[p_track_index];
		auto &keys = ct.keys;
		const double target_time = p_snap ? _snap_key_time(p_time) : p_time;
		TimelineTrackKey *key = memnew(TimelineTrackKey);
		key->set_time(target_time);
		key->set_length(p_length);
		key->set_metadata(p_metadata);

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


	int TimelinePanelBase::remove_keys(const TypedArray<TimelineTrackKey>& p_keys) {
		if (p_keys.is_empty()) {
			return 0;
		}

		std::unordered_set<TimelineTrackKey*> keys_to_remove;
		keys_to_remove.reserve(p_keys.size());
		for (int i = 0; i < p_keys.size(); i++) {
			TimelineTrackKey* key = VariantCaster<TimelineTrackKey*>::cast(p_keys[i]);
			if (key) {
				keys_to_remove.insert(key);
			}
		}
		if (keys_to_remove.empty()) {
			return 0;
		}

		_clear_key_release_preview();
		int removed_count = 0;
		for (CachedTrack& ct : _track_cache) {
			auto write_it = ct.keys.begin();
			for (auto read_it = ct.keys.begin(); read_it != ct.keys.end(); ++read_it) {
				TimelineTrackKey* key = *read_it;
				if (key && keys_to_remove.find(key) != keys_to_remove.end()) {
					memdelete(key);
					removed_count++;
					continue;
				}
				*write_it = key;
				++write_it;
			}
			ct.keys.erase(write_it, ct.keys.end());
		}

		if (removed_count > 0) {
			_refresh_track_key_metrics();
			queue_redraw();
		}
		return removed_count;
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


	TypedArray<TimelineTrackKey> TimelinePanelBase::replace_track_keys(int p_track_index, const Array& p_key_data, bool p_snap) {
		TypedArray<TimelineTrackKey> created_keys;
		ERR_FAIL_INDEX_V(p_track_index, static_cast<int>(_track_cache.size()), created_keys);

		_clear_key_release_preview();
		CachedTrack& ct = _track_cache[p_track_index];
		std::vector<TimelineTrackKey*>& keys = ct.keys;
		for (TimelineTrackKey* key : keys) {
			if (key) {
				memdelete(key);
			}
		}
		keys.clear();
		ct.max_key_length = 0.0;
		ct.max_instant_key_scale = 0.4f;

		for (int i = 0; i < p_key_data.size(); i++) {
			Variant key_data_variant = p_key_data[i];
			if (key_data_variant.get_type() != Variant::DICTIONARY) {
				continue;
			}
			Dictionary key_data = key_data_variant;
			const double target_time = p_snap || bool(key_data.get("snap", false)) ?
					_snap_key_time(double(key_data.get("time", 0.0))) :
					double(key_data.get("time", 0.0));
			const double length = double(key_data.get("length", 0.0));
			TimelineTrackKey* key = memnew(TimelineTrackKey);
			key->set_time_no_signal(target_time);
			key->set_length_no_signal(length);
			key->set_text(String(key_data.get("text", String())));
			key->set_metadata(key_data.get("metadata", Variant()));
			key->set_disabled_no_signal(bool(key_data.get("disabled", false)));
			key->set_selected_no_signal(bool(key_data.get("selected", false)));

			if (key_data.has("icon")) {
				key->set_icon(get_texture_from_variant(key_data.get("icon", Variant())));
			}
			if (key_data.has("icon_max_width")) {
				key->set_icon_max_width(float(key_data.get("icon_max_width", 0.0f)));
			}
			if (key_data.has("instant_key_scale")) {
				key->set_instant_key_scale(float(key_data.get("instant_key_scale", 0.4f)));
			}
			if (key_data.has("instant_key_normal")) {
				key->set_instant_key_normal_style(get_style_box_from_variant(key_data.get("instant_key_normal", Variant())));
			}
			if (key_data.has("instant_key_selected")) {
				key->set_instant_key_selected_style(get_style_box_from_variant(key_data.get("instant_key_selected", Variant())));
			}
			if (key_data.has("clip_key_normal")) {
				key->set_clip_key_normal_style(get_style_box_from_variant(key_data.get("clip_key_normal", Variant())));
			}
			if (key_data.has("clip_key_selected")) {
				key->set_clip_key_selected_style(get_style_box_from_variant(key_data.get("clip_key_selected", Variant())));
			}
			if (key_data.has("allowed_track_indices")) {
				key->set_allowed_track_indices(get_packed_int32_array_from_variant(key_data.get("allowed_track_indices", Variant())));
			}

			const bool allow_overlap = bool(key_data.get("allow_overlap", false));
			if (!allow_overlap && _has_key_overlap_in_track(ct, key)) {
				memdelete(key);
				continue;
			}

			key->connect("changed", callable_mp(this, &TimelinePanelBase::_on_resource_changed));
			auto it = std::lower_bound(keys.begin(), keys.end(), target_time,
				[](TimelineTrackKey* p_key, double p_time) { return p_key && p_key->get_time() < p_time; });
			keys.insert(it, key);
			if (length > ct.max_key_length) {
				ct.max_key_length = length;
			}
			if (length <= 0.0) {
				ct.max_instant_key_scale = MAX(ct.max_instant_key_scale, _get_instant_key_scale(key));
			}
			created_keys.append(key);
		}

		_mark_minimap_key_cache_dirty();
		queue_redraw();
		return created_keys;
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


	TypedArray<TimelineTrackKey> TimelinePanelBase::get_keys_in_rect(const Rect2& p_rect) const {
		return _get_keys_in_rect(p_rect);
	}


	int TimelinePanelBase::get_key_track_index(const TimelineTrackKey* p_key) const {
		if (!p_key) {
			return -1;
		}
		for (int track_index = 0; track_index < static_cast<int>(_track_cache.size()); ++track_index) {
			const auto& keys = _track_cache[track_index].keys;
			if (std::find(keys.begin(), keys.end(), p_key) != keys.end()) {
				return track_index;
			}
		}
		return -1;
	}


	Rect2 TimelinePanelBase::get_key_rect(int p_track_index, const TimelineTrackKey* p_key) {
		ERR_FAIL_INDEX_V(p_track_index, static_cast<int>(_track_cache.size()), Rect2());
		ERR_FAIL_NULL_V(p_key, Rect2());
		_sync_track_cache_geometry();
		const CachedTrack& track = _track_cache[p_track_index];
		if (track.width <= 0.0f) {
			return Rect2();
		}
		if (p_key->is_instant()) {
			return _get_instant_key_rect(track, p_key, _key_to_y(p_key));
		}
		return _get_clip_key_rect(track, _key_to_y(p_key), _key_end_to_y(p_key));
	}


	void TimelinePanelBase::_draw_key_projections(const Rect2& p_cull_rect) {
		for (const KeyProjection& projection : key_projections) {
			if (projection.track_index < 0 || projection.track_index >= static_cast<int>(_track_cache.size()) || projection.style.is_null()) {
				continue;
			}
			Object* object = ObjectDB::get_instance(projection.key_id);
			TimelineTrackKey* key = Object::cast_to<TimelineTrackKey>(object);
			if (!key || key->is_disabled()) {
				continue;
			}
			const CachedTrack& track = _track_cache[projection.track_index];
			if (track.width <= 0.0f) {
				continue;
			}
			const Rect2 key_rect = key->is_instant() ?
					_get_instant_key_rect(track, key, _key_to_y(key)) :
					_get_clip_key_rect(track, _key_to_y(key), _key_end_to_y(key));
			if (p_cull_rect.intersects(key_rect)) {
				draw_style_box(projection.style, key_rect);
			}
		}
	}


	void TimelinePanelBase::set_key_projections(const Array& p_projections) {
		key_projections.clear();
		key_projections.reserve(p_projections.size());
		for (int projection_index = 0; projection_index < p_projections.size(); projection_index++) {
			Variant projection_value = p_projections[projection_index];
			if (projection_value.get_type() != Variant::DICTIONARY) {
				continue;
			}
			Dictionary projection_data = projection_value;
			Object* key_object = projection_data.get("key", Variant());
			TimelineTrackKey* key = Object::cast_to<TimelineTrackKey>(key_object);
			Ref<StyleBox> style = get_style_box_from_variant(projection_data.get("style", Variant()));
			const int track_index = int(projection_data.get("track_index", -1));
			if (!key || style.is_null() || track_index < 0 || track_index >= static_cast<int>(_track_cache.size())) {
				continue;
			}
			KeyProjection projection;
			projection.key_id = key->get_instance_id();
			projection.track_index = track_index;
			projection.style = style;
			key_projections.push_back(projection);
		}
		queue_redraw();
	}


	void TimelinePanelBase::set_key_overlap_preview(const TypedArray<TimelineTrackKey>& p_keys) {
		std::vector<TimelineTrackKey*> keys;
		keys.reserve(p_keys.size());
		for (int key_index = 0; key_index < p_keys.size(); ++key_index) {
			TimelineTrackKey* key = Object::cast_to<TimelineTrackKey>(p_keys[key_index]);
			if (key) {
				keys.push_back(key);
			}
		}
		_update_key_release_preview(keys);
		queue_redraw();
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
