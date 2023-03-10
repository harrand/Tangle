#ifndef TANGLE_WINDOW_HPP
#define TANGLE_WINDOW_HPP
#include "tge/impl/windows/window.hpp"
#include "tge/impl/linux/window.hpp"
#include "hdk/data/handle.hpp"

namespace tge
{
	namespace detail
	{
		struct window_handle_tag{};
	}
	using window_handle = hdk::handle<detail::window_handle_tag>;

	#ifdef _WIN32
		using window = impl::window_winapi;
		inline void* get_opengl_proc_address(const char* name)
		{
			return impl::get_opengl_proc_address_windows(name);
		}
	#elif defined(__linux__)
		using window = impl::window_x11;
		inline void* get_opengl_proc_address(const char* name)
		{
			return impl::get_opengl_proc_address_linux(name);
		}
	#else
		static_assert(false, "Window Implementation does not seem to exist for this platform.");
	#endif

	window_handle create_window(window_info info = {});
	bool destroy_window(window_handle wh);
	window& get_window(window_handle wh);
	bool has_window(window_handle wh);
	std::size_t window_count();
}

#endif // TANGLE_WINDOW_HPP
