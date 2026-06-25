#ifndef TIMELINE_TRACK_ITEM_H
#define TIMELINE_TRACK_ITEM_H

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/style_box.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/core/object_id.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot {
	class TimelinePanelBase;

	class TimelineTrackItem : public Object {
		GDCLASS(TimelineTrackItem, Object)

	private:
		ObjectID owner_id;
		uint64_t track_id = 0;

		TimelinePanelBase *_get_owner() const;

	protected:
		static void _bind_methods();

	public:
		void _bind_owner(TimelinePanelBase *p_owner, uint64_t p_track_id);
		void _unbind_owner();
		bool is_valid() const;

		void set_height(float p_height);
		float get_height() const;
		void set_width(float p_width);
		float get_width() const;

		void set_background(const Color &p_background);
		Color get_background() const;
		void set_header_background(const Ref<StyleBox> &p_style);
		Ref<StyleBox> get_header_background() const;
		void set_background_fill_track(bool p_enabled);
		bool get_background_fill_track() const;
		void set_header_background_fill_track(bool p_enabled);
		bool get_header_background_fill_track() const;
		void set_header_indent(float p_indent);
		float get_header_indent() const;

		void set_text(int p_column, const String &p_text);
		String get_text(int p_column) const;
		void set_icon(int p_column, Ref<Texture2D> p_icon);
		Ref<Texture2D> get_icon(int p_column) const;
		void set_tooltip_text(int p_column, const String &p_text);
		String get_tooltip_text(int p_column) const;

		int add_button(int p_column, Ref<Texture2D> p_icon, int p_id = -1, bool p_disabled = false, const String &p_tooltip = String());
		void erase_button(int p_column, int p_button_index);
		void clear_buttons(int p_column);
		void set_button_disabled(int p_column, int p_button_index, bool p_disabled);
		bool is_button_disabled(int p_column, int p_button_index) const;
		int get_button_count(int p_column) const;
		int get_button_id(int p_column, int p_button_index) const;
		String get_button_tooltip_text(int p_column, int p_button_index) const;
	};
}

#endif // TIMELINE_TRACK_ITEM_H
