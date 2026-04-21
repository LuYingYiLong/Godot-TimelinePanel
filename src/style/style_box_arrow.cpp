#include "style_box_arrow.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/text_server.hpp>
#include <godot_cpp/classes/rendering_server.hpp>

namespace godot {
	// 辅助函数的前向声明
	inline void adapt_values(int p_index_a, int p_index_b, real_t* adapted_values, const real_t* p_values, const real_t p_width, const real_t p_max_a, const real_t p_max_b);
	inline void set_inner_corner_radius(const Rect2 style_rect, const Rect2 inner_rect, const real_t corner_radius[4], real_t* inner_corner_radius);
	inline void set_corner_scale(const Rect2& style_rect, const Rect2& inner_rect, const real_t corner_radius[4], Point2* inner_scale);
	inline void draw_rounded_rectangle(PackedVector2Array& verts, PackedInt32Array& indices, PackedColorArray& colors, const Rect2& style_rect, const real_t corner_radius[4],
		const Rect2& ring_rect, const Rect2& inner_rect, const Color& inner_color, const Color& outer_color, const int corner_detail, const Vector2& skew, bool is_filled = false);

	// 向网格中添加三角形的辅助函数
	inline void add_triangle(PackedVector2Array& verts, PackedInt32Array& indices, PackedColorArray& colors, const Point2& p_a, const Point2& p_b, const Point2& p_c, const Color& p_color) {
		int base = verts.size();
		verts.push_back(p_a);
		verts.push_back(p_b);
		verts.push_back(p_c);
		indices.push_back(base);
		indices.push_back(base + 1);
		indices.push_back(base + 2);
		colors.push_back(p_color);
		colors.push_back(p_color);
		colors.push_back(p_color);
	}

	// 向网格中添加四边形的辅助函数
	inline void add_quad(PackedVector2Array& verts, PackedInt32Array& indices, PackedColorArray& colors, const Point2& p_a, const Point2& p_b, const Point2& p_c, const Point2& p_d, const Color& p_color) {
		int base = verts.size();
		verts.push_back(p_a);
		verts.push_back(p_b);
		verts.push_back(p_c);
		verts.push_back(p_d);
		// Triangle 1: A-B-C
		indices.push_back(base);
		indices.push_back(base + 1);
		indices.push_back(base + 2);
		// Triangle 2: A-C-D
		indices.push_back(base);
		indices.push_back(base + 2);
		indices.push_back(base + 3);
		colors.push_back(p_color);
		colors.push_back(p_color);
		colors.push_back(p_color);
		colors.push_back(p_color);
	}

