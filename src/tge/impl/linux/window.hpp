#ifndef TANGLE_IMPL_LINUX_WINDOW_HPP
#define TANGLE_IMPL_LINUX_WINDOW_HPP
#ifdef __linux__
#include "tge/api/window.hpp"
#include "tge/impl/linux/tge_linux.hpp"
#include <GL/glx.h>
#include <optional>

namespace tge::impl
{
	class window_x11
	{
	public:
		window_x11(window_info info);
		window_x11(const window_x11& copy) = delete;
		window_x11(window_x11&& move) = delete;
		~window_x11();
		window_x11& operator=(const window_x11& rhs) = delete;
		window_x11& operator=(window_x11&& rhs) = delete;

		// tge::window_api<> begin
		using native = Window;
		native get_native() const;
		bool is_close_requested() const;
		hdk::vec2ui get_dimensions() const;
		void set_dimensions(hdk::vec2ui dimensions);
		std::string get_title() const;
		void set_title(std::string title);
		void update();
		bool make_opengl_context_current();
		const keyboard_state& get_keyboard_state() const;
		const mouse_state& get_mouse_state() const;
		void* get_user_data() const;
		void set_user_data(void* udata);
		// tge::window_api<> end
	private:
		void impl_request_close();
		void impl_init_opengl();

		Window wnd;
		void* userdata = nullptr;
		keyboard_state kb_state = {};
		mouse_state m_state = {};
		GLXContext ctx = nullptr;
		bool close_requested = false;
	};
	static_assert(tge::window_api<window_x11>);
	void* get_opengl_proc_address_linux(const char* name);
}

#endif // __linux__
#endif // TANGLE_IMPL_LINUX_WINDOW_HPP
