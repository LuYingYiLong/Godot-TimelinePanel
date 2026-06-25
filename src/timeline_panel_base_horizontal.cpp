#include "timeline_panel_base.h"

#include "components/timeline_indicator.h"
#include "components/timeline_marker.h"
#include "components/timeline_time_ruler.h"
#include "components/timeline_track.h"
#include "components/timeline_track_key.h"

#include <godot_cpp/classes/font.hpp>
#include <godot_cpp/classes/theme_db.hpp>
#include "theme_helpers.h"

namespace godot {
	void TimelinePanelBase::_draw_horizontal_grid() {
		const float header_left = _get_horizontal_track_header_width();
		const float top = _get_horizontal_ruler_height();
		const float bottom = top + content_height - vscroll_value;
		const float visible_left = header_left;
		const float visible_right = get_size().x;
		const double start_value = _y_to_time(visible_left);
		const double end_value = _y_to_time(visible_right);

		auto draw_time_line = [&](double p_time, const Color &p_color, float p_width) {
			const float x = static_cast<float>(_time_to_y(p_time));
			if (x < visible_left || x > visible_right) {
				return;
			}
			draw_line(Point2(x, top), Point2(x, bottom), p_color, p_width);
		};

		switch (counting_unit) {
		case BEAT: {
			const int divisions = MAX(beats_per_bar, 1);
			const double start_beat = _time_to_beat(MIN(start_value, end_value));
			const double end_beat = _time_to_beat(MAX(start_value, end_value));
			const int start_row = static_cast<int>(Math::floor(start_beat * divisions));
			const int end_row = static_cast<int>(Math::ceil(end_beat * divisions));
			for (int row = start_row; row <= end_row; row++) {
				const bool is_beat_line = (row % divisions) == 0;
				const double beat = static_cast<double>(row) / divisions;
				const float x = static_cast<float>(_beat_to_y(beat));
				if (x < visible_left || x > visible_right) {
					continue;
				}
				draw_line(Point2(x, top), Point2(x, bottom), is_beat_line ? beat_line_color : bar_line_color, is_beat_line ? beat_line_width : bar_line_width);
			}
		} break;
		case FRAME: {
			const int safe_fps = MAX(fps, 1);
			int64_t start_frame = static_cast<int64_t>(Math::floor(MIN(start_value, end_value) * safe_fps));
			int64_t end_frame = static_cast<int64_t>(Math::ceil(MAX(start_value, end_value) * safe_fps));
			int frame_interval = 1;
			if (scale < 10) frame_interval = safe_fps;
			else if (scale < 30) frame_interval = MAX(safe_fps / 2, 1);
			start_frame = (start_frame / frame_interval) * frame_interval;
			for (int64_t frame = start_frame; frame <= end_frame; frame += frame_interval) {
				const bool is_second = (frame % safe_fps) == 0;
				const float x = static_cast<float>(_frame_to_y(frame));
				if (x < visible_left || x > visible_right) {
					continue;
				}
				draw_line(Point2(x, top), Point2(x, bottom), is_second ? bar_line_color : beat_line_color, is_second ? bar_line_width : beat_line_width);
			}
		} break;
		case TIME:
		default: {
			double interval = 1.0;
			if (scale < 20) interval = 5.0;
			else if (scale < 40) interval = 2.0;
			else if (scale > 120) interval = 0.5;
			double start_time = Math::floor(MIN(start_value, end_value) / interval) * interval;
			for (double time = start_time; time <= MAX(start_value, end_value); time += interval) {
				draw_time_line(time, Math::is_equal_approx(Math::fmod(time, 1.0), 0.0) ? bar_line_color : beat_line_color, Math::is_equal_approx(Math::fmod(time, 1.0), 0.0) ? bar_line_width : beat_line_width);
			}
		} break;
		}
	}

