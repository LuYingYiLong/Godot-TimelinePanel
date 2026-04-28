#include "style_box_arrow.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/rendering_server.hpp>

namespace godot {
	// Helper function declarations.
	inline void adapt_values(int p_index_a, int p_index_b, real_t* adapted_values, const real_t* p_values, const real_t p_width, const real_t p_max_a, const real_t p_max_b);
	inline void set_inner_corner_radius(const Rect2 style_rect, const Rect2 inner_rect, const real_t corner_radius[4], real_t* inner_corner_radius);
	inline Rect2 offset_rect(const Rect2& p_rect, const Vector2& p_offset) {
		Rect2 rect = p_rect;
		rect.position += p_offset;
		return rect;
	}

	inline Point2 skew_point(const Point2& p_point, const Rect2& p_skew_rect, const Vector2& p_skew) {
		const Point2 center = p_skew_rect.get_center();
		const real_t x_skew = -p_skew.x * (p_point.y - center.y);
		const real_t y_skew = -p_skew.y * (p_point.x - center.x);
		return Point2(p_point.x + x_skew, p_point.y + y_skew);
	}

	inline void append_skewed_point(Vector<Point2>& r_points, const Point2& p_point, const Rect2& p_skew_rect, const Vector2& p_skew) {
		r_points.push_back(skew_point(p_point, p_skew_rect, p_skew));
	}

	inline void append_arc(Vector<Point2>& r_points, const Point2& p_center, real_t p_radius_x, real_t p_radius_y,
		real_t p_from_angle, real_t p_to_angle, int p_detail, const Rect2& p_skew_rect, const Vector2& p_skew) {
		const int detail = MAX(p_detail, 1);
		for (int i = 0; i <= detail; i++) {
			const real_t weight = (real_t)i / (real_t)detail;
			const real_t angle = p_from_angle + (p_to_angle - p_from_angle) * weight;
			append_skewed_point(r_points, Point2(p_center.x + Math::cos(angle) * p_radius_x, p_center.y + Math::sin(angle) * p_radius_y), p_skew_rect, p_skew);
		}
	}

	inline void fit_corner_pair(real_t& r_a, real_t& r_b, real_t p_limit) {
		const real_t sum = r_a + r_b;
		if (sum > p_limit && sum > 0) {
			const real_t scale = MAX(p_limit, 0) / sum;
			r_a *= scale;
			r_b *= scale;
		}
	}

	inline void scale_corner_radius(const Rect2& p_rect, const real_t p_corner_radius[4], real_t* r_scaled_corner_radius) {
		for (int i = 0; i < 4; i++) {
			r_scaled_corner_radius[i] = MAX(p_corner_radius[i], 0);
		}

		fit_corner_pair(r_scaled_corner_radius[CORNER_TOP_LEFT], r_scaled_corner_radius[CORNER_TOP_RIGHT], p_rect.size.width);
		fit_corner_pair(r_scaled_corner_radius[CORNER_BOTTOM_LEFT], r_scaled_corner_radius[CORNER_BOTTOM_RIGHT], p_rect.size.width);
		fit_corner_pair(r_scaled_corner_radius[CORNER_TOP_LEFT], r_scaled_corner_radius[CORNER_BOTTOM_LEFT], p_rect.size.height);
		fit_corner_pair(r_scaled_corner_radius[CORNER_TOP_RIGHT], r_scaled_corner_radius[CORNER_BOTTOM_RIGHT], p_rect.size.height);
	}

	inline void append_edge_with_arrow(Vector<Point2>& r_points, const Point2& p_base_from, const Point2& p_base_to, const Point2& p_tip,
		bool p_has_arrow, const Rect2& p_skew_rect, const Vector2& p_skew) {
		append_skewed_point(r_points, p_base_from, p_skew_rect, p_skew);
		if (p_has_arrow) {
			append_skewed_point(r_points, p_tip, p_skew_rect, p_skew);
		}
		append_skewed_point(r_points, p_base_to, p_skew_rect, p_skew);
	}

