/**************************************************************************
 *
 * bitio_m_mem.h -- Macros for bitio to memory
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
 * $Id: bitio_m_mem.h,v 1.2 2008/06/07 21:30:09 ben Exp $
 *
 **************************************************************************
 *
 *  This file contains macros for doing bitwise input and output on an array 
 *  of chars. These routines are faster than the ones in "mems" files. But 
 *  with these routines you cannot mix reads and writes on the array of chars,
 *  or multiple write, at the same time and guarantee them to work, also you 
 *  cannot seek to a point and do a write. The decode routine can detect when
 *  you run off the end of the array and will produce an approate error 
 *  message, and the encode routine will stop when it gets to the end of the 
 *  character array. 
 *
 **************************************************************************/

#ifndef H_BITIO_M_MEM
#define H_BITIO_M_MEM

#ifdef GCC_HASCLASSVISIBILITY
    #pragma GCC visibility push(default)
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



#ifndef DECODE_ERROR
#define DECODE_ERROR (fprintf(stderr,"Unexpected EOF in \"%s\" on line %d\n",\
				__FILE__, __LINE__), exit(1))
#endif



#define ENCODE_START(p,r)						\
  {									\
    register unsigned char *__pos = p;					\
    register unsigned char *__base = p;					\
    register int __remaining = r;					\
    register unsigned char __buff = 0;					\
    register unsigned char __btg = sizeof(__buff)*8;

#define ENCODE_CONTINUE(b)						\
  {									\
    register unsigned char *__pos = (b).Pos;				\
    register unsigned char *__base = (b).Base;				\
    register int __remaining = (b).Remaining;				\
    register unsigned char __buff = (b).Buff;				\
    register unsigned char __btg = (b).Btg;

#define ENCODE_BIT(b)							\
  do {									\
    __btg--;								\
    if (b) __buff |= (1 << __btg);					\
    if (!__btg)								\
      {									\
	if (__remaining)						\
	  {								\
	    *__pos++ = __buff;						\
	    __remaining--;						\
	  }  								\
	__buff = 0;							\
	__btg = sizeof(__buff)*8;					\
      }									\
  } while(0)

#define ENCODE_PAUSE(b)							\
    (b).Pos = __pos;							\
    (b).Base = __base;							\
    (b).Remaining = __remaining;					\
    (b).Buff = __buff;							\
    (b).Btg =  __btg;							\
  }

#define ENCODE_FLUSH							\
  do {									\
    if (__btg != sizeof(__buff)*8)					\
      if (__remaining)							\
	{								\
          *__pos++ = __buff;						\
          __remaining--;						\
	}  								\
    __btg = sizeof(__buff)*8;						\
  } while (0)

#define ENCODE_DONE							\
    ENCODE_FLUSH;							\
  }


#define DECODE_START(p,r)						\
  {									\
    register unsigned char *__pos = p;					\
    register unsigned char *__base = p;					\
    register int __remaining = r;					\
    register unsigned char __buff = 0;					\
    register unsigned char __btg = 0;

#define DECODE_CONTINUE(b)						\
  {									\
    register unsigned char *__pos = (b).Pos;				\
    register unsigned char *__base = (b).Base;				\
    register int __remaining = (b).Remaining;				\
    register unsigned char __buff = (b).Buff;				\
    register unsigned char __btg = (b).Btg;

#define DECODE_ADD_FF(b)						\
  do {									\
    if (!__btg)								\
      {									\
	if (__remaining)						\
	  {								\
  	    __buff = *__pos++;						\
	    __remaining--;						\
	  }								\
        else								\
          __buff = 0xff;						\
	__btg = sizeof(__buff)*8;					\
      }									\
    (b) += (b) + ((__buff >> --__btg) & 1);				\
  } while(0)

#define DECODE_ADD_00(b)						\
  do {									\
    if (!__btg)								\
      {									\
	if (__remaining)						\
	  {								\
  	    __buff = *__pos++;						\
	    __remaining--;						\
	  }								\
        else								\
          __buff = 0x00;						\
	__btg = sizeof(__buff)*8;					\
      }									\
    (b) += (b) + ((__buff >> --__btg) & 1);				\
  } while(0)

#define DECODE_BIT (__btg ? (__buff >> --__btg) & 1 :			\
   (!__remaining ? 							\
    (DECODE_ERROR, 0) :							\
    (__buff = *__pos++, __remaining--, __btg = sizeof(__buff)*8, 	\
     (__buff >> --__btg) & 1)))

#define DECODE_DONE	;						\
  }

#define DECODE_PAUSE(b)							\
    (b).Pos = __pos;							\
    (b).Base = __base;							\
    (b).Remaining = __remaining;					\
    (b).Buff = __buff;							\
    (b).Btg =  __btg;							\
  }

#define DECODE_SEEK(pos)						\
  do {									\
    register long __new_pos = (pos);					\
    __pos = __base + (__new_pos >> 3);					\
    __buff = *__pos++;							\
    __btg = 8 - (__new_pos&7);						\
    }									\
  while(0)




#ifdef GCC_HASCLASSVISIBILITY
    #pragma GCC visibility pop
#endif
#endif
