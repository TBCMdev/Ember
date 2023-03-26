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
		lexertk::token orig;
	public:
		Variable(std::any val, lexertk::token& _orig, std::vector <Base::DeclConfig> _configs) : orig(_orig), ValueHolder(val)
		{
			configs = _configs;
		}
		Variable(std::any val, std::vector <Base::DeclConfig> _configs, Base::Decl d) :ValueHolder(val)
		{
			__hasToken = false;
			configs = _configs;
			decl = d;
		}
		Variable(std::any val, std::vector <Base::DeclConfig> _configs) :ValueHolder(val)
		{
			__hasToken = false;
			configs = _configs;
		}
		Variable(std::any val, std::vector <Base::DeclConfig> _configs, int depth) :ValueHolder(val)
		{
			__hasToken = false;
			configs = _configs;
			__depth = depth;
		}
		Variable(std::any val, std::string _origStr, std::vector <Base::DeclConfig> _configs) : orig(_origStr), ValueHolder(val)
		{
			__hasToken = false;
			configs = _configs;
		}
		Variable(std::any val) : ValueHolder(val)
		{
			__hasToken = false;
			std::string null = "";
		}
		void setToken(lexertk::token& t) { orig = t; };
		void loseStringTrace() { orig = lexertk::token("UNTRACABLE"); }
		lexertk::token& getToken() { return orig; }
		virtual std::string str() override {
			return std::string("(variable) val=" + orig.value + ", decl_type: " + Base::declStr(decl));
		}
		~Variable() = default;
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
		bool returnable;
		std::vector<Base::Decl> paramTypes;
		Base::Decl returnType = Base::Decl::UNKNWN;
		ObjectCallable(bool r, std::vector < Base::Decl> types,Base::Decl* ret = nullptr) : returnable(r), paramTypes(types) 
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
		ObjectFunction(Base::Decl returnType, LFunction a, std::vector<Base::Decl> types) : ObjectCallable(false, types, &returnType), c(a) {}
		void call(std::vector<std::any>& a, ValueHolder* _this, marine::VContainer* v = nullptr, std::vector<Base::Decl>* x = nullptr) override {
			*v = c(a, _this, x);
		}
	};


	struct ObjectCommand : public ObjectCallable {
		LAction c;
	public:
		ObjectCommand(LAction a, std::vector<Base::Decl> types) : ObjectCallable(false, types), c(a) {
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
		std::unordered_map<std::string, std::shared_ptr<ObjectCallable>> functions{};
		std::unordered_map<std::string, ObjectVariable> variables;
		ObjectInheritor handler;
	public:

		static StaticObject null() {
			return StaticObject("NULL", {}, {}, {});
		}
		StaticObject(std::string n, ObjectInheritor* inheritorHandler = nullptr) : name(n) {
			if (inheritorHandler != nullptr) handler = *inheritorHandler;
		}
		StaticObject(std::string n, std::unordered_map<std::string, ObjectCommand> commands, std::unordered_map<std::string, ObjectFunction> funcs, std::unordered_map<std::string, ObjectVariable> members) : name(n), variables(members) {

			for (auto& x : commands) {
				functions.insert({ x.first, std::make_shared<ObjectCommand>(x.second) });
			}
			for (auto& x : funcs) {
				functions.insert({ x.first, std::make_shared<ObjectFunction>(x.second) });
			}
		}
		std::string getName() { return name; }
		bool hasVariable(std::string n) {
			return variables.find(n) != variables.end();
		}
		ObjectVariable* getVariable(std::string n) {
			auto f = variables.find(n);

			if (f != variables.end()) return &f->second;

			return nullptr;
		}
		bool hasFunction(std::string n) {
			return functions.find(n) != functions.end();
		}
		ObjectCallable* getFunction(std::string n) {
			auto f = functions.find(n);

			if (f != functions.end()) return f->second.get();

			return nullptr;
		}
	};



};

