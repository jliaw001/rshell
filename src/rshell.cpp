#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <queue>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

using namespace std;
using namespace boost;

// Helper Functions:
// function to check if a particular token
// is a connector and returns an int depending
// on which connector it is
// returns -1 if not a connector
int isConnector(string s);

// function to put together a string held within quotations
void makeString(queue<string> &commands, vector<char*> &cmd);

// pops everything after a '#' off the queue
void removeComments(queue<string> &commands);

// function to iterate through the command vector
// and separate the commands based on connectors
// removes tokens when command has been made
vector<char*> makeCommand(vector<string> &commands);

// fuction that forks and runs a given command
void run(vector<char*> cmd, int connector);

// function that tells whether a given string is a valid
// flag for the test command
bool isTestFlag(string s);

// Additional Functions:
// runs the test function and returns a bool
bool test(queue<string> &commands, vector<char*> &cmd);

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
static bool *group_pass;

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
    
    group_pass = static_cast<bool *>(mmap(NULL, sizeof *failed, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0));
    if(group_pass == MAP_FAILED)
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
	    
	    // assuming nothing in the group worked
	    *group_pass = false;
	    
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
		
		// makes sure nothing is left in the queue
		while(!commands.empty())
		    commands.pop();
		
		// separate the input into the commands queue
		commands.push(";");
		parseInput(input, commands);
		removeComments(commands);

        // run the commands
        runCommands(commands);
	}
	
	return 0;
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

void makeString(queue<string> &commands, vector<char*> &cmd)
{
    bool done = false;
    // get rid of the begin quote
    commands.pop();
    while(!done)
    {
        // check to see if the quotation was ended properly
        if(commands.empty())
        {
            cout << "Error: Missing ending quote" << endl;
            exit(1);
        }
        
        // checking for the ending quote
        else if(commands.front() == "\"")
        {
            commands.pop();
            done = true;
        }
        else
        {
            char *arg = new char[commands.front().size()];
            strcpy(arg, commands.front().c_str());
            cmd.push_back(arg);
            commands.pop();
        }
        
    }
}

void removeComments(queue<string> &commands)
{
    // keeps track of when a quotation starts
    bool start_quote = false;
    // keeps track of when a quotation ends
    // defaults to true since we technically "ended" nonexistant quotes
    bool end_quote = true;
    // temporary queue to hold stuff
    queue<string> temp;
    
    // main loop to iterate through and remove #'s not within quotations
    while(!commands.empty())
    {
        // checks for beginning quotes
        if(commands.front() == "\"" && !start_quote && end_quote)
        {
            start_quote = true;
            end_quote = false;
            temp.push(commands.front());
            commands.pop();
        }
        
        // checks for ending quotes
        else if(commands.front() == "\"" && start_quote && !end_quote)
        {
            start_quote = false;
            end_quote = true;
            temp.push(commands.front());
            commands.pop();
        }
        
        // deletes everything after a # if it isn't within two quotes
        else if(!start_quote && commands.front().find("#") != string::npos)
        {
            while(!commands.empty())
                commands.pop();
        }
        
        else
        {
            temp.push(commands.front());
            commands.pop();
        }
    }
    
    // get the commentless queue stored in temp back into commands
    commands = temp;
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
            // check if there's quotation marks to indicate where something
            // is just gonna be text and not a command
            if(commands.front() == "\"")
            {
                makeString(commands, cmd);
            }
            
            // checks for the test command being passed in
            else if(cmd.empty() && (commands.front() == "test" || commands.front() == "["))
            {
                *failed = test(commands, cmd);
                // the test command has already been taken care of by this point 
                // empty the cmd vector
                for(unsigned i = 0; i < cmd.size(); ++i)
                    delete cmd.at(i);
                
                // returning with an empty cmd vector shows that the command
                // has already been taken care of and the program can move on
                vector<char*> nocmd;
                return nocmd;
            }
            
            // command making stops at operators or )'s
            else if(commands.front() == ")")
            {
                done = true;
            }
            
            // default way of handling commands
            else
            {
        		char *arg = new char[commands.front().size()];
        		strcpy(arg, commands.front().c_str());
        		cmd.push_back(arg);
        		commands.pop();
            }
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
        else if(connector == OR && (*failed && !*group_pass))
            exit(0);
        else if(connector == AND && (!*failed && *group_pass))
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
        if(connector == OR && (!*failed && *group_pass))
            exit(0);
        
        if(connector == AND && (*failed && !*group_pass))
            exit(1);
        
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
		    {
		        *failed = false;
		        *group_pass = true;
		    }
		        
		    // anything else means it failed in some way
		    else
		    {
		        *group_pass = false;
		        *failed = true;
		    }
		}
    }
}

