//Goal: Make a program that takes a .gpx file and lets the user shift the time
//Useful for when tracking software was bugged or when reusing activity

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#define MAXLENGTH 1000
#define MAXLINES 10000

int findTimeLines(FILE *fileToSearch, char *timeLines[], int maxLineLength, int maxNumLines);
char *removeLeading(char stringToClean[], char firstChar);
char *removeTag(char stringToClean[], char tagToRemove[]); 

int main(int argc, char *argv[]) {
	char filePath[MAXLENGTH];
	char *goodInput;
	
	//Handle user input for filename
	switch(argc) {
		case 1: //ask for file, ask for timeshift
			printf("What is the filename? Please place it in the same directory as this program, or pass the location to the program when running the program.\n");
			do {
				goodInput = fgets(filePath, MAXLENGTH, stdin);
			} while (*goodInput == '\n');
			filePath[strlen(filePath) - 1] = '\0';
			//printf("The entered filepath was: %s\n", filePath);
			break;
		case 2: 
			strcpy(filePath, argv[1]);
			break;
		default: //output error message
			printf("Usage: either run the program with no arguments or pass the filename as the only argument.\n");
			break;
	}
	
	//Open the file
	FILE *fileGPX;	
	fileGPX = fopen(filePath, "r+");	
	if (fileGPX == NULL) {
		printf("Opening file at location: \"%s\" failed. Please check file location and try again.\n", filePath);
		return -1;
	}
	printf("File opened successfully!\n");
	
	//Parse the contents for the first matching date/time format string
	//Extract that and display to the user
	char *timeLines[MAXLINES];
	int numLines = findTimeLines(fileGPX, timeLines, MAXLENGTH, MAXLINES);
	
	// //Get the timeshift
	printf("When prompted, enter the timeshift that you wish to execute.\n");
	printf("Use a negative sign to denote moving backwards in time.\n");
	printf("The program will not deal with overflow (eg. 25 hour shifts).\n");
	
	// printf("Enter the number of years to shift: ");
		// //Get years
	// printf("Enter the number of months to shift: ");
		// //Get months
	// printf("Enter the number of days to shift: ");
		// //Get days
	// printf("Enter the number of hours to shift: ");
		// //Get hours
	// printf("Enter the number of minutes to shift: ");
		// //Get minutes
	// printf("Enter the number of seconds to shift: ");
		// //Get seconds
	
	return 0;
}

int findTimeLines(FILE* fileToSearch, char *timeLines[], int maxLineLength, int maxNumLines) {
	int i = 0;
	char tempLine[maxLineLength];
	char *tempLoc;
	
	while (tempLoc = fgets(tempLine, sizeof(tempLine), fileToSearch)) {
		tempLoc = removeLeading(tempLine, '<');		
		//printf("findTimeLines: %s", tempLoc);
				
		if (strncmp(tempLoc, "<time>", strlen("<time>")) == 0) {
			tempLoc = removeTag(tempLoc, "<time>");
			timeLines[i++] = tempLoc;
			printf("line %d: %s\n", i - 1, timeLines[i - 1]);
			if (i - 1 == maxNumLines) {
				printf("findTimeLines: too many lines!\n");
				return i - 1;
			}
		}
	}
	
	printf("findTimeLines: found %d <time> lines!\n", i);
	
	return i;	
}

//Removes everything up to firstChar
char *removeLeading(char stringToClean[], char firstChar) {
	int c;
	int i = 0;
	
	//Index until reaching the firstChar
	while ((c = (stringToClean[i++])) != firstChar) {
		;
	} 
	
	//Return the index of the firstChar
	return &stringToClean[--i];
}

//Usage: removes <tag> from start and </tag> end of line
char *removeTag(char stringToClean[], char tagToRemove[]) {
	//Truncate the end of the string by terminating after the last non-tag character
	stringToClean[strlen(stringToClean) - (strlen(tagToRemove) + 2)] = '\0';

	//Truncate the beginning of the string by pointing to the first non-tag character
	return &stringToClean[strlen(tagToRemove)];
}

//Open file from command line argument

//Read contents line-by-line
	//Compare string to find something that matches date format
	//Output first instance to user
	//Request what user would like first instance to be
	//Compare goal date/time and current date/time
		//Do some mod math to figure out YEAR/MONTH/DATE HH:MM:SS change
	//Rewrite each line with correct date

//Close file and output success message