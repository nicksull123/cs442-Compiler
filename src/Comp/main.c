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
    OpenFiles(argv[1], NULL);
    if (argc == 3)
        aFile = fopen(argv[2], "w");
    else
        aFile = stdout;

    yyparse();
}

