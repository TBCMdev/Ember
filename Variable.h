#pragma once
#include <iostream>
#include <sstream>
#include <any>
#include <vector>
#include <unordered_map>
#include "MError.h"
#include "Base.h"
#include "VContainer.h"
#include "ValueHolder.h"
namespace marine {
	class StaticObject;
	class Variable : public ValueHolder {
	protected:
		std::string name;
		lexertk::token orig;
	public:
		Variable(std::string& _name, std::any val, lexertk::token& _orig, std::vector <Base::DeclConfig> _configs) : orig(_orig), ValueHolder(val), name(_name)
		{
			configs = _configs;
		}
		Variable(std::string& _name, std::any val, std::vector <Base::DeclConfig> _configs) :ValueHolder(val), name(_name)
		{
			__hasToken = false;
			configs = _configs;
		}
		Variable(std::string& _name, std::any val, std::string _origStr, std::vector <Base::DeclConfig> _configs) : orig(_origStr), ValueHolder(val), name(_name)
		{
			__hasToken = false;
			configs = _configs;
		}
		Variable(std::any val) : ValueHolder(val)
		{
			__hasToken = false;
		}
		void setToken(lexertk::token& t) { orig = t; };
		void loseStringTrace() { orig = lexertk::token("UNTRACABLE"); }
		template <typename T>
		void setTokenStr(T s) { orig = lexertk::token(anyToStr<T>(s)); }
		std::string getName() { return name; }
		lexertk::token& getToken() { return orig; }
		virtual std::string str() override {
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
		Base::Decl returnType = Base::Decl::UNKNWN;
		ObjectCallable(const char* n, bool r, std::vector < Base::Decl> types,Base::Decl* ret = nullptr) : name(n), returnable(r), paramTypes(types) 
		{
			if (ret != nullptr) {
				returnType = *ret;
			}
		}
	public:
		virtual void call(std::vector<std::any>& a, ValueHolder* _this, marine::VContainer* v = nullptr, std::vector<Base::Decl>* x = nullptr) { throw marine::errors::RuntimeError("ObjectCallable (function in object) is null."); }

	};

	using LAction = void (*)(std::vector<std::any>, ValueHolder* _this, std::vector<Base::Decl>*);

	using LFunction = marine::VContainer(*)(std::vector<std::any>, ValueHolder* _this, std::vector<Base::Decl>*);


	struct ObjectFunction : public ObjectCallable {
		LFunction c;
	public:
		ObjectFunction(const char* name, Base::Decl returnType, LFunction a, std::vector<Base::Decl> types) : ObjectCallable(name, false, types, &returnType), c(a) {}
		void call(std::vector<std::any>& a, ValueHolder* _this, marine::VContainer* v = nullptr, std::vector<Base::Decl>* x = nullptr) override {
			*v = c(a, _this, x);
		}
	};


	struct ObjectCommand : public ObjectCallable {
		LAction c;
	public:
		ObjectCommand(const char* name, LAction a, std::vector<Base::Decl> types) : ObjectCallable(name, false, types), c(a) {
			std::cout << "created Object Command: " << name << std::endl;
		}
		void call(std::vector<std::any>& a, ValueHolder* _this, marine::VContainer* v = nullptr, std::vector<Base::Decl>* x = nullptr) override {
			c(a, _this, x);
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




	class DynamicObject{

	using Container = std::unordered_map<std::string, VContainer>;

	protected:
		Container _container;

	public:
		DynamicObject(){}
		DynamicObject(Container& items): _container(items) {}
		Container::iterator get(std::string x) {
			return _container.find(x);
		}
		void set(std::string key, VContainer& val) {
			_container.find(key)->second = val;
		}
		bool add(std::string key, VContainer& val) {
			if (_container.count(key)) return false;

			_container.insert({ key, val });
			return true;
		}
		bool has(std::string key) {
			return _container.count(key);
		}
	};
	class StaticObject{
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