	void TimelinePanelBase::_draw_horizontal_time_ruler_ticks() {
		if (time_ruler.is_null()) {
			return;
		}

		const float header_left = _get_horizontal_track_header_width();
		const float ruler_top = 0.0f;
		const float ruler_bottom = _get_horizontal_ruler_height();
		const float visible_left = header_left;
		const float visible_right = get_size().x;
		const float major_tick_height = time_ruler->get_major_tick_height();
		const float minor_tick_height = time_ruler->get_minjor_tick_height();
		const float major_tick_width = time_ruler->get_major_tick_width();
		const float minor_tick_width = time_ruler->get_minjor_tick_width();
		const Color tick_color = time_ruler->get_tick_color();
		const Ref<Font> font = ThemeDB::get_singleton()->get_fallback_font();
		const double start_time = _y_to_time(visible_left);
		const double end_time = _y_to_time(visible_right);

		auto draw_tick = [&](float p_x, bool p_major, const String &p_label) {
			if (p_x < visible_left || p_x > visible_right) {
				return;
			}
			const float tick_height = p_major ? major_tick_height : minor_tick_height;
			const float tick_width = p_major ? major_tick_width : minor_tick_width;
			draw_line(Point2(p_x, ruler_bottom - tick_height), Point2(p_x, ruler_bottom), tick_color, tick_width);
			if (p_major && font.is_valid()) {
				draw_string(font, Point2(p_x + 3.0f, ruler_top + 12.0f), p_label, HORIZONTAL_ALIGNMENT_LEFT, -1, 12, tick_color);
			}
		};

		switch (counting_unit) {
		case BEAT: {
			const int divisions = MAX(beats_per_bar, 1);
			const double start_beat = _time_to_beat(MIN(start_time, end_time));
			const double end_beat = _time_to_beat(MAX(start_time, end_time));
			const int start_row = static_cast<int>(Math::floor(start_beat * divisions));
			const int end_row = static_cast<int>(Math::ceil(end_beat * divisions));
			for (int row = start_row; row <= end_row; row++) {
				const bool major = (row % divisions) == 0;
				const double beat = static_cast<double>(row) / divisions;
				draw_tick(static_cast<float>(_beat_to_y(beat)), major, String::num_int64(row / divisions));
			}
		} break;
		case FRAME: {
			const int safe_fps = MAX(fps, 1);
			int64_t start_frame = static_cast<int64_t>(Math::floor(MIN(start_time, end_time) * safe_fps));
			int64_t end_frame = static_cast<int64_t>(Math::ceil(MAX(start_time, end_time) * safe_fps));
			int frame_interval = 1;
			if (scale < 10) frame_interval = safe_fps;
			else if (scale < 30) frame_interval = MAX(safe_fps / 2, 1);
			start_frame = (start_frame / frame_interval) * frame_interval;
			for (int64_t frame = start_frame; frame <= end_frame; frame += frame_interval) {
				const bool major = (frame % safe_fps) == 0;
				draw_tick(static_cast<float>(_frame_to_y(frame)), major, String::num_int64(frame / safe_fps) + "s");
			}
		} break;
		case TIME:
		default: {
			double interval = 1.0;
			if (scale < 20) interval = 5.0;
			else if (scale < 40) interval = 2.0;
			else if (scale > 120) interval = 0.5;
			double time = Math::floor(MIN(start_time, end_time) / interval) * interval;
			for (; time <= MAX(start_time, end_time); time += interval) {
				draw_tick(static_cast<float>(_time_to_y(time)), true, String::num(time, 1));
			}
		} break;
		}
	}

