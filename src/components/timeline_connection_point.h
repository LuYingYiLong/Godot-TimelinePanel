#ifndef TIMELINE_CONNECTION_POINT_H
#define TIMELINE_CONNECTION_POINT_H

#include <cstdint>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/variant/vector2.hpp>

namespace godot {
	class TimelineConnection;

	class TimelineConnectionPoint : public Object {
		GDCLASS(TimelineConnectionPoint, Object)

	private:
		uint64_t owner_id = 0;
		int64_t point_id = -1;
		Vector2 position = Vector2();
		Vector2 in_handle = Vector2(-64.0f, 0.0f);
		Vector2 out_handle = Vector2(64.0f, 0.0f);
		bool selected = false;
		Variant meta;

		TimelineConnection* _get_owner() const;
		void _bind_owner(TimelineConnection* p_owner, int64_t p_point_id);
		void _unbind_owner();
		void _set_local_position(const Vector2 &p_position);
		void _set_local_in_handle(const Vector2 &p_handle);
		void _set_local_out_handle(const Vector2 &p_handle);
		void _set_local_selected(bool p_selected);
		void _set_local_metadata(const Variant &p_meta);

	protected:
		static void _bind_methods();

	public:
		friend class TimelineConnection;

		void set_position(const Vector2 &p_position);
		Vector2 get_position() const;

		void set_in_handle(const Vector2 &p_handle);
		Vector2 get_in_handle() const;

		void set_out_handle(const Vector2 &p_handle);
		Vector2 get_out_handle() const;

		void set_selected(bool p_selected);
		void set_selected_no_signal(bool p_selected);
		bool is_selected() const;

		void set_metadata(const Variant &p_meta);
		Variant get_metadata() const;

		int64_t get_point_id() const;
		bool is_bound() const;
	};
}

#endif // TIMELINE_CONNECTION_POINT_H