bool isTestFlag(string s)
{
    string flags = "-e -f -d";
    if(flags.find(s) != string::npos)
        return true;
    return false;
}

// valid flags: -e: exists
//              -f: is a regular file
//              -d: is a directory
bool test(queue<string> &commands, vector<char*> &cmd)
{
    // keeps track of whether or not the test flag was passed
    // symbolically
    bool symbolic = false;
    
    // keeps track of which flag the user passed in
    // defaults to -e if no flag is passed in
    string flag = "-e";
    
    // struct to hold all the info about a path
    struct stat buffer;
    
    // get rid of the "test" or [
    if(commands.front() == "[")
        symbolic = true;
    commands.pop();
    
    // checking for flags the user may have passed in
    if(isTestFlag(commands.front()))
    {
        flag = commands.front();
        commands.pop();
    }
    
    // checking for invalid flags being passed in
    // if invalid, remove everything until the next command
    if(commands.front().at(0) == '-')
    {
        cout << "Error: invalid flag(s)" << endl;
        while(!isConnector(commands.front()))
            commands.pop();
        
        return true;
    }

    // stat() doesn't like paths starting with '/' so we're gonna
    // need to delete that
    if(commands.front().at(0) == '/')
        commands.front().erase(0, 1);
    
    // all the cmd vector needs to hold in here is the
    // one char* containing the path to check
    char *path = new char[commands.front().size()];
    strcpy(path, commands.front().c_str());
    cmd.push_back(path);
    commands.pop();
    
    // if test was called symbolically, then we have to
    // get rid of the close ]
    if(symbolic)
        commands.pop();
        
    // int to keep track of if stat failed or not    
    int status = stat(cmd.front(), &buffer);
    
    // checks if stat failed or not
    if(status < 0)
    {
        perror("could not get file status");
        return true;
    }
    
    if(flag == "-e")
        return false;
    
    // check the flag and run the appropriate checks
    if(flag == "-f")
    {
        if(S_ISREG(buffer.st_mode))
            return false;
     
        return true;       
    }
    
    if(flag == "-d")
    {
        if(S_ISDIR(buffer.st_mode))
            return false;
        
        return true;
    }
    
    return true;
}

void parseInput(string input, queue<string> &commands)
{
    char_separator<char> sep(" ;()[]\"", ";()[]\"", keep_empty_tokens);
	tokenizer< char_separator<char> >  cmds(input, sep);
	tokenizer< char_separator<char> >::iterator it = cmds.begin();
	for(; it != cmds.end(); ++it)
	    if(*it != "")
	        commands.push(*it);
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
        if(commands.front() == ")")
        {
            *failed = false;
            commands.pop();
            return;
        }
        
        // this should always run at least once since
        // the queue starts with a ;
        if(!commands.empty() && commands.front() != "(" && (isConnector(commands.front()) >= 0))
        {
            connector = isConnector(commands.front());
            commands.pop();
        }
        
        // if we run into a parentheses, we got a whole lot to do now
        if(commands.front() == "(")
        {
            if(connector == SEMI_COLON)
            {
                commands.pop();
                runCommands(commands);
            }
            
            else if(connector == OR && !*group_pass)
            {
                commands.pop();
                runCommands(commands);
            }
            
            else if(connector == AND && *group_pass)
            {
                commands.pop();
                runCommands(commands);
            }
            else
            {
                while(commands.front() != ")")
                    commands.pop();
                commands.pop();
            }
        }
        
        if(commands.empty())
            break;
        
        if(isConnector(commands.front()) >= 0  || commands.front() == ")")
            continue;
        
        // this would only fail the first time if the queue
        // was empty
        if(!commands.empty())
        {
            cmd = makeCommand(commands);
            if(!cmd.empty())
                run(cmd, connector);
        }
        
        // deallocates memory
        if(!cmd.empty())
        {
            for(unsigned i = 0; i < cmd.size(); ++i)
                delete cmd.at(i);
        }
    }
}
