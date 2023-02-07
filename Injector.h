#pragma once
#include <functional>
#include "Variable.h"
namespace marine {
	template<typename Return, typename... Param>
	void _injectFunction(std::function<Return(Param...)> func) {

	}
};