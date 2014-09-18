/*******************************************************************************
* Author:      Andrew Seligman
* Date:        March 11, 2014
* File:        File_dir_info.c
* Purpose:     This program accepts a file or directory as a command line
*               argument and displays information about the given file or
*               directory. If the argument is a filename, the following 
*               information is displayed:
*                   Name
*                   Size (in bytes)
*                   Permission bits
*                   Owner (username and uid)
*                   Last access date and time
*                   Last modification date and time
*               If the argument is a directory name, the program will process
*               the directory and any subdirectories and display a directory
*               hierarchy. For each file encountered, the following information
*               is displayed:
*                   Name
*                   Size (in bytes)
*                   Last access date and time
*******************************************************************************/

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <pwd.h>
#include <dirent.h>

int fileOrDir(struct stat*);
void fileInfo(char*, struct stat*);
char* getUserName(uid_t);
char* getPerms(mode_t, char*);
void dirInfo(char*, char*);
void processFile(char*, char*, struct stat*);
char* nameTrim(char*, char*);

int main(int argc, char** argv)
{    
    struct stat st;
    int modeNum;
    char tabs[50];
    
    strcpy(tabs, "");
	
	if(argc != 2)
	{
		printf("Usage: %s file|directory\n", argv[0]);
        return -1;
	}
    else if(stat(argv[1], &st) == -1)
    {
        perror("Error in main");
        return -1;
    }

    modeNum = fileOrDir(&st);
    
    if(modeNum == -1)
    {
        printf("Error: %s is not a file or directory\n", argv[1]);
    }
    else if(modeNum == 1)
    {
        fileInfo(argv[1], &st);
    }
    else
    {
        printf("===========================================================\n");
        dirInfo(argv[1], tabs);
        printf("===========================================================\n");
    }
	
	return 0;
}

/*******************************************************************************
* Function name:  fileOrDir
*                                                                             
* Description:    Determine if a stat structure contains information about a
*                   file, directory, or other
*                                                                             
* Parameters:     struct stat* st - IMPORT - stat struct to examine
*                                                                             
* Return Value:   1 if a file
*                 2 if a directory
*                -1 if other
*******************************************************************************/
int fileOrDir(struct stat* st)
{
	if(S_ISREG(st->st_mode))
        return 1;
    else if(S_ISDIR(st->st_mode))
        return 2;
    else
        return -1;
}

/*******************************************************************************
* Function name:  fileInfo
*                                                                             
* Description:    Print information about the given file
*                                                                             
* Parameters:     char* fileName  - IMPORT - name of the file
*                 struct stat* st - IMPORT - stat struct containing file info
*                                                                             
* Return Value:   none
*******************************************************************************/
void fileInfo(char* fileName, struct stat* st)
{
    uid_t userID = st->st_uid;
    mode_t perms = st->st_mode;
    char f[FILENAME_MAX];
    char* p;
    
    printf("====================================================\n");
    printf("File Name:\t   %s\n", nameTrim(fileName, f));
    printf("File Size:\t   %d bytes\n", st->st_size);
    printf("Permission Bits:   %s (%04o)\n", getPerms(perms, p), perms & 07777);
    printf("Owner User-ID:\t   %s (%d)\n", getUserName(userID), userID);
    printf("Last Access:\t   %s", ctime(&(st->st_atime)));
    printf("Last Modification: %s", ctime(&(st->st_mtime)));
    printf("====================================================\n");
}

/*******************************************************************************
* Function name:  getUserName
*                                                                             
* Description:    Returns a user's login name for the given user ID
*                                                                             
* Parameters:     uid_t userID - IMPORT - user ID to look up
*                                                                             
* Return Value:   pointer to a character string containing user's name
*******************************************************************************/
char* getUserName(uid_t userID)
{
    struct passwd* pass;
    
    pass = getpwuid(userID);
    
    return pass->pw_name;
}

