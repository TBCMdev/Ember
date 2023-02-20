#pragma once
#include "../VContainer.h"
#include "../String.h"
#include "../helpers.h"
#include "../protector.h"

#define CENTURION_NO_SDL_TTF
#define CENTURION_NO_SDL_MIXER


#define ASSERT_WINDOW_LOADED if(MAIN_WIND_HWND == nullptr) return;

#include "centurion.hpp"

#include <windows.h>
#include <thread>


namespace marine {

	// using Action = void* (*)(std::vector<std::any>, std::vector<Base::Decl>*);


#pragma region _VISUALS_WINDOW

	class _CE_WINDOW_HWND;


	bool _init_ce_window;
	static _CE_WINDOW_HWND* MAIN_WIND_HWND;

	class _CE_WINDOW_HWND {

	public:
		struct _CE_WINDOW_CFG {
			std::string title = "Ember Window";

		};
		struct _CE_INPUT_HWND {
			enum MOUSEEVENT
			{
				M1,M2,M3DOWN, M3UP, M3CLICK, MOUSE_MOVE, ANNONAMOUS
			};
			enum KEYBOARDEVENT
			{
				KEY, KEYDOWN, KEYUP,
				
			};
			template<typename _T>
			VContainer wrapEvent(_T& m) {
				return VContainer((int)m, -1, Base::Decl::INT);
			}
			std::map<KEYBOARDEVENT, Lambda> e_kb;
			std::map<MOUSEEVENT, Lambda> e_mo;
			
			void mouseMove(float x_coord, float y_coord) {
				auto _end = e_mo.end();
				auto x = e_mo.find(MOUSEEVENT::MOUSE_MOVE);

				if (x != _end)
					Lambda::callFromCPP(&x->second, { VContainer(x_coord, -1, Base::Decl::FLOAT), VContainer(y_coord, -1, Base::Decl::FLOAT) });
			}
			void _evfirem(MOUSEEVENT e) {
				auto _end = e_mo.end();
				auto x = e_mo.find(e);
				if (x != _end)
					Lambda::callFromCPP(&x->second, { /*wrapEvent(e)*/});
			}
			void _evfirek(std::string k_chr, KEYBOARDEVENT k) {
				auto _end = e_kb.end();
				auto x = e_kb.find(k);

				if (x != _end)
					Lambda::callFromCPP(&x->second, {
					VContainer(String(k_chr, false), -1, Base::Decl::STRING), wrapEvent(k)
						});
			}
#pragma region connector
			void _push_mevent(std::vector<std::any> a, std::vector<Base::Decl>* x) {
				const auto [state, button, lambda] = cast<int, int, Lambda>(a);

				// make more user friendly other than static cast [TODO]

				e_mo.insert({ static_cast<MOUSEEVENT>(button), lambda });
			}
			void _push_kevent(std::vector<std::any> a, std::vector<Base::Decl>* x) {

				const auto [state, key, lambda] = cast<int, int, Lambda>(a);

				e_kb.insert({ static_cast<KEYBOARDEVENT>(key), lambda });

			}
#pragma endregion
		};

	protected:
		_CE_WINDOW_CFG config;
		bool __flag_open = true;



	public:
		_CE_INPUT_HWND input;
		_CE_WINDOW_HWND(std::string& title) {
			config.title = title;
		};
		void close() { __flag_open = false; }
		bool wantsopen() { return __flag_open; }
		void _mainloop_(bool* const open_flag = nullptr) {
			const cen::sdl sdl;
			// main loop here.

			cen::window hwnd;
			cen::renderer renderer = hwnd.make_renderer();

			hwnd.show();

			hwnd.set_title(config.title);


			cen::event_handler handler;
			if (open_flag != nullptr)
				*open_flag = true;
			while (__flag_open) {
				while (handler.poll()) {
					if (handler.is<cen::quit_event>()) {
						__flag_open = false;
						break;	
					}

					if (handler.is<cen::mouse_button_event>()) {
						auto x = handler.get<cen::mouse_button_event>();
						if (x.pressed()) {
							auto y = x.button();
							input._evfirem(y == cen::mouse_button::left ? _CE_INPUT_HWND::MOUSEEVENT::M1 : 
							(y == cen::mouse_button::right ? _CE_INPUT_HWND::MOUSEEVENT::M2 : (y == cen::mouse_button::middle ? 
								_CE_INPUT_HWND::MOUSEEVENT::M3DOWN : _CE_INPUT_HWND::MOUSEEVENT::ANNONAMOUS)));
						}
					}
				}
				renderer.clear_with(cen::colors::white);
				renderer.present();
			
			}
		}
	};

	

	void addMouseEvent(std::vector<std::any> x, std::vector<Base::Decl>* h) {
		ASSERT_WINDOW_LOADED;

		
		MAIN_WIND_HWND->input._push_mevent(x, h);
	}

	void addKBEvent(std::vector<std::any> x, std::vector<Base::Decl>* h) {

		ASSERT_WINDOW_LOADED;

		MAIN_WIND_HWND->input._push_kevent(x, h);
	}

	VContainer createWindow(std::vector<std::any> a, std::vector<Base::Decl>* x) {

		if (_init_ce_window) return VContainer(-1, -1, Base::Decl::INT); // we only support one window instance as of now.
		_init_ce_window = true;
		auto [s] = cast<String>(a);

		bool _await = false;
		if (MAIN_WIND_HWND == nullptr) {
			static _CE_WINDOW_HWND HWND_instance(s.get()); // create the instance.
			MAIN_WIND_HWND = &HWND_instance;
		}

		auto _THR_WIN_HWND_MAINLOOP = ([&]() {
			MAIN_WIND_HWND->_mainloop_(&_await);
		});
		// starts the window thread.
		std::thread _thr(_THR_WIN_HWND_MAINLOOP);

		while (!_await) {}

		_thr.detach();

		return VContainer(1, -1, Base::Decl::INT);
	}
	void initWindow(std::vector<std::any> a, std::vector<Base::Decl>* h) {
		auto [x, str] = cast<int, String>(a);

		if (x) {
			a.erase(a.begin());
			createWindow(a, h); 
		}
		else {
			static _CE_WINDOW_HWND HWND_instance(str.get());
			MAIN_WIND_HWND = &HWND_instance;
		}

	}

#pragma endregion
#pragma region _GAME_LIB

	//Game util goes here.

#pragma endregion
#pragma region _GUI_LIB

#pragma endregion
#pragma region _INJECT_
	using injector_f = std::vector<
		std::tuple<
		const char*,
		int,
		std::vector<Base::Decl>,
		std::function<marine::VContainer(std::vector<std::any>, std::vector<Base::Decl>*)>>>;
	injector_f __get_to_be_injected_f()
	{
		return (
			injector_f{
				{"initCenturionExternWindow", 1, {Base::Decl::STRING}, createWindow}
			});
	}
	using injector_a = std::vector<
		std::tuple<
		const char*,
		int,
		std::vector<Base::Decl>,
		std::function<void(std::vector<std::any>, std::vector<Base::Decl>*)>>>;
	injector_a __get_to_be_injected_a()
	{
		return (
			injector_a{
				{"centextMEventAdd", 3, {Base::Decl::INT, Base::Decl::INT, Base::Decl::LAMBDA}, addMouseEvent},
				{"centextKBEventAdd", 3, {Base::Decl::INT, Base::Decl::INT, Base::Decl::LAMBDA}, addKBEvent},
				{"createCenWindowRequirements", 2, {Base::Decl::INT, Base::Decl::STRING}, initWindow}
			
			});
	}
#pragma endregion
};
