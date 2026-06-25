#include "timeline_track_item.h"

#include "../timeline_panel_base.h"

namespace godot {
	TimelinePanelBase *TimelineTrackItem::_get_owner() const {
		Object *owner_object = ObjectDB::get_instance(owner_id);
		return Object::cast_to<TimelinePanelBase>(owner_object);
	}

	void TimelineTrackItem::_bind_methods() {
		ClassDB::bind_method(D_METHOD("is_valid"), &TimelineTrackItem::is_valid);

		ClassDB::bind_method(D_METHOD("set_height", "height"), &TimelineTrackItem::set_height);
		ClassDB::bind_method(D_METHOD("get_height"), &TimelineTrackItem::get_height);
		ClassDB::bind_method(D_METHOD("set_width", "width"), &TimelineTrackItem::set_width);
		ClassDB::bind_method(D_METHOD("get_width"), &TimelineTrackItem::get_width);

		ClassDB::bind_method(D_METHOD("set_background", "background"), &TimelineTrackItem::set_background);
		ClassDB::bind_method(D_METHOD("get_background"), &TimelineTrackItem::get_background);
		ClassDB::bind_method(D_METHOD("set_header_background", "style"), &TimelineTrackItem::set_header_background);
		ClassDB::bind_method(D_METHOD("get_header_background"), &TimelineTrackItem::get_header_background);
		ClassDB::bind_method(D_METHOD("set_background_fill_track", "enabled"), &TimelineTrackItem::set_background_fill_track);
		ClassDB::bind_method(D_METHOD("get_background_fill_track"), &TimelineTrackItem::get_background_fill_track);
		ClassDB::bind_method(D_METHOD("set_header_background_fill_track", "enabled"), &TimelineTrackItem::set_header_background_fill_track);
		ClassDB::bind_method(D_METHOD("get_header_background_fill_track"), &TimelineTrackItem::get_header_background_fill_track);
		ClassDB::bind_method(D_METHOD("set_header_indent", "indent"), &TimelineTrackItem::set_header_indent);
		ClassDB::bind_method(D_METHOD("get_header_indent"), &TimelineTrackItem::get_header_indent);

		ClassDB::bind_method(D_METHOD("set_text", "column", "text"), &TimelineTrackItem::set_text);
		ClassDB::bind_method(D_METHOD("get_text", "column"), &TimelineTrackItem::get_text);
		ClassDB::bind_method(D_METHOD("set_icon", "column", "icon"), &TimelineTrackItem::set_icon);
		ClassDB::bind_method(D_METHOD("get_icon", "column"), &TimelineTrackItem::get_icon);
		ClassDB::bind_method(D_METHOD("set_tooltip_text", "column", "text"), &TimelineTrackItem::set_tooltip_text);
		ClassDB::bind_method(D_METHOD("get_tooltip_text", "column"), &TimelineTrackItem::get_tooltip_text);

		ClassDB::bind_method(D_METHOD("add_button", "column", "icon", "id", "disabled", "tooltip"), &TimelineTrackItem::add_button, DEFVAL(-1), DEFVAL(false), DEFVAL(String()));
		ClassDB::bind_method(D_METHOD("erase_button", "column", "button_index"), &TimelineTrackItem::erase_button);
		ClassDB::bind_method(D_METHOD("clear_buttons", "column"), &TimelineTrackItem::clear_buttons);
		ClassDB::bind_method(D_METHOD("set_button_disabled", "column", "button_index", "disabled"), &TimelineTrackItem::set_button_disabled);
		ClassDB::bind_method(D_METHOD("is_button_disabled", "column", "button_index"), &TimelineTrackItem::is_button_disabled);
		ClassDB::bind_method(D_METHOD("get_button_count", "column"), &TimelineTrackItem::get_button_count);
		ClassDB::bind_method(D_METHOD("get_button_id", "column", "button_index"), &TimelineTrackItem::get_button_id);
		ClassDB::bind_method(D_METHOD("get_button_tooltip_text", "column", "button_index"), &TimelineTrackItem::get_button_tooltip_text);
	}

	void TimelineTrackItem::_bind_owner(TimelinePanelBase *p_owner, uint64_t p_track_id) {
		owner_id = p_owner != nullptr ? p_owner->get_instance_id() : ObjectID();
		track_id = p_track_id;
	}

	void TimelineTrackItem::_unbind_owner() {
		owner_id = ObjectID();
		track_id = 0;
	}

	bool TimelineTrackItem::is_valid() const {
		TimelinePanelBase *owner = _get_owner();
		return owner != nullptr && owner->_is_track_item_valid(this, track_id);
	}

	void TimelineTrackItem::set_height(float p_height) {
		TimelinePanelBase *owner = _get_owner();
		if (owner != nullptr) {
			owner->_track_item_set_height(this, track_id, p_height);
		}
	}

	float TimelineTrackItem::get_height() const {
		TimelinePanelBase *owner = _get_owner();
		return owner != nullptr ? owner->_track_item_get_height(this, track_id) : 0.0f;
	}

	void TimelineTrackItem::set_width(float p_width) {
		set_height(p_width);
	}

	float TimelineTrackItem::get_width() const {
		return get_height();
	}

	void TimelineTrackItem::set_background(const Color &p_background) {
		TimelinePanelBase *owner = _get_owner();
		if (owner != nullptr) {
			owner->_track_item_set_background(this, track_id, p_background);
		}
	}

