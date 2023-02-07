#pragma once
#include "../VContainer.h"
#include "../String.h"
#include "../helpers.h"
#include <windows.h>


namespace marine {

	// using Action = void* (*)(std::vector<std::any>, std::vector<Base::Decl>*);


#pragma region _VISUALS_WINDOW
	VContainer createWindow(std::vector<std::any> a, std::vector<Base::Decl>*) {

		auto [s] = cast<String>(a);

		const wchar_t CLASS_NAME[] = L"Sample Window Class";
		const wchar_t* name = (wchar_t*)s.get().c_str();
		WNDCLASS wc = {};

		// wc.lpfnWndProc = WindowProc;
		wc.lpszClassName = CLASS_NAME;

		RegisterClass(&wc);

		HWND windowHandle = CreateWindowEx(0, CLASS_NAME, name, WS_OVERLAPPEDWINDOW,            // Window style

		// Size and position
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

			NULL,       // Parent window    
			NULL,       // Menu
			NULL,
			NULL
		);


		if (windowHandle == NULL) return VContainer(-1,-1,Base::Decl::INT);

		ShowWindow(windowHandle, 0);

		MSG m = {};

		while (GetMessage(&m, NULL, 0, 0) > 0)
		{
			TranslateMessage(&m);
			DispatchMessage(&m);
		}

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
