#pragma once
#include <vector>
#include "Variable.h"
#include "inb.h"
#include "Class.h"

namespace marine {
	class Scope {
		std::vector<std::vector<std::shared_ptr<ValueHolder>>> var_stack;
		std::vector<std::vector<std::shared_ptr<Function>>> func_stack;

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
		void popStack() {
			core_variables = var_stack.back();
			core_functions = func_stack.back();
			var_stack.pop_back();
			func_stack.pop_back();
		}

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
			for (auto& y : *x) {
				core_variables.push_back(std::make_shared<Variable>(y));
			}
		}
		std::vector<std::shared_ptr<ValueHolder>>& getVariables() {
			return core_variables;
		}
		std::vector<std::shared_ptr<Function>>& getFunctions() {
			return core_functions;
		}
		void addFunction(Function& f) {
			core_functions.push_back(std::make_shared<Function>(f));
		}
		void addVariable(Variable& f) {

			core_variables.push_back(std::make_shared<Variable>(f));
		}

	};
}