	Color TimelineTrackItem::get_background() const {
		TimelinePanelBase *owner = _get_owner();
		return owner != nullptr ? owner->_track_item_get_background(this, track_id) : Color();
	}

	void TimelineTrackItem::set_header_background(const Ref<StyleBox> &p_style) {
		TimelinePanelBase *owner = _get_owner();
		if (owner != nullptr) {
			owner->_track_item_set_header_background(this, track_id, p_style);
		}
	}

	Ref<StyleBox> TimelineTrackItem::get_header_background() const {
		TimelinePanelBase *owner = _get_owner();
		return owner != nullptr ? owner->_track_item_get_header_background(this, track_id) : Ref<StyleBox>();
	}

	void TimelineTrackItem::set_background_fill_track(bool p_enabled) {
		TimelinePanelBase *owner = _get_owner();
		if (owner != nullptr) {
			owner->_track_item_set_header_background_fill_track(this, track_id, p_enabled);
		}
	}

	bool TimelineTrackItem::get_background_fill_track() const {
		TimelinePanelBase *owner = _get_owner();
		return owner != nullptr && owner->_track_item_get_header_background_fill_track(this, track_id);
	}

	void TimelineTrackItem::set_header_background_fill_track(bool p_enabled) {
		set_background_fill_track(p_enabled);
	}

	bool TimelineTrackItem::get_header_background_fill_track() const {
		return get_background_fill_track();
	}

	void TimelineTrackItem::set_header_indent(float p_indent) {
		TimelinePanelBase *owner = _get_owner();
		if (owner != nullptr) {
			owner->_track_item_set_header_indent(this, track_id, p_indent);
		}
	}

	float TimelineTrackItem::get_header_indent() const {
		TimelinePanelBase *owner = _get_owner();
		return owner != nullptr ? owner->_track_item_get_header_indent(this, track_id) : 0.0f;
	}

	void TimelineTrackItem::set_text(int p_column, const String &p_text) {
		TimelinePanelBase *owner = _get_owner();
		if (owner != nullptr) {
			owner->_track_item_set_text(this, track_id, p_column, p_text);
		}
	}

	String TimelineTrackItem::get_text(int p_column) const {
		TimelinePanelBase *owner = _get_owner();
		return owner != nullptr ? owner->_track_item_get_text(this, track_id, p_column) : String();
	}

	void TimelineTrackItem::set_icon(int p_column, Ref<Texture2D> p_icon) {
		TimelinePanelBase *owner = _get_owner();
		if (owner != nullptr) {
			owner->_track_item_set_icon(this, track_id, p_column, p_icon);
		}
	}

	Ref<Texture2D> TimelineTrackItem::get_icon(int p_column) const {
		TimelinePanelBase *owner = _get_owner();
		return owner != nullptr ? owner->_track_item_get_icon(this, track_id, p_column) : Ref<Texture2D>();
	}

	void TimelineTrackItem::set_tooltip_text(int p_column, const String &p_text) {
		TimelinePanelBase *owner = _get_owner();
		if (owner != nullptr) {
			owner->_track_item_set_tooltip_text(this, track_id, p_column, p_text);
		}
	}

	String TimelineTrackItem::get_tooltip_text(int p_column) const {
		TimelinePanelBase *owner = _get_owner();
		return owner != nullptr ? owner->_track_item_get_tooltip_text(this, track_id, p_column) : String();
	}

	int TimelineTrackItem::add_button(int p_column, Ref<Texture2D> p_icon, int p_id, bool p_disabled, const String &p_tooltip) {
		TimelinePanelBase *owner = _get_owner();
		return owner != nullptr ? owner->_track_item_add_button(this, track_id, p_column, p_icon, p_id, p_disabled, p_tooltip) : -1;
	}

	void TimelineTrackItem::erase_button(int p_column, int p_button_index) {
		TimelinePanelBase *owner = _get_owner();
		if (owner != nullptr) {
			owner->_track_item_erase_button(this, track_id, p_column, p_button_index);
		}
	}

	void TimelineTrackItem::clear_buttons(int p_column) {
		TimelinePanelBase *owner = _get_owner();
		if (owner != nullptr) {
			owner->_track_item_clear_buttons(this, track_id, p_column);
		}
	}

	void TimelineTrackItem::set_button_disabled(int p_column, int p_button_index, bool p_disabled) {
		TimelinePanelBase *owner = _get_owner();
		if (owner != nullptr) {
			owner->_track_item_set_button_disabled(this, track_id, p_column, p_button_index, p_disabled);
		}
	}

	bool TimelineTrackItem::is_button_disabled(int p_column, int p_button_index) const {
		TimelinePanelBase *owner = _get_owner();
		return owner != nullptr && owner->_track_item_is_button_disabled(this, track_id, p_column, p_button_index);
	}

	int TimelineTrackItem::get_button_count(int p_column) const {
		TimelinePanelBase *owner = _get_owner();
		return owner != nullptr ? owner->_track_item_get_button_count(this, track_id, p_column) : 0;
	}

	int TimelineTrackItem::get_button_id(int p_column, int p_button_index) const {
		TimelinePanelBase *owner = _get_owner();
		return owner != nullptr ? owner->_track_item_get_button_id(this, track_id, p_column, p_button_index) : -1;
	}

	String TimelineTrackItem::get_button_tooltip_text(int p_column, int p_button_index) const {
		TimelinePanelBase *owner = _get_owner();
		return owner != nullptr ? owner->_track_item_get_button_tooltip_text(this, track_id, p_column, p_button_index) : String();
	}
}
