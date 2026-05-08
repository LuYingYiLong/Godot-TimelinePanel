#include "timeline_connection.h"

#include <godot_cpp/core/class_db.hpp>

namespace godot {
	void TimelineConnection::_bind_methods() {
		BIND_ENUM_CONSTANT(CURVE_LINE);
		BIND_ENUM_CONSTANT(CURVE_BEZIER);

		ADD_GROUP("Points", "");
		ClassDB::bind_method(D_METHOD("add_point", "position", "index"), &TimelineConnection::add_point, DEFVAL(-1));
		ClassDB::bind_method(D_METHOD("remove_point", "point"), &TimelineConnection::remove_point);
		ClassDB::bind_method(D_METHOD("remove_point_at", "index"), &TimelineConnection::remove_point_at);
		ClassDB::bind_method(D_METHOD("clear_points"), &TimelineConnection::clear_points);
		ClassDB::bind_method(D_METHOD("set_points", "points"), &TimelineConnection::set_points);
		ClassDB::bind_method(D_METHOD("get_points"), &TimelineConnection::get_points);
		ClassDB::bind_method(D_METHOD("get_point", "index"), &TimelineConnection::get_point);
		ClassDB::bind_method(D_METHOD("get_point_count"), &TimelineConnection::get_point_count);
		ClassDB::bind_method(D_METHOD("get_point_index", "point"), &TimelineConnection::get_point_index);

		ClassDB::bind_method(D_METHOD("set_point_position", "index", "position"), &TimelineConnection::set_point_position);
		ClassDB::bind_method(D_METHOD("get_point_position", "index"), &TimelineConnection::get_point_position);
		ClassDB::bind_method(D_METHOD("set_point_in_handle", "index", "handle"), &TimelineConnection::set_point_in_handle);
		ClassDB::bind_method(D_METHOD("get_point_in_handle", "index"), &TimelineConnection::get_point_in_handle);
		ClassDB::bind_method(D_METHOD("set_point_out_handle", "index", "handle"), &TimelineConnection::set_point_out_handle);
		ClassDB::bind_method(D_METHOD("get_point_out_handle", "index"), &TimelineConnection::get_point_out_handle);

		ADD_GROUP("Compatibility Points", "");
		ClassDB::bind_method(D_METHOD("set_from_position", "position"), &TimelineConnection::set_from_position);
		ClassDB::bind_method(D_METHOD("get_from_position"), &TimelineConnection::get_from_position);
		ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "from_position"), "set_from_position", "get_from_position");

		ClassDB::bind_method(D_METHOD("set_to_position", "position"), &TimelineConnection::set_to_position);
		ClassDB::bind_method(D_METHOD("get_to_position"), &TimelineConnection::get_to_position);
		ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "to_position"), "set_to_position", "get_to_position");

		ADD_GROUP("Compatibility Bezier Handles", "");
		ClassDB::bind_method(D_METHOD("set_from_out_handle", "handle"), &TimelineConnection::set_from_out_handle);
		ClassDB::bind_method(D_METHOD("get_from_out_handle"), &TimelineConnection::get_from_out_handle);
		ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "from_out_handle"), "set_from_out_handle", "get_from_out_handle");

		ClassDB::bind_method(D_METHOD("set_to_in_handle", "handle"), &TimelineConnection::set_to_in_handle);
		ClassDB::bind_method(D_METHOD("get_to_in_handle"), &TimelineConnection::get_to_in_handle);
		ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "to_in_handle"), "set_to_in_handle", "get_to_in_handle");

		ADD_GROUP("Drawing", "");
		ClassDB::bind_method(D_METHOD("set_curve_mode", "mode"), &TimelineConnection::set_curve_mode);
		ClassDB::bind_method(D_METHOD("get_curve_mode"), &TimelineConnection::get_curve_mode);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "curve_mode", PROPERTY_HINT_ENUM, "Line,Bezier"), "set_curve_mode", "get_curve_mode");

		ClassDB::bind_method(D_METHOD("set_color", "color"), &TimelineConnection::set_color);
		ClassDB::bind_method(D_METHOD("get_color"), &TimelineConnection::get_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "color"), "set_color", "get_color");

		ClassDB::bind_method(D_METHOD("set_width", "width"), &TimelineConnection::set_width);
		ClassDB::bind_method(D_METHOD("get_width"), &TimelineConnection::get_width);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "width", PROPERTY_HINT_RANGE, "0,32,0.1,or_greater,suffix:px"), "set_width", "get_width");

		ClassDB::bind_method(D_METHOD("set_curve_segments", "segments"), &TimelineConnection::set_curve_segments);
		ClassDB::bind_method(D_METHOD("get_curve_segments"), &TimelineConnection::get_curve_segments);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "curve_segments", PROPERTY_HINT_RANGE, "2,128,1,or_greater"), "set_curve_segments", "get_curve_segments");

		ClassDB::bind_method(D_METHOD("set_edit_enabled", "enabled"), &TimelineConnection::set_edit_enabled);
		ClassDB::bind_method(D_METHOD("is_edit_enabled"), &TimelineConnection::is_edit_enabled);
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "edit_enabled"), "set_edit_enabled", "is_edit_enabled");

		ClassDB::bind_method(D_METHOD("set_disabled", "disabled"), &TimelineConnection::set_disabled);
		ClassDB::bind_method(D_METHOD("is_disabled"), &TimelineConnection::is_disabled);
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "disabled"), "set_disabled", "is_disabled");

		ClassDB::bind_method(D_METHOD("set_metadata", "meta"), &TimelineConnection::set_metadata);
		ClassDB::bind_method(D_METHOD("get_metadata"), &TimelineConnection::get_metadata);
	}

	TimelineConnection::TimelineConnection() {
		_insert_point_data(0, Vector2(), Vector2(-64.0f, 0.0f), Vector2(80.0f, 0.0f), Variant(), false);
		_insert_point_data(1, Vector2(160.0f, 80.0f), Vector2(-80.0f, 0.0f), Vector2(64.0f, 0.0f), Variant(), false);
	}

	TimelineConnection::~TimelineConnection() {
		_delete_all_point_handles();
	}

	TimelineConnection::PointData* TimelineConnection::_get_point_data_by_id(int64_t p_id) {
		for (PointData &point : points) {
			if (point.id == p_id) {
				return &point;
			}
		}
		return nullptr;
	}

	const TimelineConnection::PointData* TimelineConnection::_get_point_data_by_id(int64_t p_id) const {
		for (const PointData &point : points) {
			if (point.id == p_id) {
				return &point;
			}
		}
		return nullptr;
	}

	TimelineConnectionPoint* TimelineConnection::_insert_point_data(int p_index, const Vector2 &p_position, const Vector2 &p_in_handle, const Vector2 &p_out_handle, const Variant &p_meta, bool p_notify) {
		if (p_index < 0 || p_index > static_cast<int>(points.size())) {
			p_index = static_cast<int>(points.size());
		}

		PointData point;
		point.id = next_point_id++;
		point.position = p_position;
		point.in_handle = p_in_handle;
		point.out_handle = p_out_handle;
		point.meta = p_meta;
		point.handle = memnew(TimelineConnectionPoint);
		_sync_point_handle(point);
		points.insert(points.begin() + p_index, point);

		if (p_notify) {
			emit_changed();
		}
		return point.handle;
	}

	void TimelineConnection::_sync_point_handle(PointData &p_point) {
		if (p_point.handle == nullptr) {
			p_point.handle = memnew(TimelineConnectionPoint);
		}

		p_point.handle->_bind_owner(this, p_point.id);
		p_point.handle->_set_local_position(p_point.position);
		p_point.handle->_set_local_in_handle(p_point.in_handle);
		p_point.handle->_set_local_out_handle(p_point.out_handle);
		p_point.handle->_set_local_selected(p_point.selected);
		p_point.handle->_set_local_metadata(p_point.meta);
	}

	void TimelineConnection::_delete_point_handle(PointData &p_point) {
		if (p_point.handle == nullptr) {
			return;
		}

		p_point.handle->_unbind_owner();
		memdelete(p_point.handle);
		p_point.handle = nullptr;
	}

	void TimelineConnection::_delete_all_point_handles() {
		for (PointData &point : points) {
			_delete_point_handle(point);
		}
	}

	bool TimelineConnection::_set_point_position_by_id(int64_t p_id, const Vector2 &p_position) {
		PointData* point = _get_point_data_by_id(p_id);
		if (point == nullptr) {
			return false;
		}

		point->position = p_position;
		if (point->handle != nullptr) {
			point->handle->_set_local_position(p_position);
			point->handle->emit_signal("changed");
		}
		emit_changed();
		return true;
	}

	Vector2 TimelineConnection::_get_point_position_by_id(int64_t p_id, const Vector2 &p_fallback) const {
		const PointData* point = _get_point_data_by_id(p_id);
		return point != nullptr ? point->position : p_fallback;
	}

	bool TimelineConnection::_set_point_in_handle_by_id(int64_t p_id, const Vector2 &p_handle) {
		PointData* point = _get_point_data_by_id(p_id);
		if (point == nullptr) {
			return false;
		}

		point->in_handle = p_handle;
		if (point->handle != nullptr) {
			point->handle->_set_local_in_handle(p_handle);
			point->handle->emit_signal("changed");
		}
		emit_changed();
		return true;
	}

	Vector2 TimelineConnection::_get_point_in_handle_by_id(int64_t p_id, const Vector2 &p_fallback) const {
		const PointData* point = _get_point_data_by_id(p_id);
		return point != nullptr ? point->in_handle : p_fallback;
	}

	bool TimelineConnection::_set_point_out_handle_by_id(int64_t p_id, const Vector2 &p_handle) {
		PointData* point = _get_point_data_by_id(p_id);
		if (point == nullptr) {
			return false;
		}

		point->out_handle = p_handle;
		if (point->handle != nullptr) {
			point->handle->_set_local_out_handle(p_handle);
			point->handle->emit_signal("changed");
		}
		emit_changed();
		return true;
	}

	Vector2 TimelineConnection::_get_point_out_handle_by_id(int64_t p_id, const Vector2 &p_fallback) const {
		const PointData* point = _get_point_data_by_id(p_id);
		return point != nullptr ? point->out_handle : p_fallback;
	}

	bool TimelineConnection::_set_point_selected_by_id(int64_t p_id, bool p_selected, bool p_signal) {
		PointData* point = _get_point_data_by_id(p_id);
		if (point == nullptr) {
			return false;
		}

		point->selected = p_selected;
		if (point->handle != nullptr) {
			point->handle->_set_local_selected(p_selected);
			if (p_signal) {
				point->handle->emit_signal("changed");
			}
		}
		if (p_signal) {
			emit_changed();
		}
		return true;
	}

	bool TimelineConnection::_is_point_selected_by_id(int64_t p_id, bool p_fallback) const {
		const PointData* point = _get_point_data_by_id(p_id);
		return point != nullptr ? point->selected : p_fallback;
	}

	bool TimelineConnection::_set_point_metadata_by_id(int64_t p_id, const Variant &p_meta) {
		PointData* point = _get_point_data_by_id(p_id);
		if (point == nullptr) {
			return false;
		}

		point->meta = p_meta;
		if (point->handle != nullptr) {
			point->handle->_set_local_metadata(p_meta);
		}
		return true;
	}

	Variant TimelineConnection::_get_point_metadata_by_id(int64_t p_id, const Variant &p_fallback) const {
		const PointData* point = _get_point_data_by_id(p_id);
		return point != nullptr ? point->meta : p_fallback;
	}

	TimelineConnectionPoint* TimelineConnection::add_point(const Vector2 &p_position, int p_index) {
		return _insert_point_data(p_index, p_position, Vector2(-64.0f, 0.0f), Vector2(64.0f, 0.0f), Variant(), true);
	}

	void TimelineConnection::remove_point(TimelineConnectionPoint* p_point) {
		const int index = get_point_index(p_point);
		if (index >= 0) {
			remove_point_at(index);
		}
	}

	void TimelineConnection::remove_point_at(int p_index) {
		if (p_index < 0 || p_index >= static_cast<int>(points.size())) {
			return;
		}

		_delete_point_handle(points[p_index]);
		points.erase(points.begin() + p_index);
		emit_changed();
	}

	void TimelineConnection::clear_points() {
		_delete_all_point_handles();
		points.clear();
		emit_changed();
	}

	void TimelineConnection::set_points(const TypedArray<TimelineConnectionPoint> &p_points) {
		_delete_all_point_handles();
		points.clear();

		for (int i = 0; i < p_points.size(); i++) {
			TimelineConnectionPoint* point = VariantCaster<TimelineConnectionPoint*>::cast(p_points[i]);
			if (point == nullptr) {
				continue;
			}

			const Vector2 position = point->get_position();
			const Vector2 in_handle = point->get_in_handle();
			const Vector2 out_handle = point->get_out_handle();
			const bool selected = point->is_selected();
			const Variant point_meta = point->get_metadata();
			_insert_point_data(static_cast<int>(points.size()), position, in_handle, out_handle, point_meta, false);
			points.back().selected = selected;
			_sync_point_handle(points.back());
		}

		emit_changed();
	}

	TypedArray<TimelineConnectionPoint> TimelineConnection::get_points() const {
		TypedArray<TimelineConnectionPoint> result;
		for (const PointData &point : points) {
			result.push_back(point.handle);
		}
		return result;
	}

	TimelineConnectionPoint* TimelineConnection::get_point(int p_index) const {
		if (p_index < 0 || p_index >= static_cast<int>(points.size())) {
			return nullptr;
		}
		return points[p_index].handle;
	}

	int TimelineConnection::get_point_count() const {
		return static_cast<int>(points.size());
	}

	int TimelineConnection::get_point_index(const TimelineConnectionPoint* p_point) const {
		if (p_point == nullptr) {
			return -1;
		}

		const int64_t id = p_point->get_point_id();
		for (int i = 0; i < static_cast<int>(points.size()); i++) {
			if (points[i].id == id || points[i].handle == p_point) {
				return i;
			}
		}
		return -1;
	}

	void TimelineConnection::set_point_position(int p_index, const Vector2 &p_position) {
		if (p_index < 0 || p_index >= static_cast<int>(points.size())) {
			return;
		}
		_set_point_position_by_id(points[p_index].id, p_position);
	}

	Vector2 TimelineConnection::get_point_position(int p_index) const {
		if (p_index < 0 || p_index >= static_cast<int>(points.size())) {
			return Vector2();
		}
		return points[p_index].position;
	}

	void TimelineConnection::set_point_in_handle(int p_index, const Vector2 &p_handle) {
		if (p_index < 0 || p_index >= static_cast<int>(points.size())) {
			return;
		}
		_set_point_in_handle_by_id(points[p_index].id, p_handle);
	}

	Vector2 TimelineConnection::get_point_in_handle(int p_index) const {
		if (p_index < 0 || p_index >= static_cast<int>(points.size())) {
			return Vector2();
		}
		return points[p_index].in_handle;
	}

	void TimelineConnection::set_point_out_handle(int p_index, const Vector2 &p_handle) {
		if (p_index < 0 || p_index >= static_cast<int>(points.size())) {
			return;
		}
		_set_point_out_handle_by_id(points[p_index].id, p_handle);
	}

	Vector2 TimelineConnection::get_point_out_handle(int p_index) const {
		if (p_index < 0 || p_index >= static_cast<int>(points.size())) {
			return Vector2();
		}
		return points[p_index].out_handle;
	}

	void TimelineConnection::set_from_position(const Vector2 &p_position) {
		if (points.empty()) {
			_insert_point_data(0, p_position, Vector2(-64.0f, 0.0f), Vector2(80.0f, 0.0f), Variant(), true);
			return;
		}
		_set_point_position_by_id(points.front().id, p_position);
	}

	Vector2 TimelineConnection::get_from_position() const {
		return points.empty() ? Vector2() : points.front().position;
	}

	void TimelineConnection::set_to_position(const Vector2 &p_position) {
		if (points.empty()) {
			_insert_point_data(0, Vector2(), Vector2(-64.0f, 0.0f), Vector2(80.0f, 0.0f), Variant(), false);
		}
		if (points.size() == 1) {
			_insert_point_data(1, p_position, Vector2(-80.0f, 0.0f), Vector2(64.0f, 0.0f), Variant(), true);
			return;
		}
		_set_point_position_by_id(points.back().id, p_position);
	}

	Vector2 TimelineConnection::get_to_position() const {
		return points.empty() ? Vector2() : points.back().position;
	}

	void TimelineConnection::set_from_out_handle(const Vector2 &p_handle) {
		if (points.empty()) {
			_insert_point_data(0, Vector2(), Vector2(-64.0f, 0.0f), p_handle, Variant(), true);
			return;
		}
		_set_point_out_handle_by_id(points.front().id, p_handle);
	}

	Vector2 TimelineConnection::get_from_out_handle() const {
		return points.empty() ? Vector2() : points.front().out_handle;
	}

	void TimelineConnection::set_to_in_handle(const Vector2 &p_handle) {
		if (points.empty()) {
			_insert_point_data(0, Vector2(160.0f, 80.0f), p_handle, Vector2(64.0f, 0.0f), Variant(), true);
			return;
		}
		_set_point_in_handle_by_id(points.back().id, p_handle);
	}

	Vector2 TimelineConnection::get_to_in_handle() const {
		return points.empty() ? Vector2() : points.back().in_handle;
	}

	void TimelineConnection::set_curve_mode(CurveMode p_mode) {
		curve_mode = p_mode;
		emit_changed();
	}

	TimelineConnection::CurveMode TimelineConnection::get_curve_mode() const {
		return curve_mode;
	}

	void TimelineConnection::set_color(const Color &p_color) {
		color = p_color;
		emit_changed();
	}

	Color TimelineConnection::get_color() const {
		return color;
	}

	void TimelineConnection::set_width(float p_width) {
		width = p_width;
		emit_changed();
	}

	float TimelineConnection::get_width() const {
		return width;
	}

	void TimelineConnection::set_curve_segments(int p_segments) {
		curve_segments = p_segments;
		emit_changed();
	}

	int TimelineConnection::get_curve_segments() const {
		return curve_segments;
	}

	void TimelineConnection::set_edit_enabled(bool p_enabled) {
		edit_enabled = p_enabled;
		emit_changed();
	}

	bool TimelineConnection::is_edit_enabled() const {
		return edit_enabled;
	}

	void TimelineConnection::set_disabled(bool p_disabled) {
		disabled = p_disabled;
		emit_changed();
	}

	bool TimelineConnection::is_disabled() const {
		return disabled;
	}

	void TimelineConnection::set_metadata(const Variant &p_meta) {
		meta = p_meta;
	}

	Variant TimelineConnection::get_metadata() const {
		return meta;
	}
}
