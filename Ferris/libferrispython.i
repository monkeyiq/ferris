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

    $Id: libferrispython.i,v 1.1 2005/07/04 08:57:30 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/


/* libferris.i */
%module libferrispython

%include "typemaps.i"
%include std_list.i

//
// python only
//
%rename(outout) Ferris::Time::Benchmark::print;

%rename(operator_plusplus)        operator++();
%rename(operator_plusplusint)     operator++(int);
%rename(operator_minusminus)      operator--();
%rename(operator_minusminusint)   operator--(int);
%rename(operator_atindex)         operator[];

%include "libferris.i"