/*******************************************************************************
* Function name:  getPerms
*                                                                             
* Description:    Returns a 10-character string of file access permissions for
*                   the given file mode
*                                                                             
* Parameters:     mode_t mode - IMPORT - file mode
*                 char* p     - IMPORT/EXPORT - external pointer to the 
*                   character string
*                                                                             
* Return Value:   pointer to a character string containing permissions
*******************************************************************************/
char* getPerms(mode_t mode, char* p)
{
    char perms[10];
    p = perms;
    
    strcpy(perms, "----------");
    
    //owner permissions
    if(mode & S_IRUSR)
        perms[1] = 'r';
    if(mode & S_IWUSR)
        perms[2] = 'w';
    if(mode & S_IXUSR)
        perms[3] = 'x';
    
    //group permissions
    if(mode & S_IRGRP)
        perms[4] = 'r';
    if(mode & S_IWGRP)
        perms[5] = 'w';
    if(mode & S_IXGRP)
        perms[6] = 'x';
    
    //others permissions
    if(mode & S_IROTH)
        perms[7] = 'r';
    if(mode & S_IWOTH)
        perms[8] = 'w';
    if(mode & S_IXOTH)
        perms[9] = 'x';
    
    return p;
}

/*******************************************************************************
* Function name:  dirInfo
*                                                                             
* Description:    Recursively print information about the subdirectories within 
*                   a directory. Files found within a directory are passed to 
*                   processFile() to print file-specific information
*                                                                             
* Parameters:     char* dirName - IMPORT - name of the directory to begin
*                   processing
*                 char* indent  - IMPORT/EXPORT - string used to format 
*                   indentation while printing
*                                                                             
* Return Value:   none
*******************************************************************************/
void dirInfo(char* dirName, char* indent)
{
    int dirLen, modeNum;
    DIR* entries;
    struct stat st;
    struct dirent* directory;
    char currPath[FILENAME_MAX];
    char nextIndent[50];
    
    //set the indent for printing at the next (deeper) level
    strcpy(nextIndent, indent);
    strcat(nextIndent, "  ");
    
    entries = opendir(dirName);
    if(entries == NULL)
        perror("Error in dirInfo (opendir)");
    
    errno = 0;
    directory = readdir(entries);
    if(errno != 0)
        perror("Error in dirInfo (readdir)");

    while(directory != NULL)
    {
        //set the current path for the file/directory
        strcpy(currPath, dirName);
        strcat(currPath, "/");
        strcat(currPath, directory->d_name);
        
        if(strcmp(directory->d_name, ".") != 0 && 
                strcmp(directory->d_name, "..") != 0)
        {
            if(stat(currPath, &st) == -1)
            {
                perror("Error in dirInfo (stat)");
                break;
            }
            
            modeNum = fileOrDir(&st);
            
            if(modeNum == -1)
                printf("Error: %s is not a file or directory\n", 
                        directory->d_name);
            else if(modeNum == 1)
                processFile(currPath, indent, &st);
            else
            {
                printf("%s%s/\n", indent, directory->d_name);
                dirInfo(currPath, nextIndent);
            }
        }

        errno = 0;
        directory = readdir(entries);
        if(errno != 0)
            perror("Error in dirInfo (readdir)");
    }
    
    closedir(entries);
}

/*******************************************************************************
* Function name:  processFile
*                                                                             
* Description:    Print formatted information about the given file
*                                                                             
* Parameters:     char* fileName  - IMPORT - name of the file to process
*                 char* indent    - IMPORT - string used to format indentation 
*                   while printing
*                 struct stat* st - IMPORT - stat struct containing file info
*                                                                             
* Return Value:   none
*******************************************************************************/
void processFile(char* fileName, char* indent, struct stat* st)
{
    char f[FILENAME_MAX];

    printf("%s%-20s\t%8d bytes\t%s", indent, nameTrim(fileName, f),
            st->st_size, ctime(&(st->st_atime)));
}

/*******************************************************************************
* Function name:  nameTrim
*                                                                             
* Description:    Removes leading directory information from a file path
*                                                                             
* Parameters:     char* dir  - IMPORT - full directory path
*                 char* name - IMPORT/EXPORT - external pointer to the 
*                   trimmed file name
*                                                                             
* Return Value:   pointer to a character string containing trimmed file name
*******************************************************************************/
char* nameTrim(char* dir, char* name)
{
    char* loc;
    char* end;
    char temp[FILENAME_MAX];
    
    end = strtok(dir, "/");
    while(end != NULL)
    {
		loc = end;
		end = strtok(NULL, "/");
	}	
    
    strcpy(temp, loc);
    name = temp;
    
    return name;
}