	void StyleBoxArrow::_bind_methods() {
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

		// 确定箭头方向并计算主体/箭头矩形
		Side arrow_side = SIDE_TOP;
		switch (arrow_direction) {
			case ARROW_TOP: arrow_side = SIDE_TOP; break;
			case ARROW_RIGHT: arrow_side = SIDE_RIGHT; break;
			case ARROW_BOTTOM: arrow_side = SIDE_BOTTOM; break;
			case ARROW_LEFT: arrow_side = SIDE_LEFT; break;
		}

		real_t arrow_w = arrow_width;
		Rect2 body_rect = style_rect;
		Rect2 arrow_rect = style_rect;

		// 将 style_rect 分割为 body_rect（用于圆角矩形）和 arrow_rect（用于三角形尖端）
		// 箭头位于所选边的中心
		if (arrow_side == SIDE_TOP) {
			arrow_w = MIN(arrow_w, style_rect.size.width);
			body_rect.position.y += arrow_w;
			body_rect.size.y = MAX(body_rect.size.y - arrow_w, 0);
			arrow_rect.size.y = arrow_w;
		} else if (arrow_side == SIDE_BOTTOM) {
			arrow_w = MIN(arrow_w, style_rect.size.width);
			body_rect.size.y = MAX(body_rect.size.y - arrow_w, 0);
			arrow_rect.position.y = body_rect.position.y + body_rect.size.y;
			arrow_rect.size.y = arrow_w;
		} else if (arrow_side == SIDE_LEFT) {
			arrow_w = MIN(arrow_w, style_rect.size.height);
			body_rect.position.x += arrow_w;
			body_rect.size.x = MAX(body_rect.size.x - arrow_w, 0);
			arrow_rect.size.x = arrow_w;
		} else { // SIDE_RIGHT
			arrow_w = MIN(arrow_w, style_rect.size.height);
			body_rect.size.x = MAX(body_rect.size.x - arrow_w, 0);
			arrow_rect.position.x = body_rect.position.x + body_rect.size.x;
			arrow_rect.size.x = arrow_w;
		}

		// 如果箭头占据整个矩形，则回退到无箭头绘制
		bool has_arrow = arrow_w > 0 && body_rect.size.width > 0 && body_rect.size.height > 0;

		const bool rounded_corners = (corner_radius[0] > 0) || (corner_radius[1] > 0) || (corner_radius[2] > 0) || (corner_radius[3] > 0);
		const bool aa_on = (rounded_corners || !skew.is_zero_approx()) && anti_aliased;
		const bool blend_on = blend_border && draw_border;

		Color border_color_alpha = Color(border_color.r, border_color.g, border_color.b, 0);
		Color border_color_blend = (draw_center ? bg_color : border_color_alpha);
		Color border_color_inner = blend_on ? border_color_blend : border_color;

		// 为 body_rect 调整边框
		real_t width = MAX(body_rect.size.width, 0);
		real_t height = MAX(body_rect.size.height, 0);
		real_t adapted_border[4] = { 1000000.0, 1000000.0, 1000000.0, 1000000.0 };
		adapt_values(SIDE_TOP, SIDE_BOTTOM, adapted_border, border_width, height, height, height);
		adapt_values(SIDE_LEFT, SIDE_RIGHT, adapted_border, border_width, width, width, width);

		// 为 body_rect 调整圆角
		real_t adapted_corner[4] = { 1000000.0, 1000000.0, 1000000.0, 1000000.0 };
		adapt_values(CORNER_TOP_RIGHT, CORNER_BOTTOM_RIGHT, adapted_corner, corner_radius, height, height - adapted_border[SIDE_BOTTOM], height - adapted_border[SIDE_TOP]);
		adapt_values(CORNER_TOP_LEFT, CORNER_BOTTOM_LEFT, adapted_corner, corner_radius, height, height - adapted_border[SIDE_BOTTOM], height - adapted_border[SIDE_TOP]);
		adapt_values(CORNER_TOP_LEFT, CORNER_TOP_RIGHT, adapted_corner, corner_radius, width, width - adapted_border[SIDE_RIGHT], width - adapted_border[SIDE_LEFT]);
		adapt_values(CORNER_BOTTOM_LEFT, CORNER_BOTTOM_RIGHT, adapted_corner, corner_radius, width, width - adapted_border[SIDE_RIGHT], width - adapted_border[SIDE_LEFT]);

		Rect2 infill_rect = body_rect.grow_individual(-adapted_border[SIDE_LEFT], -adapted_border[SIDE_TOP], -adapted_border[SIDE_RIGHT], -adapted_border[SIDE_BOTTOM]);
		Rect2 border_style_rect = body_rect;

		real_t aa_size_scaled = 1.0f;
		if (aa_on) {
			real_t scale_factor = 0.0f;
			if (scale_factor == 0.0) {
				scale_factor = 1.0;
			}
			aa_size_scaled = aa_size / scale_factor;
		}

		if (aa_on) {
			for (int i = 0; i < 4; i++) {
				if (border_width[i] > 0) {
					border_style_rect = border_style_rect.grow_side((Side)i, -aa_size_scaled);
				}
			}
		}

		PackedVector2Array verts;
		PackedInt32Array indices;
		PackedColorArray colors;
		PackedVector2Array uvs;

		// 绘制阴影（仅针对主体，为简化省略箭头阴影）
		if (draw_shadow) {
			Rect2 shadow_inner_rect = body_rect;
			shadow_inner_rect.position += shadow_offset;
			Rect2 shadow_rect = body_rect.grow(shadow_size);
			shadow_rect.position += shadow_offset;
			Color shadow_color_transparent = Color(shadow_color.r, shadow_color.g, shadow_color.b, 0);

			draw_rounded_rectangle(verts, indices, colors, shadow_inner_rect, adapted_corner,
				shadow_rect, shadow_inner_rect, shadow_color, shadow_color_transparent, corner_detail, skew);
			if (draw_center) {
				draw_rounded_rectangle(verts, indices, colors, shadow_inner_rect, adapted_corner,
					shadow_inner_rect, shadow_inner_rect, shadow_color, shadow_color, corner_detail, skew, true);
			}
		}

		// 绘制主体圆角矩形
		// 边框（无抗锯齿）
		if (draw_border && !aa_on) {
			draw_rounded_rectangle(verts, indices, colors, border_style_rect, adapted_corner,
				border_style_rect, infill_rect, border_color_inner, border_color, corner_detail, skew);
		}
		// 填充（无抗锯齿）
		if (draw_center && (!aa_on || blend_on)) {
			draw_rounded_rectangle(verts, indices, colors, border_style_rect, adapted_corner,
				infill_rect, infill_rect, bg_color, bg_color, corner_detail, skew, true);
		}

		// 主体抗锯齿
		if (aa_on) {
			real_t aa_border_width[4] = {};
			real_t aa_border_width_half[4] = {};
			real_t aa_fill_width[4] = {};
			real_t aa_fill_width_half[4] = {};

			if (draw_border) {
				for (int i = 0; i < 4; i++) {
					if (border_width[i] > 0) {
						aa_border_width[i] = aa_size_scaled;
						aa_border_width_half[i] = aa_size_scaled * 0.5;
						aa_fill_width[i] = 0;
						aa_fill_width_half[i] = 0;
					} else {
						aa_border_width[i] = 0;
						aa_border_width_half[i] = 0;
						aa_fill_width[i] = aa_size_scaled;
						aa_fill_width_half[i] = aa_size_scaled * 0.5;
					}
				}
			} else {
				for (int i = 0; i < 4; i++) {
					aa_border_width[i] = 0;
					aa_border_width_half[i] = 0;
					aa_fill_width[i] = aa_size_scaled;
					aa_fill_width_half[i] = aa_size_scaled * 0.5;
				}
			}

			if (draw_center) {
				Rect2 infill_rect_aa_transparent = infill_rect.grow_individual(aa_fill_width_half[SIDE_LEFT], aa_fill_width_half[SIDE_TOP],
					aa_fill_width_half[SIDE_RIGHT], aa_fill_width_half[SIDE_BOTTOM]);
				Rect2 infill_rect_aa_colored = infill_rect_aa_transparent.grow_individual(-aa_fill_width[SIDE_LEFT], -aa_fill_width[SIDE_TOP],
					-aa_fill_width[SIDE_RIGHT], -aa_fill_width[SIDE_BOTTOM]);
				if (!blend_on) {
					draw_rounded_rectangle(verts, indices, colors, border_style_rect, adapted_corner,
						infill_rect_aa_colored, infill_rect_aa_colored, bg_color, bg_color, corner_detail, skew, true);
				}
				if (!blend_on || !draw_border) {
					Color alpha_bg = Color(bg_color.r, bg_color.g, bg_color.b, 0);
					draw_rounded_rectangle(verts, indices, colors, border_style_rect, adapted_corner,
						infill_rect_aa_transparent, infill_rect_aa_colored, bg_color, alpha_bg, corner_detail, skew);
				}
			}

			if (draw_border) {
				Rect2 inner_rect_aa_colored = infill_rect.grow_individual(aa_border_width_half[SIDE_LEFT], aa_border_width_half[SIDE_TOP],
					aa_border_width_half[SIDE_RIGHT], aa_border_width_half[SIDE_BOTTOM]);
				Rect2 inner_rect_aa_transparent = inner_rect_aa_colored.grow_individual(-aa_border_width[SIDE_LEFT], -aa_border_width[SIDE_TOP],
					-aa_border_width[SIDE_RIGHT], -aa_border_width[SIDE_BOTTOM]);
				Rect2 outer_rect_aa_transparent = body_rect.grow_individual(aa_border_width_half[SIDE_LEFT], aa_border_width_half[SIDE_TOP],
					aa_border_width_half[SIDE_RIGHT], aa_border_width_half[SIDE_BOTTOM]);
				Rect2 outer_rect_aa_colored = border_style_rect.grow_individual(aa_border_width_half[SIDE_LEFT], aa_border_width_half[SIDE_TOP],
					aa_border_width_half[SIDE_RIGHT], aa_border_width_half[SIDE_BOTTOM]);

				draw_rounded_rectangle(verts, indices, colors, border_style_rect, adapted_corner,
					outer_rect_aa_colored, ((blend_on) ? infill_rect : inner_rect_aa_colored), border_color_inner, border_color, corner_detail, skew);
				if (!blend_on) {
					draw_rounded_rectangle(verts, indices, colors, border_style_rect, adapted_corner,
						inner_rect_aa_colored, inner_rect_aa_transparent, border_color_blend, border_color, corner_detail, skew);
				}
				draw_rounded_rectangle(verts, indices, colors, border_style_rect, adapted_corner,
					outer_rect_aa_transparent, outer_rect_aa_colored, border_color, border_color_alpha, corner_detail, skew);
			}
		}

		// 绘制箭头（三角形 + 可选边框）
		if (has_arrow && arrow_w > 0) {
			Point2 tip, base_left, base_right;
			real_t half_base = 0;

			if (arrow_side == SIDE_TOP) {
				half_base = arrow_rect.size.width * 0.5;
				tip = Point2(arrow_rect.position.x + half_base, arrow_rect.position.y);
				base_left = Point2(arrow_rect.position.x, arrow_rect.position.y + arrow_rect.size.y);
				base_right = Point2(arrow_rect.position.x + arrow_rect.size.width, arrow_rect.position.y + arrow_rect.size.y);
			} else if (arrow_side == SIDE_BOTTOM) {
				half_base = arrow_rect.size.width * 0.5;
				tip = Point2(arrow_rect.position.x + half_base, arrow_rect.position.y + arrow_rect.size.y);
				base_left = Point2(arrow_rect.position.x + arrow_rect.size.width, arrow_rect.position.y);
				base_right = Point2(arrow_rect.position.x, arrow_rect.position.y);
			} else if (arrow_side == SIDE_LEFT) {
				half_base = arrow_rect.size.height * 0.5;
				tip = Point2(arrow_rect.position.x, arrow_rect.position.y + half_base);
				base_left = Point2(arrow_rect.position.x + arrow_rect.size.x, arrow_rect.position.y + arrow_rect.size.y);
				base_right = Point2(arrow_rect.position.x + arrow_rect.size.x, arrow_rect.position.y);
			} else {
				half_base = arrow_rect.size.height * 0.5;
				tip = Point2(arrow_rect.position.x + arrow_rect.size.x, arrow_rect.position.y + half_base);
				base_left = Point2(arrow_rect.position.x, arrow_rect.position.y);
				base_right = Point2(arrow_rect.position.x, arrow_rect.position.y + arrow_rect.size.y);
			}

			// 绘制箭头中心填充
			if (draw_center) {
				add_triangle(verts, indices, colors, tip, base_left, base_right, bg_color);
			}

			// 绘制箭头边框（作为三角形周围的细环）
			if (draw_border) {
				// 计算箭头边缘的边框厚度
				real_t bw = 0;
				if (arrow_side == SIDE_TOP) bw = adapted_border[SIDE_BOTTOM];
				else if (arrow_side == SIDE_BOTTOM) bw = adapted_border[SIDE_TOP];
				else if (arrow_side == SIDE_LEFT) bw = adapted_border[SIDE_RIGHT];
				else bw = adapted_border[SIDE_LEFT];

				if (bw > 0) {
					// 通过将每条边向内移动 bw 来计算内部三角形
					// 对于三角形，我们沿着每条边的法线偏移来缩小
					Point2 inner_tip, inner_base_left, inner_base_right;

					// 边向量
					Vector2 e1 = base_left - tip;
					Vector2 e2 = base_right - base_left;
					Vector2 e3 = tip - base_right;

					// 指向内部的法线（对于逆时针绕向：尖端 -> 左基点 -> 右基点）
					Vector2 n1 = Vector2(-e1.y, e1.x).normalized();
					Vector2 n2 = Vector2(-e2.y, e2.x).normalized();
					Vector2 n3 = Vector2(-e3.y, e3.x).normalized();

					// 将每条边向内偏移 bw，然后求交
					auto intersect_lines = [](const Point2& p1, const Vector2& d1, const Point2& p2, const Vector2& d2) -> Point2 {
						real_t det = d1.x * d2.y - d1.y * d2.x;
						if (Math::is_zero_approx(det)) {
							return p1;
						}
						real_t t = ((p2.x - p1.x) * d2.y - (p2.y - p1.y) * d2.x) / det;
						return p1 + d1 * t;
					};

					Point2 edge1_p1 = tip + n1 * bw;
					Point2 edge2_p1 = base_left + n2 * bw;
					Point2 edge3_p1 = base_right + n3 * bw;

					inner_tip = intersect_lines(edge1_p1, e1, edge3_p1, e3);
					inner_base_left = intersect_lines(edge1_p1, e1, edge2_p1, e2);
					inner_base_right = intersect_lines(edge2_p1, e2, edge3_p1, e3);

					// 将边框绘制为连接外三角形边和内三角形边的 3 个四边形
					add_quad(verts, indices, colors, tip, inner_tip, inner_base_left, base_left, border_color);
					add_quad(verts, indices, colors, base_left, inner_base_left, inner_base_right, base_right, border_color);
					add_quad(verts, indices, colors, base_right, inner_base_right, inner_tip, tip, border_color);
				}
			}
		}

		// 计算 UV 坐标
		Rect2 uv_rect = style_rect.grow(aa_on ? aa_size_scaled : 0);
		uvs.resize(verts.size());
		Point2* uvs_ptr = uvs.ptrw();
		for (int i = 0; i < verts.size(); i++) {
			uvs_ptr[i].x = (verts[i].x - uv_rect.position.x) / uv_rect.size.width;
			uvs_ptr[i].y = (verts[i].y - uv_rect.position.y) / uv_rect.size.height;
		}

		// 绘制样式框
		RenderingServer* vs = RenderingServer::get_singleton();
		vs->canvas_item_add_triangle_array(p_to_canvas_item, indices, verts, colors, uvs);
	}

