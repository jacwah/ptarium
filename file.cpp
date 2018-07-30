#include "file.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 256

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
	const int NumFields = 12;
	char *Fields[NumFields];
	int Lineno = 0;
	World->Count = 0;

	while (GetLine(Line, File)) {
		Lineno++;
		if (*Line == '#')
			continue;

		int GotFields = GetCsvFields(Fields, Line, NumFields);
		if (GotFields != NumFields) {
			printf("Line %d: expected %d fields, got %d\n", Lineno, NumFields, GotFields);
			continue;
		}

		strncpy(World->Name[World->Count], Fields[0], MAX_NAME);

		sscanf(Fields[ 1], "%f", &World->Color[World->Count].x);
		sscanf(Fields[ 2], "%f", &World->Color[World->Count].y);
		sscanf(Fields[ 3], "%f", &World->Color[World->Count].z);

		if (!sscanf(Fields[4], "%f", World->Radius + World->Count)) {
			printf("Line %d, field %d: expected float\n", Lineno, 1);
			continue;
		}
		sscanf(Fields[ 5], "%f", &World->Mass[World->Count]);
		sscanf(Fields[ 6], "%f", &World->Position[World->Count].x);
		sscanf(Fields[ 7], "%f", &World->Position[World->Count].y);
		sscanf(Fields[ 8], "%f", &World->Position[World->Count].z);
		sscanf(Fields[ 9], "%f", &World->Velocity[World->Count].x);
		sscanf(Fields[10], "%f", &World->Velocity[World->Count].y);
		sscanf(Fields[11], "%f", &World->Velocity[World->Count].z);
		World->Count++;
	}
}
