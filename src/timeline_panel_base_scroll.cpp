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
	void TimelinePanelBase::_scroll(ScrollBar* p_scroll, double p_amount) {
		_scroll_to(p_scroll, p_scroll->get_value() + p_amount);
	}


	void TimelinePanelBase::_scroll_to(ScrollBar* p_scroll, double p_pos) {
		double pre_scroll = p_scroll->get_value();
		p_scroll->set_value(p_pos);
		if (!Math::is_equal_approx(pre_scroll, p_scroll->get_value())) {
			p_scroll->emit_signal("scrolling");
		}
	}


	void TimelinePanelBase::_update_scroll_bar() {
		if (hscroll == nullptr || vscroll == nullptr) return;

		Size2 hmin = hscroll->get_combined_minimum_size();
		Size2 vmin = vscroll->get_combined_minimum_size();
		Size2 size = get_size();

		float content_width = _calculate_header_width();
		const float top_header_height = panel_orientation == PANEL_ORIENTATION_HORIZONTAL ? _get_horizontal_ruler_height() : header_height;
		const float vertical_content_available_height = MAX(size.y - top_header_height, 0.0f);

		bool v_scroll_show = vertical_scroll_mode == SCROLL_MODE_SHOW_ALWAYS ||
			(vertical_scroll_mode == SCROLL_MODE_AUTO && content_height > vertical_content_available_height);
		const float visible_minimap_width = draw_minimap && minimap_width > 0 && v_scroll_show ? static_cast<float>(minimap_width) : 0.0f;
		const float vertical_side_width = v_scroll_show ? vmin.x + visible_minimap_width : 0.0f;
		const float horizontal_available_width = MAX(size.x - vertical_side_width, 0.0f);
		bool h_scroll_show = horizontal_scroll_mode == SCROLL_MODE_SHOW_ALWAYS ||
			(horizontal_scroll_mode == SCROLL_MODE_AUTO && content_width > horizontal_available_width);

		updating_scroll = true;

		if (h_scroll_show) {
			hscroll->set_anchor_and_offset(SIDE_LEFT, ANCHOR_BEGIN, 0);
			hscroll->set_anchor_and_offset(SIDE_RIGHT, ANCHOR_END, -vertical_side_width);
			hscroll->set_anchor_and_offset(SIDE_TOP, ANCHOR_END, -hmin.y);
			hscroll->set_anchor_and_offset(SIDE_BOTTOM, ANCHOR_END, 0);
			hscroll->set_max(content_width);
			hscroll->set_page(horizontal_available_width);
			hscroll->show();
		}
		else {
			hscroll->hide();
			hscroll->set_value(0);
		}

		if (v_scroll_show) {
			vscroll->set_anchor_and_offset(SIDE_LEFT, ANCHOR_END, -vmin.x);
			vscroll->set_anchor_and_offset(SIDE_RIGHT, ANCHOR_END, 0);
			vscroll->set_anchor_and_offset(SIDE_TOP, ANCHOR_BEGIN, top_header_height);
			vscroll->set_anchor_and_offset(SIDE_BOTTOM, ANCHOR_END, h_scroll_show ? -hmin.y : 0);
			vscroll->set_max(content_height);
			vscroll->set_page(MAX(size.y - top_header_height - (h_scroll_show ? hmin.y : 0), 0.0f));
			vscroll->show();
		}
		else {
			vscroll->hide();
			vscroll->set_value(0);
		}

		hscroll_value = hscroll->get_value();
		vscroll_value = vscroll->get_value();
		updating_scroll = false;
	}


	void TimelinePanelBase::_h_scroll_changed(double p_value) {
		if (updating_scroll) return;
		hscroll_value = p_value;
		queue_redraw();
	}


	void TimelinePanelBase::_v_scroll_changed(double p_value) {
		if (updating_scroll) return;
		vscroll_value = p_value;
		queue_redraw();
	}


	HScrollBar* TimelinePanelBase::get_h_scroll_bar() const {
		return hscroll;
	}


	VScrollBar* TimelinePanelBase::get_v_scroll_bar() const {
		return vscroll;
	}


	void TimelinePanelBase::set_h_scroll(int p_pos) {
		hscroll->set_value(p_pos);
	}


	int TimelinePanelBase::get_h_scroll() const {
		return hscroll->get_value();
	}


	void TimelinePanelBase::set_v_scroll(int p_pos) {
		vscroll->set_value(p_pos);
	}


	int TimelinePanelBase::get_v_scroll() const {
		return vscroll->get_value();
	}


	void TimelinePanelBase::set_horizontal_custom_step(float p_custom_step) {
		hscroll->set_custom_step(p_custom_step);
	}


	float TimelinePanelBase::get_horizontal_custom_step() const {
		return hscroll->get_custom_step();
	}


	void TimelinePanelBase::set_vertical_custom_step(float p_custom_step) {
		vscroll->set_custom_step(p_custom_step);
	}


	float TimelinePanelBase::get_vertical_custom_step() const {
		return vscroll->get_custom_step();
	}


	void TimelinePanelBase::set_horizontal_scroll_mode(ScrollMode p_mode) {
		if (horizontal_scroll_mode == p_mode) {
			return;
		}

		horizontal_scroll_mode = p_mode;
		update_minimum_size();
	}


	TimelinePanelBase::ScrollMode TimelinePanelBase::get_horizontal_scroll_mode() const {
		return horizontal_scroll_mode;
	}


	void TimelinePanelBase::set_vertical_scroll_mode(ScrollMode p_mode) {
		if (vertical_scroll_mode == p_mode) {
			return;
		}

		vertical_scroll_mode = p_mode;
		update_minimum_size();
	}


	TimelinePanelBase::ScrollMode TimelinePanelBase::get_vertical_scroll_mode() const {
		return vertical_scroll_mode;
	}


	void TimelinePanelBase::set_deadzone(int p_deadzone) {
		deadzone = p_deadzone;
	}


	int TimelinePanelBase::get_deadzone() const {
		return deadzone;
	}


}
