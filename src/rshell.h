#ifndef RSHELL_H
#define RSHELL_H

#include <iostream>
#include <boost/tokenizer.hpp>

using namespace std;
using namespace boost;

class rshell
{
private:
	string input;

public:
	rshell();
	void setInput(string userInput);
	tokenizer<> parseInput();
	void runCommands(tokenizer<> args);
};

#endif