	void TimelinePanelBase::_draw_horizontal_panel() {
		draw_rect(Rect2(Vector2(0, 0), get_size()), _get_background_color());

		header_width = _calculate_header_width();
		content_width = header_width;
		_sync_track_cache_geometry();
		_update_content_height();

		const float header_left = _get_horizontal_track_header_width();
		const float ruler_height = _get_horizontal_ruler_height();
		const float tracks_bottom = ruler_height + content_height - vscroll_value;
		const float hscroll_height = hscroll != nullptr && hscroll->is_visible() ? hscroll->get_combined_minimum_size().y : 0.0f;
		const float visible_bottom = MAX(get_size().y - hscroll_height, ruler_height);
		const float indicator_line_bottom = MIN(visible_bottom, MAX(ruler_height, tracks_bottom));
		const float indicator_line_length = MAX(indicator_line_bottom - ruler_height * 0.5f, 0.0f);

		for (int64_t i = 0; i < tracks.size() && i < static_cast<int64_t>(_track_cache.size()); i++) {
			const Color track_background = tracks[i].background;
			if (track_background.a <= 0.0f) continue;

			const CachedTrack &ct = _track_cache[i];
			if (ct.width <= 0.0f) continue;

			const float y = ruler_height + ct.x_offset - vscroll_value;
			if (y + ct.width < ruler_height || y > get_size().y) continue;

			draw_rect(Rect2(Vector2(header_left, y), Vector2(MAX(get_size().x - header_left, 0.0f), ct.width)), track_background);
		}

		_draw_horizontal_grid();

		for (int64_t i = 0; i < tracks.size() && i < static_cast<int64_t>(_track_cache.size()); i++) {
			const TrackData &track = tracks[i];
			if (!track.header_background_fill_track) continue;

			const CachedTrack &ct = _track_cache[i];
			if (ct.width <= 0.0f) continue;
			const float y = ruler_height + ct.x_offset - vscroll_value;
			if (y + ct.width < ruler_height || y > get_size().y) continue;

			if (track.header_background.is_valid()) {
				draw_style_box(track.header_background, Rect2(Vector2(0.0f, y), Vector2(get_size().x, ct.width)));
			}
		}

		for (const CachedTrack &ct : _track_cache) {
			if (ct.width <= 0.0f) continue;

			const float y = ruler_height + ct.x_offset - vscroll_value;
			if (y + ct.width < ruler_height || y > get_size().y) {
				continue;
			}
			draw_line(Point2(header_left, y + ct.width), Point2(get_size().x, y + ct.width), separator_color, separator_width);
		}

		Rect2 key_cull_rect(Vector2(header_left, ruler_height), Vector2(MAX(get_size().x - header_left, 0.0f), MAX(get_size().y - ruler_height, 0.0f)));
		Ref<StyleBox> key_release_preview_style = _get_key_release_preview_style();
		std::vector<Rect2> key_release_preview_rects;
		Ref<StyleBox> key_allowed_overlap_preview_style = _get_key_allowed_overlap_preview_style();
		std::vector<Rect2> key_allowed_overlap_preview_rects;
		struct DeferredStyleDraw {
			Rect2 rect;
			Ref<StyleBox> style;
		};
		std::vector<DeferredStyleDraw> selected_key_overlays;

		for (size_t i = 0; i < _track_cache.size(); i++) {
			const CachedTrack &ct = _track_cache[i];
			if (ct.width <= 0.0f) continue;

			const float track_y = ruler_height + ct.x_offset - vscroll_value;
			if (track_y + ct.width < ruler_height || track_y > get_size().y) continue;

			for (TimelineTrackKey *key : ct.keys) {
				if (!key || key->is_disabled()) continue;

				Rect2 key_rect;
				if (key->is_instant()) {
					key_rect = _get_instant_key_rect(ct, key, _key_to_y(key));
				}
				else {
					key_rect = _get_clip_key_rect(ct, _key_to_y(key), _key_end_to_y(key));
				}
				if (!key_cull_rect.intersects(key_rect)) continue;

				Ref<StyleBox> style = key->is_instant() ? _get_instant_key_normal_style(key) : _get_clip_key_normal_style(key);
				if (style.is_valid()) {
					draw_style_box(style, key_rect);
				}
				if (key->is_selected()) {
					style = key->is_instant() ? _get_instant_key_selected_style(key) : _get_clip_key_selected_style(key);
					if (style.is_valid()) {
						selected_key_overlays.push_back({ key_rect, style });
					}
				}
				if (_is_key_release_previewed(key)) {
					key_release_preview_rects.push_back(key_rect);
				}
				else if (_is_key_allowed_overlap_previewed(key)) {
					key_allowed_overlap_preview_rects.push_back(key_rect);
				}
			}
		}

		for (const DeferredStyleDraw &overlay : selected_key_overlays) {
			if (overlay.style.is_valid()) {
				draw_style_box(overlay.style, overlay.rect);
			}
		}

		if (key_allowed_overlap_preview_style.is_valid()) {
			for (const Rect2& preview_rect : key_allowed_overlap_preview_rects) {
				draw_style_box(key_allowed_overlap_preview_style, preview_rect);
			}
		}

		if (key_release_preview_style.is_valid()) {
			for (const Rect2 &preview_rect : key_release_preview_rects) {
				draw_style_box(key_release_preview_style, preview_rect);
			}
		}

		_draw_key_projections(key_cull_rect);

		TypedArray<TimelineIndicator> all_indicators;
		if (time_ruler.is_valid()) {
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
				Ref<TimelineIndicator> indicator = all_indicators[i];
				double time = current_time;
				Ref<TimelineMarker> marker = indicator;
				if (marker.is_valid()) {
					time = marker->get_time();
				}
				const float x = static_cast<float>(_time_to_y(time));
				Rect2 header_rect(Vector2(x - 24.0f, 0.0f), Vector2(48.0f, ruler_height));
				indicator->draw(get_canvas_item(), header_rect, _format_indicator_time(time), indicator_line_length, true);
			}
		}

