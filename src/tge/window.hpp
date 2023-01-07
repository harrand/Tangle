#ifndef TANGLE_WINDOW_HPP
#define TANGLE_WINDOW_HPP
#include "tge/impl/windows/window.hpp"

namespace tge
{
	#ifdef _WIN32
		using window = impl::window_winapi;
		inline void* get_opengl_proc_address(const char* name)
		{
			return impl::get_opengl_proc_address_windows(name);
		}
	#else
		static_assert(false, "Window Implementation does not seem to exist for this platform.");
	#endif
}

#endif // TANGLE_WINDOW_HPP
