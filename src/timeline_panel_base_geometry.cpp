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

#include "theme_helpers.h"

namespace {
	constexpr float INSTANT_KEY_BASE_SIZE = 32.0f;
}

namespace godot {
	std::vector<TimelinePanelBase::BPM> TimelinePanelBase::_get_sorted_bpms() const {
		std::vector<BPM> result;
		Array keys = bpms.keys();
		result.reserve(keys.size());

		for (int index = 0; index < keys.size(); ++index) {
			double time = keys[index];
			Variant value = bpms.get(time, Variant());
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

		std::sort(result.begin(), result.end(), [](const BPM& p_a, const BPM& p_b) {
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
			const BPM& previous = result[index - 1];
			const double sec_delta = MAX(result[index].time - previous.time, 0.0);
			result[index].beat = previous.beat + sec_delta * previous.bpm / 60.0;
			result[index].has_beat = true;
		}

		return result;
	}


	void TimelinePanelBase::_build_time_to_beat_map() {
		Array map;
		const std::vector<BPM> bpm_points = _get_sorted_bpms();

		for (int index = 0; index < static_cast<int>(bpm_points.size()); ++index) {
			const BPM& point = bpm_points[index];
			const double to_sec = index == static_cast<int>(bpm_points.size()) - 1 ? duration : bpm_points[index + 1].time;
			const double to_beat = index == static_cast<int>(bpm_points.size()) - 1 ? _beat_total : bpm_points[index + 1].beat;
			Dictionary result;
			result["from_sec"] = point.time;
			result["to_sec"] = MAX(to_sec, point.time);
			result["from_beat"] = point.beat;
			result["to_beat"] = to_beat;
			result["bpm"] = point.bpm;
			map.append(result);
		}
		beat_map = map;
	}


	void TimelinePanelBase::_build_beat_to_time_map() {
		Array map;
		const std::vector<BPM> bpm_points = _get_sorted_bpms();

		for (int index = 0; index < static_cast<int>(bpm_points.size()); ++index) {
			const BPM& point = bpm_points[index];
			const double to_sec = index == static_cast<int>(bpm_points.size()) - 1 ? duration : bpm_points[index + 1].time;
			const double to_beat = index == static_cast<int>(bpm_points.size()) - 1 ? _beat_total : bpm_points[index + 1].beat;
			Dictionary result;
			result["from_sec"] = point.time;
			result["to_sec"] = MAX(to_sec, point.time);
			result["from_beat"] = point.beat;
			result["to_beat"] = to_beat;
			result["bpm"] = point.bpm;
			map.append(result);
		}
		time_map = map;
	}


	void TimelinePanelBase::_calculate_beat_total() {
		const std::vector<BPM> bpm_points = _get_sorted_bpms();
		if (bpm_points.empty()) {
			_beat_total = 0.0;
			return;
		}

		const BPM& last = bpm_points.back();
		const double sec_left = MAX(duration - last.time, 0.0);
		_beat_total = MAX(last.beat + sec_left * last.bpm / 60.0, 0.0);
	}


	void TimelinePanelBase::_calculate_row_total() {
		_row_total = _beat_total * beats_per_bar;
	}


	double TimelinePanelBase::_time_to_y(double p_time) const {
		if (panel_orientation == PANEL_ORIENTATION_HORIZONTAL) {
			if (counting_unit == BEAT) {
				return _beat_to_y(_time_to_beat(p_time));
			}
			return _get_horizontal_track_header_width() + p_time * scale - hscroll_value;
		}

		if (counting_unit == BEAT) {
			return _beat_to_y(_time_to_beat(p_time));
		}
		else {
			if (bar_number_direction == BAR_NUMBER_BOTTOM_UP) {
				return header_height + content_height - p_time * scale - vscroll_value;
			}
			return header_height + p_time * scale - vscroll_value;
		}
	}


	double TimelinePanelBase::_y_to_time(double p_y) const {
		if (panel_orientation == PANEL_ORIENTATION_HORIZONTAL) {
			if (counting_unit == BEAT) {
				return _beat_to_time(_y_to_beat(p_y));
			}
			double time = (p_y - _get_horizontal_track_header_width() + hscroll_value) / scale;
			return CLAMP(time, 0.0, duration);
		}

		double time;
		if (bar_number_direction == BAR_NUMBER_BOTTOM_UP) {
			if (counting_unit == BEAT) {
				time = _beat_to_time(_y_to_beat(p_y));
			}
			else {
				time = (header_height + content_height - p_y - vscroll_value) / scale;
			}
		}
		else {
			if (counting_unit == BEAT) {
				time = _beat_to_time(_y_to_beat(p_y));
			}
			else {
				time = (p_y - header_height + vscroll_value) / scale;
			}
		}
		if (time < 0) time = 0;
		if (time > duration) time = duration;
		return time;
	}


	double TimelinePanelBase::_time_to_beat(double time_sec) const {
		Array seg_map = beat_map;

		for (int index = 0; index < seg_map.size(); ++index) {
			Dictionary seg = seg_map[index];
			double from_sec = seg.get("from_sec", 0.0);
			double to_sec = seg.get("to_sec", 0.0);
			double from_beat = seg.get("from_beat", 0.0);
			double to_beat = seg.get("to_beat", 0.0);

			if (time_sec >= from_sec && time_sec <= to_sec) {
				const double sec_length = to_sec - from_sec;
				if (sec_length <= 0.0) {
					return from_beat;
				}
				double time = (time_sec - from_sec) / sec_length;
				return UtilityFunctions::lerpf(from_beat, to_beat, time);
			}
		}

		if (seg_map.is_empty()) return 0.0;

		Dictionary last = seg_map[seg_map.size() - 1];
		double to_sec = last.get("to_sec", 0.0);
		double to_beat = last.get("to_beat", 0.0);
		double bpm = last.get("bpm", 120.0);
		if (bpm <= 0.0) bpm = 120.0;
		double extra_sec = time_sec - to_sec;
		double extra_beat_cout = extra_sec * bpm / 60.0;
		return to_beat + extra_beat_cout;
	}


	double TimelinePanelBase::_beat_to_time(double beat) const {
		Array seg_map = time_map;

		for (int index = 0; index < seg_map.size(); ++index) {
			Dictionary seg = seg_map[index];
			double from_sec = seg.get("from_sec", 0.0);
			double from_beat = seg.get("from_beat", 0.0);
			double to_beat = seg.get("to_beat", 0.0);
			double bpm = seg.get("bpm", 120.0);
			if (bpm <= 0.0) bpm = 120.0;

			if (beat < from_beat) continue;
			if (beat >= to_beat) continue;

			const double beat_length = to_beat - from_beat;
			if (beat_length <= 0.0) {
				return from_sec;
			}
			double time = (beat - from_beat) / beat_length;
			return UtilityFunctions::lerpf(from_sec, from_sec + beat_length * 60.0 / bpm, time);
		}

		if (seg_map.is_empty()) return 0.0;

		Dictionary last = seg_map[seg_map.size() - 1];
		double from_sec = last.get("from_sec", 0.0);
		double from_beat = last.get("from_beat", 0.0);
		double bpm = last.get("bpm", 120.0);
		if (bpm <= 0.0) bpm = 120.0;
		double extra_beats = beat - from_beat;
		double extra_sec = extra_beats * 60.0 / bpm;
		return from_sec + extra_sec;
	}


	double TimelinePanelBase::_beat_to_y(double p_beat) const {
		if (panel_orientation == PANEL_ORIENTATION_HORIZONTAL) {
			return _get_horizontal_track_header_width() + p_beat * scale - hscroll_value;
		}

		if (bar_number_direction == BAR_NUMBER_BOTTOM_UP) {
			return header_height + content_height - p_beat * scale - vscroll_value;
		}
		return header_height + p_beat * scale - vscroll_value;
	}


	double TimelinePanelBase::_y_to_beat(double p_y) const {
		if (panel_orientation == PANEL_ORIENTATION_HORIZONTAL) {
			double beat = (p_y - _get_horizontal_track_header_width() + hscroll_value) / scale;
			if (beat < 0) beat = 0;
			if (beat > _beat_total) beat = _beat_total;
			return beat;
		}

		double beat;
		if (bar_number_direction == BAR_NUMBER_BOTTOM_UP) {
			beat = (header_height + content_height - p_y - vscroll_value) / scale;
		}
		else {
			beat = (p_y - header_height + vscroll_value) / scale;
		}
		if (beat < 0) beat = 0;
		if (beat > _beat_total) beat = _beat_total;
		return beat;
	}


	double TimelinePanelBase::_frame_to_y(int64_t p_frame) const {
		if (panel_orientation == PANEL_ORIENTATION_HORIZONTAL) {
			return _get_horizontal_track_header_width() + p_frame * scale - hscroll_value;
		}

		if (bar_number_direction == BAR_NUMBER_BOTTOM_UP) {
			return header_height + content_height - p_frame * scale - vscroll_value;
		}
		return header_height + p_frame * scale - vscroll_value;
	}


	int64_t TimelinePanelBase::_y_to_frame(double p_y) const {
		if (panel_orientation == PANEL_ORIENTATION_HORIZONTAL) {
			double frame = (p_y - _get_horizontal_track_header_width() + hscroll_value) / scale;
			if (frame < 0) frame = 0;
			double total_frames = duration * fps;
			if (frame > total_frames) frame = total_frames;
			return static_cast<int64_t>(frame);
		}

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


	void TimelinePanelBase::_refresh_track_key_metrics() {
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


	void TimelinePanelBase::_get_visible_key_time_range(float p_y_margin, double& r_start, double& r_end) const {
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


	double TimelinePanelBase::_key_to_y(const TimelineTrackKey* p_key) const {
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


	double TimelinePanelBase::_key_end_to_y(const TimelineTrackKey* p_key) const {
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


	float TimelinePanelBase::_get_instant_key_scale(const TimelineTrackKey* p_key) const {
		if (p_key) {
			const float key_scale = p_key->get_instant_key_scale();
			if (key_scale != 0.4f) {
				return key_scale;
			}
		}
		if (!style_cache.theme_caches_valid) {
			_load_theme_stylebox_caches();
		}
		if (style_cache.instant_key_scale_theme >= 0) {
			return static_cast<float>(style_cache.instant_key_scale_theme) / 100.0f;
		}
		if (style_cache.instant_key_scale != 0.4f) {
			return style_cache.instant_key_scale;
		}
		return 0.4f;
	}


	float TimelinePanelBase::_get_instant_key_size(const TimelineTrackKey* p_key) const {
		return MAX(INSTANT_KEY_BASE_SIZE * _get_instant_key_scale(p_key), 0.0f);
	}


	Rect2 TimelinePanelBase::_get_instant_key_rect(const CachedTrack& p_track, const TimelineTrackKey* p_key, double p_y) const {
		float key_size = _get_instant_key_size(p_key);
		if (panel_orientation == PANEL_ORIENTATION_HORIZONTAL) {
			float pos_x = static_cast<float>(p_y) - key_size * 0.5f;
			float pos_y = _get_horizontal_ruler_height() + p_track.x_offset - vscroll_value + (p_track.width - key_size) * 0.5f;
			return Rect2(pos_x, pos_y, key_size, key_size);
		}

		float pos_x = p_track.x_offset - hscroll_value + (p_track.width - key_size) * 0.5f;
		float pos_y = static_cast<float>(p_y) - key_size * 0.5f;
		return Rect2(pos_x, pos_y, key_size, key_size);
	}


	Rect2 TimelinePanelBase::_get_clip_key_rect(const CachedTrack& p_track, double p_y, double p_y_end) const {
		float y = static_cast<float>(p_y);
		float y_end = static_cast<float>(p_y_end);
		if (panel_orientation == PANEL_ORIENTATION_HORIZONTAL) {
			if (y_end < y) {
				return Rect2(y_end, _get_horizontal_ruler_height() + p_track.x_offset - vscroll_value, y - y_end, p_track.width);
			}
			return Rect2(y, _get_horizontal_ruler_height() + p_track.x_offset - vscroll_value, y_end - y, p_track.width);
		}

		if (y_end < y) {
			return Rect2(p_track.x_offset - hscroll_value, y_end, p_track.width, y - y_end);
		}
		return Rect2(p_track.x_offset - hscroll_value, y, p_track.width, y_end - y);
	}


	String TimelinePanelBase::_format_indicator_time(double p_time) const {
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


	float TimelinePanelBase::_get_time_ruler_width() const {
		return time_ruler.is_valid() ? time_ruler->get_width() : 0.0f;
	}


	float TimelinePanelBase::_get_horizontal_track_header_width() const {
		return MAX(header_height, 0.0f);
	}


	float TimelinePanelBase::_get_horizontal_track_height() const {
		return 32.0f;
	}


	float TimelinePanelBase::_get_horizontal_ruler_height() const {
		return _get_time_ruler_width();
	}


	float TimelinePanelBase::_calculate_track_span() const {
		float span = 0.0f;
		for (const TrackData &track : tracks) {
			span += track.height;
		}
		return span;
	}


	float TimelinePanelBase::_calculate_header_width() const {
		if (panel_orientation == PANEL_ORIENTATION_HORIZONTAL) {
			return _get_horizontal_track_header_width() + _calculate_grid_height();
		}
		return _get_time_ruler_width() + _calculate_track_span();
	}


	float TimelinePanelBase::_calculate_grid_height() const {
		switch (counting_unit) {
		case BEAT: {
			double beat_height = _beat_total * scale;
			if (beat_height < scale) beat_height = scale;
			return beat_height;
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


	Vector2 TimelinePanelBase::_get_minimum_size() const {
		float min_width = 0.0f;
		const float top_header_height = panel_orientation == PANEL_ORIENTATION_HORIZONTAL ? _get_horizontal_ruler_height() : header_height;
		float min_height = top_header_height;

		if (horizontal_scroll_mode == SCROLL_MODE_DISABLED) {
			min_width = _calculate_header_width();
			bool v_scroll_show = vertical_scroll_mode == SCROLL_MODE_SHOW_ALWAYS ||
				vertical_scroll_mode == SCROLL_MODE_RESERVE ||
				(vertical_scroll_mode == SCROLL_MODE_AUTO && content_height > get_size().y - top_header_height);
			if (v_scroll_show && vscroll && vscroll->get_parent()) {
				min_width += vscroll->get_minimum_size().x;
				if (draw_minimap && minimap_width > 0) {
					min_width += minimap_width;
				}
			}
		}

		if (vertical_scroll_mode == SCROLL_MODE_DISABLED) {
			min_height = top_header_height + content_height;
			bool h_scroll_show = horizontal_scroll_mode == SCROLL_MODE_SHOW_ALWAYS ||
				horizontal_scroll_mode == SCROLL_MODE_RESERVE ||
				(horizontal_scroll_mode == SCROLL_MODE_AUTO && _calculate_header_width() > get_size().x);
			if (h_scroll_show && hscroll && hscroll->get_parent()) {
				min_height += hscroll->get_minimum_size().y;
			}
		}

		return Size2(min_width, min_height);
	}


	double TimelinePanelBase::get_time_from_position(const double p_position) const {
		return _y_to_time(p_position);
	}


	double TimelinePanelBase::get_frame_from_position(const double p_position) const {
		return _y_to_frame(p_position);
	}


	double TimelinePanelBase::get_beat_from_position(const double p_position) const {
		return _y_to_beat(p_position);
	}


	double TimelinePanelBase::get_position_from_time(double p_time) const {
		return _time_to_y(p_time);
	}


	double TimelinePanelBase::get_position_from_frame(int64_t p_frame) const {
		return _frame_to_y(p_frame);
	}


	double TimelinePanelBase::get_position_from_beat(double p_beat) const {
		return _beat_to_y(p_beat);
	}


}
