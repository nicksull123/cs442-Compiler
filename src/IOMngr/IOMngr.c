// IOMngr.c
// Author: Nicholas Sullivan
#include "IOMngr.h"

FILE *sf;
FILE *lf;
uint32_t line;
uint32_t col;
uint32_t line_len;
uint8_t err;
char line_buf[MAXLINE];

static void
write_line(FILE *out)
{
    fprintf(out, "%d. %s", GetCurrentLine(), line_buf);
}

bool
OpenFiles(const char *ASourceName, const char *AListingName)
{
    if (!(sf = fopen(ASourceName, "r")))
    {
        return 0;
    }
    lf = NULL;
    if (AListingName)
    {
        if (!(lf = fopen(AListingName, "w+")))
        {
            fclose(sf);
            return 0;
        }
    }
    line = 0;
    col = 0;
    line_len = 0;
    err = 0;
    return 1;
}

void
CloseFiles()
{
    fclose(sf);
    if (lf)
    {
        fclose(lf);
    }
}

char
GetSourceChar()
{
    if (col >= line_len)
    {
        if (!fgets(line_buf, MAXLINE, sf))
        {
            return EOF;
        }
        line++;
        col = 0;
        err = 0;
        line_len = strlen(line_buf);
        if (lf)
        {
            write_line(lf);
        }
    }
    return line_buf[col++];
}

void
WriteIndicator(int AColumn)
{
    FILE *out = lf;
    if (!out)
    {
        out = stdout;
        if (!err)
        {
            write_line(out);
        }
    }
    fprintf(out, "%*c\n", AColumn + 3, '^');
    err = 1;
}

void
WriteMessage(const char *AMessage)
{
    FILE *out = lf;
    if (!out)
    {
        out = stdout;
    }
    fprintf(out, "%s\n", AMessage);
}

int
GetCurrentLine()
{
    return line;
}

int
GetCurrentColumn()
{
    return col;
}
