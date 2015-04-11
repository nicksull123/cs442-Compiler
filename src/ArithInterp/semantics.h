#include <stdio.h>
#include <stdlib.h>
#include "../IOMngr/IOMngr.h"
#include "../SymTab/SymTab.h"

void printSymTab();

void storeVar(char *name, int32_t v);

int32_t getVal(char *name);

int32_t doMult(int32_t x, int32_t y);

int32_t doPlus(int32_t x, int32_t y);
