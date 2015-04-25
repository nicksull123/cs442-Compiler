#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXLINE 1024

bool OpenFiles(const char *ASourceName, const char *AListingName);

void CloseFiles();

char GetSourceChar();

void WriteIndicator(int AColumn);

void WriteMessage(const char *AMessage);

int GetCurrentLine();

int GetCurrentColumn();
