#ifndef TIMELINE_TRACK_KEY_H
#define TIMELINE_TRACK_KEY_H

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/style_box.hpp>
#include <godot_cpp/classes/texture2d.hpp>

namespace godot {
	class TimelineTrackKey : public Object {
		GDCLASS(TimelineTrackKey, Object)

	private:
		double time = 0.0;
		double length = 0.0;
		String text;
		Ref<Texture2D> icon;
		Ref<StyleBox> instant_key_normal_style;
		Ref<StyleBox> instant_key_selected_style;
		Ref<StyleBox> clip_key_normal_style;
		Ref<StyleBox> clip_key_selected_style;
		float icon_max_width = 0.0f;
		float instant_key_scale = 0.4f;
		bool selected = false;
		bool disabled = false;
		Variant meta;

	protected:
		static void _bind_methods();

	public:
		void set_selected(bool p_selected);
		void set_selected_no_signal(bool p_selected);
		bool is_selected() const;

		void set_disabled(bool p_disabled);
		void set_disabled_no_signal(bool p_disabled);
		bool is_disabled() const;

		void set_time(double p_time);
		double get_time() const;

		void set_length(double p_length);
		double get_length() const;

		void set_text(const String& p_text);
		String get_text() const;

		void set_icon(const Ref<Texture2D>& p_icon);
		Ref<Texture2D> get_icon() const;

		void set_icon_max_width(const float p_width);
		float get_icon_max_width() const;

		void set_instant_key_scale(const float p_scale);
		float get_instant_key_scale() const;

		void set_instant_key_normal_style(const Ref<StyleBox>& p_style);
		Ref<StyleBox> get_instant_key_normal_style() const;

		void set_instant_key_selected_style(Ref<StyleBox> p_style);
		Ref<StyleBox> get_instant_key_selected_style() const;

		void set_clip_key_normal_style(const Ref<StyleBox>& p_style);
		Ref<StyleBox> get_clip_key_normal_style() const;

		void set_clip_key_selected_style(Ref<StyleBox> p_style);
		Ref<StyleBox> get_clip_key_selected_style() const;

		void set_metadata(const Variant& p_meta);
		Variant get_metadata() const;

		bool is_instant() const;
	};
}

#endif // !TIMELINE_TRACK_KEY_H
