#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <errno.h>

#ifdef _WIN32

#include <windows.h>
#define chdir _chdir

#else
#include <unistd.h>

#endif

#define true 1
#define MAX_LENGTH 1024
#define DELIMS " \t\r\n"

int count;
extern int errno;
char currentDir[MAX_LENGTH];

//Linked list data struct to store commands
typedef struct Node{
  char* command;
  char* args;
  struct Node *next;
} Node;

Node *head = NULL;

/*Command Supported Functions*/
void openShell();
void movetodir(char *arg);
void whereami();
void replay(char *arg, FILE *commands);
void history(char *arg);
void byebye(FILE *commands);
void start(char *program, char *paramaters);
void background(char *program, char *paramaters);
void dalekPID(char *arg);

/*Linked List implementation and Other Supporting Functions*/
void print_reverse(Node *head);
Node *createNode(char* command, char* args);
void insert(char* command, char* args);
void deleteAll();
int dirExists(const char* const path);
void saveHistory(Node *head, FILE *commands);
char* realpath(const char *path, char *resolved_path);


int main(int argc, char *argv[])
{
    openShell();
    
    count = 0;

    pid_t pid;
    char *arg;
    char *token;
    char *parameters;
    char *real_path;
    char line[MAX_LENGTH];

    FILE *commands = fopen("commands.txt", "w");   //file to save history

    getcwd(currentDir, sizeof(currentDir));  

    while(true) {

        printf("# ");
        
        fgets(line, MAX_LENGTH, stdin);
        
        token = strtok(line, DELIMS); // command parsing

        if(token != NULL) 
        {
            errno = 0;
                
            if (!strcmp(token, "movetodir")) 
            {   
                arg = strtok(NULL, DELIMS); //gets argument from terminal

                movetodir(arg);
            }            
            else if (!strcmp(token, "whereami")) 
            {
                whereami();
            }
            else if (!strcmp(token, "replay")) {

                arg = strtok(NULL, DELIMS); //gets argument from terminal
                
                replay(arg, commands);
            }     
            else if (!strcmp(token, "history")) {

                arg = strtok(NULL, DELIMS); //gets argument from terminal

                history(arg);
            }           
            else if (!strcmp(token, "byebye")) 
            {      
                byebye(commands);
            }
            else if (!strcmp(token, "start")) 
            {
                arg = strtok(NULL, DELIMS); //gets program from terminal
                
                if(arg != NULL) {
                    parameters = strtok(NULL, "\n"); //gets parameter from terminal
                    start(arg, parameters);   
                }        
                else
                    printf("Start requires program"); 
            }
            else if (!strcmp(token, "background")) 
            {
                arg = strtok(NULL, DELIMS); //gets program from terminal
                
                if(arg != NULL) {       
                    parameters = strtok(NULL, "\n"); //gets parameter from terminal       
                    background(arg, parameters);   
                }
                else
                    printf("Background requires program");  
            }    
            else if (!strcmp(token, "dalek")) 
            {
                arg = strtok(NULL, DELIMS); //gets argument from terminal

                dalekPID(arg); 
            }                           
            else
                system(token);

            if (errno) perror("Command failed");
        }
            
    }

    fclose(commands);

    return 0;
}


void openShell() { 
    puts("\n***WELCOME TO MY SHELL***"
        "\nList of Commands supported:"
        "\n# movetodir directory"
        "\n# whereami"
        "\n# history [-c]"
        "\n# byebye"
        "\n# replay number"
        "\n# start program [paramaters]"
        "\n# byebye"
        "\n# background program [parameters]"
        "\n# dalek PID\n\n"
    ); 
    return; 
} 

// Command checks whether the specified directory exists and it is a directory
void movetodir(char *arg){

    char tempArg[MAX_LENGTH];
    strcpy(tempArg, arg);
  
    char tempDir[MAX_LENGTH];

    if (!arg) {
        insert("movetodir", NULL);
        fprintf(stderr, "cd missing argument.\n");
    }
    else 
    {    
        insert("movetodir", tempArg);
       
        getcwd(tempDir, sizeof(currentDir)); // gets current Directory        
        strcat(tempDir, "/");
        strcat(tempDir, arg);

        char *path = realpath(tempDir, NULL); // realpath comfirmation
                    
        if(path == NULL)
        {   
            printf("Specified current directory does not exist\n");
        }
        else
        {   
            if(dirExists(path) == 1) { //If directory exists
                strcpy(currentDir, tempDir);
                printf("%s\n", currentDir); 
            }
            else
                printf("Specified current directory does not exist\n");
        }
    }       
}

