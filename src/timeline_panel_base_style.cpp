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

#include "theme_helpers.h"

namespace godot {
	void TimelinePanelBase::_load_theme_stylebox_caches() const {
		TimelinePanelBase* self = const_cast<TimelinePanelBase*>(this);
		self->style_cache.instant_key_normal_theme = get_theme_stylebox_or(self, "TimelinePanel", "instant_key_normal");
		self->style_cache.instant_key_selected_theme = get_theme_stylebox_or(self, "TimelinePanel", "instant_key_selected");
		self->style_cache.clip_key_normal_theme = get_theme_stylebox_or(self, "TimelinePanel", "clip_key_normal");
		self->style_cache.clip_key_selected_theme = get_theme_stylebox_or(self, "TimelinePanel", "clip_key_selected");
		self->style_cache.key_release_preview_theme = get_theme_stylebox_or(self, "TimelinePanel", "key_release_preview");
		self->style_cache.key_allowed_overlap_preview_theme = get_theme_stylebox_or(self, "TimelinePanel", "key_allowed_overlap_preview");
		self->style_cache.selection_rect_theme = get_theme_stylebox_or(self, "TimelinePanel", "selection_rect");
		self->style_cache.instant_key_scale_theme = get_theme_constant_or(self, "TimelinePanel", "instant_key_scale", -1);
		self->style_cache.theme_caches_valid = true;
	}

	Ref<StyleBox> TimelinePanelBase::_get_instant_key_normal_style(const TimelineTrackKey *p_key) const {
		if (p_key && p_key->get_instant_key_normal_style().is_valid()) {
			return p_key->get_instant_key_normal_style();
		}
		if (!style_cache.theme_caches_valid) {
			_load_theme_stylebox_caches();
		}
		if (style_cache.instant_key_normal_theme.is_valid()) {
			return style_cache.instant_key_normal_theme;
		}
		return style_cache.instant_key_normal_fallback;
	}


	Ref<StyleBox> TimelinePanelBase::_get_instant_key_selected_style(const TimelineTrackKey *p_key) const {
		if (p_key && p_key->get_instant_key_selected_style().is_valid()) {
			return p_key->get_instant_key_selected_style();
		}
		if (!style_cache.theme_caches_valid) {
			_load_theme_stylebox_caches();
		}
		if (style_cache.instant_key_selected_theme.is_valid()) {
			return style_cache.instant_key_selected_theme;
		}
		return style_cache.instant_key_selected_fallback;
	}


	Ref<StyleBox> TimelinePanelBase::_get_clip_key_normal_style(const TimelineTrackKey *p_key) const {
		if (p_key && p_key->get_clip_key_normal_style().is_valid()) {
			return p_key->get_clip_key_normal_style();
		}
		if (!style_cache.theme_caches_valid) {
			_load_theme_stylebox_caches();
		}
		if (style_cache.clip_key_normal_theme.is_valid()) {
			return style_cache.clip_key_normal_theme;
		}
		return style_cache.clip_key_normal_fallback;
	}


	Ref<StyleBox> TimelinePanelBase::_get_clip_key_selected_style(const TimelineTrackKey *p_key) const {
		if (p_key && p_key->get_clip_key_selected_style().is_valid()) {
			return p_key->get_clip_key_selected_style();
		}
		if (!style_cache.theme_caches_valid) {
			_load_theme_stylebox_caches();
		}
		if (style_cache.clip_key_selected_theme.is_valid()) {
			return style_cache.clip_key_selected_theme;
		}
		return style_cache.clip_key_selected_fallback;
	}


	Ref<StyleBox> TimelinePanelBase::_get_key_release_preview_style() const {
		if (!style_cache.theme_caches_valid) {
			_load_theme_stylebox_caches();
		}
		if (style_cache.key_release_preview_theme.is_valid()) {
			return style_cache.key_release_preview_theme;
		}
		return style_cache.key_release_preview_fallback;
	}


	Ref<StyleBox> TimelinePanelBase::_get_key_allowed_overlap_preview_style() const {
		if (!style_cache.theme_caches_valid) {
			_load_theme_stylebox_caches();
		}
		if (style_cache.key_allowed_overlap_preview_theme.is_valid()) {
			return style_cache.key_allowed_overlap_preview_theme;
		}
		return style_cache.key_allowed_overlap_preview_fallback;
	}



































	Ref<StyleBox> TimelinePanelBase::_get_selection_rect_style() const {
		if (!style_cache.theme_caches_valid) {
			_load_theme_stylebox_caches();
		}
		if (style_cache.selection_rect_theme.is_valid()) {
			return style_cache.selection_rect_theme;
		}
		return Ref<StyleBox>();
	}
}
