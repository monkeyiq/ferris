/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
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

    $Id: libferrisperl.i,v 1.1 2005/07/04 08:57:30 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

%{
   #include <config.h>
%}


/* libferris.i */
%module libferris



%include "loki-smartptr.i"
%include "perlmain.i"
%include "typemaps.i"
%include "perl_std_list.i"
%include "cstring.i"

%rename(operator_assignment)   operator=;
%rename(operator_equal_to)     operator==;
%rename(operator_not_equal_to) operator!=;

%rename(operator_plusplus)        operator++();
%rename(operator_plusplusint)     operator++(int);
%rename(operator_minusminus)      operator--();
%rename(operator_minusminusint)   operator--(int);

%rename(operator_plus)            operator+;
%rename(operator_minus)           operator-;
%rename(operator_pluseq)          operator+=;
%rename(operator_minuseq)         operator-=;

%rename(operator_lt)            operator<;
%rename(operator_gt)            operator>;
%rename(operator_lteq)          operator<=;
%rename(operator_gteq)          operator>=;

%rename(operator_atindex)       operator[];
%rename(operator_functioncall)  operator();
%rename(operator_dereference)   operator*;

%ignore SL_commondb_SubCreate_dir( fh_context c, fh_context md );
%ignore SL_commondb_SubCreate_file( fh_context c, fh_context md );
%ignore SL_SubCreate_commondb( fh_context c, fh_context md );
%ignore SL_ldap_SubCreate_context( fh_context c, fh_context md );

%ignore tostr( fh_char );
%include "libferris.i"

/* %ignore fh_istreamChar::rdbuf(); */
/* %ignore fh_istreamChar_rdbuf(); */
/* %ignore Ferris::Ferris_istream<char>::rdbuf(); */
/* %ignore delete_fh_istreamChar; */
/* %template(fh_istreamChar)      Ferris::Ferris_istream<char>; */

namespace Ferris 
{
    namespace Shell
    {
        
        fh_context acquireContext( std::string path,
                                   int mode = 0,
                                   bool isDir = true );
    };
    
};

