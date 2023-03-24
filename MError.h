#pragma once
#include <iostream>
#include <exception>
#include <sstream>

#define CERROR(n) n(const char* what) : MError(MError::construct(what, #n).c_str())


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
			CERROR(SyntaxError) {}
		};
		struct IndexError : public MError {
		public:
			CERROR(IndexError) {}
		};
		struct RuntimeError : public MError {
		public:
			CERROR(RuntimeError) {}
		};
		struct TypeError : public MError {
		public:
			CERROR(TypeError) {}
		};
		struct FileError : public MError {
		public:
			CERROR(FileError) {}
		};
		struct DLLInjectError : public MError {
		public:
			CERROR(DLLInjectError) {}
		};
	}
}