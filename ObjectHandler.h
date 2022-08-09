#pragma once
#include <vector>
#include <any>
#include <iostream>
#include "helpers.h"
#include "String.h"
#include "VContainer.h"
#include "Variable.h"
using namespace marine;


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
				"toString", [](std::vector<std::any> a, Variable* self) -> VContainer
				{
					return VContainer(String(std::to_string(self->cast<int>())), -1, Base::Decl::STRING);
				}, {}
			},
			{"digits", [](std::vector<std::any> a, Variable* self) -> VContainer
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
		})
			});//INT
		return handler;
	}
#pragma endregion
	static bool isPrecompiledObject(Variable* v) {
		std::string str = Base::declStr(v->getDecl());
		for (auto& x : instance().precompiledObjects) {
			if (x.getName() == str) return true;
		}
		return false;
	}
	static StaticObject getPrecomiledObject(Variable* v) {
		std::string str = Base::declStr(v->getDecl());
		for (auto& x : instance().precompiledObjects) {
			if (x.getName() == str) return x;
		}
		return StaticObject::null();
	}

};