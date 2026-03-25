#include "v_timeline_panel.h"

#include "components/timeline_panel_time_ruler_component.h"
#include "components/timeline_panel_track_component.h"

namespace godot {
	void VTimelinePanel::_bind_methods() {
		BIND_ENUM_CONSTANT(TIME);
		BIND_ENUM_CONSTANT(FRAME);
		BIND_ENUM_CONSTANT(BEAT);

		BIND_ENUM_CONSTANT(HH_MM_SS);
		BIND_ENUM_CONSTANT(MM_SS_MS);
		BIND_ENUM_CONSTANT(SEC);

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

		ClassDB::bind_method(D_METHOD("set_step", "step"), &VTimelinePanel::set_step);
		ClassDB::bind_method(D_METHOD("get_step"), &VTimelinePanel::get_step);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "step"), "set_step", "get_step");

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

			// 处理轨道组件
			Vector2 start_pos;
			draw_line(Point2(0.0f, header_height), Point2(0.0f, get_size().y), separator_color, separator_width);
			for (int64_t i = 0; i < track_components.size(); i++) {
				Ref<TimelinePanelTrackComponent> track_component = track_components[i];
				if (track_component.is_null()) continue;

				float width = track_component->get_width();
				Color header_color = track_component->get_header_color();
				draw_rect(Rect2(start_pos, Size2(width, header_height)), header_color);

				Ref<Texture2D> header_icon = track_component->get_header_icon();
				if (header_icon.is_valid()) {
					Size2 tex_size = header_icon->get_size();
					// 计算缩放比例，选择较小的那个以保持比例并限制在区域内
					Size2 scale_size(width / tex_size.width, header_height / tex_size.height);
					float scale = scale_size.width < scale_size.height ? scale_size.width : scale_size.height;
					Size2 scaled_tex_size = tex_size * scale;
					// 计算居中偏移
					Point2 offset = (Size2(width, header_height) - scaled_tex_size) / 2;
					draw_texture_rect(header_icon, Rect2(start_pos + offset, scaled_tex_size), false);
				}

				draw_line(Point2(start_pos.x + width, header_height), Point2(start_pos.x + width, get_size().y), separator_color, separator_width);

				start_pos.x += width;
			}
			
			// 更新 header_width 供后续使用
			header_width = start_pos.x;

			// 绘制网格
			if (counting_unit == BEAT) {
				float bar_spacing = scale;								// 每小节固定像素间距
				float beat_spacing = bar_spacing / beats_per_bar;		// 每拍像素间距
				double beat_duration = 60.0f / bpm;						// 每拍持续时间
				double bar_duration = beat_duration * beats_per_bar;	// 每小节持续时间
				int64_t total_beats = Math::ceil(duration / beat_duration);
				int64_t total_bars = Math::ceil((float)total_beats / beats_per_bar);
				
				// 确保至少显示一个小节
				if (total_bars < 1) total_bars = 1;
				
				// 绘制小节线和拍线
				for (int bar = 0; bar <= total_bars; bar++) {
					float bar_y = header_height + bar * bar_spacing;
					draw_line(
						Point2(0.0f, bar_y),
						Point2(header_width, bar_y),
						bar_line_color,
						bar_line_width
					);
					
					// 在每个小节内绘制拍线
					for (int beat = 1; beat < beats_per_bar; beat++) {
						float beat_y = bar_y + beat * beat_spacing;
						// 计算该拍对应的时间，检查是否超出 duration
						float beat_time = (bar * beats_per_bar + beat) * beat_duration;
						if (beat_time > duration && duration > 0) break;
						draw_line(
							Point2(0.0f, beat_y),
							Point2(header_width, beat_y),
							beat_line_color,
							beat_line_width
						);
					}
				}
			}
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

	float VTimelinePanel::_calculate_header_width() const {
		float width = 0.0f;
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

	void VTimelinePanel::set_step(const double p_step) {
		step = p_step;
		queue_redraw();
	}

	double VTimelinePanel::get_step() const {
		return step;
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

	void VTimelinePanel::set_time_ruler_component(Ref<TimelinePanelTimeRulerComponent> p_time_ruler_component) {
		time_ruler_component = p_time_ruler_component;
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