#include "libferrishsrtest.hh"
#include "libhsrmodule.hh"
#include <iostream>
using namespace std;

void hsrfail()
{
    cerr << "hsrfail!" << endl;
    hsrmodule_public();
}

void hsrfail_local()
{
    cerr << "hsrfail LOCAL!!!" << endl;
}


void badfunc()
{
//    hsrmodule_local();
}

