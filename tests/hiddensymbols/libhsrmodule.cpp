
#include "libferrishsrtest.hh"
#include "libhsrmodule.hh"

#include <iostream>
using namespace std;

void hsrmodule_local()
{
    cerr << "hsrmodule_local!" << endl;
}

void hsrmodule_public()
{
    cerr << "hsrmodule_public!" << endl;
}

void hsrmodule_def()
{
    cerr << "hsrmodule_def!" << endl;
}

void badmodulefunc()
{
    cerr << "boo!" << endl;
//    hsrfail_local();
}


