#include "rshell.h"

using namespace std;

rshell::rshell()
{}

void rshell::setInput(string userInput)
{
	input = userInput;
}

tokenizer<> rshell::parseInput()
{
	tokenizer<> args(input);
	return args;
}

void rshell::runCommands(tokenizer<> args)
{
	for(tokenizer<>::iterator it = args.begin(); it != args.end(); ++it)
	{
		cout << *it << " ";
	}
	cout << endl;	
}
