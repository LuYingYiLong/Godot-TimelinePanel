#ifndef TIMELINE_BASE_H
#define TIMELINE_BASE_H

#include <godot_cpp/classes/resource.hpp>

namespace godot {
	class TimelineBase : public Resource {
		GDCLASS(TimelineBase, Resource)

	private:

	protected:
		static void _bind_methods();

	public:

	};
}

#endif // !TIMELINE_BASE_H
