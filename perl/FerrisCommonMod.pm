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

#     $Id: FerrisCommonMod.pm,v 1.1 2006/12/07 07:04:04 ben Exp $

# *******************************************************************************
# *******************************************************************************
# ******************************************************************************/

package FerrisCommonMod;
require Exporter;

BEGIN {
  $FerrisCommonModVersion = "1";
}

$dummy = "";
sub shouldPutFuncNamesHere {
}

@ISA       = qw( Exporter );
@EXPORT    = qw( startListing endListing startEA printEA endEA );
@EXPORT_OK = qw( $FerrisCommonModVersion );

sub xmlescape {
  $s = shift;

  $s =~ s/\&/&amp;/g;
  $s =~ s/\</&lt;/g;
  $s =~ s/\>/&gt;/g;

  return $s;
}

sub startListing {
  ($filename) = @_;

  print "<?xml version=\"1.0\"?>\n";
  print "<ferrisls url=\"" . xmlescape($filename) . "\">\n";
}

sub endListing {
      print "</ferrisls>\n";
}

sub startEA {
  print "<context ";
}

sub printEA {
  my ($key,$value) = @_;
  my $startloc     = 24;

  print " $key=\"" . xmlescape($value) . "\" ";
#  printf "%05d %05d %05d %05d %s %s\n", $startloc, length($key),
#    $startloc + length($key) + 1, length($value),
#      $key, $value;
}

sub endEA {
  print " />\n";
}




