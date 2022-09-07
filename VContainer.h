#pragma once
#include <any>
#include <vector>
#include <iostream>
#include "Base.h"
#include "ValueHolder.h"
namespace marine {
	class StaticObject;
	class VContainer : public ValueHolder {
	protected:
		std::string placeHolder;
	public:
		static VContainer null() {
			return VContainer(nullptr, -1, Base::Decl::UNKNWN);
		}
		VContainer(std::any a, int dep, Base::Decl decl, std::vector<Base::DeclConfig> d = {}): ValueHolder(a) { configs = d; __depth = dep; this->decl = decl; }
		VContainer() : ValueHolder(nullptr) { __depth = -1; }
		template <typename T>
		T cast() {
			return std::any_cast<T>(_value);
		}
		bool isDynamicObj() { return decl == Base::Decl::DYNAMIC_OBJECT; }
		std::shared_ptr<StaticObject> getObjSelf() { return connected_obj; }
		void setObjSelf(StaticObject* o) { connected_obj = std::make_shared<StaticObject>(*o); }
		void setPlaceholder(std::string s) { placeHolder = s; }
		std::string& getStringified() { return placeHolder; }
		std::any& get() { return _value; }
		void set(std::any& a, Base::Decl _new, int dep) { _value = a; decl = _new; __depth = dep; }
		Base::Decl type() { return decl; }
	};
};