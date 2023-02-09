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

#include "inb-cpp/inbuilt_std_injected.h"

#define VCONTAINERFUNC marine::VContainer(*)(std::vector<std::any>, std::vector<Base::Decl>*)

using std::string;


namespace marine {


	namespace inb {
#pragma region internal_backend
		using Action = void* (*)(std::vector<std::any>, std::vector<Base::Decl>*);

		using Function = marine::VContainer (*)(std::vector<std::any>, std::vector<Base::Decl>*);



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

			virtual void call(std::vector<std::any> a, marine::VContainer* v = nullptr, std::vector<Base::Decl>* help = nullptr) {
				throw marine::errors::RuntimeError(" a callable cannot contain no callable item.");
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
			inb_action(const char* n, int param_c, std::vector<Base::Decl> param_types, Action c) : callable(c) {
				name = n;
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
			inb_function(const char* n, int param_c, std::vector<Base::Decl> param_types, Function c) : callable(c) {
				name = n;
				parameter_count = param_c;
				paramTypes = param_types;
			}

		};
#pragma endregion
		namespace console {
			void log_s(std::vector<std::any> x, std::vector<Base::Decl>* info) {
				std::cout << std::any_cast<String>(x[0]).get() << std::endl;
			}
			void log_i(std::vector<std::any> f, std::vector<Base::Decl>* info) {
				std::cout << std::any_cast<int>(f[0]) << std::endl;

			}
			void log_l(std::vector<std::any> l, std::vector<Base::Decl>* info) {
				auto [list] = cast<ArrayList>(l);

				std::cout << list.str();
			}
			void log_any(std::vector<std::any> l, std::vector<Base::Decl>* info) {
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
						s << (std::any_cast<bool>(l[c]) == 1 ? "true" : "false");
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
		std::vector <inb_action> __NO_INCLUDE_ACTIONS = {
			{"log", 1, {Base::Decl::STRING},(void*(*)(std::vector<std::any>, std::vector<Base::Decl>*))inb::console::log_s},
			{"log", 1, {Base::Decl::INT},(void* (*)(std::vector<std::any>, std::vector<Base::Decl>*))inb::console::log_i},
			{"log", 1, {Base::Decl::LIST},(void* (*)(std::vector<std::any>, std::vector<Base::Decl>*))inb::console::log_l},
			{"log", -1, {},(void* (*)(std::vector<std::any>, std::vector<Base::Decl>*))inb::console::log_any},

		};
		std::vector<inb_function> __NO_INCLUDE_FUNCTIONS = {
			{"ask", 1, {Base::Decl::STRING}, (VCONTAINERFUNC)inb::console::ask},
			{"toint", 1, {Base::Decl::STRING}, (VCONTAINERFUNC)inb::conversion::str_to_int},
			{"range", -1, {/*-1 with no params so we can decide which overloaded func to use in the function itself.*/},(VCONTAINERFUNC)inb::util::lists::range},
			{"enumerate", 1, {Base::Decl::LIST}, (VCONTAINERFUNC)inb::util::lists::enumerate },
			{"power", 2, {Base::Decl::INT, Base::Decl::INT}, (VCONTAINERFUNC)inb::util::math::pow}
		};



		inb_action& getNoIncludeActionByName(std::string& name, std::vector<Base::Decl> paramTypes) {
			for (auto& v : __NO_INCLUDE_ACTIONS) {
				if (v.name == name && v.paramTypes == paramTypes) return v;
			}
			//iterate again to find a better match, ex unknown parameter count functions or runtime decided type parameter functions
			for (auto& v : __NO_INCLUDE_ACTIONS) {
				if (v.name == name && v.parameter_count == -1) return v; // in future consider other options like overloads!!! [TODO]
			}
			inb_action null{ "NULL", 0,{},  nullptr };
			return null;
		}
		inb_function& getNoIncludeFunctionByName(std::string& name, std::vector<Base::Decl> paramTypes) {
			for (auto& v : __NO_INCLUDE_FUNCTIONS) {
				if (v.name == name && v.paramTypes == paramTypes) return v;
			}
			for (auto& v : __NO_INCLUDE_FUNCTIONS) {
				if (v.name == name && v.parameter_count == -1) return v; // in future consider other options like overloads!!! [TODO]
			}
			inb_function null{ "NULL", 0,{},  nullptr };
			return null;
		}
		void inject_inb_std() {
			std::vector<inb_function> functions;
			std::vector<Action> actions;

			for (auto& x : __get_to_be_injected_f()) {
				functions.push_back(inb_function{std::get<0>(x) , std::get<1>(x), std::get<2>(x), std::get<3>(x)});

			}

			__NO_INCLUDE_FUNCTIONS.insert(__NO_INCLUDE_FUNCTIONS.end(), functions.begin(), functions.end());

		}
	}

	
};