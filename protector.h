#pragma once
#include <functional>
#include "MError.h"
template<typename _Ret, typename... _Params>
class functional_protector {
protected:
	std::function<_Ret(_Params...)> on_success;
	void* value;
public:
	functional_protector(std::function<_Ret(_Params...)> success, bool error_on_fail = true) : on_success(success) {

	}
	std::function<_Ret(_Params...)> get() {
		return [=](Params... x) -> _Ret {

		}
	}
};