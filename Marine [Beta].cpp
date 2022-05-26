#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <set>
#include <vector>
#include <map>
#include <filesystem>
#include "lexertk.hpp"
#include "Parser.h"

using namespace std;

static bool USING_GAME_LIB = false;
static bool debug = false;




class fileManager
{
public:
	static lexertk::generator compile(string fc, bool debug = false) // fc = file content
	{
		lexertk::generator generator;

		if (!generator.process(fc))
		{
			if (debug)
				cout << "Failed to lex: " << fc << endl;
		}

		if (debug)
			lexertk::helper::dump(generator);
		// MAIN LOGIC
		return generator;
	}
	inline static std::set<string> readflowcppdep()
	{
		std::set<string> res;
		for (const auto& file : filesystem::directory_iterator("C:\\Flow\\bin\\"))
		{
			res.insert("C:\\Flow\\bin\\" + file.path().filename().string());
		}

		return res;
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
			ifstream t(path);
			if (t)
			{
				ostringstream buffer;
				buffer << t.rdbuf();
				string str = buffer.str();

				// if (debug) cout << "returning:" + ReplaceAll(str,"\n","\\n") << endl;

				return str;
			}

			return "";
		}
		catch (exception e)
		{
			if (debug)
				cout << "an unexpected error has occured while trying to open the file." << endl;
			return "";
		}
	}

	inline static bool writeToFile(string path, string cont, string placeholder = "")
	{
		try
		{
			ofstream s(path);

			s << placeholder + cont;

			s.close();

			return true;
		}
		catch (exception e)
		{
			if (debug)
				cout << "exce:" << e.what() << endl;
			return false;
		}
	}
};
class InputParser
{

public:
	InputParser(int& argc, char** argv)
	{
		for (int i = 1; i < argc; ++i)
			this->tokens.push_back(std::string(argv[i]));
	}
	/// @author iain
	const std::string& getCmdOption(const std::string& option) const
	{
		std::vector<std::string>::const_iterator itr;
		itr = std::find(this->tokens.begin(), this->tokens.end(), option);
		if (itr != this->tokens.end() && ++itr != this->tokens.end())
		{
			return *itr;
		}
		static const std::string empty_string("");
		return empty_string;
	}
	/// @author iain
	bool cmdOptionExists(const std::string& option) const
	{
		return std::find(this->tokens.begin(), this->tokens.end(), option) != this->tokens.end();
	}

private:
	std::vector<std::string> tokens;


};
inline bool _compile(string fc, bool runCompileAfter)
{
	lexertk::generator gen(fileManager::compile(fc, false));
	marine::Parser p(gen);
	p.parse();
	return true;
}
int main(int argc, char* argv[]) {
	_compile(fileManager::readFileIntoString(argv[1]), false);
	return 1;
}