// Displays the value of the currentdir variable
void whereami(){
    insert("whereami", NULL);   
    printf("%s\n", currentDir);  
}

// Re-executes the command labeled with number in the history
void replay(char *arg, FILE *commands){
    
    int i = 1;
    Node *temp = head;
      
    char tempNum[MAX_LENGTH];
    strcpy(tempNum, arg);
  
    int number = atoi(arg);

    while(temp != NULL && i <= number){
        temp = temp->next;
        i++;
    }

    if(temp == NULL)
        printf("Command doesn't exist for this number\n");
    else {
        char *history_cmd = temp->command;
        char *history_arg = temp->args;
                    
        printf("Command in the history is: %s %s\n\n", history_cmd, (temp->args == NULL) ? " " : temp->args);

        if(!strcmp(history_cmd, "movetodir"))
            movetodir(history_arg);

        else if(!strcmp(history_cmd, "whereami"))
            whereami();

        else if(!strcmp(history_cmd, "replay"))
            replay(history_arg, commands);

        else if(!strcmp(history_cmd, "history"))
            history(NULL);

        else if(!strcmp(history_cmd, "byebye"))
            byebye(commands);

        else if(!strcmp(history_cmd, "start"))
        {

            history_cmd =  strtok(temp->args, " ");
            history_arg =  strtok(NULL, "\n");

            start(history_cmd, history_arg); 
        }
        else if(!strcmp(history_cmd, "background"))
        {
            history_cmd =  strtok(temp->args, " ");
            history_arg =  strtok(NULL, "\n");

            background(history_cmd, history_arg); 
        }
        else if(!strcmp(history_cmd, "dalek"))
            dalekPID(NULL);

        else 
            printf("Command failed");
    }

    insert("replay", tempNum); 
}

// Prints out the recently typed commands (with their arguments)
// [-c] clears the list of recently typed commands
void history(char *arg){
    Node *temp = head;

    if(!arg) {
        insert("history", NULL);
        print_reverse(temp);
        count = 0;
    }
    else if(strcmp(arg, "-c") == 0)
    {
        deleteAll();
    }
    else
        printf("Invalid argument(s) for history command\n");

}

// Terminates the mysh shell - history file is saved before termination-  
void byebye(FILE *commands) {
    if(head != NULL)
        insert("byebye", NULL);
    saveHistory(head, commands);            
    exit(0);
}

// Execute "program" + "parameters" by using fork(), exec(), waitpid() calls
void start(char *program, char *parameters){
    
    int k = 3;
    pid_t pid;
    char *temp;
    char tparam[MAX_LENGTH];
    char tempStart[MAX_LENGTH];
    char *path = realpath(program, NULL);
    
    strcpy(tempStart, program);
    
    // Checks if specified program executable and accessible
    if(path == NULL && (dirExists(path) != 1)) 
    {   
        insert("start", program);
        printf("Specified program does not exist or cannot be executed\n");
        return;
    }
    else
    {   
        if(parameters != NULL) // program with parameters
        {    
            strcpy(tparam, parameters);
         
            pid = fork();

            if(pid < 0) //creation of child proccess was unsuccessful
            {
                perror("Error");
                return;
            }            
            else if(pid == 0) //child process
            {
                //Following code creates and store program + parameters by using parsing method
                temp = malloc(sizeof(char) * (strlen(parameters) + 1));
                strcpy(temp, parameters);

                while((temp = strchr(temp, ' ')) != NULL) {
                    k++;
                    temp++;
                }

                char *arr[k];
                arr[0] = program;
                arr[k - 1] = NULL;

                for(int i = 1; i < k - 1; i++){
                    arr[i] = strtok(parameters, " ");
                    parameters = strtok(NULL, "\n"); 
                }
                execv(arr[0], arr);
            }
            else{
                int status;
                (void)waitpid(pid, &status, 0);
            }

            strcat(tempStart, " ");
            strcat(tempStart, tparam);
            insert("start", tempStart); 
        } 
        else // program without parameters 
        {
            pid = fork(); 
            
            if(pid < 0) 
            {
                perror("Error");
                return;
            }            
            else if(pid == 0) 
            {
                char *arr[] = {program, NULL};
                execv(program, arr);
            }
            else{
                int status;
                (void)waitpid(pid, &status, 0);
            }

            insert("start", tempStart);       
        }
    }
}

