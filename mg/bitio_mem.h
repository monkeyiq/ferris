/**************************************************************************
 *
 * bitio_mem.h -- Functions for bitio to memory
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
 * $Id: bitio_mem.h,v 1.3 2008/06/07 21:30:09 ben Exp $
 *
 **************************************************************************
 *
 *  This file contains function definitions for doing bitwise input and output
 *  of numbers on an array of chars. These routines are faster than the ones 
 *  in "mems" files. But with these routines you cannot mix reads and writes 
 *  on the array of chars, or multiple write, at the same time and guarantee
 *  them to work, also you cannot seek to a point and do a write. The decode
 *  routine can detect when you run off the end of the array and will produce
 *  an approate error message, and the encode routine will stop when it gets
 *  to the end of the character array. 
 *
 *
 **************************************************************************/

#ifndef H_BITIO_MEM
#define H_BITIO_MEM

#ifdef GCC_HASCLASSVISIBILITY
    #pragma GCC visibility push(default)
#endif


#ifdef __cplusplus
extern "C" {
#endif  


#ifndef mem_bitio_state_struct
#define mem_bitio_state_struct
typedef struct mem_bitio_state
  {
    unsigned char *Base;
    unsigned char *Pos;
    int Remaining;
    unsigned char Buff;
    unsigned char Btg;
  }
mem_bitio_state;
#endif

/* NOTE : All bytes are filled high bit first */


void BIO_Mem_Encode_Start (void *buf, int rem, mem_bitio_state * bs);
void BIO_Mem_Encode_Done (mem_bitio_state * bs);



void BIO_Mem_Decode_Start (void *buf, int rem, mem_bitio_state * bs);



void BIO_Mem_Unary_Encode (unsigned long val, mem_bitio_state * bs,
			   unsigned long *bits);
unsigned long BIO_Mem_Unary_Decode (mem_bitio_state * bs,
				    unsigned long *bits);



void BIO_Mem_Binary_Encode (unsigned long val, unsigned long b,
			    mem_bitio_state * bs, unsigned long *bits);
unsigned long BIO_Mem_Binary_Decode (unsigned long b, mem_bitio_state * bs,
				     unsigned long *bits);



void BIO_Mem_Gamma_Encode (unsigned long val, mem_bitio_state * bs,
			   unsigned long *bits);
unsigned long BIO_Mem_Gamma_Decode (mem_bitio_state * bs, unsigned long *bits);



void BIO_Mem_Delta_Encode (unsigned long val, mem_bitio_state * bs,
			   unsigned long *bits);
unsigned long BIO_Mem_Delta_Decode (mem_bitio_state * bs, unsigned long *bits);


void BIO_Mem_Elias_Encode (unsigned long val, unsigned long b, double s,
			   mem_bitio_state * bs, unsigned long *bits);
unsigned long BIO_Mem_Elias_Decode (unsigned long b, double s,
				 mem_bitio_state * bs, unsigned long *bits);


void BIO_Mem_Bblock_Encode (unsigned long val, unsigned long b,
			    mem_bitio_state * bs, unsigned long *bits);
unsigned long BIO_Mem_Bblock_Decode (unsigned long b, mem_bitio_state * bs,
				     unsigned long *bits);


void BIO_Mem_Decode_Seek (unsigned long pos, mem_bitio_state * bs);

#ifdef __cplusplus
}
#endif                                                  /* __cplusplus */

#ifdef GCC_HASCLASSVISIBILITY
    #pragma GCC visibility pop
#endif
#endif
