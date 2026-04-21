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

		Ref<TextLine> text_line;

	protected:
		static void _bind_methods();

	public:
		TimelineMarker();

		virtual void _draw(
			const RID& p_to_canvas_item,
			const Rect2& p_header_rect,
			const String& p_text,
			const double line_length,
			const bool vertical
		) override;

		void set_name(const String& p_name);
		String get_name() const;

		void set_annotation(const String& p_annotation);
		String get_annotation() const;

		void set_time(const double p_time);
		double get_time() const;
	};
}

#endif // !TIMELINE_MARKER_H