// Similar to start command 
// Execute "program" + "parameters" by using fork(), exec(), waitpid() calls
// Immediately prints the PID of the program it started, and returns the prompt
void background(char *program, char *parameters){
    
    int k = 3;
    pid_t pid;
    char *temp;
    char tempParam[MAX_LENGTH];
    char tempProg[MAX_LENGTH];
    
    strcpy(tempProg, program);
    
    pid_t currpid = getpid();

    printf("\nParent PID: %d\n", currpid);
   
    if(parameters != NULL)
    {   
        strcpy(tempParam, parameters);

        pid = fork();

        if(pid < 0) //creation of child proccess was unsuccessful
        {
            perror("Error");
            return;
        }            
        else if(pid == 0) //child process
        {
            currpid = getpid();
            printf("Child PID: %d\n\n", currpid);

            //Following code creates and store program + parameters by using parsing method
            temp = malloc(sizeof(char) * (strlen(parameters) + 1));
            strcpy(temp, parameters);

            while((temp = strchr(temp, ' ')) != NULL) {
                k++;
                temp++;
            }

            char *arr[k];
            arr[0] = program;
            arr[k - 1] = NULL;

            for(int i = 1; i < k - 1; i++){
                arr[i] = strtok(parameters, " ");
                parameters = strtok(NULL, "\n"); 
            }

            execv(arr[0], arr);
            
        }
        else{
             int status;
            (void)waitpid(pid, &status, 0);
        }

        strcat(tempProg, " ");
        strcat(tempProg, tempParam);
        insert("background", tempProg); 
    } 
    else 
    {   
        insert("background", tempProg);   
        
        pid = fork(); // start of child process
        
        if(pid < 0) //creation of child proccess was unsuccessful
        {
            perror("Error");
            return;
        }            
        else if(pid == 0) //child process
        {
            currpid = getpid();
            printf("Child PID: %d\n\n", currpid);

            char *arr[] = {program, NULL};
            execv(program, arr);
        }
        else{
            int status;
            (void)waitpid(pid, &status, 0);
             
        }    
        insert("background", tempProg);  
    }
    
}

// Immediately terminate the program with the specific PID 
void dalekPID(char *arg) {

    insert("dalek", arg); 

    pid_t pid = kill(atoi(arg), SIGKILL);

    if(!pid)
        printf("Killend\n");
    else
        printf("Killing unsuccessful\n");
}

/*Linked List implementation and Other Supporting Functions*/
void saveHistory(Node *head, FILE *commands){
    
    Node *temp;

	if (head == NULL) {
        fprintf(commands, "No current history");
        return;
    }
    else {   
        for (temp = head; temp != NULL; temp = temp->next)
            fprintf(commands, "%d: %s %s\n", count++, temp->command, (temp->args == NULL) ? " " : temp->args);
    }
}

void print_reverse(Node *temp)
{	
	if (temp == NULL)
    	return;
    else{
        print_reverse(temp->next);
        printf("%d: %s %s\n", count++, temp->command, (temp->args == NULL) ? " " : temp->args);
    }
}

Node *createNode(char* command, char* args)
{
	Node *temp = malloc(sizeof(Node));

	if (temp == NULL)
	{
		fprintf(stderr, "Error: Out of bounds in createNode()\n");
		exit(1);
	}
    
    temp->command = malloc(sizeof(char) * 100);
    temp->args = malloc(sizeof(char) * 100);

	temp->command = command;
    if(args == NULL) temp->args = args;
    else strcpy(temp->args, args);
    temp->next = NULL;
	return temp;
}

void insert(char* command, char* args)
{
    Node *t;
    Node *temp = createNode(command, args);

	if(head == NULL)
    {
        head = temp;
        return;
    }

    t = head;
    while(t->next != NULL)
        t = t->next;

	t->next = temp;
    return;
}

void deleteAll()
{
	if(head == NULL)
        return;
    else { 
        Node *temp;

        while(head != NULL)
        {
            temp = head;
            head = head->next;

            free(temp);
        }
    }

}

int dirExists(const char* const path)
{
    struct stat info;

    int statRC = stat( path, &info );
    if( statRC != 0 )
    {
        if (errno == ENOENT)  { return 0; } 
        if (errno == ENOTDIR) { return 0; } 
        return -1;
    }

    return ( info.st_mode & S_IFDIR ) ? 1 : 0;
}
