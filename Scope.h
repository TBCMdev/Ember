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

		std::vector<std::vector<std::shared_ptr<ValueHolder>>> var_stack;
		std::vector<std::vector<std::shared_ptr<Function>>> func_stack;

		std::shared_ptr<marine::Module> current_module;

		//INDICATES THE DEFAULT SCOPE. ALL MODULES STORED ARE CUSTOM.
		std::vector<std::shared_ptr<ValueHolder>> core_variables;
		std::vector<std::shared_ptr<Function>> core_functions;
	public:
		Scope(std::vector<Variable>& v, std::vector<Function>& f)
		{
			for (auto& x : v) {
				core_variables.push_back(std::make_shared<Variable>(x));
			}
			for (auto& x : f) {
				core_functions.push_back(std::make_shared<Function>(x));
			}
		}
		Scope(Class* c) {
			for (auto& x : c->getStructure()->members) {
				if (x->var) {
					core_variables.push_back(((ClassStructure::ClassVariable*)x.get())->defaultedValue);
				}
				else {
					core_functions.push_back(((ClassStructure::ClassFunction*)x.get())->_this);
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
			core_functions.insert(core_functions.end(), funcs.begin(), funcs.end());
			core_variables.insert(core_variables.end(), vars.begin(), vars.end());

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
				if (x->var) {
					core_variables.push_back(((ClassStructure::ClassVariable*)x.get())->defaultedValue);
				}
				else {
					core_functions.push_back(((ClassStructure::ClassFunction*)x.get())->_this);
				}
			}
		}
		void addFunctionParameters(std::vector<Variable>* x) {
			if (current_module == nullptr) {
				for (auto& y : *x) {
					core_variables.push_back(std::make_shared<Variable>(y));
				}
			}
			else {
				auto& vars = current_module->getVariables();
				for (auto& y : *x) {
					vars.push_back(std::make_shared<Variable>(y));
				}
			}
		}
		std::vector<std::shared_ptr<ValueHolder>>& getVariables() {
			return (current_module == nullptr ? core_variables : current_module->getVariables());
		}
		std::vector<std::shared_ptr<Function>>& getFunctions() {
			return (current_module == nullptr ? core_functions : current_module->getFunctions());
		}
		void addFunction(Function& f) {
			if (current_module == nullptr) 
			{
				core_functions.push_back(std::make_shared<Function>(f));
			}
			else
			{
				current_module->getFunctions().push_back(std::make_shared<Function>(f));
			}
		}
		void addVariable(Variable& f) {
			if (current_module == nullptr) {
				//we are in global default scope
				core_variables.push_back(std::make_shared<Variable>(f));
			}
			else 
			{
				current_module->getVariables().push_back(std::make_shared<Variable>(f));
			}
		}

	};
}