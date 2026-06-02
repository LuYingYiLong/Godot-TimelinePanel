#ifndef TIMELINE_CONNECTION_H
#define TIMELINE_CONNECTION_H

#include "timeline_base.h"
#include "timeline_connection_point.h"

#include <cstdint>
#include <vector>
#include <godot_cpp/classes/style_box.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/variant/vector2.hpp>

namespace godot {
	class TimelineConnection : public TimelineBase {
		GDCLASS(TimelineConnection, TimelineBase)

	public:
		enum CurveMode {
			CURVE_LINE,
			CURVE_BEZIER,
	};

	private:
		struct PointData {
			int64_t id = -1;
			Vector2 position = Vector2();
			Vector2 in_handle = Vector2(-64.0f, 0.0f);
			Vector2 out_handle = Vector2(64.0f, 0.0f);
			bool selected = false;
			Variant meta;
			TimelineConnectionPoint* handle = nullptr;
		};

		std::vector<PointData> points;
		int64_t next_point_id = 1;
		CurveMode curve_mode = CURVE_BEZIER;
		Color color = Color(1.0f, 1.0f, 1.0f, 0.65f);
		float width = -1.0f;
		int curve_segments = 24;
		bool edit_enabled = true;
		bool disabled = false;
		Ref<StyleBox> key_normal_style;
		Ref<StyleBox> key_selected_style;
		Ref<StyleBox> handle_normal_style;
		Ref<StyleBox> handle_selected_style;
		float key_scale = -1.0f;
		float handle_scale = -1.0f;
		float handle_line_width = -1.0f;
		Variant meta;

		PointData* _get_point_data_by_id(int64_t p_id);
		const PointData* _get_point_data_by_id(int64_t p_id) const;
		TimelineConnectionPoint* _insert_point_data(int p_index, const Vector2 &p_position, const Vector2 &p_in_handle, const Vector2 &p_out_handle, const Variant &p_meta, bool p_notify);
		void _sync_point_handle(PointData &p_point);
		void _delete_point_handle(PointData &p_point);
		void _delete_all_point_handles();
		bool _set_point_position_by_id(int64_t p_id, const Vector2 &p_position);
		Vector2 _get_point_position_by_id(int64_t p_id, const Vector2 &p_fallback) const;
		bool _set_point_in_handle_by_id(int64_t p_id, const Vector2 &p_handle);
		Vector2 _get_point_in_handle_by_id(int64_t p_id, const Vector2 &p_fallback) const;
		bool _set_point_out_handle_by_id(int64_t p_id, const Vector2 &p_handle);
		Vector2 _get_point_out_handle_by_id(int64_t p_id, const Vector2 &p_fallback) const;
		bool _set_point_selected_by_id(int64_t p_id, bool p_selected, bool p_signal);
		bool _is_point_selected_by_id(int64_t p_id, bool p_fallback) const;
		bool _set_point_metadata_by_id(int64_t p_id, const Variant &p_meta);
		Variant _get_point_metadata_by_id(int64_t p_id, const Variant &p_fallback) const;

	protected:
		static void _bind_methods();

	public:
		friend class TimelineConnectionPoint;

		TimelineConnection();
		~TimelineConnection();

		TimelineConnectionPoint* add_point(const Vector2 &p_position, int p_index = -1);
		void remove_point(TimelineConnectionPoint* p_point);
		void remove_point_at(int p_index);
		void clear_points();
		void set_points(const TypedArray<TimelineConnectionPoint> &p_points);
		TypedArray<TimelineConnectionPoint> get_points() const;
		TimelineConnectionPoint* get_point(int p_index) const;
		int get_point_count() const;
		int get_point_index(const TimelineConnectionPoint* p_point) const;

		void set_point_position(int p_index, const Vector2 &p_position);
		Vector2 get_point_position(int p_index) const;

		void set_point_in_handle(int p_index, const Vector2 &p_handle);
		Vector2 get_point_in_handle(int p_index) const;

		void set_point_out_handle(int p_index, const Vector2 &p_handle);
		Vector2 get_point_out_handle(int p_index) const;

		void set_from_position(const Vector2 &p_position);
		Vector2 get_from_position() const;

		void set_to_position(const Vector2 &p_position);
		Vector2 get_to_position() const;

		void set_from_out_handle(const Vector2 &p_handle);
		Vector2 get_from_out_handle() const;

		void set_to_in_handle(const Vector2 &p_handle);
		Vector2 get_to_in_handle() const;

		void set_curve_mode(CurveMode p_mode);
		CurveMode get_curve_mode() const;

		void set_color(const Color &p_color);
		Color get_color() const;

		void set_width(float p_width);
		float get_width() const;

		void set_curve_segments(int p_segments);
		int get_curve_segments() const;

		void set_key_scale(float p_scale);
		float get_key_scale() const;

		void set_key_normal_style(Ref<StyleBox> p_style);
		Ref<StyleBox> get_key_normal_style() const;

		void set_key_selected_style(Ref<StyleBox> p_style);
		Ref<StyleBox> get_key_selected_style() const;

		void set_handle_scale(float p_scale);
		float get_handle_scale() const;

		void set_handle_normal_style(Ref<StyleBox> p_style);
		Ref<StyleBox> get_handle_normal_style() const;

		void set_handle_selected_style(Ref<StyleBox> p_style);
		Ref<StyleBox> get_handle_selected_style() const;

		void set_handle_style(Ref<StyleBox> p_style);
		Ref<StyleBox> get_handle_style() const;

		void set_handle_line_width(float p_width);
		float get_handle_line_width() const;

		void set_edit_enabled(bool p_enabled);
		bool is_edit_enabled() const;

		void set_disabled(bool p_disabled);
		bool is_disabled() const;

		void set_metadata(const Variant &p_meta);
		Variant get_metadata() const;
	};
}

VARIANT_ENUM_CAST(TimelineConnection::CurveMode);

#endif // TIMELINE_CONNECTION_H
