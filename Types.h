#pragma once
#include "Base.h"
namespace marine {
	class String {
		std::string value;


	public:
		static std::string trim(std::string& s) {
			if(s[0] == '"' || s[0] == '\'')
				s.erase(0, 1);
			if(s.back() == '"' || s.back() == '\'')
				s.erase(s.length() - 1);

			return s;
		}
		String():value("") {}
		String(std::string s): value(String::trim(s)){}
		String(const char* c): value(c){}
		std::string& get() { return value; }
	};
	std::string operator +(String first, std::string sec) 
	{
		return first.get() + sec;
	}
	std::string operator +(std::string first, String sec)
	{
		return sec.get() + first;
	}
	std::string operator +(String first, String sec)
	{
		return first.get() + sec.get();
	}
	class VContainer {
		std::any _value;
		std::string placeHolder;
		unsigned int depth;
		Base::Decl decl = Base::Decl::UNKNWN;
		std::vector <Base::DeclConfig> configs;
	public:
		VContainer(std::any a, unsigned int dep, Base::Decl decl, std::vector<Base::DeclConfig> d = {}): _value(a), depth(dep), decl(decl), configs(d){}
		VContainer():_value(nullptr), depth(-1), configs({}) {}
		template <typename T>
		T cast() {
			return std::any_cast<T>(_value);
		}
		void setPlaceholder(std::string s) { placeHolder = s; }
		std::string& getStringified() { return placeHolder; }
		std::any& get() { return _value; }
		void set(std::any& a, Base::Decl _new, int dep) { _value = a; decl = _new; depth = dep; }
		Base::Decl type() { return decl; }
	};
	class Variable {
	protected:
		std::any _value;
		std::string name;
		lexertk::token orig;
		unsigned int __depth = 0;
		Base::Decl decl = Base::Decl::UNKNWN;
		std::vector <Base::DeclConfig> configs;
		bool __hasToken = true;
		std::string configsStr() {
			std::stringstream stream;
			stream << "[";

			for (size_t i = 0; i < configs.size(); ++i) {
				stream << Base::declCStr(configs[i]);
				if (i + 1 < configs.size()) stream << ", ";
			}
			return stream.str() + "]";
		}
	public:
		Variable(std::string& _name, std::any val, lexertk::token& _orig, std::vector <Base::DeclConfig> _configs) : orig(_orig), _value(val), name(_name), configs(_configs)
		{
		}
		Variable(std::string& _name, std::any val, std::vector <Base::DeclConfig> _configs) : __hasToken(false), _value(val), name(_name), configs(_configs)
		{
		}
		Variable(std::string& _name, std::any val, std::string _origStr, std::vector <Base::DeclConfig> _configs) : orig(_origStr), __hasToken(false), _value(val), name(_name), configs(_configs)
		{
		}
		int getDepth() { return __depth; }
		void setDepth(unsigned int depth) { __depth = depth; }
		void setDecl(Base::Decl d) { decl = d; }
		Base::Decl getDecl() { return decl; }
		std::any& getValue() { return _value; }
		void setValue(std::any& a, Base::Decl d) { _value = a; decl = d; }
		void setValue(VContainer& v) { _value = v.get(); decl = v.type(); }
		std::string& getName() { return name; }
		template <typename T>
		T cast() {
			return std::any_cast<T>(_value);
		}
		bool is(Base::DeclConfig d) {
			return std::count(configs.begin(), configs.end(), d) != 0;
		}
		std::any& setValue(std::any x) { _value = x; return x; }
		lexertk::token& getToken() { return orig; }
		std::string str() {
			return std::string("[var] name: " + name + ", val=" + orig.value + ", decl_type: " + Base::declStr(decl) + ", configurations: " + configsStr());
		}
	};
	class Function {
	protected:
		std::string name;
		lexertk::token& start, end;
		int start_index, end_index;
	public:
		std::vector<Variable> parameters;
		Function(std::string _name, lexertk::token& _start, lexertk::token _end, int start_, int end_, std::vector<Variable> _p) : name(_name), start(_start), end(_end), start_index(start_), end_index(end_), parameters(_p) {

		}
		std::string& getName() { return name; }
		int getStart() { return start_index; }
		int getEnd() { return end_index; }

	};
};