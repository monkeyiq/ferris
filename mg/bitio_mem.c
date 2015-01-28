/**************************************************************************
 *
 * bitio_mem.c -- Functions for bitio to memory
 * Copyright (C) 1994  Neil Sharman
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * $Id: bitio_mem.c,v 1.2 2008/06/07 21:30:09 ben Exp $
 *
 **************************************************************************/

/*
   $Log: bitio_mem.c,v $
   Revision 1.2  2008/06/07 21:30:09  ben
   auto backup

   Revision 1.1  2005/07/04 08:47:40  ben
   *** empty log message ***

   Revision 1.1  2003/03/19 12:58:56  monkeyiq
   backups

   * Revision 1.1  1994/08/22  00:24:39  tes
   * Initial placement under CVS.
   *
 */

static char *RCSID = "$Id: bitio_mem.c,v 1.2 2008/06/07 21:30:09 ben Exp $";

#ifdef GCC_HASCLASSVISIBILITY
    #pragma GCC visibility push(default)
#endif


#include "sysfuncs.h"

#include "bitio_m_mem.h"
#include "bitio_m.h"

int fprintf (FILE *, const char *,...);



void 
BIO_Mem_Encode_Start (void *buf, int rem, mem_bitio_state * bs)
{
  ENCODE_START (buf, rem)
    ENCODE_PAUSE (*bs)
}

void 
BIO_Mem_Encode_Done (mem_bitio_state * bs)
{
  ENCODE_CONTINUE (*bs)
    ENCODE_DONE
}


void 
BIO_Mem_Decode_Start (void *buf, int rem, mem_bitio_state * bs)
{
  DECODE_START (buf, rem)
    DECODE_PAUSE (*bs)
}




void 
BIO_Mem_Unary_Encode (unsigned long val, mem_bitio_state * bs,
		      unsigned long *bits)
{
  ENCODE_CONTINUE (*bs)
    if (bits)
    UNARY_ENCODE_L (val, *bits);
  else
    UNARY_ENCODE (val);
  ENCODE_PAUSE (*bs)
}


unsigned long 
BIO_Mem_Unary_Decode (mem_bitio_state * bs,
		      unsigned long *bits)
{
  register unsigned long val;
  DECODE_CONTINUE (*bs)
    if (bits)
    UNARY_DECODE_L (val, *bits);
  else
    UNARY_DECODE (val);
  DECODE_PAUSE (*bs)
    return (val);
}







void 
BIO_Mem_Binary_Encode (unsigned long val, unsigned long b,
		       mem_bitio_state * bs, unsigned long *bits)
{
  ENCODE_CONTINUE (*bs)
    if (bits)
    BINARY_ENCODE_L (val, b, *bits);
  else
    BINARY_ENCODE (val, b);
  ENCODE_PAUSE (*bs)
}


unsigned long 
BIO_Mem_Binary_Decode (unsigned long b, mem_bitio_state * bs,
		       unsigned long *bits)
{
  register unsigned long val;
  DECODE_CONTINUE (*bs)
    if (bits)
    BINARY_DECODE_L (val, b, *bits);
  else
    BINARY_DECODE (val, b);
  DECODE_PAUSE (*bs)
    return (val);
}







void 
BIO_Mem_Gamma_Encode (unsigned long val, mem_bitio_state * bs,
		      unsigned long *bits)
{
  ENCODE_CONTINUE (*bs)
    if (bits)
    GAMMA_ENCODE_L (val, *bits);
  else
    GAMMA_ENCODE (val);
  ENCODE_PAUSE (*bs)
}


unsigned long 
BIO_Mem_Gamma_Decode (mem_bitio_state * bs, unsigned long *bits)
{
  register unsigned long val;
  DECODE_CONTINUE (*bs)
    if (bits)
    GAMMA_DECODE_L (val, *bits);
  else
    GAMMA_DECODE (val);
  DECODE_PAUSE (*bs)
    return (val);
}




void 
BIO_Mem_Delta_Encode (unsigned long val, mem_bitio_state * bs,
		      unsigned long *bits)
{
  ENCODE_CONTINUE (*bs)
    if (bits)
    DELTA_ENCODE_L (val, *bits);
  else
    DELTA_ENCODE (val);
  ENCODE_PAUSE (*bs)
}


unsigned long 
BIO_Mem_Delta_Decode (mem_bitio_state * bs, unsigned long *bits)
{
  register unsigned long val;
  DECODE_CONTINUE (*bs)
    if (bits)
    DELTA_DECODE_L (val, *bits);
  else
    DELTA_DECODE (val);
  DECODE_PAUSE (*bs)
    return (val);
}

void 
BIO_Mem_Elias_Encode (unsigned long val, unsigned long b, double s,
		      mem_bitio_state * bs, unsigned long *bits)
{
  ENCODE_CONTINUE (*bs)
    if (bits)
    ELIAS_ENCODE_L (val, b, s, *bits);
  else
    ELIAS_ENCODE (val, b, s);
  ENCODE_PAUSE (*bs)
}


unsigned long 
BIO_Mem_Elias_Decode (unsigned long b, double s,
		      mem_bitio_state * bs, unsigned long *bits)
{
  register unsigned long val;
  DECODE_CONTINUE (*bs)
    if (bits)
    ELIAS_DECODE_L (val, b, s, *bits);
  else
    ELIAS_DECODE (val, b, s);
  DECODE_PAUSE (*bs)
    return (val);
}

void 
BIO_Mem_Bblock_Encode (unsigned long val, unsigned long b,
		       mem_bitio_state * bs, unsigned long *bits)
{
  ENCODE_CONTINUE (*bs)
    if (bits)
    BBLOCK_ENCODE_L (val, b, *bits);
  else
    BBLOCK_ENCODE (val, b);
  ENCODE_PAUSE (*bs)
}


unsigned long 
BIO_Mem_Bblock_Decode (unsigned long b, mem_bitio_state * bs,
		       unsigned long *bits)
{
  register unsigned long val;
  DECODE_CONTINUE (*bs)
    if (bits)
    BBLOCK_DECODE_L (val, b, *bits);
  else
    BBLOCK_DECODE (val, b);
  DECODE_PAUSE (*bs)
    return (val);
}

void 
BIO_Mem_Decode_Seek (unsigned long pos, mem_bitio_state * bs)
{
  DECODE_CONTINUE (*bs)
    DECODE_SEEK (pos);
  DECODE_PAUSE (*bs)
}


#ifdef GCC_HASCLASSVISIBILITY
    #pragma GCC visibility pop
#endif
