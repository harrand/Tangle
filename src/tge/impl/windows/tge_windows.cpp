#ifdef _WIN32
#include "tge/impl/windows/tge_windows.hpp"
#include "tge/impl/windows/window.hpp"
#include "hdk/debug.hpp"
#include <string_view>

namespace tge::impl
{
	constexpr WNDCLASSEXA wndclass_gpuacc
	{
		.cbSize = sizeof(WNDCLASSEX),
		.style = CS_OWNDC,
		.lpfnWndProc = wndproc,
		.cbClsExtra = 0,
		.cbWndExtra = 0,
		.hInstance = nullptr,
		.hIcon = nullptr,
		.hCursor = nullptr,
		.hbrBackground = nullptr,
		.lpszMenuName = nullptr,
		.lpszClassName = wndclass_name,
		.hIconSm = nullptr
	};

	void load_wgl_functions();

//--------------------------------------------------------------------------------------------------

	void initialise_windows()
	{
		auto window_class = wndclass_gpuacc;
		window_class.hInstance = GetModuleHandle(nullptr);
		RegisterClassExA(&window_class);
		load_wgl_functions();
	}

//--------------------------------------------------------------------------------------------------

	void terminate_windows()
	{
		UnregisterClassA(wndclass_name, GetModuleHandle(nullptr));
	}

//--------------------------------------------------------------------------------------------------

	LRESULT wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		auto get_window = [hwnd]()
		{
			auto wnd = reinterpret_cast<window_winapi*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
			hdk::assert(wnd != nullptr, "window_winapi userdata not setup properly. userdata was nullptr.");
			return wnd;
		};
		switch(msg)
		{
			case WM_CREATE:
			{
				// When created, the window passes a void* for create params, this is guaranteed to be a pointer to the window_winapi. We set it as userdata so we always have access to this pointer.
				auto create = reinterpret_cast<CREATESTRUCT*>(lparam);
				auto wnd = reinterpret_cast<window_winapi*>(create->lpCreateParams);
				SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(wnd));
			}
			break;
			case WM_CLOSE:
				get_window()->impl_request_close();	
				return FALSE;
			break;
			case WM_DESTROY:
				PostQuitMessage(0);
			break;
			case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hwnd, &ps);
				FillRect(hdc, &ps.rcPaint, CreateSolidBrush(RGB(0, 0, 0)));
				EndPaint(hwnd, &ps);
			}
			break;
		}
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}

//--------------------------------------------------------------------------------------------------

	wgl_function_data wgl_data = {};

	wgl_function_data get_wgl_functions()
	{
		hdk::assert(wgl_data != wgl_function_data{}, "Detected WGL functions have not been loaded properly. This should've been done by `tge::impl::initialise_windows()`. Have you forgotten to initialise?");
		return wgl_data;
	}

//--------------------------------------------------------------------------------------------------

	void load_wgl_functions()
	{
		hdk::assert(wgl_data == wgl_function_data{}, "Detected WGL functions have already been loaded, but we've been asked to load them a second time. Logic error?");
		HWND dummy = CreateWindowExA(
			0, wndclass_name,
			"Dummy Window",
			WS_OVERLAPPED,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			nullptr, nullptr, nullptr, nullptr
		);
		hdk::assert(dummy != nullptr, "Failed to create dummy window. Initialisation has gone pear-shaped, or windows is being extremely dodgy.");
		HDC dc = GetDC(dummy);
		hdk::assert(dc != nullptr, "Failed to retrieve device context for dummy window. Either initialisation has gone pear-shaped, or windows is acting extremely dodgy.");
		PIXELFORMATDESCRIPTOR dsc =
		{
			.nSize = sizeof(PIXELFORMATDESCRIPTOR),
			.nVersion = 1,
			.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
			.iPixelType = PFD_TYPE_RGBA,
			.cColorBits = 24
		};
		int format = ChoosePixelFormat(dc, &dsc);
		hdk::assert(format != 0, "Failed to describe OpenGL pixel format for dummy window. We do this whether or not you use OpenGL, but it seems something has gone very wrong.");
		// we have all this pain because you can only SetPixelFormat once on a window.
		if(!SetPixelFormat(dc, format, &dsc))
		{
			hdk::error("Failed to set pixel format for OpenGL dummy window. We do this whether or not you use OpenGL, but it seems something has gone very wrong.");
		}
		HGLRC rc = wglCreateContext(dc);
		hdk::assert(rc != nullptr, "Failed to create ancient OpenGL context for dummy window.");;
		[[maybe_unused]] bool ok = wglMakeCurrent(dc, rc);
		hdk::assert(ok, "Failed to make ancient OpenGL dummy context current.");
		auto wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");
		hdk::assert(wglGetExtensionsStringARB != nullptr, "OpenGL does not support WGL_ARB_extensions_string extension! You are most likely using an absolutely ancient GPU, or initialisation has gone extremely pear-shaped.");
		const char* ext = wglGetExtensionsStringARB(dc);
		hdk::assert(ext != nullptr, "Failed to get OpenGL WGL extension string");
		const char* start = ext;
		for(;;)
		{
			while(*ext != 0 && *ext != ' ')
			{
				ext++;
			}

			std::size_t length = ext - start;
			std::string_view cur{start, length};
			if(cur == "WGL_ARB_pixel_format")
			{
				wgl_data.wgl_choose_pixel_format_arb = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
			}
			if(cur == "WGL_ARB_create_context")
			{
				wgl_data.wgl_create_context_attribs_arb = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
			}
			if(cur == "WGL_EXT_swap_control")
			{
				wgl_data.wgl_swap_interval_ext = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
			}
			
			if(*ext == '\0')
			{
				break;
			}

			ext++;
			start = ext;
		}
		
		hdk::assert(wgl_data.wgl_choose_pixel_format_arb != nullptr, "Failed to load \"wglChoosePixelFormatARB\" function.");
		hdk::assert(wgl_data.wgl_create_context_attribs_arb != nullptr, "Failed to load \"wglCreateContextAttribsARB\" function.");
		hdk::assert(wgl_data.wgl_swap_interval_ext != nullptr, "Failed to load \"wglSwapIntervalEXT\" function.");
		wglMakeCurrent(nullptr, nullptr);
		wglDeleteContext(rc);
		ReleaseDC(dummy, dc);
		DestroyWindow(dummy);
	}
}

#endif // _WIN32
