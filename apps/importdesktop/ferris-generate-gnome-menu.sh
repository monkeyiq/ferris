#!/bin/sh
#
# This file is based on e_gen_gnome_menu from e16.
#
#
###############################################################################
# generates a file.menu format for Enlightenment out of a GNOME menu hierarchy#
#
# Copyright (C) 1999 Carsten Haitzler, Geoff Harrison  and various contributors
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies of the Software, its documentation and marketing & publicity
# materials, and acknowledgment shall be given in the documentation, materials
# and software packages that this Software was used.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
# IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
###############################################################################

# setup variables
BASE=$1;
OUTDB=${2:-~/.ferris/apps.db};
DIR=${3:-${ADIR}};

find_app_base_dir() {
  ADIR="`gnome-config --prefix`/share/gnome/apps"
  if [ ! -d "$ADIR" ]; then
    ADIR="/usr/share/gnome/apps"
  else
    return
  fi
  if [ ! -d "$ADIR" ]; then
    ADIR="/usr/share/apps"
  else
    return
  fi
  if [ ! -d "$ADIR" ]; then
    ADIR="/usr/local/share/apps"
  else
    return
  fi
  if [ ! -d "$ADIR" ]; then
    ADIR="/usr/gnome/share/apps"
  else
    return
  fi
  if [ ! -d "$ADIR" ]; then
    ADIR="/usr/local/gnome/share/apps"
  else
    return
  fi
  if [ ! -d "$ADIR" ]; then
    ADIR="/opt/gnome/share/apps"
  else
    return
  fi
  if [ ! -d "$ADIR" ]; then
    ADIR="/opt/gnome/share/gnome/apps"
  else
    return
  fi
  if [ ! -d "$ADIR" ]; then
    ADIR="/usr/X11R6/share/gnome/apps"
  else
    return
  fi
  if [ ! -d "$ADIR" ]; then
    ADIR="/opt/local/share/apps"
  else
    return
  fi
}

find_icon_base_dir() {
  IDIR="`gnome-config --prefix`/share/pixmaps"
  if [ ! -f "${IDIR}/gnome-help.png" ]; then
    IDIR="/usr/share/pixmaps"
  else
    return
  fi
  if [ ! -f "${IDIR}/gnome-help.png" ]; then
    IDIR="/usr/share/gnome/pixmaps"
  else
    return
  fi
  if [ ! -f "${IDIR}/gnome-help.png" ]; then
    IDIR="/usr/local/share/pixmaps"
  else
    return
  fi
  if [ ! -f "${IDIR}/gnome-help.png" ]; then
    IDIR="/usr/gnome/share/pixmaps"
  else
    return
  fi
  if [ ! -f "${IDIR}/gnome-help.png" ]; then
    IDIR="/usr/local/gnome/share/pixmaps"
  else
    return
  fi
  if [ ! -f "${IDIR}/gnome-help.png" ]; then
    IDIR="/opt/gnome/share/pixmaps"
  else
    return
  fi
  if [ ! -f "${IDIR}/gnome-help.png" ]; then
    IDIR="/opt/gnome/share/gnome/pixmaps"
  else
    return
  fi
  if [ ! -f "${IDIR}/gnome-help.png" ]; then
    IDIR="/usr/X11R6/share/gnome/pixmaps"
  else
    return
  fi
  if [ ! -f "${IDIR}/gnome-help.png" ]; then
    IDIR="/opt/local/share/pixmaps"
  else
    return
  fi
}

find_app_base_dir
# echo "ADIR: $ADIR"
DIR=${3:-${ADIR}};
# echo "DIR: $DIR "
# echo "BASE:  $BASE"
# echo "OUTDB: $OUTDB"

if [ -z "$BASE" ]; then
    BASE=$DIR;
fi;


# In most cases I believe the icons can be found in the directory that is
# ../../pixmaps from $ADIR.
find_icon_base_dir
GICONDIR="$IDIR"

# if the apps dir doesn't exist in the end then exit
if [ ! -d "$DIR" ]; then
  echo "import directory doesn't exist: $DIR"
  exit
fi
# if the destination dir doesnt exist - create it
if [ ! -d "~/.ferris" ]; then
  mkdir -p "~/.ferris"
fi

# function to check its a GNOME desktop file 
is_desktop() {
  VAL="`grep "\[Desktop Entry\]" $1`"
  if [ -n "$VAL" ]; then
    IS_DESKTOP_RESULT="`awk -F= 'BEGIN { n="" } END { printf("%s", n) } $1 ~ "^Name\\\['$LANG'\\\]" {n=$2;exit} $1 ~ "^Name$" {n=$2}' $1`"
    if [ -n "$IS_DESKTOP_RESULT" ]; then
      return 0
    fi
  fi
  IS_DESKTOP_RESULT=""
  return 1
}

