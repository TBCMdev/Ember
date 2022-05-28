#include <iostream>
#include <string>
#include <vector>
#include <variant>
#include <any>

#include "Math.h"


namespace marine {
#pragma region helpers
	static bool isOp(lexertk::token& t) {
		if (t.value == "+" ||
			t.value == "-" ||
			t.value == "*" ||
			t.value == "/" ||
			t.value == "%" ||
			t.value == "+="||
			t.value == "-="||
			t.value == "*="||
			t.value == "/="||
			t.value == "%=") return true;
		else {
			return false;
		}
	}
	static bool isFloat(lexertk::token& t) {
		bool dec = false;
		for (char c : t.value) {
			if (std::string("0123456789").find(c) == std::string::npos) return false;
			if (!dec && c == '.') dec = true;
			else if (dec) return false;
		}
		return true;
	}
	static bool isInt(lexertk::token& t) {
		for (char c : t.value) {
			if (std::string("0123456789").find(c) == std::string::npos) return false;
			if (c == '.') return false;
		}
		return true;
	}
#pragma endregion

	class Base {
	public:
		enum class Decl {
			INT,
			FLOAT,
			STRING,
			CUSTOM,
			UNKNWN
		};
		enum class DeclConfig {
			FIXED,
			REF,
			NONE

		};
		const static const char* declCStr(DeclConfig& x) {
			switch (x) {
			case DeclConfig::FIXED:
				return "fixed";
			case DeclConfig::REF:
				return "ref";
			case DeclConfig::NONE:
				return "NULL";
			}
		}
		const static const char* declStr(Decl& x) {
			switch (x) {
			case Decl::INT:
				return "int";
			case Decl::FLOAT:
				return "float";
			case Decl::STRING:
				return "string";
			case Decl::CUSTOM:
				return "custom";
			default:
				return "unknown";
			}
		}
		const static DeclConfig declCParse(lexertk::token& t) {
			if (t.value == "fixed") return DeclConfig::FIXED;
			else if (t.value == "ref") return DeclConfig::REF;
			return DeclConfig::NONE;
		}
		const static Decl declareParse(lexertk::token& t) {
			if (t.value == "int") return Decl::INT;
			if (t.value == "float") return Decl::FLOAT;
			if (t.value == "string") return Decl::STRING;
			return Decl::UNKNWN;
		}
		static bool is(lexertk::token& t, const char* x) {
			return t.value == x;
		}
	};
	class Variable{
	protected:
		std::any& value;
		std::string name;
		lexertk::token orig;
		Base::Decl decl = Base::Decl::UNKNWN;
		std::vector <Base::DeclConfig> configs;
		std::string configsStr() {
			std::stringstream stream;
			stream << "[";

			for (int i = 0; i < configs.size(); i++) {
				stream << Base::declCStr(configs[i]);
				if (i + 1 < configs.size()) stream << ", ";
			}
			return stream.str() + "]";
		}
	public:
		Variable(std::string& _name,std::any val,lexertk::token& _orig, std::vector <Base::DeclConfig> _configs): orig(_orig), value(val), name(_name), configs(_configs)
		{
		}
		void setDecl(Base::Decl d) { decl = d; }
		std::any& getValue() { return value; }
		std::string& getName() { return name; }
		template <typename T>
		T cast() {
			return std::any_cast<T>(value);
		}
		bool is(Base::DeclConfig d) {
			return std::count(configs.begin(), configs.end(), d) != 0;
		}
		std::any& setValue(std::any x) { value = x; return x; }
		lexertk::token& getToken() { return orig; }
		std::string str() {
			return std::string("[var] name: " + name + ", val=" + orig.value + ", decl_type: " + Base::declStr(decl) + ", configurations: " + configsStr());
		}
	};
	class MathEvaluator {
	protected:
		lexertk::generator& generator;
	public:
		MathEvaluator(lexertk::generator& gen) : generator(gen) {}
		template<typename Type>
		void evalLeft() {

		}
		template<typename Type>
		void evaluate(unsigned int start, unsigned int stop,Type& t) {
			if (stop < start) return;
			if (start > generator.size() || stop > generator.size()) return;

			int level = 0;

			while (i < stop) {
				if (Base::is(gen[i], "(")) {
					level++;
					Type t;
					evaluate(i + 1, stop, t);

				}
			}
		}
	};
	class Parser {
	protected:
		lexertk::generator& gen;
		lexertk::token current;
		std::vector<Variable> core_variables;
		int index = -1;
	public:
		Parser(lexertk::generator& generator): gen(generator) {}
#pragma region decl

