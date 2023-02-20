#pragma once
#include <functional>
#include "MError.h"



//DEPRECATED
template<typename _Ret, typename... _Params>
class functional_protector {
	typedef std::function<_Ret(_Params...)> _Func;
protected:
	_Func on_success;
	void* value = nullptr;
public:
	functional_protector(_Func success, void* const val, bool error_on_fail = true) : on_success(success), value(val) {

	}
	void set(void* const x) {
		value = x;
	}
	void* const get() {
		return value;
	}
	_Func create() {
		return [&](_Params... x) -> _Ret {
			std::cout << get();
			if (get() != nullptr) return on_success(x...);
		};
	}
};