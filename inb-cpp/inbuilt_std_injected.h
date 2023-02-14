#pragma once
#include "../VContainer.h"
#include "../String.h"
#include "../helpers.h"
#include "../protector.h"

#define CENTURION_NO_SDL_TTF
#define CENTURION_NO_SDL_MIXER
#include "centurion.hpp"

#include <windows.h>
#include <thread>



namespace marine {

	// using Action = void* (*)(std::vector<std::any>, std::vector<Base::Decl>*);


#pragma region _VISUALS_WINDOW


	bool _init_ce_window;
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
				auto x = std::find(e_mo.begin(), e_mo.end(), MOUSEEVENT::MOUSE_MOVE);

				if (x != _end)
					Lambda::callFromCPP(&x->second, { VContainer(x_coord, -1, Base::Decl::FLOAT), VContainer(y_coord, -1, Base::Decl::FLOAT) });
			}
			void _evfirem(MOUSEEVENT e) {
				auto _end = e_mo.end();
				auto x = std::find(e_mo.begin(), e_mo.end(), e);

				if (x != _end)
					Lambda::callFromCPP(&x->second, { wrapEvent(e) });
			}
			void _evfirek(std::string k_chr, KEYBOARDEVENT k) {
				auto _end = e_kb.end();
				auto x = std::find(e_kb.begin(), e_kb.end(), k);

				if (x != _end)
					Lambda::callFromCPP(&x->second, {
					VContainer(String(k_chr, false), -1, Base::Decl::STRING), wrapEvent(k)
						});
			}
#pragma region connector
			VContainer _push_mevent(std::vector<std::any> a, std::vector<Base::Decl>* x) {

			}
			VContainer _push_kevent(std::vector<std::any> a, std::vector<Base::Decl>* x) {
				return VContainer::null();
			}
#pragma endregion
		};

	protected:
		_CE_WINDOW_CFG config;
		_CE_INPUT_HWND input;
		bool __flag_open = true;



	public:
		_CE_WINDOW_HWND(std::string& title) {
			config.title = title;
		};
		void close() { __flag_open = false; }
		bool wantsopen() { return __flag_open; }
		void _mainloop_() {
			const cen::sdl sdl;
			// main loop here.

			cen::window hwnd;
			cen::renderer renderer = hwnd.make_renderer();


			hwnd.show();

			hwnd.set_title(config.title);

			cen::event_handler handler;
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
	VContainer addMouseEvent(std::vector<std::any> x, std::vector<Base::Decl> h) {

	}
	functional_protector < VContainer(std::vector<std::any>, std::vector<Base::Decl>*) centextMEventAdd;

	VContainer createWindow(std::vector<std::any> a, std::vector<Base::Decl>*) {

		if (_init_ce_window) return VContainer(-1, -1, Base::Decl::INT); // we only support one window instance as of now.
		_init_ce_window = true;
		auto [s] = cast<String>(a);

		static _CE_WINDOW_HWND HWND_instance(s.get()); // create the instance.


		auto _THR_WIN_HWND_MAINLOOP = ([&]() {
			HWND_instance._mainloop_();
		});
		// starts the window thread.
		std::thread _thr(_THR_WIN_HWND_MAINLOOP);

		_thr.detach();

		return VContainer(1, -1, Base::Decl::INT);
	}

#pragma endregion
#pragma region _GAME_LIB

	//Game util goes here.

#pragma endregion
#pragma region _GUI_LIB

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
				{"initCenturionExternWindow", 1, {Base::Decl::STRING}, createWindow}
				
			});
	}
#pragma endregion
};
