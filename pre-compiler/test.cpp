#include "../lexertk.hpp"
#include "precompile.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <set>
#include <vector>
#include <map>
using namespace std;




class fileManager
{
public:
	static lexertk::generator compile(string fc, bool debug = false) // fc = file content
	{
		lexertk::generator generator;

		if (!generator.process(fc))
		{
			if (debug)
				std::cout << "Failed to lex: " << fc << std::endl;
		}

		if (debug)
			lexertk::helper::dump(generator);
		// MAIN LOGIC
		return generator;
	}
	static bool has_suffix(const std::string& str, const std::string& suffix)
	{
		return str.size() >= suffix.size() &&
			str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
	}

	inline static string readFileIntoString(string path)
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

	inline static bool writeToFile(string path, string cont, string placeholder = "")
	{
		try
		{
			std::ofstream s(path);

			s << placeholder + cont;

			s.close();

			return true;
		}
		catch (std::exception e)
		{
			std::cout << "exce:" << e.what() << std::endl;
			return false;
		}
	}
};
int main()
{
    lexertk::generator gen(fileManager::compile(fileManager::readFileIntoString("./test.em"), false));
    auto x = pre_compile(gen);

    for(const auto& y : x._CFLAG_DEFINES){
        std::cout << y.first << '\n';
    }
}