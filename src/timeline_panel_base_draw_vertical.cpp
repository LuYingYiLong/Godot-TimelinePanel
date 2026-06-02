#include "timeline_panel_base.h"

#include "components/timeline_indicator.h"
#include "components/timeline_marker.h"
#include "components/timeline_time_ruler.h"
#include "components/timeline_track.h"
#include "components/timeline_track_key.h"

#include <algorithm>
#include <unordered_set>
#include <godot_cpp/classes/input.hpp>
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
	void TimelinePanelBase::_notification(int p_what) {
		switch (p_what) {
		case NOTIFICATION_MOUSE_EXIT:
		case NOTIFICATION_MOUSE_EXIT_SELF: {
			if (!header_resizing && !middle_mouse_panning && !clip_key_edge_dragging && !key_dragging) {
				set_default_cursor_shape(Control::CURSOR_ARROW);
			}
		} break;

		case NOTIFICATION_RESIZED: {
			_update_scroll_bar();
		} break;

		case NOTIFICATION_INTERNAL_PROCESS: {
			const double delta = get_process_delta_time();
			if (!Input::get_singleton()->is_mouse_button_pressed(MouseButton::MOUSE_BUTTON_LEFT)) {
				if (key_dragging) {
					_finish_key_drag();
				}
				if (clip_key_edge_dragging) {
					_finish_clip_key_edge_drag();
				}
			}
			if (select_pending) {
				select_timer += delta;
				if (select_timer >= long_press_time && !beyond_deadzone) {
					select_pending = false;
					selecting = true;
					// 退出滚动状态，防止后续 motion 被当成滚动
					drag_touching = false;
					beyond_deadzone = false;
					queue_redraw();
				}
			}
			_update_selection_auto_scroll(delta);
			_update_key_drag_auto_scroll(delta);
			_stop_internal_process_if_idle();
		} break;

		case NOTIFICATION_DRAW: {
			if (panel_orientation == PANEL_ORIENTATION_HORIZONTAL) {
				_draw_horizontal_panel();
				break;
			}

			draw_rect(Rect2(Vector2(0, 0), get_size()), background_color);

			// 先计算 header_width
			header_width = _calculate_header_width();
			content_width = header_width;
			_sync_track_cache_geometry();

			// 计算内容高度并更新滚动条
			_update_content_height();

			for (int64_t i = 0; i < tracks.size() && i < static_cast<int64_t>(_track_cache.size()); i++) {
				Ref<TimelineTrack> track = tracks[i];
				if (track.is_null()) continue;

				const Color track_background = track->get_background();
				if (track_background.a <= 0.0f) continue;

				const CachedTrack &ct = _track_cache[i];
				if (ct.width <= 0.0f) continue;

				const float x = ct.x_offset - hscroll_value;
				if (x + ct.width < 0.0f || x > get_size().x) continue;

				draw_rect(Rect2(Vector2(x, header_height), Vector2(ct.width, MAX(get_size().y - header_height, 0.0f))), track_background);
			}

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
				const float separator_x = start_pos.x + width - hscroll_value;
				if (separator_x >= 0.0f && separator_x <= get_size().x) {
					draw_line(Point2(separator_x, header_height), Point2(separator_x, get_size().y), separator_color, separator_width);
				}
				start_pos.x += width;
			}
			for (const CachedTrack &ct : _track_cache) {
				if (ct.width <= 0.0f) continue;

				const float separator_x = ct.x_offset - hscroll_value + ct.width;
				if (separator_x < 0.0f || separator_x > get_size().x) continue;
				draw_line(Point2(separator_x, header_height), Point2(separator_x, get_size().y), separator_color, separator_width);
			}

			// 绘制轨道键
			Rect2 key_cull_rect(Vector2(0.0f, header_height), Vector2(get_size().x, MAX(get_size().y - header_height, 0.0f)));
			double visible_start = 0.0;
			double visible_end = 0.0;
			_get_visible_key_time_range(16.0f, visible_start, visible_end);
			Ref<StyleBox> key_release_preview_style = _get_key_release_preview_style();
			std::vector<Rect2> key_release_preview_rects;
			struct DeferredStyleDraw {
				Rect2 rect;
				Ref<StyleBox> style;
			};
			std::vector<DeferredStyleDraw> selected_key_overlays;

			for (size_t i = 0; i < _track_cache.size(); i++) {
				const auto &ct = _track_cache[i];
				if (ct.width <= 0.0f) continue;

				float track_x = ct.x_offset - hscroll_value;
				if (track_x + ct.width < 0.0f || track_x > get_size().x) continue;

				const float max_instant_key_size = INSTANT_KEY_BASE_SIZE * MAX(ct.max_instant_key_scale, 0.0f);
				float key_margin = MAX(16.0f, max_instant_key_size * 0.5f + 16.0f);
				const float instant_lod_bucket_size = CLAMP(max_instant_key_size * 0.2f, 2.0f, 8.0f);
				double padded_start = 0.0;
				double padded_end = 0.0;
				_get_visible_key_time_range(key_margin, padded_start, padded_end);

				const double search_start = MIN(padded_start, visible_start - MAX(ct.max_key_length, 0.0));
				const double search_end = padded_end;
				auto key_it = std::lower_bound(ct.keys.begin(), ct.keys.end(), search_start,
					[](TimelineTrackKey* p_key, double p_time) {
						return p_key && p_key->get_time() < p_time;
					});
				Rect2 track_cull_rect = key_cull_rect.grow(key_margin);

				struct InstantKeyBucket {
					bool active = false;
					int index = 0;
					int count = 0;
					Rect2 rect;
					Ref<StyleBox> style;
				};
				InstantKeyBucket instant_key_bucket;
				InstantKeyBucket selected_instant_key_bucket;
				auto append_instant_key_bucket = [&](InstantKeyBucket &p_bucket, const Rect2 &p_rect, const Ref<StyleBox> &p_style, int p_bucket_index) {
					if (p_bucket.active && p_bucket.index != p_bucket_index) {
						if (p_bucket.style.is_valid()) {
							draw_style_box(p_bucket.style, p_bucket.rect);
						}
						p_bucket.active = false;
						p_bucket.count = 0;
					}
					if (!p_bucket.active) {
						p_bucket.active = true;
						p_bucket.index = p_bucket_index;
						p_bucket.count = 1;
						p_bucket.rect = p_rect;
						p_bucket.style = p_style;
					}
					else {
						p_bucket.count++;
						p_bucket.rect = p_bucket.rect.merge(p_rect);
						if (!p_bucket.style.is_valid()) {
							p_bucket.style = p_style;
						}
					}
				};
				auto flush_instant_key_bucket = [&]() {
					if (!instant_key_bucket.active) {
						return;
					}
					if (instant_key_bucket.style.is_valid()) {
						draw_style_box(instant_key_bucket.style, instant_key_bucket.rect);
					}
					instant_key_bucket.active = false;
					instant_key_bucket.count = 0;
				};
				auto flush_selected_instant_key_bucket = [&]() {
					if (!selected_instant_key_bucket.active) {
						return;
					}
					if (selected_instant_key_bucket.style.is_valid()) {
						selected_key_overlays.push_back({ selected_instant_key_bucket.rect, selected_instant_key_bucket.style });
					}
					selected_instant_key_bucket.active = false;
					selected_instant_key_bucket.count = 0;
				};

				for (; key_it != ct.keys.end(); ++key_it) {
					TimelineTrackKey *key = *key_it;
					if (!key || key->is_disabled()) continue;

					const double key_start = key->get_time();
					if (key_start > search_end) {
						break;
					}

					if (key->is_instant()) {
						Rect2 key_rect = _get_instant_key_rect(ct, key, _key_to_y(key));
						if (!track_cull_rect.intersects(key_rect)) continue;

						const bool release_previewed = _is_key_release_previewed(key);
						if (!release_previewed) {
							const int bucket_index = static_cast<int>(Math::floor((key_rect.get_center().y - header_height) / instant_lod_bucket_size));
							Ref<StyleBox> style = _get_instant_key_normal_style(key);
							append_instant_key_bucket(instant_key_bucket, key_rect, style, bucket_index);
							if (key->is_selected()) {
								style = _get_instant_key_selected_style(key);
								if (selected_instant_key_bucket.active && selected_instant_key_bucket.index != bucket_index) {
									flush_selected_instant_key_bucket();
								}
								append_instant_key_bucket(selected_instant_key_bucket, key_rect, style, bucket_index);
							}
							continue;
						}

						flush_instant_key_bucket();
						Ref<StyleBox> style = _get_instant_key_normal_style(key);
						if (style.is_valid()) {
							draw_style_box(style, key_rect);
						}
						if (key->is_selected()) {
							style = _get_instant_key_selected_style(key);
							if (style.is_valid()) {
								selected_key_overlays.push_back({ key_rect, style });
							}
						}
						if (release_previewed) {
							key_release_preview_rects.push_back(key_rect);
						}
					}
					else {
						flush_instant_key_bucket();
						// 防止矩形尺寸出现负数
						if (key_start > visible_end) continue;

						const double key_end = key_start + key->get_length();
						if (key_end < visible_start) continue;

						Rect2 bar_rect = _get_clip_key_rect(ct, _key_to_y(key), _key_end_to_y(key));
						if (!track_cull_rect.intersects(bar_rect)) continue;

						Ref<StyleBox> style = _get_clip_key_normal_style(key);
						if (style.is_valid()) {
							draw_style_box(style, bar_rect);
						}
						if (key->is_selected()) {
							style = _get_clip_key_selected_style(key);
							if (style.is_valid()) {
								selected_key_overlays.push_back({ bar_rect, style });
							}
						}
						if (_is_key_release_previewed(key)) {
							key_release_preview_rects.push_back(bar_rect);
						}
					}
				}
				flush_instant_key_bucket();
				flush_selected_instant_key_bucket();
			}

			for (const DeferredStyleDraw &overlay : selected_key_overlays) {
				if (overlay.style.is_valid()) {
					draw_style_box(overlay.style, overlay.rect);
				}
			}

			if (key_release_preview_style.is_valid()) {
				for (const Rect2 &preview_rect : key_release_preview_rects) {
					draw_style_box(key_release_preview_style, preview_rect);
				}
			}

			// 绘制指示器
			if (time_ruler.is_valid()) {
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
					float current_width = -hscroll_value;
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
					Rect2 header_rect = Rect2(Vector2(-hscroll_value, current_position - 8.0f), Vector2(time_ruler->get_width(), 16.0f));
					indicator->draw(get_canvas_item(), header_rect, _format_indicator_time(time), header_width, false);
				}
			}

			if (selecting || right_selecting) {
				Rect2 sel_rect = selecting ? _make_selection_rect(select_start, select_end) : _make_selection_rect(right_select_start, right_select_end);

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
			if (time_ruler.is_valid()) {
				float width = time_ruler->get_width();
				const float x = -hscroll_value;
				if (x + width >= 0.0f && x <= get_size().x) {
					Ref<StyleBox> header_background = time_ruler->get_header_background();
					Ref<Texture2D> header_icon = time_ruler->get_header_icon();
					_draw_header(Point2(x, 0.0f), width, header_background, header_icon);
				}
			}
			for (int64_t i = 0; i < tracks.size() && i < static_cast<int64_t>(_track_cache.size()); i++) {
				Ref<TimelineTrack> track = tracks[i];
				if (track.is_null()) continue;

				const CachedTrack &ct = _track_cache[i];
				if (ct.width <= 0.0f) continue;

				float x = ct.x_offset - hscroll_value;
				if (x + ct.width < 0.0f || x > get_size().x) continue;

				Ref<StyleBox> header_background = track->get_header_background();
				Ref<Texture2D> header_icon = track->get_header_icon();
				_draw_header(Point2(x, 0.0f), ct.width, header_background, header_icon, track->get_text(), track->get_header_indent());
			}

			draw_line(Point2(header_width - hscroll_value, header_height), Point2(header_width - hscroll_value, get_size().y), separator_color, separator_width);
			_draw_minimap();
		} break;
		}
	}


	void TimelinePanelBase::_draw_header(const Point2 &pos, const float width, Ref<StyleBox> header_bg, Ref<Texture2D> header_icon, const String &p_text, float p_indent) {
		_draw_header_rect(Rect2(pos, Size2(width, header_height)), header_bg, header_icon, p_text, p_indent);
	}


	void TimelinePanelBase::_draw_header_rect(const Rect2 &p_rect, Ref<StyleBox> p_header_bg, Ref<Texture2D> p_header_icon, const String &p_text, float p_indent) {
		if (p_header_bg.is_valid()) {
			draw_style_box(p_header_bg, p_rect);
		}

		const bool has_text = !p_text.is_empty();
		const float padding = has_text ? 4.0f : 0.0f;
		const float indent = has_text ? MAX(p_indent, 0.0f) : 0.0f;
		const float content_left = p_rect.position.x + indent;
		const float max_icon_width = MAX(p_rect.position.x + p_rect.size.x - content_left - padding * 2.0f, 0.0f);
		const float max_icon_height = MAX(p_rect.size.y, 0.0f);
		float text_left = content_left + padding;
		if (p_header_icon.is_valid() && max_icon_width > 0.0f && max_icon_height > 0.0f) {
			Size2 tex_size = p_header_icon->get_size();
			if (tex_size.x > 0.0f && tex_size.y > 0.0f) {
				const float max_icon_size = has_text ? MIN(MIN(max_icon_width, max_icon_height), 18.0f) : MIN(max_icon_width, max_icon_height);
				if (max_icon_size > 0.0f) {
					const float scale = MIN(max_icon_size / tex_size.x, max_icon_size / tex_size.y);
					const Size2 scaled_tex_size = tex_size * scale;
					const float icon_x = has_text ? content_left + padding : p_rect.position.x + (p_rect.size.x - scaled_tex_size.x) * 0.5f;
					const float icon_y = p_rect.position.y + (p_rect.size.y - scaled_tex_size.y) * 0.5f;
					draw_texture_rect(p_header_icon, Rect2(Point2(icon_x, icon_y), scaled_tex_size), false);
					text_left = icon_x + scaled_tex_size.x + padding;
				}
			}
		}

		if (has_text) {
			const Ref<Font> font = ThemeDB::get_singleton()->get_fallback_font();
			if (font.is_valid()) {
				const int font_size = 12;
				const float text_width = MAX(p_rect.position.x + p_rect.size.x - padding - text_left, 0.0f);
				const float baseline = p_rect.position.y + (p_rect.size.y - static_cast<float>(font_size)) * 0.5f + static_cast<float>(font_size);
				if (text_width > 0.0f) {
					draw_string(font, Point2(text_left, baseline), p_text, HORIZONTAL_ALIGNMENT_LEFT, text_width, font_size, Color(1.0f, 1.0f, 1.0f, 0.88f));
				}
			}
		}
	}


	void TimelinePanelBase::_draw_time_ruler_ticks(float p_header_width) {
		if (time_ruler.is_null()) return;

		const float ruler_width = time_ruler->get_width();
		const float ruler_left = -hscroll_value;
		const float ruler_right = ruler_left + ruler_width;
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
			double start_beat = _y_to_beat(visible_start_y);
			double end_beat = _y_to_beat(visible_end_y);
			const int divisions = MAX(beats_per_bar, 1);
			int start_row = Math::floor(Math::min(start_beat, end_beat) * divisions);
			int end_row = Math::ceil(Math::max(start_beat, end_beat) * divisions);

			for (int row = start_row; row <= end_row; row++) {
				float y = _beat_to_y(static_cast<double>(row) / divisions);
				if (y < visible_start_y || y > visible_end_y) continue;

				bool is_bar_line = (row % divisions) == 0;
				float tick_height = is_bar_line ? major_tick_height : minor_tick_height;
				float tick_width = is_bar_line ? major_tick_width : minor_tick_width;

				draw_line(
					Point2(ruler_right - tick_height, y),
					Point2(ruler_right, y),
					tick_color,
					tick_width
				);

				if (is_bar_line) {
					int bar = row / divisions;
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
							draw_string(font, Point2(ruler_left + 2.0f, y + 6.0f), text, HORIZONTAL_ALIGNMENT_LEFT, -1, 12, tick_color);
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
					Point2(ruler_right - tick_width, y),
					Point2(ruler_right, y),
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
							draw_string(font, Point2(ruler_left + 2.0f, y + 6.0f), text + "s", HORIZONTAL_ALIGNMENT_LEFT, -1, 12, Color(1, 1, 1));
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
					Point2(ruler_right - tick_width, y),
					Point2(ruler_right, y),
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
							draw_string(font, Point2(ruler_left + 2.0f, y + 6.0f), text, HORIZONTAL_ALIGNMENT_LEFT, -1, 12, Color(1, 1, 1));
						}
					}
				}
			}
			break;
		}
		}
	}


	void TimelinePanelBase::_draw_grid_beat(float p_header_width) {
		float start_pos = -hscroll_value;
		if (time_ruler.is_valid()) {
			start_pos += time_ruler->get_width();
		}
		const float end_pos = p_header_width - hscroll_value;

		float visible_start_y = header_height;
		float visible_end_y = get_size().y;

		double start_beat = _y_to_beat(visible_start_y);
		double end_beat = _y_to_beat(visible_end_y);
		const int divisions = MAX(beats_per_bar, 1);
		int start_row = Math::floor(Math::min(start_beat, end_beat) * divisions);
		int end_row = Math::ceil(Math::max(start_beat, end_beat) * divisions);

		for (int row = start_row; row <= end_row; row++) {
			float y = _beat_to_y(static_cast<double>(row) / divisions);
			if (y < visible_start_y || y > visible_end_y) continue;

			bool is_beat_line = (row % divisions) == 0;
			Color line_color = is_beat_line ? beat_line_color : bar_line_color;
			float line_w = is_beat_line ? beat_line_width : bar_line_width;

			draw_line(
				Point2(start_pos, y),
				Point2(end_pos, y),
				line_color,
				line_w
			);
		}
	}


	void TimelinePanelBase::_draw_grid_frame(float p_header_width) {
		float start_pos = -hscroll_value;
		if (time_ruler.is_valid()) {
			start_pos += time_ruler->get_width();
		}
		const float end_pos = p_header_width - hscroll_value;

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
				Point2(end_pos, y),
				line_color,
				line_width
			);
		}
	}


	void TimelinePanelBase::_draw_grid_time(float p_header_width) {
		float start_pos = -hscroll_value;
		if (time_ruler.is_valid()) {
			start_pos += time_ruler->get_width();
		}
		const float end_pos = p_header_width - hscroll_value;

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
				Point2(end_pos, y),
				line_color,
				line_width
			);
		}
	}


}
