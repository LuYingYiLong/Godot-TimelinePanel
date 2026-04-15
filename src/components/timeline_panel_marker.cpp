#include "timeline_panel_marker.h"

namespace godot {
	void TimelinePanelMarker::_bind_methods() {
		ClassDB::bind_method(D_METHOD("set_name", "name"), &TimelinePanelMarker::set_name);
		ClassDB::bind_method(D_METHOD("get_name"), &TimelinePanelMarker::get_name);
		ADD_PROPERTY(PropertyInfo(Variant::STRING, "name"), "set_name", "get_name");

		ClassDB::bind_method(D_METHOD("set_annotation", "annotation"), &TimelinePanelMarker::set_annotation);
		ClassDB::bind_method(D_METHOD("get_annotation"), &TimelinePanelMarker::get_annotation);
		ADD_PROPERTY(PropertyInfo(Variant::STRING, "annotation", PROPERTY_HINT_MULTILINE_TEXT), "set_annotation", "get_annotation");

		ClassDB::bind_method(D_METHOD("set_time", "time"), &TimelinePanelMarker::set_time);
		ClassDB::bind_method(D_METHOD("get_time"), &TimelinePanelMarker::get_time);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "time", PROPERTY_HINT_RANGE, "0,99999,0.01,or_greater,suffix:s"), "set_time", "get_time");
	}

	String TimelinePanelMarker::_get_text(const int counting_unit, const double current_value, const double current_position) const {
		return name;
	}

	void TimelinePanelMarker::set_name(const String& p_name) {
		name = p_name;
		emit_changed();
	}

	String TimelinePanelMarker::get_name() const {
		return name;
	}

	void TimelinePanelMarker::set_annotation(const String& p_annotation) {
		annotation = p_annotation;
	}

	String TimelinePanelMarker::get_annotation() const {
		return annotation;
	}

	void TimelinePanelMarker::set_time(const double p_time) {
		time = p_time;
		emit_changed();
	}

	double TimelinePanelMarker::get_time() const {
		return time;
	}
}