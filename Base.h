#pragma once
#include <iostream>
#include <string>
#include "helpers.h"

constexpr bool MARINE__DEBUG = false;



class Base {
public:
	enum class RetDecl {
		INT,
		FLOAT,
		STRING,
		CUSTOM
	};
	enum class Decl {
		INT,
		FLOAT,
		STRING,
		BOOL,
		LIST,
		DYNAMIC_OBJECT,
		STATIC_OBJECT,
		CUSTOM,
		UNKNWN
	};
	enum class DeclConfig {
		FIXED,
		REF,
		NONE

	};
	static Base::Decl declLiteralParse(lexertk::token& t) {
		if (marine::isInt(t)) return Base::Decl::INT;
		if (marine::isFloat(t)) return Base::Decl::FLOAT;
		if (marine::isString(t)) return Base::Decl::STRING;
		return Base::Decl::UNKNWN;
	}
	static const char* declCStr(DeclConfig x) {
		switch (x) {
		case DeclConfig::FIXED:
			return "fixed";
		case DeclConfig::REF:
			return "ref";
		case DeclConfig::NONE:
			return "NULL";
		default:
			return "NULL";
		}
	}
	static const char* declStr(Decl x) {
		switch (x) {
		case Decl::INT:
			return "int";
		case Decl::FLOAT:
			return "float";
		case Decl::STRING:
			return "string";
		case Decl::DYNAMIC_OBJECT:
			return "DYNAMIC object";
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
		//std::cout << "determining: " << t.value << std::endl;
		if (t.value == "int") return Decl::INT;
		if (t.value == "float") return Decl::FLOAT;
		if (t.value == "string") return Decl::STRING;
		if (t.value == "bool") return Decl::BOOL;
		if (t.value == "list") return Decl::LIST;
		if (t.value == "obj") return Decl::DYNAMIC_OBJECT;
		return Decl::UNKNWN;
	}
	static bool is(lexertk::token& t, const char* x) {
		return t.value == x;
	}
};