	inline void build_arrow_outline(Vector<Point2>& r_outline, const Rect2& p_rect, const real_t p_corner_radius[4], Side p_arrow_side,
		real_t p_arrow_width, bool p_has_arrow, int p_corner_detail, const Rect2& p_skew_rect, const Vector2& p_skew) {
		r_outline.clear();

		real_t radius[4];
		scale_corner_radius(p_rect, p_corner_radius, radius);

		const real_t left = p_rect.position.x;
		const real_t top = p_rect.position.y;
		const real_t right = p_rect.position.x + p_rect.size.x;
		const real_t bottom = p_rect.position.y + p_rect.size.y;
		const real_t center_x = p_rect.position.x + p_rect.size.x * 0.5;
		const real_t center_y = p_rect.position.y + p_rect.size.y * 0.5;
		const real_t depth = MAX(p_arrow_width, 0);
		const bool has_arrow = p_has_arrow && depth > 0;

		const Point2 top_left_center(left + radius[CORNER_TOP_LEFT], top + radius[CORNER_TOP_LEFT]);
		const Point2 top_right_center(right - radius[CORNER_TOP_RIGHT], top + radius[CORNER_TOP_RIGHT]);
		const Point2 bottom_right_center(right - radius[CORNER_BOTTOM_RIGHT], bottom - radius[CORNER_BOTTOM_RIGHT]);
		const Point2 bottom_left_center(left + radius[CORNER_BOTTOM_LEFT], bottom - radius[CORNER_BOTTOM_LEFT]);

		append_edge_with_arrow(r_outline,
			Point2(left + radius[CORNER_TOP_LEFT], top),
			Point2(right - radius[CORNER_TOP_RIGHT], top),
			Point2(center_x, top - depth),
			has_arrow && p_arrow_side == SIDE_TOP, p_skew_rect, p_skew);
		append_arc(r_outline, top_right_center, radius[CORNER_TOP_RIGHT], radius[CORNER_TOP_RIGHT], Math_PI * 1.5, Math_PI * 2.0, p_corner_detail, p_skew_rect, p_skew);

		append_edge_with_arrow(r_outline,
			Point2(right, top + radius[CORNER_TOP_RIGHT]),
			Point2(right, bottom - radius[CORNER_BOTTOM_RIGHT]),
			Point2(right + depth, center_y),
			has_arrow && p_arrow_side == SIDE_RIGHT, p_skew_rect, p_skew);
		append_arc(r_outline, bottom_right_center, radius[CORNER_BOTTOM_RIGHT], radius[CORNER_BOTTOM_RIGHT], 0, Math_PI * 0.5, p_corner_detail, p_skew_rect, p_skew);

		append_edge_with_arrow(r_outline,
			Point2(right - radius[CORNER_BOTTOM_RIGHT], bottom),
			Point2(left + radius[CORNER_BOTTOM_LEFT], bottom),
			Point2(center_x, bottom + depth),
			has_arrow && p_arrow_side == SIDE_BOTTOM, p_skew_rect, p_skew);
		append_arc(r_outline, bottom_left_center, radius[CORNER_BOTTOM_LEFT], radius[CORNER_BOTTOM_LEFT], Math_PI * 0.5, Math_PI, p_corner_detail, p_skew_rect, p_skew);

		append_edge_with_arrow(r_outline,
			Point2(left, bottom - radius[CORNER_BOTTOM_LEFT]),
			Point2(left, top + radius[CORNER_TOP_LEFT]),
			Point2(left - depth, center_y),
			has_arrow && p_arrow_side == SIDE_LEFT, p_skew_rect, p_skew);
		append_arc(r_outline, top_left_center, radius[CORNER_TOP_LEFT], radius[CORNER_TOP_LEFT], Math_PI, Math_PI * 1.5, p_corner_detail, p_skew_rect, p_skew);
	}

	inline void draw_shape_fill(PackedVector2Array& r_verts, PackedInt32Array& r_indices, PackedColorArray& r_colors,
		const Vector<Point2>& p_outline, const Point2& p_center, const Color& p_color) {
		const int outline_size = p_outline.size();
		if (outline_size < 3) {
			return;
		}

		const int base = r_verts.size();
		r_verts.push_back(p_center);
		r_colors.push_back(p_color);

		for (int i = 0; i < outline_size; i++) {
			r_verts.push_back(p_outline[i]);
			r_colors.push_back(p_color);
		}

		for (int i = 0; i < outline_size; i++) {
			r_indices.push_back(base);
			r_indices.push_back(base + 1 + i);
			r_indices.push_back(base + 1 + ((i + 1) % outline_size));
		}
	}

