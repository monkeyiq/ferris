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

#     $Id: FerrisExtMod.pm,v 1.2 2006/08/13 11:41:36 ben Exp $

# *******************************************************************************
# *******************************************************************************
# ******************************************************************************/

package FerrisExtMod;
require Exporter;

use lib '/usr/local/ferris/perl/';
use FerrisCommonMod;

BEGIN {
  $FerrisExtModVersion = "1";
}

$dummy = "";
sub shouldPutFuncNamesHere {
}

@ISA       = qw( Exporter );
@EXPORT    = qw( startListing endListing startEA printEA endEA FerrisExtModMain );
@EXPORT_OK = qw( $FerrisExtModVersion );

sub FerrisExtModMain() {

  package main;

  $command = $ARGV[0];

  init($0, @ARGV);

  if($command eq "list")
    {
      list($ARGV[1]);
    }
  elsif($command eq "copyout")
    {
      copyout($ARGV[1], $ARGV[2]);
    }
  elsif($command eq "copyin")
    {
      copyin($ARGV[1], $ARGV[2]);
    }

}
