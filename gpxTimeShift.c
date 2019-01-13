//Goal: Make a program that takes a .gpx file and lets the user shift the time
//Useful for when tracking software was bugged or when reusing activity

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAXLENGTH 1000
#define MAXLINES 10000
#define MAXYEARS 9999
#define MAXMONTHS 12
#define MAXDAYS 31
#define MAXHOURS 23
#define MAXMINUTES 59
#define MAXSECONDS 59
#define GET_TIMEZONE 1
#define NUMSPACES_FIRST 2

struct date {
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;
};

enum DateCat {
	Year, Month, Day, Hour, Minute, Second
};

int strToInt(char inputStr[], int maxPossible);
void subString(char toStr[], char fromStr[], int start, int len);
int strRemoveWS(char strOut[], char strIn[]);
struct date date_get_user(char setTimeZone);
struct date date_interp_file(char fileString[]);
struct date date_diff(struct date *goalDate, struct date *startDate);
struct date date_add(struct date *date1, struct date *date2);
void date_print_to_console(struct date *outputDate, int numLeadingWS);
void date_print_members(struct date *outputDate);
void date_write_file_line(struct date *outputDate, FILE *outputFile, int numLeadingWS); 
int getDateInt(enum DateCat category);
int findTimeLine(char lineContents[], FILE *fileToSearch, FILE *outFile);

int main(int argc, char *argv[]) {
	char filePath[MAXLENGTH];
	char *goodInput;
	
	//Handle user input for file name
	switch(argc) {
		case 1: //ask for file
			printf("What is the file name? Please place it in the same directory as this program, or pass the location to the program when running the program.\n");
			do {
				goodInput = fgets(filePath, MAXLENGTH, stdin);
			} while (*goodInput == '\n');
			filePath[strlen(filePath) - 1] = '\0';
			break;
		case 2: //copy file path from argument
			strcpy(filePath, argv[1]);
			break;
		default: //output error message with instruction
			printf("Usage: either run the program with no arguments or pass the file name as the only argument.\n");
			break;
	}
	
	//Open the file
	FILE *fileGPX;	
	fileGPX = fopen(filePath, "r");	
	if (fileGPX == NULL) {
		printf("Opening file at location: \"%s\" failed. Please check file location and try again.\n", filePath);
		return -1;
	}
	
	//Get the time zone
	struct date timeZone = date_get_user(GET_TIMEZONE);
	
	//Get the time shift
	printf("At what time would you like the file to start? All times will be shifted accordingly.\n");
	struct date timeGoal = date_get_user(0);
	
	//Account for time zone
	struct date timeGMT = date_diff(&timeGoal, &timeZone);
	
	//Read the file to figure out what the time shift should be
	char currentLine[MAXLENGTH];
	//Get the first time line only, it has 2 spaces as of 2019
	while (findTimeLine(currentLine, fileGPX, NULL) != NUMSPACES_FIRST) { }
	struct date timeStart = date_interp_file(currentLine);	
	struct date timeShift = date_diff(&timeGMT, &timeStart);
	
	//Prepare containers for writing lines
	struct date timeWrite, timeLine;
	int numReplaced = 0;
	int numSpaces = 0;
	rewind(fileGPX);
	
	//Create the output file by appending input name
	FILE *fileOut;	
	char fileOutPath[MAXLENGTH];
	//Erase ".gpx" from end of input name
	subString(fileOutPath, filePath, 0, (strlen(filePath) - 4));
	char append[] = "_new.gpx";
	strcat(fileOutPath, append);
	fileOut = fopen(fileOutPath, "w");	
	if (fileOut == NULL) {
		printf("Opening file at location: \"%s\" failed. Please check file location and try again.\n", filePath);
		return -1;
	}
	
	//Go through file line by line and change the times.
	//If numSpaces = -1, that means there are still lines in file, but the current line not a <time> line
	while ((numSpaces = findTimeLine(currentLine, fileGPX, fileOut)) != -1) {
		//Get struct date from <time> line
		timeLine = date_interp_file(currentLine);
		
		//Add the offset to a local copy
		timeWrite = date_add(&timeShift, &timeLine);
		
		//Print the shifted line to the output
		date_write_file_line(&timeWrite, fileOut, numSpaces);
		numReplaced++;
	}
	
	//Close and output success message
	fclose(fileOut);
	fclose(fileGPX);
	printf("Successfully replaced %d <time> lines!\n", numReplaced);
		
	return 0;
}

