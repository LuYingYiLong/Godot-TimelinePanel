#include "register_types.h"

#include "v_timeline_panel.h"
#include "components/timeline_panel_base.h"
#include "components/timeline_panel_indicator.h"
#include "components/timeline_panel_marker.h"
#include "components/timeline_panel_time_ruler.h"
#include "components/timeline_panel_track.h"

#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

using namespace godot;

void initialize_timeline_panel_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		return;
	}

	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	GDREGISTER_VIRTUAL_CLASS(TimelinePanelBase);
	GDREGISTER_CLASS(TimelinePanelIndicator);
	GDREGISTER_CLASS(TimelinePanelMarker);
	GDREGISTER_CLASS(TimelinePanelTimeRuler);
	GDREGISTER_CLASS(TimelinePanelTrack);

	GDREGISTER_CLASS(VTimelinePanel);
}

void uninitialize_timeline_panel_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
}

extern "C" {
	// Initialization.
	GDExtensionBool GDE_EXPORT timeline_panel_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization* r_initialization) {
		godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

		init_obj.register_initializer(initialize_timeline_panel_module);
		init_obj.register_terminator(uninitialize_timeline_panel_module);
		init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_EDITOR);

		return init_obj.init();
	}
}