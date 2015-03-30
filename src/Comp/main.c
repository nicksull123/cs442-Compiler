#include <stdio.h>
#include "Semantics.h"
#include "CodeGen.h"
#include "../SymTab/SymTab.h"
#include "../IOMngr/IOMngr.h"

extern int yyparse();

struct SymTab* table;
struct SymTab* ProcSymTab;
struct SymEntry* entry;
int inProc = 0;
FILE* aFile;

int main(int argc, char* argv[])
{
    table = CreateSymTab(33);
    // assumes there is a listing file
    OpenFiles(argv[1], argv[2]);
    if (argc == 4)
        aFile = fopen(argv[3], "w");
    else
        aFile = stdout;

    yyparse();
}

