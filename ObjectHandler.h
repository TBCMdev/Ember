#pragma once
#include <vector>
#include <any>
#include <iostream>
#include <regex>
#include "helpers.h"
#include "String.h"
#include "VContainer.h"
#include "Variable.h"

#define STATIC_OBJ_FUNC [](std::vector<std::any> a, ValueHolder* self) -> VContainer

using namespace marine;



VContainer makeContainer(std::any obj, Base::Decl d) {
	return VContainer(obj, -1, d);
}




static class ObjectHandler;


static class ObjectHandler {
private:
	std::vector<StaticObject> precompiledObjects;
public:
	ObjectHandler(std::vector<StaticObject> o) : precompiledObjects(o) {}
#pragma region OBJECTS
	static ObjectHandler& instance() {
		static ObjectHandler handler({
	StaticObject("int",
		{
			//COMMANDS
		},
		{
			//FUNCTIONS
			{
				"toString",Base::Decl::STRING, STATIC_OBJ_FUNC
				{
					return VContainer(String(std::to_string(self->cast<int>())), -1, Base::Decl::STRING);
				}, {}
			},
			{"digits",Base::Decl::INT,  STATIC_OBJ_FUNC
				{
					int i = 1;
					int c = 0;
					int casted = self->cast<int>();
					for (; i < casted; i *= 10) { c++; }

					std::cout << "num of digits:" << c << std::endl;
					return VContainer(i, -1, Base::Decl::INT);
				}, {}
			}
		},
		{
			//MEMBERS
		}), StaticObject("string", {
			//COMMANDS

		},
		 {
			 //FUNCTIONS
			  {"replace",Base::Decl::STRING, STATIC_OBJ_FUNC{
				 auto [str, rep] = cast<String,String>(a);

				 std::string x = std::regex_replace(self->cast<String>().get(), std::regex(str.get()), rep.get());
				 return makeContainer(String::noTrim(x), Base::Decl::STRING);
			  }, {Base::Decl::STRING, Base::Decl::STRING}},
			  {"length",Base::Decl::INT, STATIC_OBJ_FUNC{
				 return makeContainer(self->cast<String>().get().length(), Base::Decl::INT);
				}, {}},
			  {"toLower",Base::Decl::STRING, STATIC_OBJ_FUNC{
				 std::string copy = self->cast<String>().getCopy();
				 
				 std::transform(copy.begin(), copy.end(),copy.begin(), [](unsigned char c) { return std::tolower(c); });
				 return makeContainer(String::noTrim(copy), Base::Decl::STRING);

				}, {}},
			  {"at",Base::Decl::STRING, STATIC_OBJ_FUNC{
				auto [i] = cast<int>(a);
				auto x = self->cast<String>().get();
				if (i < 0) {
					i = (int)(x.length() + i);
				}
				 
				 return makeContainer(String::noTrim(std::string(1, x[i])), Base::Decl::STRING);

				}, {Base::Decl::INT}},
				{"endsWith",Base::Decl::BOOL, STATIC_OBJ_FUNC{

					bool ret = false;
					auto x = self->cast<String>().get();
					auto [suffix] = cast<String>(a);
					if (x.length() >= suffix.get().length()) {
						ret = (0 == x.compare(x.length() - suffix.get().length(), suffix.get().length(), suffix.get()));
					}

					return makeContainer(ret, Base::Decl::BOOL);

				}, {Base::Decl::STRING}}

			},
			 {
				 //MEMBERS
			})
			});//INT
		return handler;
	}
#pragma endregion
	static bool isPrecompiledObject(ValueHolder* v) {
		std::string str = Base::declStr(v->getDecl());
		for (auto& x : instance().precompiledObjects) {
			if (x.getName() == str) return true;
		}
		return false;
	}
	static bool isPrecompiledObject(Base::Decl v) {
		std::string str = Base::declStr(v);
		for (auto& x : instance().precompiledObjects) {
			if (x.getName() == str) return true;
		}
		return false;
	}
	static StaticObject getPrecomiledObject(ValueHolder* v) {
		std::string str = Base::declStr(v->getDecl());
		for (auto& x : instance().precompiledObjects) {
			if (x.getName() == str) return x;
		}
		return StaticObject::null();
	}
	static StaticObject getPrecomiledObject(Base::Decl b) {
		std::string str = Base::declStr(b);
		for (auto& x : instance().precompiledObjects) {
			if (x.getName() == str) return x;
		}
		return StaticObject::null();
	}
};