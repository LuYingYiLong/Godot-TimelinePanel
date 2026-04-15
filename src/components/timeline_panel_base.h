#ifndef TIMELINE_PANEL_BASE_H
#define TIMELINE_PANEL_BASE_H

#include <godot_cpp/classes/resource.hpp>

namespace godot {
	class TimelinePanelBase : public Resource {
		GDCLASS(TimelinePanelBase, Resource)

	private:

	protected:
		static void _bind_methods();

	public:

	};
}

#endif // !TIMELINE_PANEL_BASE_H
