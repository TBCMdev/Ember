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
#include "Module.h"
#include "Injector.h"

#include "validated_map.h"


#define VCONTAINERFUNC marine::VContainer(*)(std::vector<std::any>, std::vector<Base::Decl>*)

using std::string;


namespace marine {
	namespace inb {
		using Action = std::function<void(std::vector<std::any>, std::vector<Base::Decl>*)>;

		using Function = std::function<marine::VContainer(std::vector<std::any>, std::vector<Base::Decl>*)>;

		

		struct Callable {
		public:
			int parameter_count;
			std::vector <Base::Decl> paramTypes;

			virtual void call(std::vector<std::any> a, marine::VContainer* v = nullptr, std::vector<Base::Decl>* help = nullptr) {
				throw marine::errors::RuntimeError("a callable cannot contain no callable item.");
			}

		};
		struct mod_action : public Callable {
			Action callable;
			void call(std::vector<std::any> a, marine::VContainer* ignored = nullptr, std::vector<Base::Decl>* help = nullptr)
			{
				callable(a, help);
			}
		};
		struct inb_action: public Callable {
			Action callable;


			void call(std::vector<std::any> a, marine::VContainer* ignored = nullptr, std::vector<Base::Decl>* help = nullptr) override {
				callable(a, help);
			}
		public:
			inb_action(int param_c, std::vector<Base::Decl> param_types, Action c) : callable(c) {
				parameter_count = param_c;
				paramTypes = param_types;
			}
		};
		struct inb_function: public Callable {
			Function callable;

