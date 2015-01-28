/**************************************************************************
 *
 * bitio_m_random.h -- Macros for bitio to a file (random access)
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
 * $Id: bitio_m_random.h,v 1.2 2008/06/07 21:30:09 ben Exp $
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
 *
 **************************************************************************/

#ifndef H_BITIO_M_RANDOM
#define H_BITIO_M_RANDOM

#ifdef GCC_HASCLASSVISIBILITY
    #pragma GCC visibility push(default)
#endif

typedef struct random_bitio_state
  {
    FILE *File;
    unsigned char *Buf;
    unsigned long Base;
    unsigned long Used;
    unsigned long pos;
    unsigned long len;
    unsigned long sft;
  }
random_bitio_state;


#define ENCODE_START(f,l)						\
  {									\
    register FILE *__file = f;						\
    register unsigned char *__buf;					\
    register unsigned long __pos = 0;					\
    register unsigned long __base = 0;					\
    register unsigned long __used = 0;					\
    register unsigned long __len = (l)-1;				\
    register unsigned long __sft = 0;					\
    while (__len) { __sft++; __len >>=1; }				\
    __len = 1<<__sft;							\
    __buf = Xmalloc(__len);						\
    fseek(__file, 0, 0);						\
    fread(__buf, 1, __len, __file);

#define ENCODE_CONTINUE(b)						\
  {									\
    register FILE *__file = (b).File;					\
    register unsigned char *__buf = (b).Buf;				\
    register unsigned long __pos = (b).pos;				\
    register unsigned long __base = (b).Base;				\
    register unsigned long __used = (b).Used;				\
    register unsigned long __len = (b).len;				\
    register unsigned long __sft = (b).sft;

#define SEEK fprintf(stderr, "Seek to %d\n",__base)
#define READ fprintf(stderr, "Read of %d\n",__len)
#define WRITE fprintf(stderr, "Write of %d\n",__used)

#define WRITE_READ 							\
	(__used ? (fseek(__file, __base, 0),				\
                   fwrite(__buf, 1, __len, __file)) : 0,		\
        __base += __len,						\
	fseek(__file, __base, 0),					\
        fread(__buf, 1, __len, __file),					\
	__pos = 0, __used = 0)

#define ENCODE_BIT(b)							\
  do {									\
    if (b)								\
      __buf[__pos>>3] |= 0x80 >> (__pos&7);				\
    else								\
      __buf[__pos>>3] &= 0xff7f >> (__pos&7);				\
    __used = 1;								\
    __pos++;								\
    if ((__pos>>3) >= __len)						\
      (void)WRITE_READ;							\
  } while(0)

#define ENCODE_PAUSE(b)							\
    (b).File = __file;							\
    (b).Buf = __buf;							\
    (b).pos = __pos;							\
    (b).Base = __base;							\
    (b).Used = __used;							\
    (b).len = __len;							\
    (b).sft = __sft;							\
  }

#define ENCODE_FLUSH							\
  if (__used)								\
    {									\
      fseek(__file, __base, 0);						\
      fwrite(__buf, 1, __len, __file);					\
      __used = 0;							\
    }



#define ENCODE_DONE							\
    ENCODE_FLUSH;							\
    free(__buf);							\
  }


#define DECODE_START(f,l) ENCODE_START(f,l)

#define DECODE_CONTINUE(b) ENCODE_CONTINUE(b)

#define DECODE_BIT 							\
    (__pos++, (((__pos-1)>>3) >= __len ? WRITE_READ, __pos=1 : 0),	\
     ((__buf[(__pos-1)>>3] & (0x80 >> ((__pos-1)&7))) != 0))

#define DECODE_ADD_FF(b) ((b) += (b) + DECODE_BIT)

#define DECODE_ADD_00(b) DECODE_ADD_FF(b)

#define DECODE_DONE ENCODE_DONE

#define DECODE_PAUSE(b)	ENCODE_PAUSE(b)

#define ENCODE_SEEK(pos) 						\
  do {									\
    if ((((pos)>>3) >= __base) && ((((pos)+7)>>3) < (__base+__len)))   	\
      __pos = (pos)-(__base << 3); 					\
    else								\
      {									\
        ENCODE_FLUSH;							\
        __base = (pos) >> 3;						\
        __base = ((pos)  >> (__sft+3)) << __sft;			\
        fseek(__file, __base, 0);					\
        fread(__buf, 1, __len, __file);					\
        __pos = (pos)&7; 						\
        __pos = (pos)&((8 << __sft)-1);					\
      }									\
  } while(0)


#define DECODE_SEEK(pos) ENCODE_SEEK(pos)

#define ENCODE_TELL ((__base << 3) + __pos)

#define DECODE_TELL ENCODE_TELL




#ifdef GCC_HASCLASSVISIBILITY
    #pragma GCC visibility pop
#endif
#endif
