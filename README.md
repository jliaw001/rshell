##CS100 Assignment 1: Command Shell
rshell is a custom made command shell

##How to Run
1. git clone https://github.com/jliaw001/rshell.git
2. cd rshell
3. git checkout hw1
4. make
5. bin/rshell

##Dependencies
rshell uses the Tokenizer class from the boost library

##Known Bugs
- putting a space before a command causes an error.
	-i.e. " echo hello world"
- unable to handle commands that don't "just work" with execvp()
- the exit command will exit no matter what comes before it (except if it's a comment).
	-i.e. "echo hello world || exit" would still exit

##Test Directory:
1. single_command.script: tests single commands
2. multi_command.script: tests commands with ;, &&, or ||
3. commented_command.script: tests commands with comments (#)
4. exit.script:	tests exit and commands with exit