# function to get the sortorder list -if there is one
get_sortorder() {
  if [ -f "${1}/.order" ]; then
    VAL="`awk '{printf("%s ", $1);}' $1"/.order" | sed 's/,/ /g'`"
  else
    VAL=""
  fi
  GET_SORT_ORDER_RESULT="$VAL"
  for I in `/bin/ls "$1"`; do
    IS_IN="n"
    for J in $VAL; do
      if [ "$J" = "$I" ]; then
        IS_IN="y"
	continue 2
      fi
    done
    GET_SORT_ORDER_RESULT="${GET_SORT_ORDER_RESULT}${I} "
  done
  return 0
}

get_icon() {
  VAL2="`awk -F= '$1 ~ "^Icon$" {printf("%s", $2); exit;}' $1`"
  if [ -z "$VAL2" ]; then
    GET_ICON_RETURN=""
    return 1
  fi

  # If we have "convert" and the icon havsn't already been scaled down, scale
  # it down now!

  GET_ICON_RETURNDIR="${ODIR}/gnome_icons"
#   if [ ! -d "$GET_ICON_RETURNDIR" ];then
#     mkdir "$GET_ICON_RETURNDIR"
#   fi

  # The "Icon" entery can contain an absolute path, if it does forget the
  # $GICONDIR
  VAL3="`echo $VAL2 | grep /`"
  if [ "$VAL3" != "$VAL2" ];then
    if [ -n "$VAL2" ]; then
	GICON="$GICONDIR/$VAL2"
	GET_ICON_RETURN="$GET_ICON_RETURNDIR/$VAL2"
    fi
  else
    if [ -n "$VAL3" ]; then
	GICON="$VAL3"
	GET_ICON_RETURN="$GET_ICON_RETURNDIR/`basename "$VAL3"`"
    fi
  fi

    GET_ICON_RETURN=$GICON;

#   if [ -n "$CONVERT_CMD" -a -n "$GICON" -a -f "$GICON" -a ! \
#         -f "$GET_ICON_RETURN" ]; then
#     "$CONVERT_CMD" "$GICON" -geometry 16x16 "$GET_ICON_RETURN"
#   fi
  
  return 0
}

get_exec() {
  GET_EXEC_RETURN="`awk -F= '$1 ~ "^Exec$" {printf("%s", $2);}' $1`"
  if [ -z "$GET_EXEC_RETURN" ]; then
    GET_EXEC_RETURN=""
    return 1
  fi
  return 0
}

E_TITLE=""

ORDER=""
# if a .directory file exists - read it
F=$DIR"/.directory"
if [ -f $F ]; then
  is_desktop $F
  NAME="$IS_DESKTOP_RESULT"
  if [ -n "$NAME" ]; then
    E_TITLE="$NAME"
  fi
fi

get_sortorder "$DIR"
ORDER="$GET_SORT_ORDER_RESULT"

# print the menu title
echo \"Importing $E_TITLE\"
# echo "ORDER: $ORDER"

# for every subdir in the dir list or order - import it
for F in $ORDER; do
  FF="${DIR}/${F}"
  IMPORTPATH="${FF#$BASE}"
  IMPORTPATH="${IMPORTPATH##/}"

  if [ -d "$FF" ]; then
    FFF="${FF}/.directory"
    if [ -f "$FFF" ]; then
      is_desktop "$FFF"
      NAME="$IS_DESKTOP_RESULT"
    else
      NAME="`echo $F | sed 's/_/ /g'`"
    fi
    FFF="${ODIR}/${BASE}"
#     if [ ! -d "$FFF" ]; then
#       mkdir "$FFF"
#     fi
    MFILE="${FFF}/${F}.menu"

#    echo "$0" "${BASE}/${F}" "$ODIR" "$MFILE" "${DIR}/${F}"
#    "$0" "${BASE}/${F}" "$ODIR" "$MFILE" "${DIR}/${F}"
    echo "$0 $BASE $OUTDB ${DIR}/${F}  path:$IMPORTPATH"
    "$0" "$BASE" "$OUTDB" "${DIR}/${F}"
    get_icon "${FF}/.directory"
    ICO="$GET_ICON_RETURN"
    echo "\"$NAME\" \"$ICO\" menu \"$MFILE\"" 
  else
    if [ -r "$FF" ]; then
      is_desktop $FF
      NAME="$IS_DESKTOP_RESULT"
      if [ -n "$NAME" ]; then
        IMPORTPATH=`dirname $IMPORTPATH`;
	get_exec "$FF"
	EXE="$GET_EXEC_RETURN"
        get_icon "$FF"
        ICO="$GET_ICON_RETURN"
# 	echo "OUT: $OUT"
	echo "desktop=\"${FF}\" name=\"$NAME\" icon=\"$ICO\" exec=\"$EXE\" path=\"$IMPORTPATH\"" 
	 ferris-import-desktop-file -p -d "$OUTDB/$IMPORTPATH" "${FF}"
      fi
    fi
  fi
done
