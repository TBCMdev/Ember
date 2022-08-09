#pragma once
#include <any>
#include <vector>
#include <iostream>
#include "Base.h"
namespace marine {
	class VContainer {
		std::any _value;
		std::string placeHolder;
		unsigned int depth;
		Base::Decl decl = Base::Decl::UNKNWN;
		std::vector <Base::DeclConfig> configs;
	public:
		static VContainer null() {
			return VContainer(nullptr, -1, Base::Decl::UNKNWN);
		}
		VContainer(std::any a, int dep, Base::Decl decl, std::vector<Base::DeclConfig> d = {}) : _value(a), depth(dep), decl(decl), configs(d) {}
		VContainer() :_value(nullptr), depth(-1), configs({}) {}
		template <typename T>
		T cast() {
			return std::any_cast<T>(_value);
		}
		void setPlaceholder(std::string s) { placeHolder = s; }
		std::string& getStringified() { return placeHolder; }
		std::any& get() { return _value; }
		void set(std::any& a, Base::Decl _new, int dep) { _value = a; decl = _new; depth = dep; }
		Base::Decl type() { return decl; }
	};
};