#pragma once
#include <iostream>
#include <string>
#include <any>
#include <vector>
#include <sstream>
#include <functional>
#include <map>
#include <variant>
#include <tuple>
#include "String.h"
#define VCONTAINERFUNC marine::VContainer(*)(std::vector<std::any>)

using std::string;


namespace marine {

	std::map<const char*, std::vector<const char*>> INBTREE = {
		{"inb", {
			"console",
			"network",
			"files",
			"game",
	
			}

		}
	};
	namespace inb {
#pragma region function_converters
#pragma endregion
#pragma region internal_backend
		using Action = void* (*)(std::vector<std::any>);

		using Function = marine::VContainer (*)(std::vector<std::any>);


		struct IFunc {
			const char* name;
			int _param_c = 0;
			Base::RetDecl returnType;
			std::vector<Base::Decl> paramDeclTypes;
			std::vector<std::any> parameters = {};
		};
		struct Callable {
		public:
			const char* name;
			int parameter_count;
			std::vector <Base::Decl> paramTypes;

			virtual void call(std::vector<std::any> a, marine::VContainer* v = nullptr) {
				throw marine::errors::RuntimeError(" a callable cannot contain no callable item.");
			}

		};
		struct inb_action: public Callable {
			Action callable;


			void call(std::vector<std::any> a, marine::VContainer* ignored = nullptr) override {
				callable(a);
			}
		public:
			inb_action(const char* n, int param_c, std::vector<Base::Decl> param_types, Action c) : callable(c) {
				name = n;
				parameter_count = param_c;
				paramTypes = param_types;
			}
		};
		struct inb_function: public Callable {
			Function callable;

			void call(std::vector<std::any> a, marine::VContainer* v) override {
				
				*v = callable(a);
			}
		public:
			inb_function(const char* n, int param_c, std::vector<Base::Decl> param_types, Function c) : callable(c) {
				name = n;
				parameter_count = param_c;
				paramTypes = param_types;
			}

		};
#pragma endregion
		namespace console {
			void log_s(std::vector<std::any> x) {
				std::cout << std::any_cast<String>(x[0]).get() << std::endl;
			}
			void log_i(std::vector<std::any> f) {
				std::cout << std::any_cast<int>(f[0]) << std::endl;

			}
			marine::VContainer ask(std::vector<std::any> a) {
				auto [str] = cast<String>(a);

				std::cout << str.get();

				std::string s;

				std::cin >> s;

				VContainer ret(s, -1, Base::Decl::STRING);
				ret.setPlaceholder(s);

				return ret;

			}
		};
		std::vector <inb_action>__NO_INCLUDE_ACTIONS = {
			{"log", 1, {Base::Decl::STRING},(void*(*)(std::vector<std::any>))inb::console::log_s},
			{"log", 1, {Base::Decl::INT},(void* (*)(std::vector<std::any>))inb::console::log_i},

		};
		std::vector<inb_function> __NO_INCLUDE_FUNCTIONS = {
			{"ask", 1, {Base::Decl::STRING}, (VCONTAINERFUNC)inb::console::ask},
			//temporary below:
		};



		void call_no_include(IFunc _definer) {
			for (auto& v : __NO_INCLUDE_ACTIONS) {
				if (v.name == _definer.name && v.parameter_count == _definer._param_c && v.paramTypes == _definer.paramDeclTypes) {
					//std::cout << "[debug inb] CALLING: " << v.name << ", with param count of:" << v.parameter_count << std::endl;
					v.callable(_definer.parameters);
					return;
				}
			}
			std::cout << "could not find function to call." << std::endl;
		}
		inb_action& getNoIncludeActionByName(std::string& name, std::vector<Base::Decl> paramTypes) {
			for (auto& v : __NO_INCLUDE_ACTIONS) {
				if (v.name == name && v.paramTypes == paramTypes) return v;
			}
			inb_action null{ "NULL", 0,{},  nullptr };
			return null;
		}
		inb_function& getNoIncludeFunctionByName(std::string& name, std::vector<Base::Decl> paramTypes) {
			for (auto& v : __NO_INCLUDE_FUNCTIONS) {
				if (v.name == name && v.paramTypes == paramTypes) return v;
			}
			inb_function null{ "NULL", 0,{},  nullptr };
			return null;
		}
		bool matchINBLibraryName(std::string& s) {
			for (auto const& [key, val] : INBTREE) {
				if (s == key) return true;
				for (auto& x : val) {
					if (s == x) return true;
				}
			}
			return false;
		}
	}

	
};