	inline void draw_rounded_rectangle(PackedVector2Array& verts, PackedInt32Array& indices, PackedColorArray& colors, const Rect2& style_rect, const real_t corner_radius[4],
		const Rect2& ring_rect, const Rect2& inner_rect, const Color& inner_color, const Color& outer_color, const int corner_detail, const Vector2& skew, bool is_filled) {
		int64_t vert_offset = verts.size();
		int adapted_corner_detail = (corner_radius[0] > 0) || (corner_radius[1] > 0) || (corner_radius[2] > 0) || (corner_radius[3] > 0) ? corner_detail : 1;

		bool draw_border = !is_filled;

		real_t ring_corner_radius[4];
		set_inner_corner_radius(style_rect, ring_rect, corner_radius, ring_corner_radius);

		Point2 ring_scale[4];
		set_corner_scale(style_rect, ring_rect, ring_corner_radius, ring_scale);

		// 圆角半径中心点
		Vector<Point2> outer_points = {
			ring_rect.position + Vector2(ring_corner_radius[0], ring_corner_radius[0]) * ring_scale[0], //tl
			Point2(ring_rect.position.x + ring_rect.size.x - ring_corner_radius[1] * ring_scale[1].x, ring_rect.position.y + ring_corner_radius[1] * ring_scale[1].y), //tr
			ring_rect.position + ring_rect.size - Vector2(ring_corner_radius[2], ring_corner_radius[2]) * ring_scale[2], //br
			Point2(ring_rect.position.x + ring_corner_radius[3] * ring_scale[3].x, ring_rect.position.y + ring_rect.size.y - ring_corner_radius[3] * ring_scale[3].y) //bl
		};

		real_t inner_corner_radius[4];
		set_inner_corner_radius(style_rect, inner_rect, corner_radius, inner_corner_radius);

		Point2 inner_scale[4];
		set_corner_scale(style_rect, inner_rect, inner_corner_radius, inner_scale);

		Vector<Point2> inner_points = {
			inner_rect.position + Vector2(inner_corner_radius[0], inner_corner_radius[0]) * inner_scale[0], //tl
			Point2(inner_rect.position.x + inner_rect.size.x - inner_corner_radius[1] * inner_scale[1].x, inner_rect.position.y + inner_corner_radius[1] * inner_scale[1].y), //tr
			inner_rect.position + inner_rect.size - Vector2(inner_corner_radius[2], inner_corner_radius[2]) * inner_scale[2], //br
			Point2(inner_rect.position.x + inner_corner_radius[3] * inner_scale[3].x, inner_rect.position.y + inner_rect.size.y - inner_corner_radius[3] * inner_scale[3].y) //bl
		};

		// 计算顶点

		// 如果中心被填充，我们不绘制边框，直接以内环作为参考，因为对此方法的所有调用
		// 要么绘制圆环，要么绘制填充的圆角矩形，但不会同时绘制两者
		const real_t quarter_arc_rad = Math_PI / 2.0;
		const Point2 style_rect_center = style_rect.get_center();

		const int64_t colors_size = colors.size();
		const int64_t verts_size = verts.size();
		const int new_verts_amount = (adapted_corner_detail + 1) * (draw_border ? 8 : 4);

		colors.resize(colors_size + new_verts_amount);
		verts.resize(verts_size + new_verts_amount);
		Color* colors_ptr = colors.ptrw();
		Vector2* verts_ptr = verts.ptrw();

		for (int corner_idx = 0; corner_idx < 4; corner_idx++) {
			for (int detail = 0; detail <= adapted_corner_detail; detail++) {
				int idx_ofs = (adapted_corner_detail + 1) * corner_idx + detail;
				if (draw_border) {
					idx_ofs *= 2;
				}

				const real_t pt_angle = (corner_idx + detail / (double)adapted_corner_detail) * quarter_arc_rad + Math_PI;
				const real_t angle_cosine = Math::cos(pt_angle);
				const real_t angle_sine = Math::sin(pt_angle);

				{
					const real_t x = inner_corner_radius[corner_idx] * angle_cosine * inner_scale[corner_idx].x + inner_points[corner_idx].x;
					const real_t y = inner_corner_radius[corner_idx] * angle_sine * inner_scale[corner_idx].y + inner_points[corner_idx].y;
					const float x_skew = -skew.x * (y - style_rect_center.y);
					const float y_skew = -skew.y * (x - style_rect_center.x);
					verts_ptr[verts_size + idx_ofs] = Vector2(x + x_skew, y + y_skew);
					colors_ptr[colors_size + idx_ofs] = inner_color;
				}

				if (draw_border) {
					const real_t x = ring_corner_radius[corner_idx] * angle_cosine * ring_scale[corner_idx].x + outer_points[corner_idx].x;
					const real_t y = ring_corner_radius[corner_idx] * angle_sine * ring_scale[corner_idx].y + outer_points[corner_idx].y;
					const float x_skew = -skew.x * (y - style_rect_center.y);
					const float y_skew = -skew.y * (x - style_rect_center.x);
					verts_ptr[verts_size + idx_ofs + 1] = Vector2(x + x_skew, y + y_skew);
					colors_ptr[colors_size + idx_ofs + 1] = outer_color;
				}
			}
		}

		int64_t ring_vert_count = verts.size() - vert_offset;

		// 填充边框的索引和颜色
		if (draw_border) {
			int64_t indices_size = indices.size();
			indices.resize(indices_size + ring_vert_count * 3);
			int* indices_ptr = indices.ptrw();

			for (int64_t i = 0; i < ring_vert_count; i++) {
				int idx_ofs = indices_size + i * 3;
				indices_ptr[idx_ofs] = vert_offset + i % ring_vert_count;
				indices_ptr[idx_ofs + 1] = vert_offset + (i + 2) % ring_vert_count;
				indices_ptr[idx_ofs + 2] = vert_offset + (i + 1) % ring_vert_count;
			}
		}

		if (is_filled) {
			// 计算绘制圆角矩形的三角形模式
			// 由每条包含两个三角形的垂直条纹组成
			int64_t stripes_count = ring_vert_count / 2 - 1;
			int last_vert_id = ring_vert_count - 1;

			int64_t indices_size = indices.size();
			indices.resize(indices_size + stripes_count * 6);
			int* indices_ptr = indices.ptrw();

			for (int64_t i = 0; i < stripes_count; i++) {
				int idx_ofs = indices_size + i * 6;
				// Polygon 1.
				indices_ptr[idx_ofs] = vert_offset + i;
				indices_ptr[idx_ofs + 1] = vert_offset + last_vert_id - i - 1;
				indices_ptr[idx_ofs + 2] = vert_offset + i + 1;
				// Polygon 2.
				indices_ptr[idx_ofs + 3] = vert_offset + i;
				indices_ptr[idx_ofs + 4] = vert_offset + last_vert_id - i;
				indices_ptr[idx_ofs + 5] = vert_offset + last_vert_id - i - 1;
			}
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

	inline void set_corner_scale(const Rect2& style_rect, const Rect2& inner_rect, const real_t corner_radius[4], Point2* inner_scale) {
		real_t border_left = inner_rect.position.x - style_rect.position.x;
		real_t border_top = inner_rect.position.y - style_rect.position.y;
		real_t border_right = style_rect.size.width - inner_rect.size.width - border_left;
		real_t border_bottom = style_rect.size.height - inner_rect.size.height - border_top;

		// 沿边的溢出量
		// 例如，SIDE_LEFT 边是左上角和左下角之间的溢出
		// MIN(0,) 用于忽略下溢，取负是为了使值为正
		real_t edge_overflow[4] = {
			-MIN(0, inner_rect.size.y - corner_radius[CORNER_TOP_LEFT] - corner_radius[CORNER_BOTTOM_LEFT]),
			-MIN(0, inner_rect.size.x - corner_radius[CORNER_TOP_LEFT] - corner_radius[CORNER_TOP_RIGHT]),
			-MIN(0, inner_rect.size.y - corner_radius[CORNER_TOP_RIGHT] - corner_radius[CORNER_BOTTOM_RIGHT]),
			-MIN(0, inner_rect.size.x - corner_radius[CORNER_BOTTOM_LEFT] - corner_radius[CORNER_BOTTOM_RIGHT])
		};

		// 边框之和
		real_t hb_sum = border_left + border_right;
		real_t vb_sum = border_top + border_bottom;

		// 每一边与其对边之和的比率
		// 由于溢出只发生在相对的边框之间，你只需要获取每个边框相对于相关边框之和的比率
		real_t ratios[4] = {
			// 防止除以 0 错误
			hb_sum > 0 ? (border_left / hb_sum) : 0,
			vb_sum > 0 ? (border_top / vb_sum) : 0,
			hb_sum > 0 ? (border_right / hb_sum) : 0,
			vb_sum > 0 ? (border_bottom / vb_sum) : 0
		};

		// 每个角应缩小的原始量
		Point2 corner_reduction[4] = {
			Point2(edge_overflow[SIDE_TOP] * ratios[SIDE_LEFT], edge_overflow[SIDE_LEFT] * ratios[SIDE_TOP]),
			Point2(edge_overflow[SIDE_TOP] * ratios[SIDE_RIGHT], edge_overflow[SIDE_RIGHT] * ratios[SIDE_TOP]),
			Point2(edge_overflow[SIDE_BOTTOM] * ratios[SIDE_RIGHT], edge_overflow[SIDE_RIGHT] * ratios[SIDE_BOTTOM]),
			Point2(edge_overflow[SIDE_BOTTOM] * ratios[SIDE_LEFT], edge_overflow[SIDE_LEFT] * ratios[SIDE_BOTTOM]),
		};

		// 以 Point2 表示的圆角半径
		Point2 pcr[4] = {
			Point2(corner_radius[0], corner_radius[0]),
			Point2(corner_radius[1], corner_radius[1]),
			Point2(corner_radius[2], corner_radius[2]),
			Point2(corner_radius[3], corner_radius[3]),
		};

		// 如果圆角半径太小，它们不会完全缩小
		// 相邻的角将不得不缩小剩余部分（如果可以的话）
		// Minf(0) 用于忽略非剩余部分，取负是为了使值为正
		Point2 leftovers[4] = {
			-((pcr[0] - corner_reduction[0]).minf(0)),
			-((pcr[1] - corner_reduction[1]).minf(0)),
			-((pcr[2] - corner_reduction[2]).minf(0)),
			-((pcr[3] - corner_reduction[3]).minf(0)),
		};

		// 分配剩余部分后新的缩小半径
		Point2 distributed[4] = {
			((pcr[0] - corner_reduction[0] - leftovers[3] - leftovers[1]).maxf(0)),
			((pcr[1] - corner_reduction[1] - leftovers[0] - leftovers[2]).maxf(0)),
			((pcr[2] - corner_reduction[2] - leftovers[1] - leftovers[3]).maxf(0)),
			((pcr[3] - corner_reduction[3] - leftovers[2] - leftovers[0]).maxf(0)),
		};

		// 曲线应缩放多少以达到缩小后的半径
		for (int i = 0; i < 4; i++) {
			// Unshrinkable 是指即使在分配剩余部分后仍然剩余的量
			// 将其从最终缩放中排除
			Point2 unshrinkable = (leftovers[(i + 1) % 4] + leftovers[(i + 4 - 1) % 4] - distributed[i]).maxf(0);
			inner_scale[i] = distributed[i] / (pcr[i] - unshrinkable).maxf(FLT_EPSILON);
		}
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
