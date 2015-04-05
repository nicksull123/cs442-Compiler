#include <stdio.h>
#include "Semantics.h"
#include "CodeGen.h"
#include "../SymTab/SymTab.h"
#include "../IOMngr/IOMngr.h"

int yyparse();

struct SymTab* curTab;
struct SymTab *funcTab;
struct StrLitList *strList;
struct TabList *tabList;
struct ArgList *argList;

int sPos = 0;
int argPos = 0;
int paramPos = 0;
int inProc = 0;
FILE* aFile;

int main(int argc, char* argv[])
{
    tabList = NULL;
    argList = NULL;
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

