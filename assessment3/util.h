#pragma once
#include <iostream>
#include <stdio.h>

char* read_file(const char* filename)
{
	FILE* f;
	fopen_s(&f, filename, "rb");
	if (f == NULL)
		return NULL;
	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	rewind(f);
	char* bfr = (char*)malloc(sizeof(char) * (size + 1));
	if (bfr == NULL)
		return NULL;
	long ret = fread(bfr, 1, size, f);
	if (ret != size)
		return NULL;
	bfr[size] = '\0';
	return bfr;
}