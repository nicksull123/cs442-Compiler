#include <stdio.h>
#include "Semantics.h"
#include "CodeGen.h"
#include "../SymTab/SymTab.h"
#include "../IOMngr/IOMngr.h"

extern int yyparse();

struct SymTab* curTab;
struct SymTab *funcTab;
struct StrLitList *strList;
struct TabList *tabList;

int sPos = 0;
int inProc = 0;
FILE* aFile;

int main(int argc, char* argv[])
{
    tabList = NULL;
    curTab = CreateSymTab(33);
    funcTab = CreateSymTab(33);
    strList = NULL;
    OpenFiles(argv[1], NULL);
    if (argc == 3)
        aFile = fopen(argv[2], "w");
    else
        aFile = stdout;

    yyparse();
}

