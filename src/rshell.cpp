#include <iostream>
#include <boost/tokenizer.hpp>
#include <vector>

using namespace std;
using namespace boost;

// checks if the string passed in is a connector
bool isConnector(string s);
// clears cin and the commands vector to ready it for the next set of commands
void clean(vector<string> v);
// takes the user input and separates it, returning a vector of
// all the commands separated by the connectors
vector<string> parseInput(string input);

int main()
{
	string input;
	vector<string> commands;
	do
	{
		clean(commands);

		cout << "$ ";
		getline(cin, input);
		commands = parseInput(args);

		for(int i = 0; i < commands.size(); ++i)
		{
			cout << commands.at(i);
			cout << endl;
		}
		
		clean(commands);

	} while (input != "exit");
	
	return 0;
}

bool isConnector(string s)
{
	string connectors = "|| &&";
	if(s.find(';') != string::npos)
		return true;
	if(connectors.find(s) != string::npos)
		return true;
	return false;
}

void clean(vector<string> v)
{
	cin.clear();
	for(int i = 0; i < v.size(); ++i)
		v.pop_back();
}

vector<string> parseInput(tokenizer< char_separator<char> > args)
{
	// vector to store initially separate commands	
	vector<string> sep_commands;
	// vector to store commands separated by connectors
	vector<string> commands;

	// use char_separator and tokenizer to split up 
	// all the commands based on spaces
	char_separator<char> sep(" ", "", keep_empty_tokens);
	tokenizer< char_separator<char> >  args(input, sep);

	// iterator and for loop to store all the tokens into a vector
	tokenizer< char_separator<char> >::iterator it = args.begin();
	for(; it != args.end(); ++it)
		sep_commands.push_back(*it);

	// loop to put all the strings of a single command together
	// single commands are separated by connectors
	int j = 0;
	for(int i = 0; i < sep_commands.size(); ++i)
	{
		commands.push_back(sep_commands.at(i));
		// special check for only connector not separated by space
		string temp = commands.at(j);
		if(temp.find(';') == string::npos)
		{
			// concatenates commands together until reaching a connector
			++i;
			while((i < sep_commands.size()) && (!isConnector(sep_commands.at(i))))
			{
				commands.at(j) += " " + sep_commands.at(i);
				++i;
			}
			// adds the connector to the end of the string along with null char
			// and moves to the next spot in the commands vector
			if(i < sep_commands.size())
				commands.at(j) += " " + sep_commands.at(i) + '\0';
		}
		++j;		
	}
	return commands;
}