//Usage: convert a user string to a non-negative integer value, subject to maximum value maxPossible
int strToInt(char inputStr[], int maxPossible) {
	int decimal = 1;
	int tempInt = 0;
	int val, i;
		
	for ((i = (strlen(inputStr) - 1)); i >= 0; --i) {
		if ((val = inputStr[i] - '0') < 10 && val >= 0) {
			tempInt += (val * decimal);
			decimal *= 10;
		}
		else if (inputStr[i] == '\n') {
			;
		}
		else {
			printf("Bad input! Enter non-negative integers only!.\n");
			return -1;
		}
	}
	
	if (tempInt > maxPossible) {
		printf("The number you entered was too large! Max allowed: %d\n", maxPossible);
		return -1;
	}
	
	return tempInt;
}

//Usage: remove white space from lead of strIn and output how many spaces were removed
int strRemoveWS(char strOut[], char strIn[]) {
	char tempStr[MAXLENGTH];
	int numSpaces = 0;
	int i = 0;
	char *tempPtr = strIn;
	char c;
		
	while ((c = strIn[i++]) == ' ') {
		++numSpaces;
	}
	
	tempPtr += (i - 1);
	
	strcpy(strOut, tempPtr);	
	
	return numSpaces;
}

//Usage: write "len" number of characters beginning from "start" in fromStr into toStr
void subString(char toStr[], char fromStr[], int start, int len) {
	char *tempPtr = &fromStr[start];
	strncpy(toStr, tempPtr, len);
	toStr[len] = '\0';
	
	return;
}

//Usage: prompt user to provide a date
//Use setTimeZone = 1 if prompting for a time zone (GMT)
struct date date_get_user(char setTimeZone) {
	struct date tempDate;
	if (setTimeZone) {
		char sign, c;
		
		do {
			printf("Enter the time zone of the activity as prompted.\n");
			printf("Is the time zone + or - from GMT? ");
			sign = fgetc(stdin);
			while ( (c = getchar()) != '\n' && c != EOF ) { }
		} while (sign != '+' && sign != '-');
		
		if (sign == '+') sign = 1;
		else sign = -1;
		
		tempDate.year = 0;
		tempDate.month = 0;
		tempDate.day = 0;
		tempDate.hour = sign * getDateInt(Hour);
		tempDate.minute = sign * getDateInt(Minute);
		tempDate.second = 0;
	}
	else {
		tempDate.year = getDateInt(Year);
		tempDate.month = getDateInt(Month);
		tempDate.day = getDateInt(Day);
		tempDate.hour = getDateInt(Hour);
		tempDate.minute = getDateInt(Minute);
		tempDate.second = getDateInt(Second);
	}
	return tempDate;
}

//Usage: create a struct date from the XML-style line (with white space removed!)
//This may break if file standard changes. Magic numbers are index and length of the field within string
struct date date_interp_file(char fileString[]) {
	struct date tempDate;
	char tempLine[8];
	
	//Year starts at index 6 and span 4 char
	subString(tempLine, fileString, 6, 4);	
	tempDate.year = strToInt(tempLine, MAXYEARS);
	
	//Month starts at index 11 and spans 2 char
	subString(tempLine, fileString, 11, 2);
	tempDate.month = strToInt(tempLine, MAXMONTHS);
	
	//Day starts at index 14 and spans 2 char
	subString(tempLine, fileString, 14, 2);
	tempDate.day = strToInt(tempLine, MAXDAYS);
	
	//Hour starts at index 17 and spans 2 char
	subString(tempLine, fileString, 17, 2);
	tempDate.hour = strToInt(tempLine, MAXHOURS);
	
	//Minute starts at index 20 and spans 2 char
	subString(tempLine, fileString, 20, 2);
	tempDate.minute = strToInt(tempLine, MAXMINUTES);
	
	//Second starts at index 23 and spans 2 char
	subString(tempLine, fileString, 23, 2);
	tempDate.second = strToInt(tempLine, MAXSECONDS);
	
	return tempDate;
}

//Usage: subtract startDate from goalDate
//If converting from local to GMT, put the time zone in startDate
struct date date_diff(struct date *goalDate, struct date *startDate) {
	struct date tempDate;
	
	tempDate.year = goalDate->year - startDate->year;
	tempDate.month = goalDate->month - startDate->month;
	tempDate.day = goalDate->day - startDate->day;
	tempDate.hour = goalDate->hour - startDate->hour;
	tempDate.minute = goalDate->minute - startDate->minute;
	tempDate.second = goalDate->second - startDate->second;
	
	return tempDate;
}