	inline void draw_shape_ring(PackedVector2Array& r_verts, PackedInt32Array& r_indices, PackedColorArray& r_colors,
		const Vector<Point2>& p_inner_outline, const Vector<Point2>& p_outer_outline, const Color& p_inner_color, const Color& p_outer_color) {
		const int outline_size = MIN(p_inner_outline.size(), p_outer_outline.size());
		if (outline_size < 3) {
			return;
		}

		const int base = r_verts.size();
		for (int i = 0; i < outline_size; i++) {
			r_verts.push_back(p_inner_outline[i]);
			r_colors.push_back(p_inner_color);
			r_verts.push_back(p_outer_outline[i]);
			r_colors.push_back(p_outer_color);
		}

		for (int i = 0; i < outline_size; i++) {
			const int inner_a = base + i * 2;
			const int outer_a = inner_a + 1;
			const int inner_b = base + ((i + 1) % outline_size) * 2;
			const int outer_b = inner_b + 1;

			r_indices.push_back(inner_a);
			r_indices.push_back(outer_a);
			r_indices.push_back(outer_b);
			r_indices.push_back(inner_a);
			r_indices.push_back(outer_b);
			r_indices.push_back(inner_b);
		}
	}
	void StyleBoxArrow::_bind_methods() {
		BIND_ENUM_CONSTANT(ARROW_TOP);
		BIND_ENUM_CONSTANT(ARROW_RIGHT);
		BIND_ENUM_CONSTANT(ARROW_BOTTOM);
		BIND_ENUM_CONSTANT(ARROW_LEFT);

		ClassDB::bind_method(D_METHOD("set_bg_color", "color"), &StyleBoxArrow::set_bg_color);
		ClassDB::bind_method(D_METHOD("get_bg_color"), &StyleBoxArrow::get_bg_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "bg_color"), "set_bg_color", "get_bg_color");

