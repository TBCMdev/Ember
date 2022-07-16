#pragma once
#include <iostream>
#include <string>
#include "lexertk.hpp"
#include "Base.h"
namespace marine {
#pragma region helpers
	static bool isOp(lexertk::token& t) {
		if (t.value == "+" ||
			t.value == "-" ||
			t.value == "*" ||
			t.value == "/" ||
			t.value == "%" ||
			t.value == "+=" ||
			t.value == "-=" ||
			t.value == "*=" ||
			t.value == "/=" ||
			t.value == "%=") return true;
		else {
			return false;
		}
	}
	static bool isLogicalOp(lexertk::token& t) {
		if (t.value == "==" ||
			t.value == "!=" ||
			t.value == ">=" ||
			t.value == "<=") return true;
		else return false;
	}
	static bool isFloat(lexertk::token& t) {
		bool dec = false;
		for (char c : t.value) {
			if (std::string("0123456789").find(c) == std::string::npos) return false;
			if (!dec && c == '.') dec = true;
			else if (dec) return false;
		}
		if (!dec) return false;
		return true;
	}
	static bool isString(lexertk::token& s) {
		std::cout << "first:" << s.value[0] << "last: " << s.value.back() << std::endl;
		if ((s.value[0] != '"' && s.value[0] != '\'') || (s.value.back() != '"' && s.value.back() != '\'')) return false;
		return true;
	}
	static bool isInt(lexertk::token& t) {
		for (char c : t.value) {
			if (std::string("0123456789").find(c) == std::string::npos) return false;
			if (c == '.') return false;
		}
		return true;
	}
#pragma endregion
}