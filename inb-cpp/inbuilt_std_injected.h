#pragma once
#include "../VContainer.h"
#include "../String.h"
#include "../helpers.h"

#define NK_IMPLEMENTATION
#include "nuklear-master/nuklear.h"

#include <windows.h>
#include <thread>
namespace marine {

	// using Action = void* (*)(std::vector<std::any>, std::vector<Base::Decl>*);


#pragma region _VISUALS_WINDOW


	bool _init_nk_window;
	class _NK_WINDOW_HWND {

	public:
		struct _NK_WINDOW_CFG {
			std::string title;
			signed int FPS;
		};

	protected:
		_NK_WINDOW_CFG config;
		bool __flag_open = true;
		struct nk_context _nk_context;
	public:
		_NK_WINDOW_HWND(std::string& title) {
			config.title = title;

			nk_user_font font{};

			int MAX_MEMORY = 1024 * 1024 * 1000; // 1 gb
			nk_init_fixed(&_nk_context, calloc(1, MAX_MEMORY), MAX_MEMORY, &font);
		};
		void close() { __flag_open = false; }
		bool wantsopen() { return __flag_open; }
		void _mainloop_() {
			// main loop here.
			if (nk_begin(&_nk_context, "Show", nk_rect(50, 50, 220, 220),
				NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_CLOSABLE)) {
				/* fixed widget pixel width */
				nk_layout_row_static(&_nk_context, 30, 80, 1);

				/* fixed widget window ratio width */
				nk_layout_row_dynamic(&_nk_context, 30, 2);
				/* custom widget pixel width */
				nk_layout_row_begin(&_nk_context, NK_STATIC, 30, 2);
				{
				}
				nk_layout_row_end(&_nk_context);
			}
			nk_end(&_nk_context);
		}
	};

	VContainer createWindow(std::vector<std::any> a, std::vector<Base::Decl>*) {

		if (_init_nk_window) return VContainer(-1, -1, Base::Decl::INT); // we only support one window instance as of now.
		_init_nk_window = true;
		auto [s] = cast<String>(a);

		_NK_WINDOW_HWND HWND_instance(s.get()); // create the instance.


		auto _THR_WIN_HWND_MAINLOOP = ([&]() {
			while (HWND_instance.wantsopen()) {
				HWND_instance._mainloop_();
			}
		});
		// starts the window thread.
		std::thread _thr(_THR_WIN_HWND_MAINLOOP);


		return VContainer(1, -1, Base::Decl::INT);
	}

#pragma endregion
#pragma region _INJECT_
	using injector = std::vector<
		std::tuple<
		const char*,
		int,
		std::vector<Base::Decl>,
		marine::VContainer(*)(std::vector<std::any>, std::vector<Base::Decl>*)>>;
	injector __get_to_be_injected_f()
	{
		return (
			injector{
				{"window", 1, {Base::Decl::STRING}, createWindow}
			});
	}
#pragma endregion
};