		bool isDecl() {
			return Base::declareParse(cur()) != Base::Decl::UNKNWN;
		}
		bool isFuncCall() {
			//just do print for now
			if (cur().value == "print") return true;
			
			if (Base::is(getNext(), "(")) {
				int br = 1;
				for (int i = index; i < gen.size(); i++) {
					lexertk::token& t = gen[i];
					if (Base::is(t, "(")) br++;
					if (Base::is(t, ")")) br--;
					if(br == 0) break;
				}
				if (br != 0) throw ("expected '(' after function call.");
				return true;
			}
			return false;
		}
		void parseArithmatic() {
		}
		void parseDecl() {
			Base::Decl type = Base::declareParse(cur());
			std::vector<Base::DeclConfig> conf{};
			while (canAdvance()) {
				if (Base::is(advance(), ":")) {
					conf.push_back(Base::declCParse(advance()));
					while (Base::is(getNext(), ",")) {
						conf.push_back(Base::declCParse(advance(2)));
						if (!Base::is(getNext(), ",")) break;
					}
					if (conf.size() == 0) throw "no config state was supplied after ':'";
				}
				lexertk::token& decl_name = advance();
				if (Base::is(advance(), "=")) {
					//IT IS VARIABLE
					//just handle numbers for now
					if (isInt(getNext())) {
						Variable v(decl_name.value, std::stoi(advance().value), cur(), conf);
						v.setDecl(Base::Decl::INT);
						core_variables.push_back(v);
						return;
					}
					else if (isFloat(getNext())) {
						Variable v(decl_name.value, std::stof(advance().value), cur(), conf);
						v.setDecl(Base::Decl::FLOAT);
						core_variables.push_back(v);
						return;
					}
					else {
						Variable v(decl_name.value, advance().value, cur(), conf);
						v.setDecl(Base::Decl::STRING);
						core_variables.push_back(v);
						return;
					}
				}
				else if (Base::is(cur(), "(")) {
					//IT IS FUNCTION
				}

			}
		}
		void parseFuncCall() {
			//MOCK CODE
			if (cur().value == "print") {
				if (advance().value == "(") {
					Variable* v = nullptr;
					advance();
					for (int i = 0; i < core_variables.size(); i++) {
						if (core_variables[i].getName() == cur().value) {
							v = &core_variables[i];
							break;
						}
					}
					advance();
					std::cout << v->getToken().value << std::endl;
				}
			}
		}
		bool isVariableUsage() {

		}
		void parseVariableUsage() {
		}
#pragma endregion
		void parse() {
			advance();
			while (canAdvance()) {
				if (isDecl()) {
					parseDecl();
				}
				else if (isFuncCall()) {
					parseFuncCall();
				}
				else if (isVariableUsage()) {
					parseVariableUsage();
				}

				advance();
			}
			for (auto& x : core_variables) {
				std::cout << x.str() << std::endl; 
			}
		}
		lexertk::token& cur() {
			return current;
		}
		lexertk::token& advance() {
			if (!canAdvance()) return current;
			index++;
			current = gen[index];
			return gen[index];

		}
		lexertk::token& advance(unsigned int x) {
			if (!canAdvance(x)) return current;
			index+=x;
			current = gen[index];
			return gen[index];

		}
		lexertk::token& getNext() {
			if (!canAdvance()) return current;

			return gen[index + 1];
		}
		lexertk::token& getNext(unsigned int x) {
			if (!canAdvance(x)) return current;

			return gen[index + x];
		}
		bool canAdvance() {
			return index + 1 < gen.size();
		}
		bool canAdvance(unsigned int x) {
			return index + x < gen.size();
		}
	};
}