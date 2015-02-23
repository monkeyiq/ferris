/**************************************************************************
 *
 * bitio_gen.c -- General supoport routines for bitio
 * Copyright (C) 1994  Neil Sharman and Alistair Moffat
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
 * $Id: bitio_gen.c,v 1.2 2008/06/07 21:30:09 ben Exp $
 *
 **************************************************************************/

/*
   $Log: bitio_gen.c,v $
   Revision 1.2  2008/06/07 21:30:09  ben
   auto backup

   Revision 1.1  2005/07/04 08:47:40  ben
   *** empty log message ***

   Revision 1.1  2003/03/19 12:58:56  monkeyiq
   backups

   * Revision 1.1  1994/08/22  00:24:38  tes
   * Initial placement under CVS.
   *
 */

#include <config.h>

static char *RCSID = "$Id: bitio_gen.c,v 1.2 2008/06/07 21:30:09 ben Exp $";

#ifdef GCC_HASCLASSVISIBILITY
    #pragma GCC visibility push(default)
#endif

#include "sysfuncs.h"
#include "bitio_m.h"




int fprintf (FILE *, const char *,...);

unsigned long 
BIO_Unary_Length (unsigned long val)
{
  register unsigned long num;
  UNARY_LENGTH (val, num);
  return (num);
}


unsigned long 
BIO_Binary_Length (unsigned long val, unsigned long b)
{
  register unsigned long num;
  BINARY_LENGTH (val, b, num);
  return (num);
}


unsigned long 
BIO_Gamma_Length (unsigned long val)
{
  register unsigned long num;
  GAMMA_LENGTH (val, num);
  return (num);
}


unsigned long 
BIO_Delta_Length (unsigned long val)
{
  register unsigned long num;
  DELTA_LENGTH (val, num);
  return (num);
}


unsigned long 
BIO_Elias_Length (unsigned long val, unsigned long b, double s)
{
  register unsigned long num;
  ELIAS_LENGTH (val, b, s, num);
  return (num);
}

unsigned long 
BIO_Bblock_Length (unsigned long val, unsigned long b)
{
  register unsigned long num;
  BBLOCK_LENGTH (val, b, num);
  return (num);
}


int 
BIO_Bblock_Init (int N, int p)
{
  int b;
  b = (int) (0.5 + 0.6931471 * N / p);
  return (b ? b : 1);
}


int 
BIO_Bblock_Init_W (int N, int p)
{
  int logb;
  FLOORLOG_2 ((N - p) / p, logb);
  return (logb < 0 ? 1 : (1 << logb));
}

int 
BIO_Bblock_Bound_b (int N, int p, int b)
{
  int clogb;
  CEILLOG_2 (b, clogb);
  return (p * (1 + clogb) + (N - p * ((1 << clogb) - b + 1)) / b);
}

int 
BIO_Bblock_Bound (int N, int p)
{
  int b;
  b = BIO_Bblock_Init_W (N, p);
  return (BIO_Bblock_Bound_b (N, p, b));
}

/* adjustment is a hack to overcome inaccuracies in floating point,
   where (int)(ln4/ln2) can equal 1, not 2. */
int 
BIO_Gamma_Bound (int N, int p)
{
  return ((int) (p * (2 * log2 (((double) N * 1.0001) / p) + 1)));
}

int 
floorlog_2 (int b)
{
  int logb;
  FLOORLOG_2 (b, logb);
  return logb;
}

int 
ceillog_2 (int b)
{
  int logb;
  CEILLOG_2 (b, logb);
  return logb;
}

#ifdef GCC_HASCLASSVISIBILITY
    #pragma GCC visibility pop
#endif
