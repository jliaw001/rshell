Jason Liaw

##CS100 Assignment: Command Shell
rshell is a custom made command shell

##Design
This program is all contained within a single source file and contains a variety of functions that take care of certain operations needed to get and carry out inputted commands. It is separated into main functions, helper functions, and additional functions. Main functions are those that call upon all the helper functions in order to perform certain larger tasks such as parsing input or running the actual commands. There are a variety of helper functions that are called upon throughout the program to make the code cleaner and more explicit about what exactly certain parts are doing. Finally, the additional functions are custom made commands that a user can pass in to perform a certain action. 
The way global booleans are handled in order to keep track of the completion status of commands could be changed a bit to be more clear about how they will change in response to what happens when a command is performed. Right now, it is a little confusing to follow and could be improved on. The test command itself could also be moved to a separate class that could contain other custom made commands as well and this would allow for an easy way to add more commands to call upon in the main program without having to changed pre-existing code.

##How to Run
1. git clone https://github.com/jliaw001/rshell.git
2. cd rshell
3. git checkout hw2
4. make
5. bin/rshell

##Dependencies
rshell uses the Tokenizer class from the boost library

##Known Bugs
- unable to handle commands that don't "just work" with execvp()
- echoing with quotes can cause some problems when passing in certain characters
i.e. echo "(hello)" prints ( hello )

##Test Directory:
1. single_command.script: tests single commands
2. multi_command.script: tests commands with ;, &&, or ||
3. commented_command.script: tests commands with comments (#)
4. exit.script:	tests exit and commands with exit
5. test_command: tests the custom implemented test command
6. precedence_test: tests the use of the precedence operator ()
