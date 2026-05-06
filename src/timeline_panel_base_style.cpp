#include "timeline_panel_base.h"

#include "components/timeline_indicator.h"
#include "components/timeline_marker.h"
#include "components/timeline_time_ruler.h"
#include "components/timeline_track.h"
#include "components/timeline_track_key.h"

#include <algorithm>
#include <unordered_set>
#include <godot_cpp/classes/font.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/input_event_mouse_motion.hpp>
#include <godot_cpp/classes/input_event_pan_gesture.hpp>
#include <godot_cpp/classes/style_box_flat.hpp>
#include <godot_cpp/classes/theme_db.hpp>

namespace godot {
	Ref<StyleBox> TimelinePanelBase::_get_instant_key_normal_style(const TimelineTrackKey *p_key) const {
		if (p_key && p_key->get_instant_key_normal_style().is_valid()) {
			return p_key->get_instant_key_normal_style();
		}
		if (style_cache.instant_key_normal.is_valid()) {
			return style_cache.instant_key_normal;
		}
		return style_cache.instant_key_normal_fallback;
	}


	Ref<StyleBox> TimelinePanelBase::_get_instant_key_selected_style(const TimelineTrackKey *p_key) const {
		if (p_key && p_key->get_instant_key_selected_style().is_valid()) {
			return p_key->get_instant_key_selected_style();
		}
		if (style_cache.instant_key_selected.is_valid()) {
			return style_cache.instant_key_selected;
		}
		return style_cache.instant_key_selected_fallback;
	}


	Ref<StyleBox> TimelinePanelBase::_get_clip_key_normal_style(const TimelineTrackKey *p_key) const {
		if (p_key && p_key->get_clip_key_normal_style().is_valid()) {
			return p_key->get_clip_key_normal_style();
		}
		if (style_cache.clip_key_normal.is_valid()) {
			return style_cache.clip_key_normal;
		}
		return style_cache.clip_key_normal_fallback;
	}


	Ref<StyleBox> TimelinePanelBase::_get_clip_key_selected_style(const TimelineTrackKey *p_key) const {
		if (p_key && p_key->get_clip_key_selected_style().is_valid()) {
			return p_key->get_clip_key_selected_style();
		}
		if (style_cache.clip_key_selected.is_valid()) {
			return style_cache.clip_key_selected;
		}
		return style_cache.clip_key_selected_fallback;
	}


	Ref<StyleBox> TimelinePanelBase::_get_key_release_preview_style() const {
		if (style_cache.key_release_preview.is_valid()) {
			return style_cache.key_release_preview;
		}
		return style_cache.key_release_preview_fallback;
	}


	void TimelinePanelBase::set_instant_key_scale(const float p_scale) {
		style_cache.instant_key_scale = p_scale;
		_refresh_track_key_metrics();
		queue_redraw();
	}


	float TimelinePanelBase::get_instant_key_scale() const {
		return style_cache.instant_key_scale;
	}


	void TimelinePanelBase::set_instant_key_normal_style(Ref<StyleBox> p_style) {
		style_cache.instant_key_normal = p_style;
		queue_redraw();
	}


	Ref<StyleBox> TimelinePanelBase::get_instant_key_normal_style() const {
		return style_cache.instant_key_normal;
	}


	void TimelinePanelBase::set_instant_key_selected_style(Ref<StyleBox> p_style) {
		style_cache.instant_key_selected = p_style;
		queue_redraw();
	}


	Ref<StyleBox> TimelinePanelBase::get_instant_key_selected_style() const {
		return style_cache.instant_key_selected;
	}


	void TimelinePanelBase::set_clip_key_normal_style(Ref<StyleBox> p_style) {
		style_cache.clip_key_normal = p_style;
		queue_redraw();
	}


	Ref<StyleBox> TimelinePanelBase::get_clip_key_normal_style() const {
		return style_cache.clip_key_normal;
	}


	void TimelinePanelBase::set_clip_key_selected_style(Ref<StyleBox> p_style) {
		style_cache.clip_key_selected = p_style;
		queue_redraw();
	}


	Ref<StyleBox> TimelinePanelBase::get_clip_key_selected_style() const {
		return style_cache.clip_key_selected;
	}


	void TimelinePanelBase::set_selection_rect_style(Ref<StyleBox> p_style) {
		style_cache.selection_rect = p_style;
		queue_redraw();
	}


	Ref<StyleBox> TimelinePanelBase::get_selection_rect_style() const {
		return style_cache.selection_rect;
	}


	void TimelinePanelBase::set_key_release_preview_style(Ref<StyleBox> p_style) {
		style_cache.key_release_preview = p_style;
		queue_redraw();
	}


	Ref<StyleBox> TimelinePanelBase::get_key_release_preview_style() const {
		return style_cache.key_release_preview;
	}


}
