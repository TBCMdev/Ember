#pragma once
#include <iostream>
#include <sstream>
#include <any>
#include <vector>
#include "Base.h"
#include "VContainer.h"
namespace marine {
	class StaticObject;
	class Variable {
	protected:
		std::string name = "NULL";
		lexertk::token orig;
		unsigned int __depth = 0;
		Base::Decl decl = Base::Decl::UNKNWN;
		std::vector <Base::DeclConfig> configs;

		std::shared_ptr<StaticObject> connected_obj = nullptr;


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
		std::any _value;
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
		void setToken(lexertk::token& t) { orig = t; };
		void loseStringTrace() { orig = lexertk::token("UNTRACABLE"); }
		void setValue(std::any a, Base::Decl d) { _value = a; decl = d; }
		void setValue(VContainer& v) { _value = v.get(); decl = v.type(); }
		std::string getName() { return name; }
		template <typename T>
		T cast() {
			return std::any_cast<T>(_value);
		}
		std::shared_ptr<StaticObject> getObjSelf() { return connected_obj; }
		void setObjSelf(StaticObject* o) { connected_obj = std::make_shared<StaticObject>(*o); }
		bool is(Base::DeclConfig d) {
			return std::count(configs.begin(), configs.end(), d) != 0;
		}
		std::any& setValue(std::any x) { _value = x; return x; }
		lexertk::token& getToken() { return orig; }
		std::string str() {
			return std::string("[var] name: " + name + ", val=" + orig.value + ", decl_type: " + Base::declStr(decl) + ", configurations: " + configsStr());
		}
	};


	enum class Protection {
		PRIVATE,
		PUBLIC,
		PROTECTED,
		INTERNAL
	};

	struct ObjectInheritor {
		std::vector<std::shared_ptr<StaticObject>> inherited;
	};


	struct ObjectCallable {
		std::string name;
		bool returnable;
		std::vector<Base::Decl> paramTypes;
		ObjectCallable(const char* n, bool r, std::vector < Base::Decl> types) : name(n), returnable(r), paramTypes(types) {}
	public:
		virtual void call(std::vector<std::any>& a, Variable* _this, marine::VContainer* v = nullptr) { throw marine::errors::RuntimeError("ObjectCallable (function in object) is null."); }

	};

	using LAction = void (*)(std::vector<std::any>, Variable* _this);

	using LFunction = marine::VContainer(*)(std::vector<std::any>, Variable* _this);


	struct ObjectFunction : public ObjectCallable {
		LFunction c;
	public:
		ObjectFunction(const char* name, LFunction a, std::vector<Base::Decl> types) : ObjectCallable(name, false, types), c(a) {}
		void call(std::vector<std::any>& a, Variable* _this, marine::VContainer* v = nullptr) override {
			*v = c(a, _this);
		}
	};


	struct ObjectCommand : public ObjectCallable {
		LAction c;
	public:
		ObjectCommand(const char* name, LAction a, std::vector<Base::Decl> types) : ObjectCallable(name, false, types), c(a) {
			std::cout << "created Object Command: " << name << std::endl;
		}
		void call(std::vector<std::any>& a, Variable* _this, marine::VContainer* v = nullptr) override {
			c(a, _this);
		}

	};
	struct ObjectVariable {
	protected:
		Variable _this;
		Protection protection = Protection::PRIVATE;
	public:
		Variable& get(/*check if the accessor can access the variable.*/) { return _this; }
		Variable* getUnprotected() { return &_this; }
		Protection& getProtection() { return protection; }

	};





	class StaticObject {
	protected:
		std::string name;
		std::vector <std::shared_ptr<ObjectCallable>> functions{};
		std::vector<ObjectVariable> variables;
		ObjectInheritor handler;
	public:

		static StaticObject null() {
			return StaticObject("NULL", {}, {}, {});
		}
		StaticObject(std::string n, ObjectInheritor* inheritorHandler = nullptr) : name(n) {
			if (inheritorHandler != nullptr) handler = *inheritorHandler;
		}
		StaticObject(std::string n, std::vector<ObjectCommand> commands, std::vector<ObjectFunction> funcs, std::vector<ObjectVariable> members) : name(n), variables(members) {

			for (auto& x : commands) {
				functions.push_back(std::make_shared<ObjectCommand>(x));
			}
			for (auto& x : funcs) {
				functions.push_back(std::make_shared<ObjectFunction>(x));
			}
		}
		std::string getName() { return name; }
		bool hasVariable(std::string n) {
			for (auto x : variables) {
				if (x.getUnprotected()->getName() == n) return true;
			}
			return false;
		}
		ObjectVariable* getVariable(std::string n) {
			for (auto x : variables) {
				if (x.getUnprotected()->getName() == n) return &x;
			}
			return nullptr;
		}
		bool hasFunction(std::string n) {
			for (auto& x : functions) {
				if (x->name == n) return true;
			}
			return false;
		}
		ObjectCallable* getFunction(std::string n) {
			for (auto x : functions) {
				if (x->name == n) return x.get();
			}
			return nullptr;
		}
	};



};

