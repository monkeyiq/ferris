/**************************************************************************
 *
 * bitio_m.h -- Macros for bitio
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
 * $Id: bitio_m.h,v 1.2 2008/06/07 21:30:09 ben Exp $
 *
 **************************************************************************/


#ifndef H_BITIO_M
#define H_BITIO_M

#ifdef GCC_HASCLASSVISIBILITY
    #pragma GCC visibility push(default)
#endif

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef BIO_ENCODE_PROLOGUE
#define BIO_ENCODE_PROLOGUE
#endif

#ifndef BIO_DECODE_PROLOGUE
#define BIO_DECODE_PROLOGUE
#endif

#ifndef BIO_ENCODE_EPILOGUE
#define BIO_ENCODE_EPILOGUE
#endif

#ifndef BIO_DECODE_EPILOGUE
#define BIO_DECODE_EPILOGUE
#endif

#ifndef DECODE_ADD
#define DECODE_ADD(b) (b) += (b) + DECODE_BIT
#endif

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#define POSITIVE(f, x)							\
  if ((x)<=0) 								\
    fprintf(stderr,"Error: Cannot "#f" encode %lu\n",(unsigned long)x),exit(1);


#define CEILLOG_2(x,v)							\
do {									\
  register int _B_x  = (x) - 1;						\
  (v) = 0;								\
  for (; _B_x ; _B_x>>=1, (v)++);					\
} while(0)


#define FLOORLOG_2(x,v)							\
do {									\
  register int _B_x  = (x);						\
  (v) = -1;								\
  for (; _B_x ; _B_x>>=1, (v)++);					\
} while(0)


/****************************************************************************/



#define UNARY_ENCODE(x)							\
do {									\
  register unsigned long _B_x = (x);					\
  BIO_ENCODE_PROLOGUE;							\
  POSITIVE(unary, _B_x);						\
  while(--_B_x) ENCODE_BIT(0);						\
  ENCODE_BIT(1);							\
  BIO_ENCODE_EPILOGUE;							\
} while(0)

#define UNARY_ENCODE_L(x, count)					\
do {									\
  register unsigned long _B_x = (x);					\
  BIO_ENCODE_PROLOGUE;							\
  POSITIVE(unary, _B_x);						\
  (count) += _B_x;							\
  while(--_B_x) ENCODE_BIT(0);						\
  ENCODE_BIT(1);							\
  BIO_ENCODE_EPILOGUE;							\
} while(0)

#define UNARY_DECODE(x)							\
do {									\
  BIO_DECODE_PROLOGUE;							\
  (x) = 1;								\
  while (!DECODE_BIT) (x)++;						\
  BIO_DECODE_EPILOGUE;							\
} while(0)

#define UNARY_DECODE_L(x, count)					\
do {									\
  BIO_DECODE_PROLOGUE;							\
  (x) = 1;								\
  while (!DECODE_BIT) (x)++;						\
  (count) += (x);							\
  BIO_DECODE_EPILOGUE;							\
} while(0)


#define UNARY_LENGTH(x, count)						\
do {									\
  POSITIVE(unary, x);							\
  (count) = (x);							\
} while(0)


/****************************************************************************/


#define BINARY_ENCODE(x, b)						\
do {									\
  register unsigned long _B_x = (x);					\
  register unsigned long _B_b = (b);					\
  register int _B_nbits, _B_logofb, _B_thresh;				\
  BIO_ENCODE_PROLOGUE;							\
  POSITIVE(binary, _B_x);						\
  CEILLOG_2(_B_b, _B_logofb);						\
  _B_thresh = (1<<_B_logofb) - _B_b;					\
  if (--_B_x < _B_thresh)						\
    _B_nbits = _B_logofb-1;						\
  else									\
    {									\
      _B_nbits = _B_logofb;						\
      _B_x += _B_thresh;						\
    }									\
  while (--_B_nbits>=0)							\
    ENCODE_BIT((_B_x>>_B_nbits) & 0x1);					\
  BIO_ENCODE_EPILOGUE;							\
} while(0)

#define BINARY_ENCODE_L(x, b, count)					\
do {									\
  register unsigned long _B_x = (x);					\
  register unsigned long _B_b = (b);					\
  register int _B_nbits, _B_logofb, _B_thresh;				\
  BIO_ENCODE_PROLOGUE;							\
  POSITIVE(binary, _B_x);						\
  CEILLOG_2(_B_b, _B_logofb);						\
  _B_thresh = (1<<_B_logofb) - _B_b;					\
  if (--_B_x < _B_thresh)						\
    _B_nbits = _B_logofb-1;						\
  else									\
    {									\
      _B_nbits = _B_logofb;						\
      _B_x += _B_thresh;						\
    }									\
  (count) += _B_nbits;							\
  while (--_B_nbits>=0)							\
    ENCODE_BIT((_B_x>>_B_nbits) & 0x1);					\
  BIO_ENCODE_EPILOGUE;							\
} while(0)


#define BINARY_DECODE(x, b)						\
do {									\
  register unsigned long _B_x = 0;					\
  register unsigned long _B_b = (b);					\
  register int _B_i, _B_logofb, _B_thresh;				\
  BIO_DECODE_PROLOGUE;							\
  if (_B_b != 1)							\
    {									\
      CEILLOG_2(_B_b, _B_logofb);					\
      _B_thresh = (1<<_B_logofb) - _B_b;				\
      _B_logofb--;							\
      for (_B_i=0; _B_i < _B_logofb; _B_i++)			 	\
        DECODE_ADD(_B_x);						\
      if (_B_x >= _B_thresh)						\
        {								\
          DECODE_ADD(_B_x);						\
          _B_x -= _B_thresh;						\
	}								\
      (x) = _B_x+1;							\
    }									\
  else									\
    (x) = 1;								\
  BIO_DECODE_EPILOGUE;							\
} while(0)

#define BINARY_DECODE_L(x, b, count)					\
do {									\
  register unsigned long _B_x = 0;					\
  register unsigned long _B_b = (b);					\
  register int _B_i, _B_logofb, _B_thresh;				\
  BIO_DECODE_PROLOGUE;							\
  if (_B_b != 1)							\
    {									\
      CEILLOG_2(_B_b, _B_logofb);					\
      _B_thresh = (1<<_B_logofb) - _B_b;				\
      _B_logofb--;							\
      (count) += _B_logofb;					       	\
      for (_B_i=0; _B_i < _B_logofb; _B_i++)			 	\
        DECODE_ADD(_B_x);						\
      if (_B_x >= _B_thresh)						\
        {								\
          DECODE_ADD(_B_x);						\
          _B_x -= _B_thresh;						\
          (count)++;							\
	}								\
      (x) = _B_x+1;							\
    }									\
  else									\
    (x) = 1;								\
  BIO_DECODE_EPILOGUE;							\
} while(0)

#define BINARY_LENGTH(x, b, count)					\
do {									\
  register unsigned long _B_x = (x);					\
  register unsigned long _B_b = (b);					\
  register int _B_nbits, _B_logofb, _B_thresh;				\
  POSITIVE(binary, _B_x);						\
  CEILLOG_2(_B_b, _B_logofb);						\
  _B_thresh = (1<<_B_logofb) - _B_b;					\
  if (--_B_x < _B_thresh)						\
    _B_nbits = _B_logofb-1;						\
  else									\
    _B_nbits = _B_logofb;						\
  (count) = _B_nbits;							\
} while(0)

/****************************************************************************/




#define GAMMA_ENCODE(x)							\
do {									\
  register unsigned long _B_xx = (x);					\
  register int _B_logofb;						\
  register int _B_nbits;						\
  BIO_ENCODE_PROLOGUE;							\
  POSITIVE(gamma, _B_xx);						\
  FLOORLOG_2(_B_xx, _B_logofb);						\
  _B_nbits = _B_logofb+1;						\
  while(_B_logofb--) ENCODE_BIT(0);					\
  while (--_B_nbits>=0)							\
    ENCODE_BIT((_B_xx>>_B_nbits) & 0x1);				\
  BIO_ENCODE_EPILOGUE;							\
} while (0)

#define GAMMA_ENCODE_L(x, count)					\
do {									\
  register unsigned long _B_xx = (x);					\
  register int _B_logofb;						\
  register int _B_nbits;						\
  BIO_ENCODE_PROLOGUE;							\
  POSITIVE(gamma, _B_xx);						\
  FLOORLOG_2(_B_xx, _B_logofb);						\
  _B_nbits = _B_logofb+1;						\
  (count) += _B_logofb*2+1;						\
  while(_B_logofb--) ENCODE_BIT(0);					\
  while (--_B_nbits>=0)							\
    ENCODE_BIT((_B_xx>>_B_nbits) & 0x1);				\
  BIO_ENCODE_EPILOGUE;							\
} while (0)

#define GAMMA_DECODE(x)							\
do {									\
  register unsigned long _B_xx = 1;					\
  register int _B_nbits = 0;						\
  BIO_DECODE_PROLOGUE;							\
  while(!DECODE_BIT) _B_nbits++;					\
  while (_B_nbits-- > 0)						\
    DECODE_ADD(_B_xx);							\
  (x) = _B_xx;								\
  BIO_DECODE_EPILOGUE;							\
} while (0)

#define GAMMA_DECODE_L(x, count)					\
do {									\
  register unsigned long _B_xx = 1;					\
  register int _B_nbits = 0;						\
  BIO_DECODE_PROLOGUE;							\
  while(!DECODE_BIT) _B_nbits++;					\
  (count) += _B_nbits*2+1;						\
  while (_B_nbits-- > 0)						\
    DECODE_ADD(_B_xx);							\
  (x) = _B_xx;								\
  BIO_DECODE_EPILOGUE;							\
} while (0)

#define GAMMA_LENGTH(x, count)						\
do {									\
  register unsigned long _B_xx = (x);					\
  register int _B_logofb;						\
  POSITIVE(gamma, _B_xx);						\
  FLOORLOG_2(_B_xx, _B_logofb);						\
  (count) = _B_logofb*2+1;						\
} while (0)



/****************************************************************************/


#define DELTA_ENCODE(x)							\
do {									\
  register unsigned long _B_xxx = (x);					\
  register int _B_logx;							\
  FLOORLOG_2(_B_xxx, _B_logx);						\
  GAMMA_ENCODE(_B_logx+1);						\
  BINARY_ENCODE(_B_xxx+1, 1<<_B_logx);					\
} while (0)

#define DELTA_ENCODE_L(x, count)					\
do {									\
  register unsigned long _B_xxx = (x);					\
  register int _B_logx;							\
  FLOORLOG_2(_B_xxx, _B_logx);						\
  GAMMA_ENCODE_L(_B_logx+1, count);					\
  BINARY_ENCODE_L(_B_xxx+1, 1<<_B_logx, count);				\
} while (0)


#define DELTA_DECODE(x)							\
do {									\
  register unsigned long _B_xxx;					\
  register int _B_logx;							\
  GAMMA_DECODE(_B_logx); _B_logx--;					\
  BINARY_DECODE(_B_xxx, 1<<_B_logx); _B_xxx--;				\
  (x) = _B_xxx + (1<<_B_logx);						\
} while (0)

#define DELTA_DECODE_L(x, count)					\
do {									\
  register unsigned long _B_xxx;					\
  register int _B_logx;							\
  GAMMA_DECODE_L(_B_logx, count); _B_logx--;				\
  BINARY_DECODE_L(_B_xxx, 1<<_B_logx, count); _B_xxx--;			\
  (x) = _B_xxx + (1<<_B_logx);						\
} while (0)

#define DELTA_LENGTH(x, count)						\
do {									\
  register unsigned long _B_xxx = (x);					\
  register int _B_logx, _B_dcount;					\
  FLOORLOG_2(_B_xxx, _B_logx);						\
  GAMMA_LENGTH(_B_logx+1, count);					\
  BINARY_LENGTH(_B_xxx+1, 1<<_B_logx, _B_dcount);			\
  (count) += _B_dcount;							\
} while (0)


/****************************************************************************/




#define ELIAS_ENCODE(x, b, s)						\
do {									\
  register unsigned long _B_xx = (x);					\
  register unsigned long _B_b = (b);					\
  register double _B_s = (s);						\
  register int _B_lower=1, _B_upper=1;					\
  register int _B_k=0;							\
  register double _B_pow=1.0;						\
  POSITIVE(elias, _B_xx);						\
  while (_B_xx>_B_upper) 						\
    {									\
      _B_k++;								\
      _B_lower = _B_upper+1;						\
      _B_pow *= _B_s;							\
      _B_upper += _B_b*_B_pow;						\
    }									\
  UNARY_ENCODE(_B_k+1);							\
  BINARY_ENCODE(_B_xx-_B_lower+1, _B_upper-_B_lower+1);			\
} while (0)

#define ELIAS_ENCODE_L(x, b, s, count)					\
do {									\
  register unsigned long _B_xx = (x);					\
  register unsigned long _B_b = (b);					\
  register double _B_s = (s);						\
  register int _B_lower=1, _B_upper=1;					\
  register int _B_k=0;							\
  register double _B_pow=1.0;						\
  POSITIVE(elias, _B_xx);						\
  while (_B_xx>_B_upper) 						\
    {									\
      _B_k++;								\
      _B_lower = _B_upper+1;						\
      _B_pow *= _B_s;							\
      _B_upper += _B_b*_B_pow;						\
    }									\
  UNARY_ENCODE_L(_B_k+1, count);					\
  BINARY_ENCODE_L(_B_xx-_B_lower+1, _B_upper-_B_lower+1, count);	\
} while (0)

#define ELIAS_DECODE(x, b, s)						\
do {									\
  register unsigned long _B_xx;						\
  register unsigned long _B_b = (b);					\
  register double _B_s = (s);						\
  register int _B_lower=1, _B_upper=1;					\
  register int _B_k;							\
  register double _B_pow=1.0;						\
  UNARY_DECODE(_B_k); _B_k--;						\
  while (_B_k--)							\
    {									\
      _B_lower = _B_upper+1;						\
      _B_pow *= _B_s;							\
      _B_upper += _B_b*_B_pow;						\
    }									\
  BINARY_DECODE(_B_xx, _B_upper-_B_lower+1); _B_xx--;			\
  POSITIVE(gamma, _B_xx+_B_lower);				  	\
  (x) = _B_xx+_B_lower;							\
} while (0)

#define ELIAS_DECODE_L(x, b, s, count)					\
do {									\
  register unsigned long _B_xx;						\
  register unsigned long _B_b = (b);					\
  register double _B_s = (s);						\
  register int _B_lower=1, _B_upper=1;					\
  register int _B_k;							\
  register double _B_pow=1.0;						\
  UNARY_DECODE_L(_B_k, count); _B_k--;					\
  while (_B_k--)							\
    {									\
      _B_lower = _B_upper+1;						\
      _B_pow *= _B_s;							\
      _B_upper += _B_b*_B_pow;						\
    }									\
  BINARY_DECODE_L(_B_xx, _B_upper-_B_lower+1, count); _B_xx--;		\
  POSITIVE(gamma, _B_xx+_B_lower);				  	\
  (x) = _B_xx+_B_lower;							\
} while (0)

#define ELIAS_LENGTH(x, b, s, count)					\
do {									\
  register unsigned long _B_xx = (x);					\
  register unsigned long _B_b = (b);					\
  register double _B_s = (s);						\
  register int _B_lower=1, _B_upper=1;					\
  register int _B_k=0, _B_ecount;					\
  register double _B_pow=1.0;						\
  POSITIVE(gamma, _B_xx);						\
  while (_B_xx>_B_upper) 						\
    {									\
      _B_k++;								\
      _B_lower = _B_upper+1;						\
      _B_pow *= _B_s;							\
      _B_upper += _B_b*_B_pow;						\
    }									\
  UNARY_LENGTH(_B_k+1, count);						\
  BINARY_LENGTH(_B_xx-_B_lower+1, _B_upper-_B_lower+1, _B_ecount);	\
  (count) += _B_ecount;							\
} while (0)


/****************************************************************************/


#define BBLOCK_ENCODE(x, b)						\
do {									\
  register unsigned long _B_xx = (x);					\
  register unsigned long _B_bb = (b);					\
  register int _B_xdivb = 0;						\
  POSITIVE(bblock, _B_xx);						\
  _B_xx--;								\
  while (_B_xx >= _B_bb) 						\
    {									\
      _B_xdivb++;							\
      _B_xx -= _B_bb;							\
    }									\
  UNARY_ENCODE(_B_xdivb+1);						\
  BINARY_ENCODE(_B_xx+1, _B_bb);					\
} while (0)

#define BBLOCK_ENCODE_L(x, b, count)					\
do {									\
  register unsigned long _B_xx = (x);					\
  register unsigned long _B_bb = (b);					\
  register int _B_xdivb = 0;						\
  POSITIVE(bblock, _B_xx);						\
  _B_xx--;								\
  while (_B_xx >= _B_bb) 						\
    {									\
      _B_xdivb++;							\
      _B_xx -= _B_bb;							\
    }									\
  UNARY_ENCODE_L(_B_xdivb+1, count);					\
  BINARY_ENCODE_L(_B_xx+1, _B_bb, count);				\
} while (0)

#define BBLOCK_DECODE(x, b)						\
do {									\
  register unsigned long _B_x1, _B_xx = 0;				\
  register unsigned long _B_bb = (b);					\
  register int _B_xdivb;						\
  UNARY_DECODE(_B_xdivb); _B_xdivb--;					\
  while (_B_xdivb--)							\
    _B_xx += _B_bb;							\
  BINARY_DECODE(_B_x1, _B_bb);						\
  (x) = _B_xx+_B_x1;							\
} while (0)

#define BBLOCK_DECODE_L(x, b, count)					\
do {									\
  register unsigned long _B_x1, _B_xx = 0;				\
  register unsigned long _B_bb = (b);					\
  register int _B_xdivb;						\
  UNARY_DECODE_L(_B_xdivb, count); _B_xdivb--;				\
  while (_B_xdivb--)							\
    _B_xx += _B_bb;							\
  BINARY_DECODE_L(_B_x1, _B_bb, count);					\
  (x) = _B_xx+_B_x1;							\
} while (0)

#define BBLOCK_LENGTH(x, b, count)					\
do {									\
  register unsigned long _B_bcount, _B_xx = (x);			\
  register unsigned long _B_bb = (b);					\
  register int _B_xdivb = 0;						\
  POSITIVE(bblock, _B_xx);						\
  _B_xx--;								\
  while (_B_xx >= _B_bb) 						\
    {									\
      _B_xdivb++;							\
      _B_xx -= _B_bb;							\
    }									\
  UNARY_LENGTH(_B_xdivb+1, count);					\
  BINARY_LENGTH(_B_xx+1, _B_bb, _B_bcount);				\
  (count) += _B_bcount;  						\
} while (0)

#ifdef GCC_HASCLASSVISIBILITY
    #pragma GCC visibility pop
#endif
#endif
