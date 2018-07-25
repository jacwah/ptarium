#include "file.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 256

/*
		char *Field = Line;
		while (isspace(*Field)) Field++;
		Field = strtok(Field, ","); 
		Field = strtok(Field, ","); 
*/

static int GetLine(char *Buffer, FILE *File)
{
	int Input;
	int Len = 0;

	while (Input = fgetc(File), Len++ < MAX_LINE-1 && Input != EOF && Input != '\n')
		*Buffer++ = Input;

	*Buffer = '\0';
	return Len - 1;
}

static int GetCsvFields(char *Fields[], char *Str, int MaxFields)
{
	for (int Field = 0; Field < MaxFields; ++Field) {
		if (!Str)
			return Field;
		while (isspace(*Str))
			++Str;
		if (!*Str)
			return Field;
		Fields[Field] = Str;

		char *FieldEnd = strchr(Str, ',');
		char *NextField = NULL;
		if (FieldEnd)
			NextField = FieldEnd + 1;
		else {
			FieldEnd = Str;
			while (*++FieldEnd);
			return Field+1;
		}
		while (isspace(*FieldEnd-1))
			FieldEnd--;
		*FieldEnd = '\0';
		Str = NextField;
	}

	if (*Str)
		return MaxFields + 1;
	else
		return MaxFields;
}

void ReadWorldFile(world *World, FILE *File)
{
	char Line[MAX_LINE];
	char *Fields[9];
	int Lineno = 0;
	World->Count = 0;

	while (GetLine(Line, File)) {
		Lineno++;
		if (*Line == '#')
			continue;

		int NumFields = GetCsvFields(Fields, Line, 9);
		if (NumFields != 9) {
			printf("Line %d: expected 9 fields, got %d\n", Lineno, NumFields);
			continue;
		}

		strncpy(World->Name[World->Count], Fields[0], MAX_NAME);
		
		if (!sscanf(Fields[1], "%f", World->Radius + World->Count)) {
			printf("Line %d, field %d: expected float\n", Lineno, 1);
			continue;
		}
		sscanf(Fields[2], "%f", &World->Mass[World->Count]);
		sscanf(Fields[3], "%f", &World->Position[World->Count].x);
		sscanf(Fields[4], "%f", &World->Position[World->Count].y);
		sscanf(Fields[5], "%f", &World->Position[World->Count].z);
		sscanf(Fields[6], "%f", &World->Velocity[World->Count].x);
		sscanf(Fields[7], "%f", &World->Velocity[World->Count].y);
		sscanf(Fields[8], "%f", &World->Velocity[World->Count].z);
		World->Count++;
	}
}
