#include "rshell.h"

#include <cstring>

using namespace std;

rshell::rshell()
{}

void rshell::setInput(string userInput)
{
	input = userInput;
}

tokenizer<> rshell::parseInput(string userInput)
{
	tokenizer<> args(userInput);
	return args;
}

void rshell::runCommands(tokenizer<> args)
{}
