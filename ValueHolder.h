#pragma once
#include <string>
#include <iostream>
#include <vector>
#include "Base.h"
#include "String.h"
namespace marine {
	class StaticObject;
	class ValueHolder {
	protected:
		unsigned int __depth = 0;
		Base::Decl decl = Base::Decl::UNKNWN;
		std::vector <Base::DeclConfig> configs{};
		std::shared_ptr<StaticObject> connected_obj = nullptr;
		std::shared_ptr<std::string> _name = nullptr;

		bool __hasToken = true;
		std::any _value;
	public:
		ValueHolder(std::any value) : _value(value) {}
		virtual ~ValueHolder() = default;
		std::string configsStr() {
			std::stringstream stream;
			stream << "[";

			for (size_t i = 0; i < configs.size(); ++i) {
				stream << Base::declCStr(configs[i]);
				if (i + 1 < configs.size()) stream << ", ";
			}
			return stream.str() + "]";
		}
		int getDepth() { return __depth; }
		void setDepth(unsigned int depth) {
			__depth = depth; 
		}
		void setDecl(Base::Decl d) {
			decl = d;
		}
		std::string& getName() { return *_name.get(); }
		void setName(std::string& x) { _name = std::make_shared<std::string>(x); }
		Base::Decl getDecl() { return decl; }
		std::any& getValue() { return _value; }

		template <typename T>
		T cast() {
			return std::any_cast<T>(_value);
		}
		template <typename T>
		T& castRef() {
			return std::any_cast<T&>(_value);
		}


		bool isDynamicObj() { return decl == Base::Decl::DYNAMIC_OBJECT; }
		std::shared_ptr<StaticObject> getObjSelf() { return connected_obj; }
		void setObjSelf(StaticObject* o) { connected_obj = std::make_shared<StaticObject>(*o); }
		bool is(Base::DeclConfig d) {
			return std::count(configs.begin(), configs.end(), d) != 0;
		}

		std::any& setValue(std::any x) { _value = x; return x; }
		virtual std::string str() { return "ValueHolder (null)"; };
	};
}