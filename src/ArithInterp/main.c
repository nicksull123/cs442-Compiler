#include <stdio.h>
#include "../SymTab/SymTab.h"
#include "../IOMngr/IOMngr.h"

extern int yyparse();

struct SymTab *table;

int
main(int argc, char **argv)
{
    table = CreateSymTab(100);
    if(!OpenFiles(argv[1], "listing"))
    {
        printf("File Not Found\n");
        exit(1);
    }
    yyparse();
}
