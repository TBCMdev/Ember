#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <any>
#include <unordered_map>
#include "lexertk.hpp"

namespace marine {
	class Module;
	class Module {
	protected:
		//[TODO] POP_STACK, SETTING LISTS FOR CLASS INSTANCE IN SCOPE CLASS
		using V_Var = std::unordered_map<std::string, std::shared_ptr<ValueHolder>>;
		using F_Var = std::unordered_map<std::string, std::shared_ptr<Function>>;
		V_Var var_stack;
		F_Var func_stack;

		std::unordered_map<std::string, std::shared_ptr<Module>> children;
		std::string& name;
	public:
		signed int getChildCount() { return children.size(); }
		Module(std::string& n): name(n){

		}
		bool hasChildModule(std::string& n) {
			return children.find(n) != children.end();
		}
		Module* getChildModule(std::string& name) {
			auto x = children.find(name);
			return (x == children.end()) ? nullptr : x->second.get();
		}
		void addChildModule(std::shared_ptr<Module>& m) {
			children.insert({ m->getName(), m });
		}
		std::string& getName() { return name; }
		
		V_Var& getVariables() {
			return var_stack;
		}
		std::unordered_map<std::string, std::shared_ptr<Module>>& getChildren() {
			return children;
		}
		F_Var& getFunctions() {
			return func_stack;
		}
	};
}