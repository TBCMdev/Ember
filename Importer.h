#pragma once
#include <fstream>
#include <vector>
#include "lexertk.hpp"
 

namespace marine {
	class Importer {
	public:
		static bool importExists(std::string& x,std::string& rel) {
			struct stat buffer;
			//add functionality for modules
			if (stat(x.c_str(), &buffer) == 0) return true;
			x = rel + "/" + x + ".em";
			if (stat(x.c_str(), &buffer) == 0) return true;
		}
		static string readImport(string& path)
		{
			try
			{
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