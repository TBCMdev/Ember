#pragma once
#include <iostream>
class String {
	std::string value;


public:
	static std::string trim(std::string& s) {
		if (s[0] == '"' || s[0] == '\'')
			s.erase(0, 1);
		if (s.back() == '"' || s.back() == '\'')
			s.erase(s.length() - 1);

		return s;
	}
	String() :value("") {}
	String(std::string s) : value(String::trim(s)) {}
	String(const char* c) : value(c) {}
	std::string& get() { return value; }
	void set(std::string& s) { value = s; }
	void add(std::string& x) { value += x; }
};
String operator +(String first, std::string sec)
{
	first.add(sec);
	return first;
}

String operator +(std::string first, String sec)
{
	String x(first);
	x.add(sec.get());
	return x;
}
String operator +(String first, String sec)
{
	first.add(sec.get());
	return first;
}
bool operator==(String& first, String& sec) {
	return (first.get() == sec.get());
}
bool operator!=(String& first, String& sec) {
	return (first.get() != sec.get());
}

