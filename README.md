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
- putting a space before a command causes an error
	i.e. " echo hello world"
- unable to handle commands that don't "just work" with execvp()

##Test Directory:
1. single_command.sh:	 tests single commands
2. multi_command.sh:	 tests commands with ;, &&, or ||
3. commented_command.sh: tests commands with comments (#)
4. exit.sh:		 tests exit and commands with exit
