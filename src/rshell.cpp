#include <boost/tokenizer.hpp>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <sys/wait.h>
#include <vector>

using namespace std;
using namespace boost;

struct splitArgs
{
	vector<string> cmds;
	vector<string> cnctrs;
};

// Helper Functions:
// checks if the string passed in is a connector
bool isConnector(string s);
// clears target vectors
void clean(vector<string> v, vector<string> v2);
// returns an integer based on connector
int checkConnector(string s);
// function to put each component of a string into a char[]
// and moving all those char[] into a vector of char*
vector<char*> convertStr(string s);

// Main Functions:
// takes the user input and separates it, returning a vector of
// all the commands separated by the connectors
splitArgs parseInput(string input);
// function that takes the command vector and runs all the commands
// uses the connector vector to determine which commands to run
void runCommands(vector<string> cmds, vector<string> cncts);

int main()
{
	// string for user input
	string input;
	// vector to hold all the commands
	vector<string> commands;
	// vector to hold all the connectors
	// push in a ; since the first command will always tried to
	// be ran no matter what
	vector<string> connectors;
	//connectors.push_back(";");
	splitArgs cmd_vectors;

	do
	{
		clean(commands, connectors);
		cout << "$ ";
		getline(cin, input);
		cmd_vectors = parseInput(input);
		runCommands(cmd_vectors.cmds, cmd_vectors.cnctrs);		

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

void clean(vector<string> v, vector<string> v2)
{
	fflush(0);
	for(int i = 0; i < v.size(); ++i)
		v.pop_back();

	for(int i = 0; i < v2.size(); ++i)
		v2.pop_back();
}

int checkConnector(string s)
{
	if(s.find(';') != string::npos)
		return 0;
	if(s == "||")
		return 1;
	if(s == "&&")
		return 2;

	return -1;
}

vector<char*> convertStr(string s)
{	
	vector<char*> arg_list;
	char_separator<char> sep(" ", "", keep_empty_tokens);
	tokenizer< char_separator<char> > args(s, sep);	
	tokenizer< char_separator<char> >::iterator it = args.begin();
	for(; it != args.end(); ++it)
	{
		char *arg = new char[(*it).size()];
		strcpy(arg, (*it).c_str());
		arg_list.push_back(arg);
	}	
	char * t = '\0';
	arg_list.push_back(t);
	return arg_list;
}

splitArgs parseInput(string input)
{
	splitArgs cmd_vectors;
	cmd_vectors.cnctrs.push_back(";");
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
		cmd_vectors.cmds.push_back(sep_commands.at(i));
		if(cmd_vectors.cmds.at(j).find(";") == string::npos)
		{	
			// concatenates commands together until reaching a connector
			++i;
			while((i < sep_commands.size()) && (!isConnector(sep_commands.at(i))))
			{	
				cmd_vectors.cmds.at(j) += " " + sep_commands.at(i);
				++i;
			}
			
			if(i < sep_commands.size() && sep_commands.at(i).find(";") != string::npos)
			{
				string temp = sep_commands.at(i);
				sep_commands.at(i) = temp.substr(0, temp.size() - 1);
				cmd_vectors.cmds.at(j) += " " + sep_commands.at(i);
				cmd_vectors.cnctrs.push_back(";");
			}
			else if(i < sep_commands.size() && sep_commands.at(i).find(";") == string::npos)
				cmd_vectors.cnctrs.push_back(sep_commands.at(i));
	
			++j;		
		}
		else
		{
			string temp = cmd_vectors.cmds.at(j);
			cmd_vectors.cmds.at(j) = temp.substr(0, temp.size() - 1);
			cmd_vectors.cnctrs.push_back(";");
			++j;
		}
	}
	return cmd_vectors;
}

void runCommands(vector<string> cmds, vector<string> cncts)
{
	vector<char*> command_list;
	int connectorID = 0;
	bool failed = false;

	for(int i = 0; i < cmds.size(); ++i)
	{	
		connectorID = checkConnector(cncts.at(i));
		if(cmds.at(i) == "exit")
		{
			exit(0);
		}
		command_list = convertStr(cmds.at(i));	

		if(connectorID == 0)
		{	
			pid_t pid = fork();
			if(pid < 0)
			{
				perror("fork failed.");
				exit(1);
			}
			else if(pid == 0)
			{	
				failed = false;
				execvp(command_list[0], &command_list[0]);
				perror("execvp failed");
				failed = true;
				exit(1);
			}
			else
			{
				int status;
				if(wait(&status) < 0)
				{
					perror("Child process encountered an error.");
					exit(1);
				}
			}
		}
		else if(connectorID == 1 && failed)
		{
			pid_t pid = fork();
			if(pid < 0)
			{
				perror("fork failed.");
				exit(1);
			}
			else if(pid == 0)
			{	
				failed = false;
				execvp(command_list[0], &command_list[0]);
				perror("execvp failed");
				failed = true;
				exit(1);
			}
			else
			{
				int status;
				if(wait(&status) < 0)
				{
					perror("Child process encountered an error.");
					exit(1);
				}
			
			}
		}
		else if(connectorID == 2 && !failed)
		{
			pid_t pid = fork();
			if(pid < 0)
			{
				perror("fork failed.");
				exit(1);
			}
			else if(pid == 0)
			{	
				failed = false;
				execvp(command_list[0], &command_list[0]);
				perror("execvp failed");
				failed = true;
				exit(1);
			}
			else
			{
				int status;
				if(wait(&status) < 0)
				{
					perror("Child process encountered an error.");
					exit(1);
				}
			}
		}
	}
}	

