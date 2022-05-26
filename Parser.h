#include <iostream>
#include <string>
#include <vector>
#include <variant>
#include <any>

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




	struct Operator 
	{
		enum class OPTYPE {
			//- ARITHMATIC -  
			ADD,
			SUB,
			DIV,
			MULT,
			MOD,
			ADD_S,
			SUB_S,
			DIV_S,
			MULT_S,
			MOD_S,
			//- LOGICAL - 
			ASSIGN,
			EQUALS,
			NOTEQ,
			LTHAN,
			GTHAN,
			THOEQ,
			GTOEQ,
			//- UNKNOWN? -
			UNKNWN
		};
	protected:
		OPTYPE type;
		std::string x = "";
	public:
		Operator(lexertk::token& t, bool th = true) {
			if (t.value == "+") type = OPTYPE::ADD;
			else if (t.value == "+") type = OPTYPE::ADD;
			else if (t.value == "-") type = OPTYPE::SUB;
			else if (t.value == "*") type = OPTYPE::MULT;
			else if (t.value == "/") type = OPTYPE::DIV;
			else if (t.value == "%") type = OPTYPE::MOD;
			else if (t.value == "+=") type = OPTYPE::ADD_S;
			else if (t.value == "-=") type = OPTYPE::SUB_S;
			else if (t.value == "*=") type = OPTYPE::MULT_S;
			else if (t.value == "/=") type = OPTYPE::DIV_S;
			else if (t.value == "%=") type = OPTYPE::MOD_S;
			else if (t.value == "=") type = OPTYPE::ASSIGN;
			else if (t.value == "==") type = OPTYPE::EQUALS;
			else if (t.value == "!=") type = OPTYPE::NOTEQ;
			else if (t.value == ">") type = OPTYPE::GTHAN;
			else if (t.value == "<") type = OPTYPE::LTHAN;
			else if (t.value == ">=") type = OPTYPE::GTOEQ;
			else if (t.value == "<=") type = OPTYPE::THOEQ;
			else {
				//if (th) throw ("invalid syntax error: expected operator, not: '" + t.value + "'.");
				type = OPTYPE::UNKNWN;
			}
			x = t.value;
		}
		std::string str() {
			return x;
		}
		bool isValid() { return type == OPTYPE::UNKNWN; }
	};
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
		std::string& name;
		lexertk::token& orig;
		Base::Decl decl;
		std::vector <Base::DeclConfig> configs;
	public:
		Variable(std::string& _name, std::any val,lexertk::token& _orig, std::vector <Base::DeclConfig> _configs): orig(_orig), value(val), name(_name), configs(_configs)
		{
			
		}
		void setDecl(Base::Decl d) { decl = d; }
		std::any getValue() { return value; }
		template <typename T>
		T castValue() {
			if (std::count(configs.begin(), configs.end(), Base::DeclConfig::FIXED) != 0 && std::count(configs.begin(), configs.end(), Base::DeclConfig::REF) != 0) {
				return std::any_cast<const T*> (value);
			}
			else if (std::count(configs.begin(), configs.end(), Base::DeclConfig::FIXED) != 0) {
				return std::any_cast<const T> (value);
			}
			else if (std::count(configs.begin(), configs.end(), Base::DeclConfig::REF) != 0) {
				return std::any_cast<T*> (value);
			}
			return std::any_cast<T>(value);
		}
		std::any& setValue(std::any x) { value = x; return x; }
		lexertk::token& getToken() { return orig; }
		std::string str() {
			try {
				return std::string("[var] " + name + ", val: '" + orig.value + "'");
			}
			catch (std::exception& e) {
				std::cout << e.what();
				return "null";
			}
		}
	};
	
	class Parser {
	protected:
		lexertk::generator& gen;
		lexertk::token current;
		std::vector<Variable*> core_variables;
		int index = -1;
	public:
		Parser(lexertk::generator& generator): gen(generator) {}
#pragma region decl

		bool isDecl() {
			return Base::declareParse(cur()) != Base::Decl::UNKNWN;
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
					for (auto& x : conf) {
						std::cout << "FOUND CONFIG:" << Base::declCStr(x) << std::endl;
					}
					if (conf.size() == 0) throw "no config state was supplied after ':'";
				}
				lexertk::token& decl_name = advance();
				if (Base::is(advance(), "=")) {
					//IT IS VARIABLE
					//just handle numbers for now
					if (isInt(getNext())) {
						std::cout << "cur:" << cur().value;
						Variable v(decl_name.value, std::stoi(advance().value), advance(), conf);
						v.setDecl(Base::Decl::INT);
						core_variables.push_back(&v);
						return;
					}
					else if (isFloat(getNext())) {
						Variable v(decl_name.value, std::stoi(advance().value), advance(), conf);
						v.setDecl(Base::Decl::FLOAT);
						core_variables.push_back(&v);
						return;
					}
					else {
						Variable v(decl_name.value, std::stoi(advance().value), advance(), conf);
						v.setDecl(Base::Decl::STRING);
						core_variables.push_back(&v);
						return;
					}
				}
				else if (Base::is(cur(), "(")) {
					//IT IS FUNCTION
				}

			}
		}

#pragma endregion
		void parse() {
			advance();
			while (canAdvance()) {
				if (isDecl()) {
					parseDecl();
				}

				advance();
			}
			for (const auto& x : core_variables) {
				std::cout << x->str() << std::endl; 
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