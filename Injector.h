#pragma once

#include "MError.h"
#include "VContainer.h"

#include <Windows.h>
#include <any>
#include <iostream>
#include <vector>
#include <unordered_map>



#define EMBER_DLL_IMPL
#define MEMBER_COLLECTION std::unordered_map<std::string, _pair_3<void*, unsigned int, std::vector<Base::Decl>>>
#define FUNCTION_MAP std::unordered_map<std::string, std::tuple<std::vector<Base::Decl>, EMBER_DLL_FUNC>>
#define VAR_MAP std::unordered_map<std::string, std::any> // [TODO] change std::any to Variable() / VContainer()

#define EMBER_ENTRY_NAME "__edll_entry"
#define EMBER_VER_GET_NAME "__edll_ver"
//testing
#define EMBER_VERSION "0.0.1b"
#define EMBER_DLL_INJECT_DEBUG 1

#define EMBER_INB_DLL_LOCATION std::string("C:\\Program Files\\Ember (") + EMBER_VERSION + ")\\include"


#ifndef EMBER_VERSION

#error Ember version needs to be defined to avoid injecting non compatible dlls.

#endif

template< typename _T1, typename _T2, typename _T3>
struct _pair_3 {
	_T1 item1;
	_T2 item2;
	_T3 item3;
};



typedef void(__stdcall* EMBER_DLL_ENTRY)(std::unordered_map<std::string, _pair_3<void*, unsigned int, std::vector<Base::Decl>>>*);
typedef marine::VContainer(__cdecl* EMBER_DLL_FUNC)(std::vector<std::any>, std::vector <Base::Decl> *);
typedef const char* (__stdcall* EMBER_VER_GET)();






EMBER_DLL_IMPL bool _InjectEmberDLL(const std::string& _path, FUNCTION_MAP* _OutF, VAR_MAP* _OutV) {
	std::wstring _pathlp = std::wstring(_path.begin(), _path.end());
	HINSTANCE _IDLL = LoadLibrary(LPCWSTR(_pathlp.c_str()));
	if (!_IDLL) return false;

	EMBER_VER_GET _DLL_VER = (EMBER_VER_GET)GetProcAddress(_IDLL, EMBER_VER_GET_NAME);
	if (_DLL_VER == NULL) throw marine::errors::DLLInjectError("DLL does not specify what version it is.");

	const char* VER = _DLL_VER();
#if EMBER_DLL_INJECT_DEBUG
	std::cout << "Loading dll (ver: " << VER << ") ...\n";
#endif
	if (strcmp(VER, EMBER_VERSION) != 0) throw marine::errors::DLLInjectError("Library imported has an invalid ember version. This can lead to complications when using the library.");


	EMBER_DLL_ENTRY _DLL_ENTRY = (EMBER_DLL_ENTRY)GetProcAddress(_IDLL, EMBER_ENTRY_NAME);

	if (_DLL_ENTRY == NULL) throw "DLL injected does not contain an entry inject function."; // [TODO] wrap in ember::RuntimeError()

	MEMBER_COLLECTION _DLL_MEMBERS;
	_DLL_ENTRY(&_DLL_MEMBERS);

	for (const auto& member : _DLL_MEMBERS) {
		if (!member.second.item2) { // IE 0 is function, 1 is variable.
			_OutF->insert({ member.first, { member.second.item3, (EMBER_DLL_FUNC)member.second.item1 } });
#if EMBER_DLL_INJECT_DEBUG
			std::cout << "Injected Fn: " << member.first << " -> " << member.second.item1 << '\n';
#endif
		}
		else {
			_OutV->insert({ member.first, std::any(member.second.item1) });
		}
	}

	// FreeLibrary(_IDLL); might not work?! we are removing the memory addr of all the functions we stored.
	return true;
}