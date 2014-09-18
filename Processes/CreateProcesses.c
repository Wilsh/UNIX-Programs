/*******************************************************************************
* Author:      Andrew Seligman
* Date:        April 8, 2014
* File:        CreateProcesses.c
* Purpose:     This program accepts an argument N, where N is an integer between
*               1 and 12, and creates N child processes. Each child process runs
*               an empty for loop [its PID] number of times, and information 
*               about each running process is printed.
*******************************************************************************/

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

void createChildren(long);

int main(int argc, char** argv)
{
    long n;
    char* end;
    char* usage = "Usage: %s N (where N is an integer between 1 and 12)\n";
	
	if(argc != 2)
	{
		printf(usage, argv[0]);
        return 1;
    }
    
    n = strtol(argv[1], &end, 10);
    
    if(n < 1 || n > 12)
    {
        printf(usage, argv[0]);
        return 1;
    }
    
    createChildren(n);
	
	return 0;
}

/*******************************************************************************
* Function name:  createChildren
*                                                                             
* Description:    Creates a specified number of child processes and prints 
*                  information about the process IDs as a process is created
*                  or exits. Each child process runs an empty for loop [its PID]
*                  number of times.
*                                                                             
* Parameters:     long n - IMPORT - number of processes to create
*                                                                             
* Return Value:   none
*******************************************************************************/
void createChildren(long n)
{
    int i, idx, status;
    int childrenLeft = n;
    pid_t pid;

    printf("Parent PID = %d creating %d processes\n", getpid(), n);
    
    /* The following code has been modified from 
    http://faculty.kutztown.edu/frye/secure/CSC352/Examples/waitpid_ex.c
    Credit for the original code belongs to Dr. Frye */
        
    for(i = 0; i < n; i++)
    {
        // Create a child process
        if ((pid = fork()) < 0)
        {
            perror("fork");
            exit(1);
        }

        if (pid == 0)
        {
            // Child process
            int count, childPID;
            childPID = getpid();
            printf("Child process %d created: PID = %d\n", i+1, childPID);
            for(count = 1; count <= childPID; count++);
            exit(15);
        }
    }

    while(childrenLeft > 0)
    {
        // Parent process
        pid = wait(&status);

        if (WIFEXITED(status))
            printf("PID %d exits: %d\n", pid, WEXITSTATUS(status));
        else if (WIFSTOPPED(status))
            printf("PID %d stopped by: %d\n", pid, WSTOPSIG(status));
        else if (WIFSIGNALED(status))
            printf("PID %d killed by: %d\n", pid, WTERMSIG(status));
        else
            perror("Waitpid");
            
        childrenLeft--;
    }
}
