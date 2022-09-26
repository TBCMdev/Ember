#pragma once
#include "lexertk.hpp"
#include <string>
namespace marine {
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
			LHOEQ,
			GTOEQ,


			//-BRACKETS-
			LPR,
			RPR,
			//- UNKNOWN? -
			UNKNWN
		};
	protected:
		OPTYPE type;
		std::string x = "";
		lexertk::token _t;
	public:

		Operator(lexertk::token t, bool th = true) : _t(t) {
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
			else if (t.value == "<=") type = OPTYPE::LHOEQ;
			else if (t.value == "(") type = OPTYPE::LPR;
			else if (t.value == ")") type = OPTYPE::RPR;
			else {
				type = OPTYPE::UNKNWN;
			}
			x = t.value;
		}
		std::string str() {
			return x;
		}
		auto getType() { return type; }
		bool isValid() { return type != OPTYPE::UNKNWN; }
		lexertk::token& get() { return _t; }
		template<typename Type1, typename Type2>
		bool equateSimple(Type1 _1, Type2 _2) {
			switch (type) {
			case OPTYPE::EQUALS:
				return (_1 == _2);
			case OPTYPE::NOTEQ:
				return (_1 != _2);
			}
			return false;
		}
		template<typename Type1, typename Type2>
		bool equate(Type1 _1, Type2 _2) {
			switch (type) {
			case OPTYPE::EQUALS:
				return (_1 == _2);
			case OPTYPE::NOTEQ:
				return (_1 != _2);
			case OPTYPE::LTHAN:
				return (_1 < _2);
			case OPTYPE::GTHAN:
				return (_1 > _2);
			case OPTYPE::LHOEQ:
				return (_1 <= _2);
			case OPTYPE::GTOEQ:
				return (_1 >= _2);
			}
			return false;
		}
	};
}