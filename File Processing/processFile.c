/*******************************************************************************
* Author:      Andrew Seligman
* Date:        February 11, 2014
* File:        processFile.c
* Purpose:     This program accepts a person's last name as a command line
*               argument and attempts to find information on that person
*               contained within the file people.dat. If the name is found, the
*               person's first name, last name, birthdate, and age (in years and
*               months) is printed.
*******************************************************************************/

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct NameRec
{
	struct NameRec* next;
	char firstName[20];
	char lastName[20];
	int day, month, year;
};

FILE* openFile();
void readFile(FILE*, struct NameRec*);
struct NameRec* search(struct NameRec*, char*);
void printRecord(struct NameRec*);

int main(int argc, char** argv)
{
	FILE* file;
	struct NameRec Head;
	struct NameRec* Current;
	char* lastName;
	
	if(argc != 2)
	{
		printf("Usage: a.out [Last Name]\n");
	}
	else if((file = openFile("people.dat")) == NULL)
	{
		perror("An error occurred");
	}
	else
	{
		printf("File opened successfully\n");
		
		//store name given in command line argument
		lastName = (char*) malloc(sizeof(argv[1]));
		lastName = strcpy(lastName, argv[1]);
		
		//read in contents of file
		readFile(file, &Head);
		
		//check if person exists
		if(Current = search(&Head, lastName))
		{
			printf("Record found:\n");
			printRecord(Current);
		}
		else
		{
			printf("No record for %s\n", lastName);
		}
	}
	
	return 0;
}

/*******************************************************************************
* Function name:  openFile
*                                                                             
* Description:    Attempt to open given file name
*                                                                             
* Parameters:     char* fileName - IMPORT - name of file to open
*                                                                             
* Return Value:   pointer to file or null pointer if operation fails
*******************************************************************************/
FILE* openFile(char* fileName)
{
	return fopen(fileName, "r");
}

/*******************************************************************************
* Function name:  readFile
*                                                                             
* Description:    Stores the contents of the given file in a linked list
*                                                                             
* Parameters:     FILE* file - IMPORT - pointer to file to read
*                 struct NameRec* current - IMPORT/EXPORT - beginning of the
*                     linked list
*                                                                             
* Return Value:   none
*******************************************************************************/
void readFile(FILE* file, struct NameRec* current)
{
	char line[50];
	char* loc;
	char* loc2;
	char* start;
	struct NameRec* NewNameRec;
	
	fgets(line, 50, file);
	
	while(!feof(file))
	{
		//store first name
		loc = strtok(line, ",");
		loc = strtok(NULL, ",");
		strncpy(current->firstName, line, loc - line);
		
		//store last name
		loc2 = strtok(NULL, ",");
		strncpy(current->lastName, loc, loc2 - loc);
		
		//store month, day, and year
		sscanf((char*)loc2, "%d%*c%d%*c%d", &(current->month), &(current->day), &(current->year));
		
		fgets(line, 50, file);
		
		if(feof(file))
		{
			printf("Finished reading\n");
			return;
		}
		
		//create a NameRec for next record
		NewNameRec = (struct NameRec*) malloc(sizeof(struct NameRec));
		current->next = NewNameRec;
		current = NewNameRec;
	}
}

/*******************************************************************************
* Function name:  search
*                                                                             
* Description:    Searches the list of people for a given last name
*                                                                             
* Parameters:     struct NameRec* Head - IMPORT - beginning of the list
*                 char* name - IMPORT - name to search for
*                                                                             
* Return Value:   pointer to the NameRec that matches the name if found 
*                  or null pointer if not found
*******************************************************************************/
struct NameRec* search(struct NameRec* Head, char* name)
{
	struct NameRec* current = Head;
	while(current != NULL)
	{
		//debug
		//printf("%s,%s,%d/%d/%d\n", current->firstName, current->lastName, 
		//		current->month, current->day, current->year);
		if(!strcmp(current->lastName, name))
			return current;
		current = current->next;
	}
	
	return current;
}

/*******************************************************************************
* Function name:  printRecord
*                                                                             
* Description:    Prints the information held in a record and calculates the 
*                  age of the person
*                                                                             
* Parameters:     struct NameRec* Record - IMPORT - record to print
*                                                                             
* Return Value:   none
*******************************************************************************/
void printRecord(struct NameRec* Record)
{
	time_t timer;
	struct tm* birthday;
	double timeDif;
	int years, months;
	char printMonth[3];
	
	printf("First Name: %s\n", Record->firstName);
	printf("Last Name: %s\n", Record->lastName);
	
	//create tm struct with birth date
	birthday = localtime(&timer);
	birthday->tm_year = Record->year - 1900;
	birthday->tm_mon = Record->month - 1;
	birthday->tm_mday = Record->day;
	
	time(&timer);
	//calculate age in seconds
	timeDif = difftime(timer, mktime(birthday));
	years = ((int)timeDif)/31556940;
	months = (((int)timeDif)%31556940)/2592000;
	
	//get birth month in string form
	sscanf(asctime(birthday), "%*s %s", printMonth);
	
	printf("Birthdate: %s %d, %d\n", printMonth, Record->day, Record->year);
	printf("Age: %d years %d months\n", years, months);
}
