# Shell-in-C

C program called mysh replaces the command shell in Unix. 

#  movetodir directory

Command checks whether the specified directory exists and it is a directory. If it exists, the currentdir variable should be assigned this value. If the specified directory does not exist, the command should write an error message and the currentdir variable should not be changed.
Note: chdir() function call can be used!!! 

# whereami

Prints the value of the currentdir variable. 

# history [-c]

Without the parameter, prints out the recently typed commands (with their arguments), in reverse order, with numbers. For instance, it can print:
 
0: history
1: movetodir /etc
2: movetodir /eetc

 If “-c” is passed, it clears the list of recently typed commands. 
 
# byebye

Terminates the mysh shell (and it should save the history file). 

# replay number 

Re-executes the command labeled with number in the history 

# start program [parameters]

The argument “program” is the program to execute. If the argument starts with a “/” (such as /usr/bin/xterm, the shell should interpret it as a full path. Otherwise, the program will be interpreted as a relative path starting from the current directory. 

The program will be executed with the optional “parameters”. It uses fork() + exec() to start the program with the corresponding parameters, and waits until the program terminates (use the waitpid() call). 
For instance

     start /usr/bin/xterm –bg green

would bring up a terminal with a green background. The prompt would not return until the terminal is closed.

Display an error message if the specified program cannot be found or cannot be executed.  

# background program [parameters]

It is similar to the run command, but it immediately prints the PID of the program it started, and returns the prompt. 

# dalek PID

Immediately terminate the program with the specific PID (presumably started from this command line interpreter). Use the kill() function call to send a SIGKILL signal to the program. Display success or failure. 
