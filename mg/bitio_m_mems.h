/**************************************************************************
 *
 * bitio_m_mems.h -- Macros for bitio to memory (random access)
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
 * $Id: bitio_m_mems.h,v 1.2 2008/06/07 21:30:09 ben Exp $
 *
 **************************************************************************
 *
 *  This file contains macros for doing bitwise input and output on an array 
 *  of chars. These routines are slower than the ones in "mem" files. but 
 *  with these routines you can mix reads and writes, or multiple writes,  on
 *  the array of chars at the same time and guarantee them to work, also you 
 *  can seek to a point and do a write. The decode and encode macros cannot 
 *  detect when the end off the character is reached and just continue 
 *  processing.
 *
 **************************************************************************/

#ifndef H_BITIO_M_MEMS
#define H_BITIO_M_MEMS

#ifdef GCC_HASCLASSVISIBILITY
    #pragma GCC visibility push(default)
#endif

typedef struct mems_bitio_state
  {
    unsigned char *Base;
    unsigned long pos;
  }
mems_bitio_state;


#define ENCODE_START(b,p)						\
  {									\
    register unsigned char *__base = b;					\
    register unsigned long __pos = p;

#define ENCODE_CONTINUE(b)						\
  {									\
    register unsigned char *__base = (b).Base;				\
    register unsigned long __pos = (b).pos;

#define ENCODE_BIT(b)							\
  do {									\
    if (b)								\
      __base[__pos>>3] |= 0x80 >> (__pos&7);				\
    else								\
      __base[__pos>>3] &= 0xff7f >> (__pos&7);				\
    __pos++;								\
  } while(0)

#define ENCODE_PAUSE(b)							\
    (b).Base = __base;							\
    (b).pos = __pos;							\
  }

#define ENCODE_FLUSH


#define ENCODE_DONE							\
    ENCODE_FLUSH;							\
  }


#define DECODE_START(b,p)						\
  {									\
    register unsigned char *__base = b;					\
    register unsigned long __pos = p;

#define DECODE_CONTINUE(b)						\
  {									\
    register unsigned char *__base = (b).Base;				\
    register unsigned long __pos = (b).pos;

#define DECODE_ADD_FF(b)						\
  do {									\
    (b) += (b) + (__base[__pos>>3] & (0x80 >> (__pos&7)) != 0);		\
    __pos++;								\
  } while(0)

#define DECODE_ADD_00(b) DECODE_ADD_FF(b)

#define DECODE_BIT 							\
    (__pos++, ((__base[(__pos-1)>>3] & (0x80 >> ((__pos-1)&7))) != 0))

#define DECODE_DONE	;						\
  }

#define DECODE_PAUSE(b)							\
    (b).Base = __base;							\
    (b).pos = __pos;							\
  }

#define DECODE_SEEK(pos) __pos = (pos)

#define ENCODE_SEEK(pos) __pos = (pos)





#ifdef GCC_HASCLASSVISIBILITY
    #pragma GCC visibility pop
#endif
#endif
