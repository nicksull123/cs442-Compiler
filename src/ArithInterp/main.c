#include <stdio.h>
#include "../SymTab/SymTab.h"
#include "../IOMngr/IOMngr.h"

extern int yyparse();

struct SymTab *table;

int
main(int argc, char **argv)
{
    if(argc < 2){
        puts("Usage: arithInterp [srcfile] [listingfile]");
        return 1;
    }

    char *listing = NULL;
    if (argc > 2) {
        listing = argv[2];
    }

    table = CreateSymTab(100);
    if(!OpenFiles(argv[1], listing))
    {
        printf("File Not Found\n");
        return 1;
    }
    yyparse();
}
