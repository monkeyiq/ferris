#!/usr/bin/perl

# /******************************************************************************
# *******************************************************************************
# *******************************************************************************

#     libferris
#     Copyright (C) 2001 Ben Martin

#     This program is free software; you can redistribute it and/or modify
#     it under the terms of the GNU General Public License as published by
#     the Free Software Foundation; either version 2 of the License, or
#     (at your option) any later version.

#     This program is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#     GNU General Public License for more details.

#     You should have received a copy of the GNU General Public License
#     along with this program; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#     For more details see the COPYING file in the root directory of this
#     distribution.

#     $Id: FerrisFulltextExternalIndexExample.pl,v 1.1 2006/08/13 11:42:25 ben Exp $

# *******************************************************************************
# *******************************************************************************
# ******************************************************************************/

use lib '/usr/local/ferris/perl/';
use FerrisFulltextExternalIndex;

startListing( "" );

my $arg = shift;

if( $arg eq "foo"  )
  {
    # If they are wanting foo then we just give it to them.
    startEA;
    printEA "name", "foo";
    printEA "url",  "file://tmp/foo";
    endEA;
  }
if( $arg eq "special mode" )
  {
    # If called using EXTERNAL mode them terms are not broken up
    startEA;
    printEA "name", "fooe";
    printEA "url",  "file://tmp/fooe";
    endEA;
  }
else
  {
    # You'll want to do these in a loop to add each file in the result.
    startEA;
    printEA "name", "bar";
    printEA "url",  "file://tmp/bar";
    endEA;

    startEA;
    printEA "name", "baz";
    printEA "url",  "file://tmp/baz";
    endEA;
  }
endListing;
exit 0;





