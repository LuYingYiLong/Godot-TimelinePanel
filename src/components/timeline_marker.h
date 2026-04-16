#ifndef TIMELINE_MARKER_H
#define TIMELINE_MARKER_H

#include "timeline_indicator.h"

namespace godot {
	class TimelineMarker : public TimelineIndicator {
		GDCLASS(TimelineMarker, TimelineIndicator)

	private:
		String name;
		String annotation;
		double time = 0.0f;

	protected:
		static void _bind_methods();

	public:
		virtual String _get_text(const int counting_unit, const double current_value, const double current_position) const override;

		void set_name(const String& p_name);
		String get_name() const;

		void set_annotation(const String& p_annotation);
		String get_annotation() const;

		void set_time(const double p_time);
		double get_time() const;
	};
}

#endif // !TIMELINE_MARKER_H
