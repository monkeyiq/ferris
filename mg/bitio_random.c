/**************************************************************************
 *
 * bitio_random.c -- Functions for bitio to a file (random access)
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
 * $Id: bitio_random.c,v 1.1 2005/07/04 08:47:40 ben Exp $
 *
 *  Modified:
 *   - long long seek and tell ops
 *     (1999-08-03 Tim Bell <bhat@cs.mu.oz.au>)
 *     Code provided by Owen de Kretser <oldk@cs.mu.oz.au>
 **************************************************************************/

/*
   $Log: bitio_random.c,v $
   Revision 1.1  2005/07/04 08:47:40  ben
   *** empty log message ***

   Revision 1.1  2003/03/19 12:58:57  monkeyiq
   backups

   * Revision 1.1  1994/08/22  00:24:40  tes
   * Initial placement under CVS.
   *
 */

static char *RCSID = "$Id: bitio_random.c,v 1.1 2005/07/04 08:47:40 ben Exp $";



#include "sysfuncs.h"
#include "memlib.h"

#include "longlong.h"

#include "bitio_m_random.h"
#include "bitio_m.h"


void 
BIO_Random_Start (FILE * f, unsigned long len,
		  random_bitio_state * bs)
{
  ENCODE_START (f, len)
    ENCODE_PAUSE (*bs)
}

void 
BIO_Random_Done (random_bitio_state * bs)
{
  ENCODE_CONTINUE (*bs)
    ENCODE_DONE
}

void 
BIO_Random_Encode_Bit (int bit, random_bitio_state * bs)
{
  ENCODE_CONTINUE (*bs)
    ENCODE_BIT (bit);
  ENCODE_PAUSE (*bs)
}

int 
BIO_Random_Decode_Bit (random_bitio_state * bs)
{
  register int val;
  DECODE_CONTINUE (*bs)
    val = DECODE_BIT;
  DECODE_PAUSE (*bs)
    return (val);
}


void 
BIO_Random_Unary_Encode (unsigned long val, random_bitio_state * bs,
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
BIO_Random_Unary_Decode (random_bitio_state * bs,
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
BIO_Random_Binary_Encode (unsigned long val, unsigned long b,
			  random_bitio_state * bs, unsigned long *bits)
{
  ENCODE_CONTINUE (*bs)
    if (bits)
    BINARY_ENCODE_L (val, b, *bits);
  else
    BINARY_ENCODE (val, b);
  ENCODE_PAUSE (*bs)
}


unsigned long 
BIO_Random_Binary_Decode (unsigned long b,
			  random_bitio_state * bs,
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
BIO_Random_Gamma_Encode (unsigned long val, random_bitio_state * bs,
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
BIO_Random_Gamma_Decode (random_bitio_state * bs,
			 unsigned long *bits)
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
BIO_Random_Delta_Encode (unsigned long val, random_bitio_state * bs,
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
BIO_Random_Delta_Decode (random_bitio_state * bs,
			 unsigned long *bits)
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
BIO_Random_Elias_Encode (unsigned long val, unsigned long b, double s,
			 random_bitio_state * bs, unsigned long *bits)
{
  ENCODE_CONTINUE (*bs)
    if (bits)
    ELIAS_ENCODE_L (val, b, s, *bits);
  else
    ELIAS_ENCODE (val, b, s);
  ENCODE_PAUSE (*bs)
}


unsigned long 
BIO_Random_Elias_Decode (unsigned long b, double s,
			 random_bitio_state * bs,
			 unsigned long *bits)
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
BIO_Random_Bblock_Encode (unsigned long val, unsigned long b,
			  random_bitio_state * bs, unsigned long *bits)
{
  ENCODE_CONTINUE (*bs)
    if (bits)
    BBLOCK_ENCODE_L (val, b, *bits);
  else
    BBLOCK_ENCODE (val, b);
  ENCODE_PAUSE (*bs)
}


unsigned long 
BIO_Random_Bblock_Decode (unsigned long b,
			  random_bitio_state * bs,
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
BIO_Random_Seek (unsigned long pos, random_bitio_state * bs)
{
  ENCODE_CONTINUE (*bs)
    ENCODE_SEEK (pos);
  ENCODE_PAUSE (*bs)
}
void 
BIO_Random_Flush (random_bitio_state * bs)
{
  ENCODE_CONTINUE (*bs)
    ENCODE_FLUSH;
  ENCODE_PAUSE (*bs)
}

unsigned long 
BIO_Random_Tell (random_bitio_state * bs)
{
  register unsigned long t;
  ENCODE_CONTINUE (*bs)
    t = ENCODE_TELL;
  ENCODE_PAUSE (*bs)
    return (t);
}

#ifdef USE_LONG_LONG

void 
BIO_Random_Seek_LL (mg_ullong pos, random_bitio_state * bs)
{
  ENCODE_CONTINUE(*bs)

    if ((((pos) >> 3) >= (mg_ullong)__base) &&
	((((pos)+7) >> 3) < (mg_ullong)(__base + __len)))
      {
	__pos = (long)((pos) - (mg_ullong)(__base << 3));
      }
    else
      {
        ENCODE_FLUSH;
	__base = (long)(((pos) >> (__sft+3)) << __sft);
	
	fseek(__file,__base,0);
	fread(__buf,1,__len,__file);
	__pos = (long)((pos) & ((8 << __sft)-1));
      }
  
  ENCODE_PAUSE(*bs)
}

mg_ullong 
BIO_Random_Tell_LL (random_bitio_state * bs)
{
  mg_ullong t;
  ENCODE_CONTINUE(*bs)
    t = (((mg_ullong)__base) << 3ull) + __pos;
  ENCODE_PAUSE(*bs)
  return(t);  
}

#endif /* USE_LONG_LONG */
