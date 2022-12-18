#pragma once
#include <iostream>
#include <exception>
#include <sstream>
namespace marine {
	namespace errors {
		struct MError : public std::exception {
		public:
			static std::string construct(const char* message, const char* name) {
				std::stringstream stream;
				stream << "[" << name << "] " << message;
				return stream.str();
			}
			//stack trace
			MError(const char* w): std::exception(w){}
		};
		struct SyntaxError : public MError {
		public:
			SyntaxError(const char* what) : MError(MError::construct(what, "Syntax Error").c_str()) {}
		};
		struct IndexError : public MError {
		public:
			IndexError(const char* what) : MError(MError::construct(what, "Index Error").c_str()) {}
		};
		struct RuntimeError : public MError {
		public:
			RuntimeError(const char* what) : MError(MError::construct(what, "Runtime Error").c_str()) {}
		};
		struct TypeError : public MError {
		public:
			TypeError(const char* what) : MError(MError::construct(what, "Type Error").c_str()) {}
		};
		struct FileError : public MError {
		public:
			FileError(const char* what) : MError(MError::construct(what, "File Error").c_str()) {}
		};
	}
}