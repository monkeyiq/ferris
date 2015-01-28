/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris ls
    Copyright (C) 2001 Ben Martin

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    For more details see the COPYING file in the root directory of this
    distribution.

    $Id: sigtoy.cpp,v 1.2 2008/05/25 21:30:06 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <string>
#include <iostream>


#include <sigc++/sigc++.h>
#include <sigc++/object.h>
#include <sigc++/slot.h>
#include <sigc++/object_slot.h>

using namespace std;


class Grunt : public sigc::trackable
{
    string name;
    
public:
    Grunt( string s ) : name(s) 
        {
        }

    typedef sigc::signal1< void, string > GruntSig;
    GruntSig sig;
    GruntSig& getSig() { return sig; }
    
    
    
};

class Mux : public sigc::trackable
{
public:

    typedef sigc::signal1< void, string > MuxSig;
    MuxSig sig;
    MuxSig& getSig() { return sig; }


    void z_cb( string s )
        {
            cerr << "z_cb s:" << s << endl;
            getSig().emit( s );
        }
    
    void monitor( Grunt* g )
        {
            g->getSig().connect( sigc::mem_fun( *this, &Mux::z_cb ) );
        }
    
};


class Test : public sigc::trackable
{
public:
    Test()
        {
            Grunt* g1 = new Grunt("Grunt 1");
            Grunt* g2 = new Grunt("Grunt 2");
            Mux* mux = new Mux();

            mux->monitor( g1 );
            mux->monitor( g2 );
            
            mux->getSig().connect(sigc::mem_fun( *this, &Test::t_cb));

            g1->getSig().emit("Grunt 1 ... sig1");
            g1->getSig().emit("Grunt 2 ... sig2");
            
        }

    void t_cb( string s )
        {
            cerr << "t_cb s:" << s << endl;
        }
    
    
};



class SelfKiller
{
public:

    void die()
        {
            delete this;
        }
    

};


class statout 
{
public:
    statout()
        {
            cout << "hi" << endl;
        }
};

static statout statoutobj;



int main( int argc, const char** argv )
{
    
     try { throw int(4);}
     catch(...) {cerr << "!!!!!!!!! O K  !!!!!!!!!!\n\n\n";}
    
//     try { throw int(4);}
//     catch(...) {cerr << "!!!!!!!!! O K  !!!!!!!!!!\n\n\n";}


//    Test t;
    
//     SelfKiller* s = new SelfKiller();
//     s->die();
    

    
    return 0;
}
