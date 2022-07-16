#pragma once
#include <iostream>
#include <exception>
#include <sstream>
namespace marine {
	namespace errors {
		struct MError : public std::exception {
		public:
			const char* what_c;
			static const char* construct(const char* message, const char* name) {
				std::stringstream stream;
				stream << "[" << name << "]" << "Error: " << message;
				return stream.str().c_str();
			}
			//stack trace
			MError(const char* w): what_c(w){}
			virtual const char* what() throw() { return what_c; }
		};
		struct SyntaxError : private MError {
			using MError::MError;
		public:
			const char* what() throw() override {
				return MError::construct(what_c, "Syntax Error");
			}
		};
		struct IndexError : private MError {
			using MError::MError;
		public:
			const char* what() throw() override {
				return MError::construct(what_c, "Index Out Of Range Error");
			}
		};
		struct RuntimeError : private MError {
			using MError::MError;
		public:
			const char* what() throw() override {
				return MError::construct(what_c, "Runtime Error");
			}
		};
		struct TypeError : private MError {
			using MError::MError;
		public:
			const char* what() throw() override {
				return MError::construct(what_c, "Type Error");
			}
		};
	}
}