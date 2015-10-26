#include <iostream>

#include "rshell.h"

using namespace std;

int main()
{
	rshell shell;
	string input;
	
	do
	{
		cout << "$ ";
		getline(cin, input);
		shell.setInput(input);
		shell.runCommands(shell.parseInput());	
	} while(input != "exit");

	return 0;
}