		ADD_GROUP("Border", "border_");
		ClassDB::bind_method(D_METHOD("set_border_color", "color"), &StyleBoxArrow::set_border_color);
		ClassDB::bind_method(D_METHOD("get_border_color"), &StyleBoxArrow::get_border_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "border_color"), "set_border_color", "get_border_color");

		ADD_GROUP("Border Width", "border_width_");
		ClassDB::bind_method(D_METHOD("set_border_width_all", "width"), &StyleBoxArrow::set_border_width_all);
		ClassDB::bind_method(D_METHOD("get_border_width_min"), &StyleBoxArrow::get_border_width_min);
		ClassDB::bind_method(D_METHOD("set_border_width", "margin", "width"), &StyleBoxArrow::set_border_width);
		ClassDB::bind_method(D_METHOD("get_border_width", "margin"), &StyleBoxArrow::get_border_width);
		ADD_PROPERTYI(PropertyInfo(Variant::INT, "border_width_left", PROPERTY_HINT_RANGE, "0,100,1,or_greater,suffix:px"), "set_border_width", "get_border_width", SIDE_LEFT);
		ADD_PROPERTYI(PropertyInfo(Variant::INT, "border_width_top", PROPERTY_HINT_RANGE, "0,100,1,or_greater,suffix:px"), "set_border_width", "get_border_width", SIDE_TOP);
		ADD_PROPERTYI(PropertyInfo(Variant::INT, "border_width_right", PROPERTY_HINT_RANGE, "0,100,1,or_greater,suffix:px"), "set_border_width", "get_border_width", SIDE_RIGHT);
		ADD_PROPERTYI(PropertyInfo(Variant::INT, "border_width_bottom", PROPERTY_HINT_RANGE, "0,100,1,or_greater,suffix:px"), "set_border_width", "get_border_width", SIDE_BOTTOM);

		ClassDB::bind_method(D_METHOD("set_border_blend", "blend"), &StyleBoxArrow::set_border_blend);
		ClassDB::bind_method(D_METHOD("get_border_blend"), &StyleBoxArrow::get_border_blend);
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "border_blend"), "set_border_blend", "get_border_blend");

		ADD_GROUP("Corner Radius", "corner_radius_");
		ClassDB::bind_method(D_METHOD("set_corner_radius_all", "radius"), &StyleBoxArrow::set_corner_radius_all);
		ClassDB::bind_method(D_METHOD("set_corner_radius", "corner", "radius"), &StyleBoxArrow::set_corner_radius);
		ClassDB::bind_method(D_METHOD("get_corner_radius", "corner"), &StyleBoxArrow::get_corner_radius);
		ADD_PROPERTYI(PropertyInfo(Variant::INT, "corner_radius_top_left", PROPERTY_HINT_RANGE, "0,100,1,or_greater,suffix:px"), "set_corner_radius", "get_corner_radius", CORNER_TOP_LEFT);
		ADD_PROPERTYI(PropertyInfo(Variant::INT, "corner_radius_top_right", PROPERTY_HINT_RANGE, "0,100,1,or_greater,suffix:px"), "set_corner_radius", "get_corner_radius", CORNER_TOP_RIGHT);
		ADD_PROPERTYI(PropertyInfo(Variant::INT, "corner_radius_bottom_right", PROPERTY_HINT_RANGE, "0,100,1,or_greater,suffix:px"), "set_corner_radius", "get_corner_radius", CORNER_BOTTOM_RIGHT);
		ADD_PROPERTYI(PropertyInfo(Variant::INT, "corner_radius_bottom_left", PROPERTY_HINT_RANGE, "0,100,1,or_greater,suffix:px"), "set_corner_radius", "get_corner_radius", CORNER_BOTTOM_LEFT);

		ADD_GROUP("Expand Margins", "expand_margin_");
		ClassDB::bind_method(D_METHOD("set_expand_margin", "margin", "size"), &StyleBoxArrow::set_expand_margin);
		ClassDB::bind_method(D_METHOD("set_expand_margin_all", "size"), &StyleBoxArrow::set_expand_margin_all);
		ClassDB::bind_method(D_METHOD("get_expand_margin", "margin"), &StyleBoxArrow::get_expand_margin);
		ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "expand_margin_left", PROPERTY_HINT_RANGE, "0,100,1,or_greater,suffix:px"), "set_expand_margin", "get_expand_margin", SIDE_LEFT);
		ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "expand_margin_top", PROPERTY_HINT_RANGE, "0,100,1,or_greater,suffix:px"), "set_expand_margin", "get_expand_margin", SIDE_TOP);
		ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "expand_margin_right", PROPERTY_HINT_RANGE, "0,100,1,or_greater,suffix:px"), "set_expand_margin", "get_expand_margin", SIDE_RIGHT);
		ADD_PROPERTYI(PropertyInfo(Variant::FLOAT, "expand_margin_bottom", PROPERTY_HINT_RANGE, "0,100,1,or_greater,suffix:px"), "set_expand_margin", "get_expand_margin", SIDE_BOTTOM);

		ClassDB::bind_method(D_METHOD("set_draw_center", "draw_center"), &StyleBoxArrow::set_draw_center);
		ClassDB::bind_method(D_METHOD("is_draw_center_enabled"), &StyleBoxArrow::is_draw_center_enabled);
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "draw_center"), "set_draw_center", "is_draw_center_enabled");

		ClassDB::bind_method(D_METHOD("set_skew", "skew"), &StyleBoxArrow::set_skew);
		ClassDB::bind_method(D_METHOD("get_skew"), &StyleBoxArrow::get_skew);
		ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "skew"), "set_skew", "get_skew");

		ADD_GROUP("Shadow", "shadow_");
		ClassDB::bind_method(D_METHOD("set_shadow_color", "color"), &StyleBoxArrow::set_shadow_color);
		ClassDB::bind_method(D_METHOD("get_shadow_color"), &StyleBoxArrow::get_shadow_color);
		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "shadow_color"), "set_shadow_color", "get_shadow_color");

		ClassDB::bind_method(D_METHOD("set_shadow_size", "size"), &StyleBoxArrow::set_shadow_size);
		ClassDB::bind_method(D_METHOD("get_shadow_size"), &StyleBoxArrow::get_shadow_size);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "shadow_size", PROPERTY_HINT_RANGE, "0,100,1,or_greater,suffix:px"), "set_shadow_size", "get_shadow_size");

		ClassDB::bind_method(D_METHOD("set_shadow_offset", "offset"), &StyleBoxArrow::set_shadow_offset);
		ClassDB::bind_method(D_METHOD("get_shadow_offset"), &StyleBoxArrow::get_shadow_offset);
		ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "shadow_offset", PROPERTY_HINT_NONE, "suffix:px"), "set_shadow_offset", "get_shadow_offset");

		ADD_GROUP("Anti Aliasing", "anti_aliasing_");
		ClassDB::bind_method(D_METHOD("set_anti_aliased", "anti_aliased"), &StyleBoxArrow::set_anti_aliased);
		ClassDB::bind_method(D_METHOD("is_anti_aliased"), &StyleBoxArrow::is_anti_aliased);
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "anti_aliasing"), "set_anti_aliased", "is_anti_aliased");
		ClassDB::bind_method(D_METHOD("set_aa_size", "size"), &StyleBoxArrow::set_aa_size);
		ClassDB::bind_method(D_METHOD("get_aa_size"), &StyleBoxArrow::get_aa_size);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "anti_aliasing_size", PROPERTY_HINT_RANGE, "0.01,10,0.001,suffix:px"), "set_aa_size", "get_aa_size");

		ClassDB::bind_method(D_METHOD("set_corner_detail", "detail"), &StyleBoxArrow::set_corner_detail);
		ClassDB::bind_method(D_METHOD("get_corner_detail"), &StyleBoxArrow::get_corner_detail);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "corner_detail", PROPERTY_HINT_RANGE, "1,20,1"), "set_corner_detail", "get_corner_detail");

		ClassDB::bind_method(D_METHOD("set_arrow_width", "detail"), &StyleBoxArrow::set_arrow_width);
		ClassDB::bind_method(D_METHOD("get_arrow_width"), &StyleBoxArrow::get_arrow_width);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "arrow_width"), "set_arrow_width", "get_arrow_width");

		ClassDB::bind_method(D_METHOD("set_arrow_direction", "direction"), &StyleBoxArrow::set_arrow_direction);
		ClassDB::bind_method(D_METHOD("get_arrow_direction"), &StyleBoxArrow::get_arrow_direction);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "arrow_direction", PROPERTY_HINT_ENUM, "Top,Right,Bottom,Left"), "set_arrow_direction", "get_arrow_direction");
	}

	void StyleBoxArrow::_validate_property(PropertyInfo& p_property) const {
		if (!Engine::get_singleton()->is_editor_hint()) {
			return;
		}
		if (!anti_aliased && p_property.name == StringName("anti_aliasing_size")) {
			p_property.usage = PROPERTY_USAGE_NO_EDITOR;
		}
	}

	Rect2 StyleBoxArrow::_get_draw_rect(const Rect2& p_rect) const {
		Rect2 draw_rect = p_rect.grow_individual(expand_margin[SIDE_LEFT], expand_margin[SIDE_TOP], expand_margin[SIDE_RIGHT], expand_margin[SIDE_BOTTOM]);

		if (shadow_size > 0) {
			Rect2 shadow_rect = draw_rect.grow(shadow_size);
			shadow_rect.position += shadow_offset;
			draw_rect = draw_rect.merge(shadow_rect);
		}

		return draw_rect;
	}

	void StyleBoxArrow::_draw(const RID& p_to_canvas_item, const Rect2& p_rect) const {
		bool draw_border = (border_width[0] > 0) || (border_width[1] > 0) || (border_width[2] > 0) || (border_width[3] > 0);
		bool draw_shadow = (shadow_size > 0);
		if (!draw_border && !draw_center && !draw_shadow) {
			return;
		}

		Rect2 style_rect = p_rect.grow_individual(expand_margin[SIDE_LEFT], expand_margin[SIDE_TOP], expand_margin[SIDE_RIGHT], expand_margin[SIDE_BOTTOM]);
		if (Math::is_zero_approx(style_rect.size.width) || Math::is_zero_approx(style_rect.size.height)) {
			return;
		}

		{
			Side arrow_side = SIDE_TOP;
			switch (arrow_direction) {
			case ARROW_TOP: arrow_side = SIDE_TOP; break;
			case ARROW_RIGHT: arrow_side = SIDE_RIGHT; break;
			case ARROW_BOTTOM: arrow_side = SIDE_BOTTOM; break;
			case ARROW_LEFT: arrow_side = SIDE_LEFT; break;
			}

			real_t arrow_w = MAX(arrow_width, 0);
			Rect2 body_rect = style_rect;

			if (arrow_side == SIDE_TOP) {
				arrow_w = MIN(arrow_w, style_rect.size.height);
				body_rect.position.y += arrow_w;
				body_rect.size.y = MAX(body_rect.size.y - arrow_w, 0);
			}
			else if (arrow_side == SIDE_BOTTOM) {
				arrow_w = MIN(arrow_w, style_rect.size.height);
				body_rect.size.y = MAX(body_rect.size.y - arrow_w, 0);
			}
			else if (arrow_side == SIDE_LEFT) {
				arrow_w = MIN(arrow_w, style_rect.size.width);
				body_rect.position.x += arrow_w;
				body_rect.size.x = MAX(body_rect.size.x - arrow_w, 0);
			}
			else {
				arrow_w = MIN(arrow_w, style_rect.size.width);
				body_rect.size.x = MAX(body_rect.size.x - arrow_w, 0);
			}

			if (Math::is_zero_approx(body_rect.size.width) || Math::is_zero_approx(body_rect.size.height)) {
				return;
			}

			bool has_arrow = arrow_w > 0;
			const bool rounded_corners = (corner_radius[0] > 0) || (corner_radius[1] > 0) || (corner_radius[2] > 0) || (corner_radius[3] > 0);
			const bool aa_on = (rounded_corners || has_arrow || !skew.is_zero_approx()) && anti_aliased;
			const bool blend_on = blend_border && draw_border;

			Color border_color_inner = blend_on && draw_center ? bg_color : border_color;

			real_t adapted_border[4] = { 1000000.0, 1000000.0, 1000000.0, 1000000.0 };
			adapt_values(SIDE_TOP, SIDE_BOTTOM, adapted_border, border_width, body_rect.size.height, body_rect.size.height, body_rect.size.height);
			adapt_values(SIDE_LEFT, SIDE_RIGHT, adapted_border, border_width, body_rect.size.width, body_rect.size.width, body_rect.size.width);

			real_t adapted_corner[4] = { 1000000.0, 1000000.0, 1000000.0, 1000000.0 };
			adapt_values(CORNER_TOP_RIGHT, CORNER_BOTTOM_RIGHT, adapted_corner, corner_radius, body_rect.size.height, body_rect.size.height - adapted_border[SIDE_BOTTOM], body_rect.size.height - adapted_border[SIDE_TOP]);
			adapt_values(CORNER_TOP_LEFT, CORNER_BOTTOM_LEFT, adapted_corner, corner_radius, body_rect.size.height, body_rect.size.height - adapted_border[SIDE_BOTTOM], body_rect.size.height - adapted_border[SIDE_TOP]);
			adapt_values(CORNER_TOP_LEFT, CORNER_TOP_RIGHT, adapted_corner, corner_radius, body_rect.size.width, body_rect.size.width - adapted_border[SIDE_RIGHT], body_rect.size.width - adapted_border[SIDE_LEFT]);
			adapt_values(CORNER_BOTTOM_LEFT, CORNER_BOTTOM_RIGHT, adapted_corner, corner_radius, body_rect.size.width, body_rect.size.width - adapted_border[SIDE_RIGHT], body_rect.size.width - adapted_border[SIDE_LEFT]);

			Rect2 infill_rect = body_rect.grow_individual(-adapted_border[SIDE_LEFT], -adapted_border[SIDE_TOP], -adapted_border[SIDE_RIGHT], -adapted_border[SIDE_BOTTOM]);
			real_t inner_corner[4];
			set_inner_corner_radius(body_rect, infill_rect, adapted_corner, inner_corner);

			real_t aa_size_scaled = 1.0f;
			if (aa_on) {
				aa_size_scaled = aa_size;
			}

			PackedVector2Array verts;
			PackedInt32Array indices;
			PackedColorArray colors;
			PackedVector2Array uvs;

			Rect2 skew_rect = style_rect;
			Vector<Point2> outer_outline;
			Vector<Point2> inner_outline;
			build_arrow_outline(outer_outline, body_rect, adapted_corner, arrow_side, arrow_w, has_arrow, corner_detail, skew_rect, skew);
			build_arrow_outline(inner_outline, infill_rect, inner_corner, arrow_side, arrow_w, has_arrow, corner_detail, skew_rect, skew);

			if (draw_shadow) {
				Rect2 shadow_body_rect = offset_rect(body_rect, shadow_offset);
				Rect2 shadow_outer_rect = body_rect.grow(shadow_size);
				shadow_outer_rect.position += shadow_offset;
				Rect2 shadow_skew_rect = offset_rect(style_rect, shadow_offset);

				real_t shadow_outer_corner[4] = {
					adapted_corner[0] + shadow_size,
					adapted_corner[1] + shadow_size,
					adapted_corner[2] + shadow_size,
					adapted_corner[3] + shadow_size
				};
				Color shadow_color_transparent = Color(shadow_color.r, shadow_color.g, shadow_color.b, 0);
				Vector<Point2> shadow_outline;
				Vector<Point2> shadow_outer_outline;

				build_arrow_outline(shadow_outline, shadow_body_rect, adapted_corner, arrow_side, arrow_w, has_arrow, corner_detail, shadow_skew_rect, skew);
				build_arrow_outline(shadow_outer_outline, shadow_outer_rect, shadow_outer_corner, arrow_side, arrow_w, has_arrow, corner_detail, shadow_skew_rect, skew);
				draw_shape_ring(verts, indices, colors, shadow_outline, shadow_outer_outline, shadow_color, shadow_color_transparent);
				if (draw_center) {
					draw_shape_fill(verts, indices, colors, shadow_outline, skew_point(shadow_body_rect.get_center(), shadow_skew_rect, skew), shadow_color);
				}
			}

			if (draw_center) {
				if (draw_border) {
					draw_shape_fill(verts, indices, colors, inner_outline, skew_point(infill_rect.get_center(), skew_rect, skew), bg_color);
				}
				else {
					draw_shape_fill(verts, indices, colors, outer_outline, skew_point(body_rect.get_center(), skew_rect, skew), bg_color);
				}
			}

			if (draw_border) {
				draw_shape_ring(verts, indices, colors, inner_outline, outer_outline, border_color_inner, border_color);
			}

			if (aa_on) {
				Rect2 aa_rect = body_rect.grow(aa_size_scaled);
				real_t aa_corner[4] = {
					adapted_corner[0] + aa_size_scaled,
					adapted_corner[1] + aa_size_scaled,
					adapted_corner[2] + aa_size_scaled,
					adapted_corner[3] + aa_size_scaled
				};
				Color edge_color = draw_border ? border_color : bg_color;
				Color edge_color_alpha = Color(edge_color.r, edge_color.g, edge_color.b, 0);
				Vector<Point2> aa_outline;

				build_arrow_outline(aa_outline, aa_rect, aa_corner, arrow_side, arrow_w, has_arrow, corner_detail, skew_rect, skew);
				draw_shape_ring(verts, indices, colors, outer_outline, aa_outline, edge_color, edge_color_alpha);
			}

			Rect2 uv_rect = style_rect.grow(aa_on ? aa_size_scaled : 0);
			uvs.resize(verts.size());
			Point2* uvs_ptr = uvs.ptrw();
			for (int i = 0; i < verts.size(); i++) {
				uvs_ptr[i].x = (verts[i].x - uv_rect.position.x) / uv_rect.size.width;
				uvs_ptr[i].y = (verts[i].y - uv_rect.position.y) / uv_rect.size.height;
			}

			RenderingServer* vs = RenderingServer::get_singleton();
			vs->canvas_item_add_triangle_array(p_to_canvas_item, indices, verts, colors, uvs);
			return;
		}

	}


	inline void adapt_values(int p_index_a, int p_index_b, real_t* adapted_values, const real_t* p_values, const real_t p_width, const real_t p_max_a, const real_t p_max_b) {
		real_t value_a = p_values[p_index_a];
		real_t value_b = p_values[p_index_b];
		real_t factor = MIN(1.0, p_width / (value_a + value_b));
		adapted_values[p_index_a] = MIN(MIN(value_a * factor, p_max_a), adapted_values[p_index_a]);
		adapted_values[p_index_b] = MIN(MIN(value_b * factor, p_max_b), adapted_values[p_index_b]);
	}

	inline void set_inner_corner_radius(const Rect2 style_rect, const Rect2 inner_rect, const real_t corner_radius[4], real_t* inner_corner_radius) {
		real_t border_left = inner_rect.position.x - style_rect.position.x;
		real_t border_top = inner_rect.position.y - style_rect.position.y;
		real_t border_right = style_rect.size.width - inner_rect.size.width - border_left;
		real_t border_bottom = style_rect.size.height - inner_rect.size.height - border_top;

		inner_corner_radius[0] = MAX(corner_radius[0] - MIN(border_top, border_left), 0); // Top left.
		inner_corner_radius[1] = MAX(corner_radius[1] - MIN(border_top, border_right), 0); // Top right.
		inner_corner_radius[2] = MAX(corner_radius[2] - MIN(border_bottom, border_right), 0); // Bottom right.
		inner_corner_radius[3] = MAX(corner_radius[3] - MIN(border_bottom, border_left), 0); // Bottom left.
	}

	void StyleBoxArrow::set_bg_color(const Color& p_color) {
		bg_color = p_color;
		emit_changed();
	}

	Color StyleBoxArrow::get_bg_color() const {
		return bg_color;
	}

	void StyleBoxArrow::set_border_color(const Color& p_color) {
		border_color = p_color;
		emit_changed();
	}

	Color StyleBoxArrow::get_border_color() const {
		return border_color;
	}

	void StyleBoxArrow::set_border_width_all(int p_size) {
		border_width[0] = p_size;
		border_width[1] = p_size;
		border_width[2] = p_size;
		border_width[3] = p_size;
		emit_changed();
	}

	int StyleBoxArrow::get_border_width_min() const {
		return MIN(MIN(border_width[0], border_width[1]), MIN(border_width[2], border_width[3]));
	}

	void StyleBoxArrow::set_border_width(Side p_side, int p_width) {
		ERR_FAIL_INDEX((int)p_side, 4);
		border_width[p_side] = p_width;
		emit_changed();
	}

	int StyleBoxArrow::get_border_width(Side p_side) const {
		ERR_FAIL_INDEX_V((int)p_side, 4, 0);
		return border_width[p_side];
	}

	void StyleBoxArrow::set_border_blend(bool p_blend) {
		blend_border = p_blend;
		emit_changed();
	}

	bool StyleBoxArrow::get_border_blend() const {
		return blend_border;
	}

	void StyleBoxArrow::set_corner_radius(const Corner p_corner, const int radius) {
		ERR_FAIL_INDEX((int)p_corner, 4);
		corner_radius[p_corner] = radius;
		emit_changed();
	}

	void StyleBoxArrow::set_corner_radius_all(int radius) {
		for (int i = 0; i < 4; i++) {
			corner_radius[i] = radius;
		}

		emit_changed();
	}

	void StyleBoxArrow::set_corner_radius_individual(const int radius_top_left, const int radius_top_right, const int radius_bottom_right, const int radius_bottom_left) {
		corner_radius[0] = radius_top_left;
		corner_radius[1] = radius_top_right;
		corner_radius[2] = radius_bottom_right;
		corner_radius[3] = radius_bottom_left;

		emit_changed();
	}

	int StyleBoxArrow::get_corner_radius(const Corner p_corner) const {
		ERR_FAIL_INDEX_V((int)p_corner, 4, 0);
		return corner_radius[p_corner];
	}

	void StyleBoxArrow::set_corner_detail(const int& p_corner_detail) {
		corner_detail = CLAMP(p_corner_detail, 1, 20);
		emit_changed();
	}

	int StyleBoxArrow::get_corner_detail() const {
		return corner_detail;
	}

	void StyleBoxArrow::set_expand_margin(Side p_side, float p_size) {
		ERR_FAIL_INDEX((int)p_side, 4);
		expand_margin[p_side] = p_size;
		emit_changed();
	}

	void StyleBoxArrow::set_expand_margin_all(float p_expand_margin_size) {
		for (int i = 0; i < 4; i++) {
			expand_margin[i] = p_expand_margin_size;
		}
		emit_changed();
	}

	void StyleBoxArrow::set_expand_margin_individual(float p_left, float p_top, float p_right, float p_bottom) {
		expand_margin[SIDE_LEFT] = p_left;
		expand_margin[SIDE_TOP] = p_top;
		expand_margin[SIDE_RIGHT] = p_right;
		expand_margin[SIDE_BOTTOM] = p_bottom;
		emit_changed();
	}

	float StyleBoxArrow::get_expand_margin(Side p_side) const {
		ERR_FAIL_INDEX_V((int)p_side, 4, 0.0);
		return expand_margin[p_side];
	}

	void StyleBoxArrow::set_draw_center(bool p_enabled) {
		draw_center = p_enabled;
		emit_changed();
	}

	bool StyleBoxArrow::is_draw_center_enabled() const {
		return draw_center;
	}

	void StyleBoxArrow::set_skew(Vector2 p_skew) {
		skew = p_skew;
		emit_changed();
	}

	Vector2 StyleBoxArrow::get_skew() const {
		return skew;
	}

	void StyleBoxArrow::set_shadow_color(const Color& p_color) {
		shadow_color = p_color;
		emit_changed();
	}

	Color StyleBoxArrow::get_shadow_color() const {
		return shadow_color;
	}

	void StyleBoxArrow::set_shadow_size(const int& p_size) {
		shadow_size = p_size;
		emit_changed();
	}

	int StyleBoxArrow::get_shadow_size() const {
		return shadow_size;
	}

	void StyleBoxArrow::set_shadow_offset(const Point2& p_offset) {
		shadow_offset = p_offset;
		emit_changed();
	}

	Point2 StyleBoxArrow::get_shadow_offset() const {
		return shadow_offset;
	}

	void StyleBoxArrow::set_anti_aliased(const bool& p_anti_aliased) {
		anti_aliased = p_anti_aliased;
		emit_changed();
		notify_property_list_changed();
	}

	bool StyleBoxArrow::is_anti_aliased() const {
		return anti_aliased;
	}

	void StyleBoxArrow::set_aa_size(const real_t p_aa_size) {
		aa_size = CLAMP(p_aa_size, 0.01, 10);
		emit_changed();
	}

	real_t StyleBoxArrow::get_aa_size() const {
		return aa_size;
	}

	void StyleBoxArrow::set_arrow_width(float p_width) {
		arrow_width = p_width;
		emit_changed();
	}

	float StyleBoxArrow::get_arrow_width() const {
		return arrow_width;
	}

	void StyleBoxArrow::set_arrow_direction(ArrowDirection p_direction) {
		arrow_direction = p_direction;
		emit_changed();
	}

	StyleBoxArrow::ArrowDirection StyleBoxArrow::get_arrow_direction() const {
		return arrow_direction;
	}
}
