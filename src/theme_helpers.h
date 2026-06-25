#ifndef TIMELINE_PANEL_THEME_HELPERS_H
#define TIMELINE_PANEL_THEME_HELPERS_H

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/theme.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/string_name.hpp>

namespace {

inline godot::Ref<godot::Theme> get_inherited_theme(godot::Control *p_owner) {
	godot::Control *control = p_owner;
	while (control != nullptr) {
		const godot::Ref<godot::Theme> theme = control->get_theme();
		if (theme.is_valid()) {
			return theme;
		}
		control = control->get_parent_control();
	}
	return godot::Ref<godot::Theme>();
}

inline godot::Color get_theme_color_or(godot::Control *p_owner, const godot::StringName &p_type, const godot::StringName &p_name, const godot::Color &p_default) {
	const godot::Ref<godot::Theme> theme = get_inherited_theme(p_owner);
	if (theme.is_valid() && theme->has_color(p_name, p_type)) {
		return theme->get_color(p_name, p_type);
	}
	return p_default;
}

inline int32_t get_theme_constant_or(godot::Control *p_owner, const godot::StringName &p_type, const godot::StringName &p_name, int32_t p_default) {
	const godot::Ref<godot::Theme> theme = get_inherited_theme(p_owner);
	if (theme.is_valid() && theme->has_constant(p_name, p_type)) {
		return theme->get_constant(p_name, p_type);
	}
	return p_default;
}

inline godot::Ref<godot::StyleBox> get_theme_stylebox_or(godot::Control *p_owner, const godot::StringName &p_type, const godot::StringName &p_name) {
	const godot::Ref<godot::Theme> theme = get_inherited_theme(p_owner);
	if (theme.is_valid() && theme->has_stylebox(p_name, p_type)) {
		return theme->get_stylebox(p_name, p_type);
	}
	return godot::Ref<godot::StyleBox>();
}

} // namespace

#endif // TIMELINE_PANEL_THEME_HELPERS_H
