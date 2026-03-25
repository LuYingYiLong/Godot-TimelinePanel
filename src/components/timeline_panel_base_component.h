#ifndef TIMELINE_PANEL_BASE_COMPONENT_H
#define TIMELINE_PANEL_BASE_COMPONENT_H

#include <godot_cpp/classes/resource.hpp>

namespace godot {
	class TimelinePanelBaseComponent : public Resource {
		GDCLASS(TimelinePanelBaseComponent, Resource)

	private:

	protected:
		static void _bind_methods();

	public:

	};
}

#endif // !TIMELINE_PANEL_BASE_COMPONENT_H