//Usage: add two dates and handle overflow
struct date date_add(struct date *date1, struct date *date2) {
	struct date tempDate;
	int carry = 0;
	tempDate.second = date1->second + date2->second;
	if (tempDate.second > MAXSECONDS) {
		carry = tempDate.second / (MAXSECONDS + 1);
		tempDate.second %= (MAXSECONDS + 1);
	}
	tempDate.minute = date1->minute + date2->minute + carry;
	carry = 0;
	if (tempDate.minute > MAXMINUTES) {
		carry = tempDate.minute / (MAXMINUTES + 1);
		tempDate.minute %= (MAXMINUTES + 1);
	}
	tempDate.hour = date1->hour + date2->hour + carry;
	carry = 0;
	if (tempDate.hour > MAXHOURS) {
		carry = tempDate.hour / (MAXHOURS + 1);
		tempDate.hour %= (MAXHOURS + 1);
	}
	tempDate.day = date1->day + date2->day + carry;
	carry = 0;
	if (tempDate.day > MAXDAYS) {
		carry = tempDate.day / (MAXDAYS + 1);
		tempDate.day %= (MAXDAYS + 1);
	}
	tempDate.month = date1->month + date2->month + carry;
	carry = 0;
	if (tempDate.month > MAXMONTHS) {
		carry = tempDate.month / (MAXMONTHS + 1);		
		tempDate.month %= (MAXMONTHS + 1);
	}
	tempDate.year = date1->year + date2->year + carry;
	if (tempDate.year > MAXYEARS) {
		printf("Error: date overflow! Maximum year allowed: %d\n", MAXYEARS);
		tempDate.year = MAXYEARS;
	}
	return tempDate;
}

void date_print_to_console(struct date *outputDate, int numLeadingWS) {
	//Add the white spaces back in
	while(numLeadingWS-- > 0) {
		printf(" ");
	}
	printf("<time>%04d-%02d-%02dT%02d:%02d:%02dZ</time>\n", outputDate->year, outputDate->month, outputDate->day, outputDate->hour, outputDate->minute, outputDate->second);
	
	return;	
}

//Usage: debug function to show all members of the date struct
void date_print_members(struct date *outputDate) {
	printf("Year: %d\n", outputDate->year);
	printf("Month: %d\n", outputDate->month);
	printf("Day: %d\n", outputDate->day);
	printf("Hour: %d\n", outputDate->hour);
	printf("Minute: %d\n", outputDate->minute);
	printf("Second: %d\n", outputDate->second);
}

//Usage: write XML-style line containing date info to the file with a certain number of leading white spaces
void date_write_file_line(struct date *outputDate, FILE *outputFile, int numLeadingWS) {
	//printf("Writing to file\n");
	//Add the white spaces back in
	while(numLeadingWS-- > 0) {
		fprintf(outputFile, " ");
	}
	//printf("Whitespace printed!\n");
	fprintf(outputFile, "<time>%04d-%02d-%02dT%02d:%02d:%02dZ</time>\n", outputDate->year, outputDate->month, outputDate->day, outputDate->hour, outputDate->minute, outputDate->second);
	
	return;	
}

//Usage: get an integer value for a date member (year, month, etc.)
int getDateInt(enum DateCat category) {
	int tempVal;
	int maxVal;
	char rawInput[MAXLENGTH];
	do {
		switch (category) {			
			case Year: 
				maxVal = MAXYEARS;
				printf("Year:\t");
				break;
			case Month:
				maxVal = MAXMONTHS;
				printf("Month:\t");
				break;
			case Day:
				maxVal = MAXDAYS;
				printf("Day:\t");
				break;
			case Hour:
				maxVal = MAXHOURS;
				printf("Hour:\t");
				break;
			case Minute:
				maxVal = MAXMINUTES;
				printf("Minute:\t");
				break;
			case Second:
				maxVal = MAXSECONDS;
				printf("Second:\t");
				break;
		} 
	} while(!fgets(rawInput, MAXLENGTH, stdin) || (tempVal = strToInt(rawInput, maxVal)) == -1);	
	return tempVal;
}

//Usage: pass in string to store contents of next time line, then search through file
//Removes white spaces and returns number of leading white spaces
int findTimeLine(char lineContents[], FILE *fileToSearch, FILE *outFile) {
	int whiteSpaces = 0;
	char tempLine1[MAXLENGTH];
	char tempLine2[MAXLENGTH];
	//Get the line from the file onto tempLine1
	while (fgets(tempLine1, sizeof(tempLine1), fileToSearch) != NULL) {
		//Remove whitespaces from tempLine1 and store that in lineContents
		whiteSpaces = strRemoveWS(lineContents, tempLine1);
		//Isolate possible <time> tag from lineContents
		subString(tempLine2, lineContents, 0, 6);
		//If that was a <time> tag, return number of leading white spaces
		if (!strcmp(tempLine2, "<time>")) {
			return whiteSpaces;
		}
		//Only do this if output file was passed in
		else if (outFile != NULL) {
			//Add back the white spaces that were stripped
			while(whiteSpaces-- > 0) {
				fprintf(outFile, " ");
			}
			//Print the non-<time> line back to the file
			fprintf(outFile, "%s", lineContents);
		}
	}	
	return -1;
}
