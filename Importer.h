#pragma once
#include <fstream>
#include <vector>
#include <unordered_map>
#include <filesystem>

#include "lexertk.hpp"

#define _EMBER_INB_INCLUDE_DIR "./include_em"

std::unordered_map<std::string, std::string> _EMBER_INB_INCLUDES;
namespace marine {
	class Importer {
	public:
		static bool initialize() {
			for (const auto& x : std::filesystem::directory_iterator(_EMBER_INB_INCLUDE_DIR)) {
				auto y = x.path();
				_EMBER_INB_INCLUDES.insert({y.filename().replace_extension().string(),y.string()});
			}
			return true;
		}
		static bool importExists(std::string& x,std::string& rel, bool* ifmod = nullptr) {
			struct stat buffer;
			//add functionality for modules
			std::cout << x;
			if (_EMBER_INB_INCLUDES.find(x) != _EMBER_INB_INCLUDES.end() || x == "inb") {
				if (ifmod != nullptr) *ifmod = true;
				return true;
			}
			if (stat(x.c_str(), &buffer) == 0) return true;
			x = rel + "/" + x + ".em";
			if (stat(x.c_str(), &buffer) == 0) return true;
		}
		static string readImport(string& path, bool inb = false, bool* __needs_inject__ = nullptr)
		{
			try
			{
				if (path == "inb") {
					if (__needs_inject__ != nullptr) *__needs_inject__ = true;
					return "";
				}

				path = inb ? _EMBER_INB_INCLUDES.find(path)->second : path;


				std::ifstream t(path);
				if (t)
				{
					std::ostringstream buffer;
					buffer << t.rdbuf();
					string str = buffer.str();

					// if (debug) cout << "returning:" + ReplaceAll(str,"\n","\\n") << endl;

					return str;
				}

				return "";
			}
			catch (std::exception e)
			{
				std::cout << "an unexpected error has occured while trying to open the file." << std::endl;
				return "";
			}
		}
		static lexertk::generator parseImport(string& fc)
		{
			lexertk::generator generator;

			if (!generator.process(fc))
			{
				std::cout << "Failed to lex: " << fc << std::endl;
			}

			// MAIN LOGIC
			return generator;
		}
	};
};