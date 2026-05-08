#include "timeline_connection_point.h"

#include "timeline_connection.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/object.hpp>

namespace godot {
	void TimelineConnectionPoint::_bind_methods() {
		ClassDB::bind_method(D_METHOD("set_position", "position"), &TimelineConnectionPoint::set_position);
		ClassDB::bind_method(D_METHOD("get_position"), &TimelineConnectionPoint::get_position);
		ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "position"), "set_position", "get_position");

		ClassDB::bind_method(D_METHOD("set_in_handle", "handle"), &TimelineConnectionPoint::set_in_handle);
		ClassDB::bind_method(D_METHOD("get_in_handle"), &TimelineConnectionPoint::get_in_handle);
		ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "in_handle"), "set_in_handle", "get_in_handle");

		ClassDB::bind_method(D_METHOD("set_out_handle", "handle"), &TimelineConnectionPoint::set_out_handle);
		ClassDB::bind_method(D_METHOD("get_out_handle"), &TimelineConnectionPoint::get_out_handle);
		ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "out_handle"), "set_out_handle", "get_out_handle");

		ClassDB::bind_method(D_METHOD("set_selected", "selected"), &TimelineConnectionPoint::set_selected);
		ClassDB::bind_method(D_METHOD("set_selected_no_signal", "selected"), &TimelineConnectionPoint::set_selected_no_signal);
		ClassDB::bind_method(D_METHOD("is_selected"), &TimelineConnectionPoint::is_selected);
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "selected"), "set_selected", "is_selected");

		ClassDB::bind_method(D_METHOD("set_metadata", "meta"), &TimelineConnectionPoint::set_metadata);
		ClassDB::bind_method(D_METHOD("get_metadata"), &TimelineConnectionPoint::get_metadata);

		ClassDB::bind_method(D_METHOD("get_point_id"), &TimelineConnectionPoint::get_point_id);
		ClassDB::bind_method(D_METHOD("is_bound"), &TimelineConnectionPoint::is_bound);

		ADD_SIGNAL(MethodInfo("changed"));
	}

	TimelineConnection* TimelineConnectionPoint::_get_owner() const {
		if (owner_id == 0) {
			return nullptr;
		}

		Object* object = ObjectDB::get_instance(owner_id);
		return Object::cast_to<TimelineConnection>(object);
	}

	void TimelineConnectionPoint::_bind_owner(TimelineConnection* p_owner, int64_t p_point_id) {
		owner_id = p_owner != nullptr ? p_owner->get_instance_id() : 0;
		point_id = p_point_id;
	}

	void TimelineConnectionPoint::_unbind_owner() {
		owner_id = 0;
		point_id = -1;
	}

	void TimelineConnectionPoint::_set_local_position(const Vector2 &p_position) {
		position = p_position;
	}

	void TimelineConnectionPoint::_set_local_in_handle(const Vector2 &p_handle) {
		in_handle = p_handle;
	}

	void TimelineConnectionPoint::_set_local_out_handle(const Vector2 &p_handle) {
		out_handle = p_handle;
	}

	void TimelineConnectionPoint::_set_local_selected(bool p_selected) {
		selected = p_selected;
	}

	void TimelineConnectionPoint::_set_local_metadata(const Variant &p_meta) {
		meta = p_meta;
	}

	void TimelineConnectionPoint::set_position(const Vector2 &p_position) {
		TimelineConnection* owner = _get_owner();
		if (owner != nullptr && owner->_set_point_position_by_id(point_id, p_position)) {
			return;
		}

		position = p_position;
		emit_signal("changed");
	}

	Vector2 TimelineConnectionPoint::get_position() const {
		const TimelineConnection* owner = _get_owner();
		if (owner != nullptr) {
			return owner->_get_point_position_by_id(point_id, position);
		}
		return position;
	}

	void TimelineConnectionPoint::set_in_handle(const Vector2 &p_handle) {
		TimelineConnection* owner = _get_owner();
		if (owner != nullptr && owner->_set_point_in_handle_by_id(point_id, p_handle)) {
			return;
		}

		in_handle = p_handle;
		emit_signal("changed");
	}

	Vector2 TimelineConnectionPoint::get_in_handle() const {
		const TimelineConnection* owner = _get_owner();
		if (owner != nullptr) {
			return owner->_get_point_in_handle_by_id(point_id, in_handle);
		}
		return in_handle;
	}

	void TimelineConnectionPoint::set_out_handle(const Vector2 &p_handle) {
		TimelineConnection* owner = _get_owner();
		if (owner != nullptr && owner->_set_point_out_handle_by_id(point_id, p_handle)) {
			return;
		}

		out_handle = p_handle;
		emit_signal("changed");
	}

	Vector2 TimelineConnectionPoint::get_out_handle() const {
		const TimelineConnection* owner = _get_owner();
		if (owner != nullptr) {
			return owner->_get_point_out_handle_by_id(point_id, out_handle);
		}
		return out_handle;
	}

	void TimelineConnectionPoint::set_selected(bool p_selected) {
		TimelineConnection* owner = _get_owner();
		if (owner != nullptr && owner->_set_point_selected_by_id(point_id, p_selected, true)) {
			return;
		}

		selected = p_selected;
		emit_signal("changed");
	}

	void TimelineConnectionPoint::set_selected_no_signal(bool p_selected) {
		TimelineConnection* owner = _get_owner();
		if (owner != nullptr && owner->_set_point_selected_by_id(point_id, p_selected, false)) {
			return;
		}

		selected = p_selected;
	}

	bool TimelineConnectionPoint::is_selected() const {
		const TimelineConnection* owner = _get_owner();
		if (owner != nullptr) {
			return owner->_is_point_selected_by_id(point_id, selected);
		}
		return selected;
	}

	void TimelineConnectionPoint::set_metadata(const Variant &p_meta) {
		TimelineConnection* owner = _get_owner();
		if (owner != nullptr && owner->_set_point_metadata_by_id(point_id, p_meta)) {
			return;
		}

		meta = p_meta;
	}

	Variant TimelineConnectionPoint::get_metadata() const {
		const TimelineConnection* owner = _get_owner();
		if (owner != nullptr) {
			return owner->_get_point_metadata_by_id(point_id, meta);
		}
		return meta;
	}

	int64_t TimelineConnectionPoint::get_point_id() const {
		return point_id;
	}

	bool TimelineConnectionPoint::is_bound() const {
		return _get_owner() != nullptr;
	}
}
