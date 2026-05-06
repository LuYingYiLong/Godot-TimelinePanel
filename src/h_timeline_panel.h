#ifndef H_TIMELINE_PANEL_H
#define H_TIMELINE_PANEL_H

#include "timeline_panel_base.h"

namespace godot {
	class HTimelinePanel : public TimelinePanelBase {
		GDCLASS(HTimelinePanel, TimelinePanelBase)

	protected:
		static void _bind_methods();

	public:
		using CountingUnit = TimelinePanelBase::CountingUnit;
		using TimeFormat = TimelinePanelBase::TimeFormat;
		using BeatFormat = TimelinePanelBase::BeatFormat;
		using BarNumberDirection = TimelinePanelBase::BarNumberDirection;
		using ScrollMode = TimelinePanelBase::ScrollMode;

		HTimelinePanel();
	};
}

#endif // !H_TIMELINE_PANEL_H
