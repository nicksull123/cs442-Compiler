int
INTERNALPrintBool(bool val)
{
    if(val)
    {
        print("true");
    }
    else
    {
        print("false");
    }
}

float
INTERNALPowFloat(float base, int pow)
{
    int i;
    float curVal;
    i = 0;
    curVal = 1.0;
    while(i < pow)
    {
        curVal = curVal * base;
        i = i + 1;
    }   
    return curVal;
}   

int
INTERNALPowInt(int base, int pow)
{
    int i;
    int curVal;
    i = 0;
    curVal = 1;
    while(i < pow)
    {
        curVal = curVal * base;
        i = i + 1;
    }
    return curVal;
}

int
INTERNALPrintSp(int count)
{
    int i;
    i = 0;
    while(i < count)
    {
        print(" ");
        i = i + 1;
    }
}

int
INTERNALPrintLn()
{
    print("\n");
}

int
INTERNALNegateInt(int val)
{
    return 0 - val;
}

float
INTERNALNegateFloat(float val)
{
    return 0 - val;
}

