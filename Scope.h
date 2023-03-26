#pragma once
#include <vector>
#include <unordered_map>
#include <optional>
#include <functional>
#include "Variable.h"
#include "inb.h"
#include "Class.h"
#include "Module.h"

namespace marine {
	template<typename Key, typename Val>
	using SmartMap = std::unordered_map<Key, std::shared_ptr<Val>>;

	class Scope {
		
		
		
		//std::vector<std::shared_ptr<marine::Module>> modules;
		SmartMap<std::string, Module> modules;

		std::vector<SmartMap<std::string, ValueHolder>> var_stack;
		std::vector<SmartMap<std::string, Function>> func_stack;

		std::shared_ptr<marine::Module> current_module;

		//INDICATES THE DEFAULT SCOPE. ALL MODULES STORED ARE CUSTOM.

		SmartMap<std::string, ValueHolder> core_variables;
		SmartMap<std::string, Function> core_functions;
	public:
		Scope(Class* c) {
			for (auto& x : c->getStructure()->members) {
				if (x.second->var) {
					auto* y = (ClassStructure::ClassVariable*)x.second.get();
					core_variables.insert({ x.first, y->defaultedValue });
				}
				else {
					auto y = (ClassStructure::ClassFunction*)x.second.get();
					core_functions.insert({ x.first, y->_this });
				}
			}
		}
		Scope(){}
		bool moduleExists(std::vector<std::string*> x) {
			for (auto& y : x) {

			}
		}
		auto& getCurrentModule() {
			return current_module;
		}
		void setCurrentModule(std::shared_ptr<Module> m) {
			current_module = m;
		}
		void setCurrentModule(Module* m) {
			current_module.reset(m);
		}
		// Adds on a modules contents.
		void mergeModule(Module* m) {
			auto& funcs = m->getFunctions();
			auto& vars = m->getVariables();
			core_functions.insert(funcs.begin(), funcs.end());
			core_variables.insert(vars.begin(), vars.end());

			// also add the child modules.
			modules.merge(m->getChildren());

		}
		template<typename Ret>
		Ret enterModuleTemporarily(Module* toEnter, std::function<Ret()> f) {
			if (current_module != nullptr) {
				// TODO: POSSIBLE ERROR:
				// FIX:
				// current_module = std::make_shared<Module>(*ToEnter);
				Module m = *current_module.get();

				current_module = std::make_shared<Module>(*toEnter);
				Ret x = f();
				current_module.reset(&m);
				return x;
			}
			else {
				current_module = std::make_shared<Module>(*toEnter);
				Ret x = f();
				current_module.reset();
				return x;
			}
		}
		SmartMap<std::string, Module>& getModules() { return modules; }
		Module* findModule(std::string& t) {
			auto x = modules.find(t);

			return (x == modules.end() ? nullptr : x->second.get());
		}
		void popStack() {
			core_variables = var_stack.back();
			core_functions = func_stack.back();
			var_stack.pop_back();
			func_stack.pop_back();
		}
		void newModule(std::shared_ptr<Module>& m) { modules.insert({ m->getName(), m}); }
		void setLists(ClassInstance* c) {

			var_stack.push_back(core_variables);
			func_stack.push_back(core_functions);


			core_variables.clear();
			core_functions.clear();
			for (auto& x : *c->getMembers()) {
				if (x.second->var) {
					auto y = (ClassStructure::ClassVariable*)x.second.get();
					core_variables.insert({ x.first, y->defaultedValue });
				}
				else {
					auto y = (ClassStructure::ClassFunction*)x.second.get();
					core_functions.insert({ x.first, y->_this });
				}
			}
		}
		void addFunctionParameters(std::unordered_map<std::string, Variable>& x) {
			if (current_module == nullptr) {
				for (auto it = x.begin(); it != x.end();) {
					core_variables.insert({ it->first, std::make_shared<Variable>(it->second)});
					it++;
				}
			}
			else {
				auto& vars = current_module->getVariables();
				for (auto it = x.begin(); it != x.end();) {
					vars.insert({ it->first, std::make_shared<Variable>(it->second) });
					it++;
				}
			}
		}
		SmartMap<std::string, ValueHolder>& getVariables() {
			return (current_module == nullptr ? core_variables : current_module->getVariables());
		}
		SmartMap<std::string, Function>& getFunctions() {
			return (current_module == nullptr ? core_functions : current_module->getFunctions());
		}
		void addFunction(Function& f) {
			if (current_module == nullptr) 
			{
				core_functions.insert({ f.getName(), std::make_shared<Function>(f) });
			}
			else
			{
				current_module->getFunctions().insert({ f.getName(), std::make_shared<Function>(f) });
			}
		}
		void addVariable(Variable& f, const std::string& name) {
			if (current_module == nullptr) {
				//we are in global default scope
				core_variables.insert({ name, std::make_shared<Variable>(f) });
			}
			else 
			{
				current_module->getVariables().insert({ name, std::make_shared<Variable>(f) });
			}
		}

	};
}