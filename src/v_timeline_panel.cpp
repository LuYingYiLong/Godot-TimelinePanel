#include "v_timeline_panel.h"

#include "components/timeline_panel_time_ruler_component.h"
#include "components/timeline_panel_track_component.h"

#include <godot_cpp/classes/font.hpp>
#include <godot_cpp/classes/theme_db.hpp>

namespace godot {
	void VTimelinePanel::_bind_methods() {
		BIND_ENUM_CONSTANT(TIME);
		BIND_ENUM_CONSTANT(FRAME);
		BIND_ENUM_CONSTANT(BEAT);

		BIND_ENUM_CONSTANT(HH_MM_SS);
		BIND_ENUM_CONSTANT(MM_SS_MS);
		BIND_ENUM_CONSTANT(SEC);

		BIND_ENUM_CONSTANT(BAR_NUMBER_TOP_DOWN);
		BIND_ENUM_CONSTANT(BAR_NUMBER_BOTTOM_UP);

		ClassDB::bind_method(D_METHOD("set_background_color", "background_color"), &VTimelinePanel::set_background_color);
		ClassDB::bind_method(D_METHOD("get_background_color"), &VTimelinePanel::get_background_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "background_color"), "set_background_color", "get_background_color");

		ClassDB::bind_method(D_METHOD("set_header_height", "header_height"), &VTimelinePanel::set_header_height);
		ClassDB::bind_method(D_METHOD("get_header_height"), &VTimelinePanel::get_header_height);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "header_height"), "set_header_height", "get_header_height");

		ClassDB::bind_method(D_METHOD("set_duration", "duration"), &VTimelinePanel::set_duration);
		ClassDB::bind_method(D_METHOD("get_duration"), &VTimelinePanel::get_duration);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "duration", PROPERTY_HINT_RANGE, "0,99999,0.01,or_greater,suffix:s"), "set_duration", "get_duration");

		ClassDB::bind_method(D_METHOD("set_scale", "scale"), &VTimelinePanel::set_scale);
		ClassDB::bind_method(D_METHOD("get_scale"), &VTimelinePanel::get_scale);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "scale", PROPERTY_HINT_RANGE, "1,128,0.1,or_greater"), "set_scale", "get_scale");

		ClassDB::bind_method(D_METHOD("set_counting_unit", "unit"), &VTimelinePanel::set_counting_unit);
		ClassDB::bind_method(D_METHOD("get_counting_unit"), &VTimelinePanel::get_counting_unit);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "counting_unit", PROPERTY_HINT_ENUM, "Time,Frame,Beat"), "set_counting_unit", "get_counting_unit");

		ADD_GROUP("Time", "time_");
		ClassDB::bind_method(D_METHOD("set_time_format", "format"), &VTimelinePanel::set_time_format);
		ClassDB::bind_method(D_METHOD("get_time_format"), &VTimelinePanel::get_time_format);
		ADD_PROPERTY(PropertyInfo(Variant::STRING, "time_format"), "set_time_format", "get_time_format");

		ClassDB::bind_method(D_METHOD("set_show_milliseconds", "show_milliseconds"), &VTimelinePanel::set_show_milliseconds);
		ClassDB::bind_method(D_METHOD("get_show_milliseconds"), &VTimelinePanel::get_show_milliseconds);
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "time_show_milliseconds"), "set_show_milliseconds", "get_show_milliseconds");

		ADD_GROUP("Frame", "frame_");
		ClassDB::bind_method(D_METHOD("set_fps", "fps"), &VTimelinePanel::set_fps);
		ClassDB::bind_method(D_METHOD("get_fps"), &VTimelinePanel::get_fps);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "frame_fps"), "set_fps", "get_fps");

		ClassDB::bind_method(D_METHOD("set_show_subdivision", "show_subdivision"), &VTimelinePanel::set_show_subdivision);
		ClassDB::bind_method(D_METHOD("get_show_subdivision"), &VTimelinePanel::get_show_subdivision);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "frame_show_subdivision"), "set_show_subdivision", "get_show_subdivision");

		ADD_GROUP("Beat", "beat_");
		ClassDB::bind_method(D_METHOD("set_bpm", "bpm"), &VTimelinePanel::set_bpm);
		ClassDB::bind_method(D_METHOD("get_bpm"), &VTimelinePanel::get_bpm);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "beat_bpm"), "set_bpm", "get_bpm");

		ClassDB::bind_method(D_METHOD("set_beat_per_bar", "num"), &VTimelinePanel::set_beat_per_bar);
		ClassDB::bind_method(D_METHOD("get_beat_per_bar"), &VTimelinePanel::get_beat_per_bar);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "beat_per_bar"), "set_beat_per_bar", "get_beat_per_bar");

		ClassDB::bind_method(D_METHOD("set_bar_line_color", "color"), &VTimelinePanel::set_bar_line_color);
		ClassDB::bind_method(D_METHOD("get_bar_line_color"), &VTimelinePanel::get_bar_line_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "beat_bar_line_color"), "set_bar_line_color", "get_bar_line_color");

		ClassDB::bind_method(D_METHOD("set_bar_line_width", "width"), &VTimelinePanel::set_bar_line_width);
		ClassDB::bind_method(D_METHOD("get_bar_line_width"), &VTimelinePanel::get_bar_line_width);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "beat_bar_line_width"), "set_bar_line_width", "get_bar_line_width");

		ClassDB::bind_method(D_METHOD("set_beat_line_color", "color"), &VTimelinePanel::set_beat_line_color);
		ClassDB::bind_method(D_METHOD("get_beat_line_color"), &VTimelinePanel::get_beat_line_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "beat_beat_line_color"), "set_beat_line_color", "get_beat_line_color");

		ClassDB::bind_method(D_METHOD("set_beat_line_width", "width"), &VTimelinePanel::set_beat_line_width);
		ClassDB::bind_method(D_METHOD("get_beat_line_width"), &VTimelinePanel::get_beat_line_width);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "beat_beat_line_width"), "set_beat_line_width", "get_beat_line_width");

		ClassDB::bind_method(D_METHOD("set_bar_number_direction", "direction"), &VTimelinePanel::set_bar_number_direction);
		ClassDB::bind_method(D_METHOD("get_bar_number_direction"), &VTimelinePanel::get_bar_number_direction);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "beat_bar_number_direction", PROPERTY_HINT_ENUM, "Top Down,Bottom Up"), "set_bar_number_direction", "get_bar_number_direction");
		ADD_GROUP("", "");

		ADD_GROUP("Separator", "separator_");
		ClassDB::bind_method(D_METHOD("set_separator_color", "separator_color"), &VTimelinePanel::set_separator_color);
		ClassDB::bind_method(D_METHOD("get_separator_color"), &VTimelinePanel::get_separator_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "separator_color"), "set_separator_color", "get_separator_color");

		ClassDB::bind_method(D_METHOD("set_separator_width", "separator_width"), &VTimelinePanel::set_separator_width);
		ClassDB::bind_method(D_METHOD("get_separator_width"), &VTimelinePanel::get_separator_width);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "separator_width"), "set_separator_width", "get_separator_width");
		ADD_GROUP("", "");

		ADD_GROUP("Components", "component_");
		ClassDB::bind_method(D_METHOD("set_time_ruler_component", "time_ruler_component"), &VTimelinePanel::set_time_ruler_component);
		ClassDB::bind_method(D_METHOD("get_time_ruler_component"), &VTimelinePanel::get_time_ruler_component);
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "component_time_ruler", PROPERTY_HINT_RESOURCE_TYPE, "TimelinePanelTimeRulerComponent"), "set_time_ruler_component", "get_time_ruler_component");

		ClassDB::bind_method(D_METHOD("set_track_components", "track_components"), &VTimelinePanel::set_track_components);
		ClassDB::bind_method(D_METHOD("get_track_components"), &VTimelinePanel::get_track_components);
		ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "component_track", PROPERTY_HINT_ARRAY_TYPE, "TimelinePanelTrackComponent", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_EDITOR_INSTANTIATE_OBJECT), "set_track_components", "get_track_components");
	}

	void VTimelinePanel::_notification(int p_what) {
		switch (p_what) {
		case NOTIFICATION_DRAW: {
			draw_rect(Rect2(Vector2(0, 0), get_size()), background_color);
			Vector2 start_pos;

			// 处理时间尺组件
			if (time_ruler_component.is_valid()) {
				float width = time_ruler_component->get_width();
				Color header_color = time_ruler_component->get_header_color();
				Ref<Texture2D> header_icon = time_ruler_component->get_header_icon();
				_draw_header(start_pos, width, header_color, header_icon);

				start_pos.x += width;
			}
			draw_line(Point2(start_pos.x, header_height), Point2(start_pos.x, get_size().y), separator_color, separator_width);

			// 处理轨道组件
			for (int64_t i = 0; i < track_components.size(); i++) {
				Ref<TimelinePanelTrackComponent> track_component = track_components[i];
				if (track_component.is_null()) continue;

				float width = track_component->get_width();
				Color header_color = track_component->get_header_color();
				Ref<Texture2D> header_icon = track_component->get_header_icon();
				_draw_header(start_pos, width, header_color, header_icon);
				draw_line(Point2(start_pos.x + width, header_height), Point2(start_pos.x + width, get_size().y), separator_color, separator_width);

				start_pos.x += width;
			}

			// 更新 header_width 供后续使用
			header_width = start_pos.x;

			// 绘制网格和时间尺刻度
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
		} break;
		}
	}

	void VTimelinePanel::_validate_property(PropertyInfo& p_property) const {
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

	VTimelinePanel::VTimelinePanel() {

	}

	void VTimelinePanel::_draw_header(const Point2& pos, const float width, const Color& header_color, Ref<Texture2D> header_icon) {
		draw_rect(Rect2(pos, Size2(width, header_height)), header_color);

		if (header_icon.is_valid()) {
			Size2 tex_size = header_icon->get_size();
			// 计算缩放比例，选择较小的那个以保持比例并限制在区域内
			Size2 scale_size(width / tex_size.width, header_height / tex_size.height);
			float scale = scale_size.width < scale_size.height ? scale_size.width : scale_size.height;
			Size2 scaled_tex_size = tex_size * scale;
			// 计算居中偏移
			Point2 offset = (Size2(width, header_height) - scaled_tex_size) / 2;
			draw_texture_rect(header_icon, Rect2(pos + offset, scaled_tex_size), false);
		}
	}

	float VTimelinePanel::_time_to_y(double p_time) const {
		if (bar_number_direction == BAR_NUMBER_BOTTOM_UP) {
			return get_size().y - p_time * scale;
		}
		return header_height + p_time * scale;
	}

	double VTimelinePanel::_y_to_time(float p_y) const {
		if (bar_number_direction == BAR_NUMBER_BOTTOM_UP) {
			return (get_size().y - p_y) / scale;
		}
		return (p_y - header_height) / scale;
	}

	float VTimelinePanel::_beat_to_y(double p_beat) const {
		double beat_duration = 60.0 / bpm;
		double time = p_beat * beat_duration;
		return _time_to_y(time);
	}

	double VTimelinePanel::_y_to_beat(float p_y) const {
		double time = _y_to_time(p_y);
		double beat_duration = 60.0 / bpm;
		return time / beat_duration;
	}

	float VTimelinePanel::_frame_to_y(int64_t p_frame) const {
		double time = (double)p_frame / fps;
		return _time_to_y(time);
	}

	int64_t VTimelinePanel::_y_to_frame(float p_y) const {
		double time = _y_to_time(p_y);
		return static_cast<int64_t>(time * fps);
	}

	void VTimelinePanel::_draw_time_ruler_ticks(float p_header_width) {
		if (time_ruler_component.is_null()) return;

		float ruler_width = time_ruler_component->get_width();
		float major_tick_width = time_ruler_component->get_major_tick_width();
		float minor_tick_width = time_ruler_component->get_minjor_tick_width();
		Color tick_color = time_ruler_component->get_tick_color();
		float visible_start_y = header_height;
		float visible_end_y = get_size().y;

		switch (counting_unit) {
		case BEAT: {
			// 计算可见范围内的小节和拍
			double start_beat = _y_to_beat(visible_start_y);
			double end_beat = _y_to_beat(visible_end_y);
			// 确保 start_bar <= end_bar (考虑 Bottom Up 模式)
			int start_bar = Math::floor(Math::min(start_beat, end_beat) / beats_per_bar);
			int end_bar = Math::ceil(Math::max(start_beat, end_beat) / beats_per_bar);

			// 绘制小节数字
			for (int bar = start_bar; bar <= end_bar; bar++) {
				float y = _beat_to_y(bar * beats_per_bar);
				if (y < visible_start_y || y > visible_end_y) continue;

				draw_line(
					Point2(ruler_width - 12.0f, y),
					Point2(ruler_width, y),
					tick_color,
					major_tick_width
				);

				// 小节数字，转换函数已经处理了方向，直接显示 bar 即可
				// 跳过太靠近边缘的小节数字 (避免与 header 重叠或显示0)
				float margin = 2.0f;
				bool should_draw_number = true;
				if (bar_number_direction == BAR_NUMBER_BOTTOM_UP) {
					// Bottom Up: 底部是 0，检查是否太靠近底部
					if (y > get_size().y - margin) should_draw_number = false;
				} else {
					// Top Down: 顶部是 0，检查是否太靠近 header
					if (y < visible_start_y + margin) should_draw_number = false;
				}
				
				if (should_draw_number) {
					// 使用默认字体绘制文字
					String text = String::num_int64(bar);
					Ref<Font> font = ThemeDB::get_singleton()->get_fallback_font();
					if (font.is_valid()) {
						draw_string(font, Point2(2.0f, y + 6.0f), text, HORIZONTAL_ALIGNMENT_LEFT, -1, 12, tick_color);
					}
				}
			}

			// 绘制拍刻度
			for (int bar = start_bar; bar <= end_bar; bar++) {
				for (int beat = 1; beat < beats_per_bar; beat++) {
					double beat_num = bar * beats_per_bar + beat;
					float y = _beat_to_y(beat_num);
					if (y < visible_start_y || y > visible_end_y) continue;

					draw_line(
						Point2(ruler_width - 6.0f, y),
						Point2(ruler_width, y),
						tick_color,
						minor_tick_width
					);
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
				float tick_width = is_second ? 12.0f : 6.0f;

				draw_line(
					Point2(ruler_width - tick_width, y),
					Point2(ruler_width, y),
					tick_color
				);

				// 秒数标签
				if (is_second) {
					// 跳过太靠近边缘的标签
					float margin = 2.0f;
					bool should_draw_label = true;
					if (bar_number_direction == BAR_NUMBER_BOTTOM_UP) {
						if (y > get_size().y - margin) should_draw_label = false;
					} else {
						if (y < visible_start_y + margin) should_draw_label = false;
					}
					
					if (should_draw_label) {
						String text = String::num_int64(frame / fps);
						Ref<Font> font = ThemeDB::get_singleton()->get_fallback_font();
						if (font.is_valid()) {
							draw_string(font, Point2(2.0f, y + 6.0f), text + "s", HORIZONTAL_ALIGNMENT_LEFT, -1, 12, Color(1, 1, 1));
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
				float tick_width = is_major ? 12.0f : 6.0f;

				draw_line(
					Point2(ruler_width - tick_width, y),
					Point2(ruler_width, y),
					tick_color
				);

				// 时间标签
				if (is_major) {
					// 跳过太靠近边缘的标签
					float margin = 2.0f;
					bool should_draw_label = true;
					if (bar_number_direction == BAR_NUMBER_BOTTOM_UP) {
						if (y > get_size().y - margin) should_draw_label = false;
					} else {
						if (y < visible_start_y + margin) should_draw_label = false;
					}
					
					if (should_draw_label) {
						String text;
						switch (time_format) {
						case HH_MM_SS:
							text = String::num_int64((int)t / 3600) + ":" +
								String::num_int64(((int)t % 3600) / 60).pad_zeros(2) + ":" +
								String::num_int64((int)t % 60).pad_zeros(2);
							break;
						case MM_SS_MS:
							text = String::num_int64((int)t / 60) + ":" +
								String::num_int64((int)t % 60).pad_zeros(2);
							if (show_milliseconds) {
								text += "." + String::num_int64((int)((t - (int)t) * 100)).pad_zeros(2);
							}
							break;
						case SEC:
						default:
							text = String::num(t, (show_milliseconds ? 2 : 0)) + "s";
							break;
						}
						Ref<Font> font = ThemeDB::get_singleton()->get_fallback_font();
						if (font.is_valid()) {
							draw_string(font, Point2(2.0f, y + 6.0f), text, HORIZONTAL_ALIGNMENT_LEFT, -1, 12, Color(1, 1, 1));
						}
					}
				}
			}
			break;
		}
		}
	}

	void VTimelinePanel::_draw_grid_beat(float p_header_width) {
		float start_pos = 0.0f;
		if (time_ruler_component.is_valid()) {
			start_pos = time_ruler_component->get_width();
		}

		float visible_start_y = header_height;
		float visible_end_y = get_size().y;

		// 计算可见范围内的小节和拍
		double start_beat = _y_to_beat(visible_start_y);
		double end_beat = _y_to_beat(visible_end_y);
		// 确保 start_bar <= end_bar (考虑 Bottom Up 模式）
		int start_bar = Math::floor(Math::min(start_beat, end_beat) / beats_per_bar);
		int end_bar = Math::ceil(Math::max(start_beat, end_beat) / beats_per_bar);

		// 绘制小节线
		for (int bar = start_bar; bar <= end_bar; bar++) {
			float y = _beat_to_y(bar * beats_per_bar);
			if (y < visible_start_y || y > visible_end_y) continue;

			draw_line(
				Point2(start_pos, y),
				Point2(p_header_width, y),
				bar_line_color,
				bar_line_width
			);
		}

		for (int bar = start_bar; bar <= end_bar; bar++) {
			for (int beat = 1; beat < beats_per_bar; beat++) {
				double beat_num = bar * beats_per_bar + beat;
				float y = _beat_to_y(beat_num);
				if (y < visible_start_y || y > visible_end_y) continue;

				draw_line(
					Point2(start_pos, y),
					Point2(p_header_width, y),
					beat_line_color,
					beat_line_width
				);
			}
		}
	}

	void VTimelinePanel::_draw_grid_frame(float p_header_width) {
		float start_pos = 0.0f;
		if (time_ruler_component.is_valid()) {
			start_pos = time_ruler_component->get_width();
		}

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
				Point2(p_header_width, y),
				line_color,
				line_width
			);
		}
	}

	void VTimelinePanel::_draw_grid_time(float p_header_width) {
		float start_pos = 0.0f;
		if (time_ruler_component.is_valid()) {
			start_pos = time_ruler_component->get_width();
		}

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
				Point2(p_header_width, y),
				line_color,
				line_width
			);
		}
	}

	float VTimelinePanel::_calculate_header_width() const {
		float width = 0.0f;

		if (time_ruler_component.is_valid()) {
			width += time_ruler_component->get_width();
		}

		for (int64_t i = 0; i < track_components.size(); i++) {
			Ref<TimelinePanelTrackComponent> track_component = track_components[i];
			if (track_component.is_null()) continue;
			width += track_component->get_width();
		}

		return width;
	}

	float VTimelinePanel::_calculate_grid_height() const {
		switch (counting_unit) {
		case BEAT: {
			float beat_duration = 60.0f / bpm;
			float bar_duration = beat_duration * beats_per_bar;
			int total_beats = Math::ceil(duration / beat_duration);
			int total_bars = Math::ceil((float)total_beats / beats_per_bar);
			if (total_bars < 1) total_bars = 1;
			return total_bars * scale;
		}
		case FRAME: {
			// 每帧高度 = scale / fps
			int total_frames = Math::ceil(duration * fps);
			if (total_frames < 1) total_frames = 1;
			return total_frames * (scale / fps);
		}
		case TIME:
		default: {
			// 每秒高度 = scale
			float time_height = duration * scale;
			if (time_height < scale) time_height = scale;
			return time_height;
		}
		}
	}

	Vector2 VTimelinePanel::_get_minimum_size() const {
		float min_width = _calculate_header_width();
		float min_height = header_height + _calculate_grid_height();
		return Size2(min_width, min_height);
	}

	void VTimelinePanel::set_background_color(const Color& p_background_color) {
		background_color = p_background_color;
		queue_redraw();
	}

	Color VTimelinePanel::get_background_color() const {
		return background_color;
	}

	void VTimelinePanel::set_separator_color(const Color& p_separator_color) {
		separator_color = p_separator_color;
		queue_redraw();
	}

	Color VTimelinePanel::get_separator_color() const {
		return separator_color;
	}

	void VTimelinePanel::set_separator_width(const float p_width) {
		separator_width = p_width;
		queue_redraw();
	}

	float VTimelinePanel::get_separator_width() const {
		return separator_width;
	}

	void VTimelinePanel::set_header_height(const float p_height) {
		header_height = p_height;
		queue_redraw();
		update_minimum_size();
	}

	float VTimelinePanel::get_header_height() const {
		return header_height;
	}

	void VTimelinePanel::set_duration(const double p_duration) {
		duration = p_duration;
		queue_redraw();
		update_minimum_size();
	}

	double VTimelinePanel::get_duration() const {
		return duration;
	}

	void VTimelinePanel::set_scale(const float p_scale) {
		scale = p_scale;
		queue_redraw();
		update_minimum_size();
	}

	float VTimelinePanel::get_scale() const {
		return scale;
	}

	void VTimelinePanel::set_counting_unit(CountingUnit p_unit) {
		counting_unit = p_unit;
		notify_property_list_changed();
		queue_redraw();
		update_minimum_size();
	}

	VTimelinePanel::CountingUnit VTimelinePanel::get_counting_unit() const {
		return counting_unit;
	}

	void VTimelinePanel::set_time_format(TimeFormat p_time_format) {
		time_format = p_time_format;
		queue_redraw();
	}

	VTimelinePanel::TimeFormat VTimelinePanel::get_time_format() const {
		return time_format;
	}

	void VTimelinePanel::set_show_milliseconds(const bool p_show_milliseconds) {
		show_milliseconds = p_show_milliseconds;
		queue_redraw();
	}

	bool VTimelinePanel::get_show_milliseconds() const {
		return show_milliseconds;
	}

	void VTimelinePanel::set_fps(const int p_fps) {
		fps = p_fps;
		queue_redraw();
		update_minimum_size();
	}

	int VTimelinePanel::get_fps() const {
		return fps;
	}

	void VTimelinePanel::set_show_subdivision(const bool p_show_subdivision) {
		show_subdivision = p_show_subdivision;
		queue_redraw();
	}

	bool VTimelinePanel::get_show_subdivision() const {
		return show_subdivision;
	}

	void VTimelinePanel::set_bpm(const int p_bpm) {
		bpm = p_bpm;
		queue_redraw();
		update_minimum_size();
	}

	int VTimelinePanel::get_bpm() const {
		return bpm;
	}

	void VTimelinePanel::set_beat_per_bar(const int p_beats_per_bar) {
		beats_per_bar = p_beats_per_bar;
		queue_redraw();
		update_minimum_size();
	}

	int VTimelinePanel::get_beat_per_bar() const {
		return beats_per_bar;
	}

	void VTimelinePanel::set_bar_line_color(const Color& p_color) {
		bar_line_color = p_color;
		queue_redraw();
	}

	Color VTimelinePanel::get_bar_line_color() const {
		return bar_line_color;
	}

	void VTimelinePanel::set_bar_line_width(const float p_width) {
		bar_line_width = p_width;
		queue_redraw();
	}

	float VTimelinePanel::get_bar_line_width() const {
		return bar_line_width;
	}

	void VTimelinePanel::set_beat_line_color(const Color& p_color) {
		beat_line_color = p_color;
		queue_redraw();
	}

	Color VTimelinePanel::get_beat_line_color() const {
		return beat_line_color;
	}

	void VTimelinePanel::set_beat_line_width(const float p_width) {
		beat_line_width = p_width;
		queue_redraw();
	}

	float VTimelinePanel::get_beat_line_width() const {
		return beat_line_width;
	}

	void VTimelinePanel::set_bar_number_direction(BarNumberDirection p_direction) {
		bar_number_direction = p_direction;
		queue_redraw();
	}

	VTimelinePanel::BarNumberDirection VTimelinePanel::get_bar_number_direction() const {
		return bar_number_direction;
	}

	void VTimelinePanel::set_time_ruler_component(Ref<TimelinePanelTimeRulerComponent> p_time_ruler_component) {
		time_ruler_component = p_time_ruler_component;
		queue_redraw();
		update_minimum_size();
	}

	Ref<TimelinePanelTimeRulerComponent> VTimelinePanel::get_time_ruler_component() const {
		return time_ruler_component;
	}

	void VTimelinePanel::set_track_components(const TypedArray<TimelinePanelTrackComponent>& p_track_components) {
		track_components = p_track_components;
		queue_redraw();
		update_minimum_size();
	}

	TypedArray<TimelinePanelTrackComponent> VTimelinePanel::get_track_components() const {
		return track_components;
	}
}