			void call(std::vector<std::any> a, marine::VContainer* v, std::vector<Base::Decl>* help = nullptr) override {
				*v = callable(a, help);
			}
		public:
			inb_function(int param_c, std::vector<Base::Decl> param_types, Function c) : callable(c) {
				parameter_count = param_c;
				paramTypes = param_types;
			}

		};
		namespace console {
			VContainer log_any(std::vector<std::any> l, std::vector<Base::Decl>* info) {
				std::stringstream s;
				int c = 0;
				int len = l.size();
				for (const auto& x : *info) {
					switch (x) {
					case Base::Decl::INT:
						s << std::any_cast<int>(l[c]);
						break;
					case Base::Decl::STRING:
						s << std::any_cast<String>(l[c]).get();
						break;
					case Base::Decl::FLOAT:
						s << std::any_cast<float>(l[c]);
						break;
					case Base::Decl::BOOL:
						s << (std::any_cast<bool>(l[c]) == true ? "true" : "false");
						break;
					case Base::Decl::LIST:
						s << std::any_cast<ArrayList>(l[c]).str();
						break;
					default:
						throw marine::errors::SyntaxError("unknown type.");
					}

					if(c < len) s << " ";
					c++;
				}
				std::cout << s.str() << "\n";
				return VContainer::null();
			}
			marine::VContainer ask(std::vector<std::any> a, std::vector<Base::Decl>* info) {
				auto [str] = cast<String>(a);

				std::cout << str.get();

				std::string s;

				std::cin >> s;

				VContainer ret(String(s), -1, Base::Decl::STRING);
				ret.setPlaceholder(s);
				return ret;

			}
		};
		namespace conversion {
			VContainer str_to_int(std::vector<std::any> a, std::vector<Base::Decl>* info) {
				auto [x] = cast<String>(a);
				try {
					return VContainer(stoi(x.get()), -1, Base::Decl::INT);
				}
				catch(...) {
					throw marine::errors::TypeError("could not convert string to int.");
				}

			}
		};
		namespace util {
			namespace math {
				VContainer pow(std::vector<std::any> a, std::vector<Base::Decl>* info) {
					auto [x, p] = cast<int, int>(a);
					return VContainer(((int)std::pow(x, p)), -1, Base::Decl::INT);
				}
			}
			namespace lists {
				VContainer range(std::vector<std::any> a, std::vector<Base::Decl>* info) {
					signed int size = info->size();
					if (size == 0) throw marine::errors::IndexError("invalid parameters passed to range function.");
					switch ((*info)[0]) {
					case Base::Decl::INT: {
						switch (size) {
						case 1:
						{
							ArrayList list;
							int R = std::any_cast<int>(a[0]);
							for (int L = 0; L <= R; L++)
							{
								list.add<int>(L, -1, Base::Decl::INT);
							}


							return VContainer(list, -1, Base::Decl::LIST);
						}
						case 2:
						{
							ArrayList list;
							auto [L, R] = cast<int, int>(a);
							for (; L <= R; L++)
							{
								list.add<int>(L, -1, Base::Decl::INT);
							}


							return VContainer(list, -1, Base::Decl::LIST);
						}
						case 3: {
							ArrayList list;
							auto [L, R, I] = cast<int, int, int>(a);
							for (; L <= R; L += I)
							{
								list.add<int>(L, -1, Base::Decl::INT);
							}


							return VContainer(list, -1, Base::Decl::LIST);
						}
						default:
							throw marine::errors::SyntaxError("range integer function only takes up to 3 arguments.");

						}
						break;
					}
					default:
						throw marine::errors::TypeError("invalid type of parameter passed to range function.");
					}
				}
				VContainer enumerate(std::vector<std::any> a, std::vector<Base::Decl>* info) {
					//make support for other enumerable types in future...
					if (info->at(0) != Base::Decl::LIST) throw marine::errors::SyntaxError("'enumerate function only allows: 'list', as its parameters at this time.'");
					ArrayList iter;
					auto [x] = cast<ArrayList>(a);

					int X = 0;
					int S = x.length();

					for (; X < S; X++) {
						VContainer* ref = x.getItemRef(X);
						iter.add(ArrayList({ VContainer(X, -1, Base::Decl::INT), VContainer(*ref, -1, ref->getDecl()) }), -1, Base::Decl::LIST);
					}

					return VContainer(iter, -1, Base::Decl::LIST);
				
				}
			}
		}
		validated_map <std::pair<std::string, std::vector<Base::Decl>>, inb_function, true> __NO_INCLUDE_FUNCTIONS{ {
			{
				{"ask", {Base::Decl::STRING}},
					{1, {Base::Decl::STRING}, (VCONTAINERFUNC)inb::console::ask} },
			{
				{"toint", {Base::Decl::STRING}},
					{1, {Base::Decl::STRING}, (VCONTAINERFUNC)inb::conversion::str_to_int}},
			{
				{"range", {}},
					{ -1, {/*-1 with no params so we can decide which overloaded func to use in the function itself.*/},(VCONTAINERFUNC)inb::util::lists::range} },
			{
				{"enumerate", {Base::Decl::LIST}},
					{1, {Base::Decl::LIST}, (VCONTAINERFUNC)inb::util::lists::enumerate }},
			{
				{"power", {Base::Decl::INT}},
					{2, {Base::Decl::INT, Base::Decl::INT}, (VCONTAINERFUNC)inb::util::math::pow}},
			{
				{"log", {Base::Decl::RUNTIME_DECIDED}},
					{-1, {}, (VCONTAINERFUNC)inb::console::log_any}
			}
		} };
		inb_function* getNoIncludeFunctionByName(std::string& name, std::vector<Base::Decl> paramTypes) {
			try {
				return __NO_INCLUDE_FUNCTIONS[{name, paramTypes}];
			}
			catch (std::exception& s) { return nullptr; }
		}
		void inject_inb_std(std::unordered_map<std::string, std::any>* VMapOUT = nullptr) {
			std::unordered_map<std::string, std::tuple<std::vector<Base::Decl>, EMBER_DLL_FUNC>> FMap;
			std::unordered_map<std::string, std::any> VMap;


			if (!_InjectEmberDLL(EMBER_INB_DLL_LOCATION + "\\em_included.dll", &FMap, &VMap)) {
				throw marine::errors::DLLInjectError("main 'included' could not be injected. Fatal crash. Please check the Ember install dir for missing dlls.");
			}
			if (!_InjectEmberDLL(EMBER_INB_DLL_LOCATION + "\\em_inb.dll", &FMap, &VMap)) {
				throw marine::errors::DLLInjectError("INB DLL could not be injected. Fatal crash. Please check the Ember install dir for missing dlls.");
			}
			for (const auto& entry : FMap) {
				const auto& v = std::get<0>(entry.second);
				__NO_INCLUDE_FUNCTIONS.insert({entry.first, v}, { (int)v.size(), v, std::get<1>(entry.second)});
			}
			if (VMapOUT != nullptr) *VMapOUT = VMap;

		}
	}

	
};