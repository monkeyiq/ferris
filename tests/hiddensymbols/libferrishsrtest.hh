
#include <Ferris/HiddenSymbolSupport.hh>

#define HIDD  __attribute__ ((visibility("hidden")))

//HIDD void hsrfail();
FERRISEXP_DLLLOCAL void hsrfail_local();
//void hsrfail();
FERRISEXP_DLLPUBLIC void hsrfail();
