#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <queue>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

using namespace std;
using namespace boost;

// Helper Functions:
// function to remove everything after a '#'
// as these are all comments and can be ignored
string removeComments(string s);

// function to check if a particular token
// is a connector and returns an int depending
// on which connector it is
// returns -1 if not a connector
int isConnector(string s);

// function to iterate through the command vector
// and separate the commands based on connectors
// removes tokens when command has been made
vector<char*> makeCommand(vector<string> &commands);

// fuction that forks and runs a given command
void run(vector<char*> cmd, int connector);

// Main Functions:
// parses the input and stores the tokens
// in a vector
void parseInput(string input, queue<string> &commands);

// main function that takes the commands and runs them
// with execvp
void runCommands(queue<string> &commands);


// booleans that keep track of the status for
// whether or not a command executed correctly
// pfailed is for checking commands within
// precedence operators
static bool *failed;
//static bool * pfailed;

// some global constants for connectors
const int SEMI_COLON = 0;
const int OR = 1;
const int AND = 2;

int main()
{
	// string for user input
	string input;
	
	// queue to hold actual commands and flags
	queue<string> commands;
	
	// using mmap to preserve the bool through child processes
	failed = static_cast<bool *>(mmap(NULL, sizeof *failed, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0));
    if(failed == MAP_FAILED)
    {
        perror("Could not return state of process");
        exit(1);
    }
    
	// setting up login and hostname
	char name[512];
	bool gotName = true;
	if(gethostname(name, sizeof(name)) != 0)
	{
		gotName = false;
		perror("Could not retrieve hostname.");
	}
	
	// setting up login
	char * login = getlogin();
	if(!login)
		perror("Could not retrieve login name.");

	while (input != "exit")
	{
	    // set initial value to false since nothing has failed yet
	    *failed = false;
	    
	    // prints console
		// only prints host and login if both existed
		if(gotName && login)
			cout << '[' << login << '@' << name << ']';
		cout << "$ ";
		
		// grab the input and prepare it for parsing
		fflush(0);
		getline(cin, input);
		if(input == "")
		    continue;
		    
		trim(input);
		input = removeComments(input);
		
		// separate the input into the commands vector
		commands.push(";");
		parseInput(input, commands);

        // run the commands
        runCommands(commands);
	}
	
	return 0;
}

string removeComments(string s)
{
	// if there are no comments, we're done
	if(s.find('#') == string::npos)
		return s;

	return s.substr(0, s.find('#'));
}

int isConnector(string s)
{
    if(s == ";")
        return SEMI_COLON;
    if(s == "||")
        return OR;
    if(s == "&&")
        return AND;
        
    return -1;
}

void parseInput(string input, queue<string> &commands)
{
    char_separator<char> sep(" ;()[]", ";()[]", keep_empty_tokens);
	tokenizer< char_separator<char> >  cmds(input, sep);
	tokenizer< char_separator<char> >::iterator it = cmds.begin();
	for(; it != cmds.end(); ++it)
	    if(*it != "")
	        commands.push(*it);
}

vector<char*> makeCommand(queue<string> &commands)
{
    vector<char*> cmd;
    bool done = false;
    while(!done)
    {
        // less than 0 means it's not a connector
        if(!commands.empty() && isConnector(commands.front()) < 0)
        {
    		char *arg = new char[commands.front().size()];
    		strcpy(arg, commands.front().c_str());
    		cmd.push_back(arg);
    		commands.pop();
        }
        else
        {
            done = true;
        }
    }
	char * t = '\0';
	cmd.push_back(t);
	return cmd;
}

void run(vector<char*> cmd, int connector)
{
    // check to see if the program should exit or not...
    if(strcmp(cmd.front(), "exit") == 0)
    {
        if(connector == SEMI_COLON)
            exit(0);
        else if(connector == OR && *failed)
            exit(0);
        else if(connector == AND && !*failed)
            exit(0);
        else
            return;
    }
    
    // ...otherwise we continue as usual
    // forks
    pid_t pid = fork();
    
    // something went horribly wrong
    if(pid < 0)
    {
        perror("fork failed.");
        *failed = true;
        exit(1);
    }
    
    // we're in the child
    else if(pid == 0)
    {
        if(connector == OR && !*failed)
            exit(0);
        
        if(connector == AND && *failed)
            exit(0);
        
        else
        {
            int status = execvp(cmd[0], &cmd[0]);
            if (status < 0)
            {
                *failed = true;
                perror("execvp failed");
                exit(1);
            }
        }
    }
    
    // we're in the parent
    else
    {
        int status;
		if(wait(&status) < 0)
		{
			perror("Child process encountered an error.");
			*failed = true;
			exit(1);
		}
		
		// check the exit status of the child
		else
		{
		    int exit_status = WEXITSTATUS(status);
		    
		    // 0 is the only time it succeeds
		    if(exit_status == 0)
		        *failed = false;
		        
		    // anything else means it failed in some way
		    else
		        *failed = true;
		}
    }
}

void runCommands(queue<string> &commands)
{
    // vector of char* to store command in a way
    // that execvp() can take in
    vector<char*> cmd;
    
    // determines what the previous connector was
    int connector = 0;
    
    while(!commands.empty())
    {
        // this should always run at least once since
        // the queue starts with a ;
        if(!commands.empty())
        {
            connector = isConnector(commands.front());
            commands.pop();
        }
        
        // this would only fail the first time if the queue
        // was empty
        if(!commands.empty())
        {
            cmd = makeCommand(commands);
            run(cmd, connector);
        }
        
        // deallocates memory
        for(int i = 0; i < cmd.size(); ++i)
            delete cmd.at(i);
    }
}