		for (int64_t i = 0; i < tracks.size() && i < static_cast<int64_t>(_track_cache.size()); i++) {
			const CachedTrack &ct = _track_cache[i];
			if (ct.width <= 0.0f) continue;
			const float y = ruler_height + ct.x_offset - vscroll_value;
			if (y + ct.width < ruler_height || y > get_size().y) continue;

			_draw_track_header_rect(static_cast<int>(i), Rect2(Vector2(0.0f, y), Vector2(_get_horizontal_track_header_width(), ct.width)));
		}

		if (time_ruler.is_valid()) {
			Ref<StyleBox> ruler_bg = time_ruler->get_header_background();
			Ref<Texture2D> ruler_icon = time_ruler->get_header_icon();
			_draw_header_rect(Rect2(Vector2(0.0f, 0.0f), Vector2(header_left, ruler_height)), ruler_bg, ruler_icon);
			if (ruler_bg.is_valid()) {
				draw_style_box(ruler_bg, Rect2(Vector2(header_left, 0.0f), Vector2(MAX(get_size().x - header_left, 0.0f), ruler_height)));
			}
		}
		_draw_horizontal_time_ruler_ticks();

		if (time_ruler.is_valid()) {
			for (int i = 0; i < all_indicators.size(); i++) {
				Ref<TimelineIndicator> indicator = all_indicators[i];
				double time = current_time;
				Ref<TimelineMarker> marker = indicator;
				if (marker.is_valid()) {
					time = marker->get_time();
				}
				const float x = static_cast<float>(_time_to_y(time));
				Rect2 header_rect(Vector2(x - 24.0f, 0.0f), Vector2(48.0f, ruler_height));
				indicator->draw(get_canvas_item(), header_rect, _format_indicator_time(time), 0.0, true);
			}
		}

		if (selecting || right_selecting) {
			Rect2 sel_rect = selecting ? _make_selection_rect(select_start, select_end) : _make_selection_rect(right_select_start, right_select_end);
			Ref<StyleBox> style = _get_selection_rect_style();
			if (style.is_valid()) {
				draw_style_box(style, sel_rect);
			}
			else {
				draw_rect(sel_rect, Color(1.0f, 1.0f, 1.0f, 0.3f));
				draw_rect(sel_rect, Color(1.0f, 1.0f, 1.0f), false);
			}
		}

		const float separator_bottom = MIN(visible_bottom, MAX(ruler_height, tracks_bottom));
		draw_line(Point2(header_left, ruler_height), Point2(header_left, separator_bottom), separator_color, separator_width);
		draw_line(Point2(0.0f, ruler_height), Point2(get_size().x, ruler_height), separator_color, separator_width);
		_draw_minimap();
	}
}
