#define BMCOUNTOPT 1

// command to generate from bm 3.4.0
//  cat bmconst.h  bmfwd.h  bmdef.h bmfunc.h bmblocks.h    bmvmin.h bm.h encoding.h bmalloc.h bmundef.h | sed 's/#include "/\/\/#include "/g'  >|/tmp/b34.hh; dos2unix  /tmp/b34.hh
// add in this header and comment it out for C++
// Add in the SmallObj stuff.


//
// Small object allocation, by default both ptr_allocator and block_allocator use this to
// speed up small allocations.
//
#include <FerrisLoki/loki/SmallObj.h>


        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/


// This file contains the bitmagic bm.3.3.0 library source all bunched
// into one file.

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
///  LIC  //////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// BitMagic Library.

// BitMagic library is open source software and distributed under the
// MIT license. The only restriction is to mention the author in any
// work derived from this.




// Copyright (c) 2002-2003 Anatoliy Kuznetsov.

// Permission is hereby granted, free of charge, to any person 
// obtaining a copy of this software and associated documentation 
// files (the "Software"), to deal in the Software without restriction, 
// including without limitation the rights to use, copy, modify, merge, 
// publish, distribute, sublicense, and/or sell copies of the Software, 
// and to permit persons to whom the Software is furnished to do so, 
// subject to the following conditions:

// The above copyright notice and this permission notice shall be included 
// in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR 
// OTHER DEALINGS IN THE SOFTWARE.


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
///  README  ///////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// How to build BitMagic library:

// BM is small software package and you probably can just take the
// sources and put it into your project directly. All library sources are in src
// directory.

// However if you want to use our makefiles you need to follow the next simple
// instructions:


// Unix:
// -----

// - Apply environment variables by runing bmenv.sh :
// $ . bmenv.sh

// - use GNU make (gmake) to build installation.

// $gmake rebuild

// or (DEBUG version)
 
// $gmake DEBUG=YES rebuild

// The default compiler on Unix and CygWin is g++.
// If you want to change the default you can do that in makefile.in
// (should be pretty easy to do)


// Windows:
// --------

// If you use cygwin installation please follow general Unix recommendations.
// If you use MSVC please use supplied bm.dsw and corresponding project files.


// Fine tuning and optimizations:
// ------------------------------

// All BM fine tuning parameters are controlled by the preprocessor defines.


// =================================================================================

// BM library includes some code optimized for 64-bit systems. To turn this
// optimization on you need to #define BM64OPT in you makefile or in bm.h file.

// To turn SSE2 optimization #define BMSSE2OPT 
// You will need compiler supporting Intel SSE2 intrinsics. 
// It could be MSVC .Net or Intel C++. 

// The limitation is that you cannot use BM64OPT and BMSSE2OPT together.

// =================================================================================

// BM library supports "restrict" keyword, some compilers 
// (for example Intel C++ for Itanium) generate better
// code (out of order load-stores) when restrict keyword is helping. This option is 
// turned OFF by default since most of the C++ compilers does not support it. 
// To turn it ON please #define BM_HASRESTRICT in your project. Some compilers
// use "__restrict" keyword for this purpose. You can correct if by
// defineing BMRESTRICT macro to correct keyword. 

// =================================================================================

// Bitcounting optimization can be turned ON by defining BMCOUNTOPT.
// Please note this optimization is not completely thread safe. 
// bvector<> template keeps mutable variable inside it and update it 
// when it count() function is called. So it creates a certain chance that this
// function will be called from multiple threads and crash on updating this 
// variable. 

// =================================================================================

// If you want to use BM library in STL-free project you need to define
// BM_NO_STL variable. It will disable inclusion of certain headers and also 
// will make bvector iterators incompatible with STL algorithms 
// (which you said you are not using anyway).

// =================================================================================

// Different compilers use different conventions about return by value
// optimizations.

// For instance operator or for bitvector can be implemented in two
// different ways:

// --------- With explicit temp variable

// template<class A, class MS> 
// inline bvector<A, MS> operator| (const bvector<A, MS>& v1,
//                                  const bvector<A>& v2)
// {
//     bvector<A, MS> ret(v1);
//     ret.bit_or(v2);
//     return ret;
// }

// --------- Temp variable is hidden 

// template<class A, class MS> 
// inline bvector<A, MS> operator| (const bvector<A, MS>& v1,
//                                  const bvector<A>& v2)
// {
//     return bvector<A, MS>(v1).bit_or(v2);
// }

// ----------

// Some compilers can successfully optimize one or both of this cases.
// BM implementation uses hidden temp variable unless you define 
// BM_USE_EXPLICIT_TEMP

// =================================================================================




// Thank you for using BitMagic library!
// 	Anatoliy Kuznetsov (anatoliy_kuznetsov at yahoo.com)



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
///  bm.h    ///////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/*
Copyright(c) 2002-2005 Anatoliy Kuznetsov(anatoliy_kuznetsov at yahoo.com)

PermiFor more information please visit:  http://bmagic.sourceforge.net
ssion is hereby granted, free of charge, to any person 
obtaining a copy of this software and associated documentation 
files (the "Software"), to deal in the Software without restriction, 
including without limitation the rights to use, copy, modify, merge, 
publish, distribute, sublicense, and/or sell copies of the Software, 
and to permit persons to whom the Software is furnished to do so, 
subject to the following conditions:

The above copyright notice and this permission notice shall be included 
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR 
OTHER DEALINGS IN THE SOFTWARE.

For more information please visit:  http://bmagic.sourceforge.net

*/

#ifndef BM__H__INCLUDED__
#define BM__H__INCLUDED__

#include <string.h>
#include <assert.h>
#include <limits.h>

// define BM_NO_STL if you use BM in "STL free" environment and want
// to disable any references to STL headers
#ifndef BM_NO_STL
# include <iterator>
#endif


////////////////////////////////////////////////////////////////////////////////
//#include "bmconst.h"

/*
Copyright(c) 2002-2005 Anatoliy Kuznetsov(anatoliy_kuznetsov at yahoo.com)

Permission is hereby granted, free of charge, to any person 
obtaining a copy of this software and associated documentation 
files (the "Software"), to deal in the Software without restriction, 
including without limitation the rights to use, copy, modify, merge, 
publish, distribute, sublicense, and/or sell copies of the Software, 
and to permit persons to whom the Software is furnished to do so, 
subject to the following conditions:

The above copyright notice and this permission notice shall be included 
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR 
OTHER DEALINGS IN THE SOFTWARE.

For more information please visit:  http://bmagic.sourceforge.net

*/

#ifndef BMCONST__H__INCLUDED__
#define BMCONST__H__INCLUDED__

namespace FerrisBitMagic
{

#ifdef _WIN32

typedef unsigned __int64 id64_t;

#else

typedef unsigned long long id64_t;

#endif

typedef unsigned int   id_t;
typedef unsigned int   word_t;
typedef unsigned short short_t;



const unsigned id_max = 0xFFFFFFFF;

// Data Block parameters

const unsigned set_block_size  = 2048u;
const unsigned set_block_shift = 16u;
const unsigned set_block_mask  = 0xFFFFu;
const unsigned set_blkblk_mask = 0xFFFFFFu;

// Word parameters

const unsigned set_word_shift = 5u;
const unsigned set_word_mask  = 0x1Fu;


// GAP related parameters.

typedef unsigned short gap_word_t;

const unsigned gap_max_buff_len = 1280;
const unsigned gap_max_bits = 65536;
const unsigned gap_equiv_len = 
   (sizeof(FerrisBitMagic::word_t) * FerrisBitMagic::set_block_size) / sizeof(gap_word_t);
const unsigned gap_levels = 4;
const unsigned gap_max_level = FerrisBitMagic::gap_levels - 1;


// Block Array parameters

const unsigned set_array_size = 256u;
const unsigned set_array_shift = 8u;
const unsigned set_array_mask  = 0xFFu;
const unsigned set_total_blocks = (FerrisBitMagic::set_array_size * FerrisBitMagic::set_array_size);

const unsigned bits_in_block = FerrisBitMagic::set_block_size * sizeof(FerrisBitMagic::word_t) * 8;
const unsigned bits_in_array = FerrisBitMagic::bits_in_block * FerrisBitMagic::set_array_size;


#ifdef BM64OPT

typedef id64_t  wordop_t;
const id64_t    all_bits_mask = 0xffffffffffffffffULL;
    
# define DECLARE_TEMP_BLOCK(x)  FerrisBitMagic::id64_t x[FerrisBitMagic::set_block_size / 2]; 
const unsigned set_block_size_op  = FerrisBitMagic::set_block_size / 2;


#else

typedef word_t wordop_t;
const word_t all_bits_mask = 0xffffffff;

# define DECLARE_TEMP_BLOCK(x)  unsigned x[FerrisBitMagic::set_block_size]; 
const unsigned set_block_size_op  = FerrisBitMagic::set_block_size;

#endif

} // namespace

#endif


////////////////////////////////////////////////////////////////////////////////
//#include "bmdef.h"

// Copyright(c) 2002-2005 Anatoliy Kuznetsov(anatoliy_kuznetsov at yahoo.com)


// BM library internal header
//
// Set all required preprocessor defines



#ifndef BM_ASSERT

# ifndef BM_NOASSERT
#  include <assert.h>
#  define BM_ASSERT assert
# else
#  define BM_ASSERT(x)
# endif

#endif


#define FULL_BLOCK_ADDR all_set<true>::_block._p
#define IS_VALID_ADDR(addr) (addr && (addr != FULL_BLOCK_ADDR))
#define IS_FULL_BLOCK(addr) (addr == FULL_BLOCK_ADDR)
#define IS_EMPTY_BLOCK(addr) (addr == 0)

// Macro definitions to manipulate bits in pointers
// This trick is based on the fact that pointers allocated by malloc are
// aligned and bit 0 is never set. It means we are safe to use it.
// BM library keeps GAP flag in pointer.

// Note: this hack is not universally portable so if it does not work
// in some particular case disable it by defining BM_DISBALE_BIT_IN_PTR

#ifdef BM_DISBALE_BIT_IN_PTR

# define BMGAP_PTR(ptr)    ((FerrisBitMagic::gap_word_t*)ptr)
# define BMSET_PTRGAP(ptr) (void(0))
# define BM_IS_GAP(obj, ptr, idx) ( obj.is_block_gap(idx) ) 

#else

# if ULONG_MAX == 0xffffffff   // 32-bit

#  define BMPTR_SETBIT0(ptr)   ( ((FerrisBitMagic::id_t)ptr) | 1 )
#  define BMPTR_CLEARBIT0(ptr) ( ((FerrisBitMagic::id_t)ptr) & ~(FerrisBitMagic::id_t)1 )
#  define BMPTR_TESTBIT0(ptr)  ( ((FerrisBitMagic::id_t)ptr) & 1 )

# else // 64-bit

#  define BMPTR_SETBIT0(ptr)   ( ((FerrisBitMagic::id64_t)ptr) | 1 )
#  define BMPTR_CLEARBIT0(ptr) ( ((FerrisBitMagic::id64_t)ptr) & ~(FerrisBitMagic::id64_t)1 )
#  define BMPTR_TESTBIT0(ptr)  ( ((FerrisBitMagic::id64_t)ptr) & 1 )

# endif

# define BMGAP_PTR(ptr) ((FerrisBitMagic::gap_word_t*)BMPTR_CLEARBIT0(ptr))
# define BMSET_PTRGAP(ptr) ptr = (FerrisBitMagic::word_t*)BMPTR_SETBIT0(ptr)
# define BM_IS_GAP(obj, ptr, idx) ( BMPTR_TESTBIT0(ptr)!=0 )

#endif



#ifdef BM_HASRESTRICT
# ifndef BMRESTRICT
#  define BMRESTRICT restrict
# endif
#else
# define BMRESTRICT 
#endif




#ifndef BMSSE2OPT

# ifndef BM_SET_MMX_GUARD
#  define BM_SET_MMX_GUARD
# endif

#else

# ifndef BM_SET_MMX_GUARD
#  define BM_SET_MMX_GUARD  sse2_empty_guard  bm_mmx_guard_;
# endif

#endif


////////////////////////////////////////////////////////////////////////////////



// Vector based optimizations are incompatible with 64-bit optimization
// which is considered a form of vectorization
#ifdef BMSSE2OPT
# undef BM64OPT
# define BMVECTOPT
////////////////////////////////////////////////////////////////////////////////
//# include "bmsse2.h"
/*
Copyright(c) 2002-2005 Anatoliy Kuznetsov(anatoliy_kuznetsov at yahoo.com)

Permission is hereby granted, free of charge, to any person 
obtaining a copy of this software and associated documentation 
files (the "Software"), to deal in the Software without restriction, 
including without limitation the rights to use, copy, modify, merge, 
publish, distribute, sublicense, and/or sell copies of the Software, 
and to permit persons to whom the Software is furnished to do so, 
subject to the following conditions:

The above copyright notice and this permission notice shall be included 
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR 
OTHER DEALINGS IN THE SOFTWARE.

For more information please visit:  http://bmagic.sourceforge.net

*/


#ifndef BMSSE2__H__INCLUDED__
#define BMSSE2__H__INCLUDED__


//    Header implements processor specific intrinsics declarations for SSE2
//    instruction set
#include<emmintrin.h>



namespace FerrisBitMagic
{

/** @defgroup SSE2 Processor specific optimizations for SSE2 instructions
 *  @ingroup bmagic
 */


/*! 
  @brief SSE2 reinitialization guard class

  SSE2 requires to call _mm_empty() if we are intermixing
  MMX integer commands with floating point arithmetics.
  This class guards critical code fragments where SSE2 integer
  is used.

  @ingroup SSE2

*/
class sse2_empty_guard
{
public:
    __forceinline sse2_empty_guard() 
    {
        _mm_empty();
    }

    __forceinline ~sse2_empty_guard() 
    {
        _mm_empty();
    }
};

/*
# ifndef BM_SET_MMX_GUARD
#  define BM_SET_MMX_GUARD  sse2_empty_guard  bm_mmx_guard_;
# endif
*/

/*! 
    @brief XOR array elements to specified mask
    *dst = *src ^ mask

    @ingroup SSE2
*/
__forceinline 
void sse2_xor_arr_2_mask(__m128i* BMRESTRICT dst, 
                         const __m128i* BMRESTRICT src, 
                         const __m128i* BMRESTRICT src_end,
                         FerrisBitMagic::word_t mask)
{
     __m128i xmm2 = _mm_set_epi32(mask, mask, mask, mask);
     do
     {
        __m128i xmm1 = _mm_load_si128(src);

        xmm1 = _mm_xor_si128(xmm1, xmm2);
        _mm_store_si128(dst, xmm1);
        ++dst;
        ++src;

     } while (src < src_end);
}

/*! 
    @brief Inverts array elements and NOT them to specified mask
    *dst = ~*src & mask

    @ingroup SSE2
*/
__forceinline 
void sse2_andnot_arr_2_mask(__m128i* BMRESTRICT dst, 
                            const __m128i* BMRESTRICT src, 
                            const __m128i* BMRESTRICT src_end,
                            FerrisBitMagic::word_t mask)
{
     __m128i xmm2 = _mm_set_epi32(mask, mask, mask, mask);
     do
     {
        //_mm_prefetch((const char*)(src)+1024, _MM_HINT_NTA);
        //_mm_prefetch((const char*)(src)+1088, _MM_HINT_NTA);

        __m128i xmm1 = _mm_load_si128(src);

        xmm1 = _mm_andnot_si128(xmm1, xmm2); // xmm1 = (~xmm1) & xmm2 
        _mm_store_si128(dst, xmm1);
        ++dst;
        ++src;

     } while (src < src_end);
}

/*! 
    @brief AND array elements against another array
    *dst &= *src

    @ingroup SSE2
*/
__forceinline 
void sse2_and_arr(__m128i* BMRESTRICT dst, 
                  const __m128i* BMRESTRICT src, 
                  const __m128i* BMRESTRICT src_end)
{
    __m128i xmm1, xmm2;
    do
    {
        _mm_prefetch((const char*)(src)+512,  _MM_HINT_NTA);
    
        xmm1 = _mm_load_si128(src++);
        xmm2 = _mm_load_si128(dst);
        xmm1 = _mm_and_si128(xmm1, xmm2);
        _mm_store_si128(dst++, xmm1);
        
        xmm1 = _mm_load_si128(src++);
        xmm2 = _mm_load_si128(dst);
        xmm1 = _mm_and_si128(xmm1, xmm2);
        _mm_store_si128(dst++, xmm1);

        xmm1 = _mm_load_si128(src++);
        xmm2 = _mm_load_si128(dst);
        xmm1 = _mm_and_si128(xmm1, xmm2);
        _mm_store_si128(dst++, xmm1);

        xmm1 = _mm_load_si128(src++);
        xmm2 = _mm_load_si128(dst);
        xmm1 = _mm_and_si128(xmm1, xmm2);
        _mm_store_si128(dst++, xmm1);

    } while (src < src_end);

}



/*! 
    @brief OR array elements against another array
    *dst |= *src

    @ingroup SSE2
*/
__forceinline 
void sse2_or_arr(__m128i* BMRESTRICT dst, 
                 const __m128i* BMRESTRICT src, 
                 const __m128i* BMRESTRICT src_end)
{
    __m128i xmm1, xmm2;
    do
    {
        _mm_prefetch((const char*)(src)+512,  _MM_HINT_NTA);
    
        xmm1 = _mm_load_si128(src++);
        xmm2 = _mm_load_si128(dst);
        xmm1 = _mm_or_si128(xmm1, xmm2);
        _mm_store_si128(dst++, xmm1);
        
        xmm1 = _mm_load_si128(src++);
        xmm2 = _mm_load_si128(dst);
        xmm1 = _mm_or_si128(xmm1, xmm2);
        _mm_store_si128(dst++, xmm1);

        xmm1 = _mm_load_si128(src++);
        xmm2 = _mm_load_si128(dst);
        xmm1 = _mm_or_si128(xmm1, xmm2);
        _mm_store_si128(dst++, xmm1);

        xmm1 = _mm_load_si128(src++);
        xmm2 = _mm_load_si128(dst);
        xmm1 = _mm_or_si128(xmm1, xmm2);
        _mm_store_si128(dst++, xmm1);

    } while (src < src_end);
}

/*! 
    @brief OR array elements against another array
    *dst |= *src

    @ingroup SSE2
*/
__forceinline 
void sse2_xor_arr(__m128i* BMRESTRICT dst, 
                  const __m128i* BMRESTRICT src, 
                  const __m128i* BMRESTRICT src_end)
{
    __m128i xmm1, xmm2;
    do
    {
        _mm_prefetch((const char*)(src)+512,  _MM_HINT_NTA);
    
        xmm1 = _mm_load_si128(src++);
        xmm2 = _mm_load_si128(dst);
        xmm1 = _mm_xor_si128(xmm1, xmm2);
        _mm_store_si128(dst++, xmm1);
        
        xmm1 = _mm_load_si128(src++);
        xmm2 = _mm_load_si128(dst);
        xmm1 = _mm_xor_si128(xmm1, xmm2);
        _mm_store_si128(dst++, xmm1);

        xmm1 = _mm_load_si128(src++);
        xmm2 = _mm_load_si128(dst);
        xmm1 = _mm_xor_si128(xmm1, xmm2);
        _mm_store_si128(dst++, xmm1);

        xmm1 = _mm_load_si128(src++);
        xmm2 = _mm_load_si128(dst);
        xmm1 = _mm_xor_si128(xmm1, xmm2);
        _mm_store_si128(dst++, xmm1);

    } while (src < src_end);
}


/*! 
    @brief AND-NOT (SUB) array elements against another array
    *dst &= ~*src

    @ingroup SSE2
*/
__forceinline 
void sse2_sub_arr(__m128i* BMRESTRICT dst, 
                 const __m128i* BMRESTRICT src, 
                 const __m128i* BMRESTRICT src_end)
{
    __m128i xmm1, xmm2;
    do
    {
        _mm_prefetch((const char*)(src)+512,  _MM_HINT_NTA);
    
        xmm1 = _mm_load_si128(src++);
        xmm2 = _mm_load_si128(dst);
        xmm1 = _mm_andnot_si128(xmm1, xmm2);
        _mm_store_si128(dst++, xmm1);
        
        xmm1 = _mm_load_si128(src++);
        xmm2 = _mm_load_si128(dst);
        xmm1 = _mm_andnot_si128(xmm1, xmm2);
        _mm_store_si128(dst++, xmm1);

        xmm1 = _mm_load_si128(src++);
        xmm2 = _mm_load_si128(dst);
        xmm1 = _mm_andnot_si128(xmm1, xmm2);
        _mm_store_si128(dst++, xmm1);

        xmm1 = _mm_load_si128(src++);
        xmm2 = _mm_load_si128(dst);
        xmm1 = _mm_andnot_si128(xmm1, xmm2);
        _mm_store_si128(dst++, xmm1);

    } while (src < src_end);    
}

/*! 
    @brief SSE2 block memset
    *dst = value

    @ingroup SSE2
*/

__forceinline 
void sse2_set_block(__m128i* BMRESTRICT dst, 
                    __m128i* BMRESTRICT dst_end, 
                    FerrisBitMagic::word_t value)
{
    __m128i xmm0 = _mm_set_epi32 (value, value, value, value);
    do
    {            
        _mm_store_si128(dst, xmm0);
/*        
        _mm_store_si128(dst+1, xmm0);
        _mm_store_si128(dst+2, xmm0);
        _mm_store_si128(dst+3, xmm0);

        _mm_store_si128(dst+4, xmm0);
        _mm_store_si128(dst+5, xmm0);
        _mm_store_si128(dst+6, xmm0);
        _mm_store_si128(dst+7, xmm0);

        dst += 8;
*/        
    } while (++dst < dst_end);
    
    _mm_sfence();
}

/*! 
    @brief SSE2 block copy
    *dst = *src

    @ingroup SSE2
*/
__forceinline 
void sse2_copy_block(__m128i* BMRESTRICT dst, 
                     const __m128i* BMRESTRICT src, 
                     const __m128i* BMRESTRICT src_end)
{
    __m128i xmm0, xmm1, xmm2, xmm3;
    do
    {
        _mm_prefetch((const char*)(src)+512,  _MM_HINT_NTA);
    
        xmm0 = _mm_load_si128(src+0);
        xmm1 = _mm_load_si128(src+1);
        xmm2 = _mm_load_si128(src+2);
        xmm3 = _mm_load_si128(src+3);
        
        _mm_store_si128(dst+0, xmm0);
        _mm_store_si128(dst+1, xmm1);
        _mm_store_si128(dst+2, xmm2);
        _mm_store_si128(dst+3, xmm3);
        
        xmm0 = _mm_load_si128(src+4);
        xmm1 = _mm_load_si128(src+5);
        xmm2 = _mm_load_si128(src+6);
        xmm3 = _mm_load_si128(src+7);
        
        _mm_store_si128(dst+4, xmm0);
        _mm_store_si128(dst+5, xmm1);
        _mm_store_si128(dst+6, xmm2);
        _mm_store_si128(dst+7, xmm3);
        
        src += 8;
        dst += 8;
        
    } while (src < src_end);    
}


/*! 
    @brief Invert array elements
    *dst = ~*dst
    or
    *dst ^= *dst 

    @ingroup SSE2
*/
__forceinline 
void sse2_invert_arr(FerrisBitMagic::word_t* first, FerrisBitMagic::word_t* last)
{
    __m128i xmm1 = _mm_set_epi32(0xFFFFFFFF, 0xFFFFFFFF, 
                                 0xFFFFFFFF, 0xFFFFFFFF);
    __m128i* wrd_ptr = (__m128i*)first;

    do 
    {
        _mm_prefetch((const char*)(wrd_ptr)+512,  _MM_HINT_NTA);
        
        __m128i xmm0 = _mm_load_si128(wrd_ptr);
        xmm0 = _mm_xor_si128(xmm0, xmm1);
        _mm_store_si128(wrd_ptr, xmm0);
        ++wrd_ptr;
    } while (wrd_ptr < (__m128i*)last);
}



/*!
    SSE2 optimized bitcounting function implements parallel bitcounting
    algorithm for SSE2 instruction set.

<pre>
unsigned CalcBitCount32(unsigned b)
{
    b = (b & 0x55555555) + (b >> 1 & 0x55555555);
    b = (b & 0x33333333) + (b >> 2 & 0x33333333);
    b = (b + (b >> 4)) & 0x0F0F0F0F;
    b = b + (b >> 8);
    b = (b + (b >> 16)) & 0x0000003F;
    return b;
}
</pre>

    @ingroup SSE2

*/
inline 
FerrisBitMagic::id_t sse2_bit_count(const __m128i* block, const __m128i* block_end)
{
    const unsigned mu1 = 0x55555555;
    const unsigned mu2 = 0x33333333;
    const unsigned mu3 = 0x0F0F0F0F;
    const unsigned mu4 = 0x0000003F;

    // Loading masks
    __m128i m1 = _mm_set_epi32 (mu1, mu1, mu1, mu1);
    __m128i m2 = _mm_set_epi32 (mu2, mu2, mu2, mu2);
    __m128i m3 = _mm_set_epi32 (mu3, mu3, mu3, mu3);
    __m128i m4 = _mm_set_epi32 (mu4, mu4, mu4, mu4);
    __m128i mcnt;
    mcnt = _mm_xor_si128(m1, m1); // cnt = 0

    __m128i tmp1, tmp2;
    do
    {        
        __m128i b = _mm_load_si128(block);
        ++block;

        // b = (b & 0x55555555) + (b >> 1 & 0x55555555);
        tmp1 = _mm_srli_epi32(b, 1);                    // tmp1 = (b >> 1 & 0x55555555)
        tmp1 = _mm_and_si128(tmp1, m1); 
        tmp2 = _mm_and_si128(b, m1);                    // tmp2 = (b & 0x55555555)
        b    = _mm_add_epi32(tmp1, tmp2);               //  b = tmp1 + tmp2

        // b = (b & 0x33333333) + (b >> 2 & 0x33333333);
        tmp1 = _mm_srli_epi32(b, 2);                    // (b >> 2 & 0x33333333)
        tmp1 = _mm_and_si128(tmp1, m2); 
        tmp2 = _mm_and_si128(b, m2);                    // (b & 0x33333333)
        b    = _mm_add_epi32(tmp1, tmp2);               // b = tmp1 + tmp2

        // b = (b + (b >> 4)) & 0x0F0F0F0F;
        tmp1 = _mm_srli_epi32(b, 4);                    // tmp1 = b >> 4
        b = _mm_add_epi32(b, tmp1);                     // b = b + (b >> 4)
        b = _mm_and_si128(b, m3);                       //           & 0x0F0F0F0F

        // b = b + (b >> 8);
        tmp1 = _mm_srli_epi32 (b, 8);                   // tmp1 = b >> 8
        b = _mm_add_epi32(b, tmp1);                     // b = b + (b >> 8)

        // b = (b + (b >> 16)) & 0x0000003F;
        tmp1 = _mm_srli_epi32 (b, 16);                  // b >> 16
        b = _mm_add_epi32(b, tmp1);                     // b + (b >> 16)
        b = _mm_and_si128(b, m4);                       // (b >> 16) & 0x0000003F;

        mcnt = _mm_add_epi32(mcnt, b);                  // mcnt += b

    } while (block < block_end);

    __declspec(align(16)) FerrisBitMagic::id_t tcnt[4];
    _mm_store_si128((__m128i*)tcnt, mcnt);

    return tcnt[0] + tcnt[1] + tcnt[2] + tcnt[3];
}

__forceinline 
__m128i sse2_and(__m128i a, __m128i b)
{
    return _mm_and_si128(a, b);
}

__forceinline 
__m128i sse2_or(__m128i a, __m128i b)
{
    return _mm_or_si128(a, b);
}


__forceinline 
__m128i sse2_xor(__m128i a, __m128i b)
{
    return _mm_xor_si128(a, b);
}

__forceinline 
__m128i sse2_sub(__m128i a, __m128i b)
{
    return _mm_andnot_si128(b, a);
}


template<class Func>
FerrisBitMagic::id_t sse2_bit_count_op(const __m128i* BMRESTRICT block, 
                           const __m128i* BMRESTRICT block_end,
                           const __m128i* BMRESTRICT mask_block,
                           Func sse2_func)
{
    const unsigned mu1 = 0x55555555;
    const unsigned mu2 = 0x33333333;
    const unsigned mu3 = 0x0F0F0F0F;
    const unsigned mu4 = 0x0000003F;

    // Loading masks
    __m128i m1 = _mm_set_epi32 (mu1, mu1, mu1, mu1);
    __m128i m2 = _mm_set_epi32 (mu2, mu2, mu2, mu2);
    __m128i m3 = _mm_set_epi32 (mu3, mu3, mu3, mu3);
    __m128i m4 = _mm_set_epi32 (mu4, mu4, mu4, mu4);
    __m128i mcnt;
    mcnt = _mm_xor_si128(m1, m1); // cnt = 0
    do
    {
        __m128i tmp1, tmp2;
        __m128i b = _mm_load_si128(block++);

        tmp1 = _mm_load_si128(mask_block++);
        
        b = sse2_func(b, tmp1);
                        
        // b = (b & 0x55555555) + (b >> 1 & 0x55555555);
        tmp1 = _mm_srli_epi32(b, 1);                    // tmp1 = (b >> 1 & 0x55555555)
        tmp1 = _mm_and_si128(tmp1, m1); 
        tmp2 = _mm_and_si128(b, m1);                    // tmp2 = (b & 0x55555555)
        b    = _mm_add_epi32(tmp1, tmp2);               //  b = tmp1 + tmp2

        // b = (b & 0x33333333) + (b >> 2 & 0x33333333);
        tmp1 = _mm_srli_epi32(b, 2);                    // (b >> 2 & 0x33333333)
        tmp1 = _mm_and_si128(tmp1, m2); 
        tmp2 = _mm_and_si128(b, m2);                    // (b & 0x33333333)
        b    = _mm_add_epi32(tmp1, tmp2);               // b = tmp1 + tmp2

        // b = (b + (b >> 4)) & 0x0F0F0F0F;
        tmp1 = _mm_srli_epi32(b, 4);                    // tmp1 = b >> 4
        b = _mm_add_epi32(b, tmp1);                     // b = b + (b >> 4)
        b = _mm_and_si128(b, m3);                       //           & 0x0F0F0F0F

        // b = b + (b >> 8);
        tmp1 = _mm_srli_epi32 (b, 8);                   // tmp1 = b >> 8
        b = _mm_add_epi32(b, tmp1);                     // b = b + (b >> 8)
        
        // b = (b + (b >> 16)) & 0x0000003F;
        tmp1 = _mm_srli_epi32 (b, 16);                  // b >> 16
        b = _mm_add_epi32(b, tmp1);                     // b + (b >> 16)
        b = _mm_and_si128(b, m4);                       // (b >> 16) & 0x0000003F;

        mcnt = _mm_add_epi32(mcnt, b);                  // mcnt += b

    } while (block < block_end);

    __declspec(align(16)) FerrisBitMagic::id_t tcnt[4];
    _mm_store_si128((__m128i*)tcnt, mcnt);

    return tcnt[0] + tcnt[1] + tcnt[2] + tcnt[3];
}




#define VECT_XOR_ARR_2_MASK(dst, src, src_end, mask)\
    sse2_xor_arr_2_mask((__m128i*)(dst), (__m128i*)(src), (__m128i*)(src_end), mask)

#define VECT_ANDNOT_ARR_2_MASK(dst, src, src_end, mask)\
    sse2_andnot_arr_2_mask((__m128i*)(dst), (__m128i*)(src), (__m128i*)(src_end), mask)

#define VECT_BITCOUNT(first, last) \
    sse2_bit_count((__m128i*) (first), (__m128i*) (last)) 

#define VECT_BITCOUNT_AND(first, last, mask) \
    sse2_bit_count_op((__m128i*) (first), (__m128i*) (last), (__m128i*) (mask), sse2_and) 

#define VECT_BITCOUNT_OR(first, last, mask) \
    sse2_bit_count_op((__m128i*) (first), (__m128i*) (last), (__m128i*) (mask), sse2_or) 

#define VECT_BITCOUNT_XOR(first, last, mask) \
    sse2_bit_count_op((__m128i*) (first), (__m128i*) (last), (__m128i*) (mask), sse2_xor) 

#define VECT_BITCOUNT_SUB(first, last, mask) \
    sse2_bit_count_op((__m128i*) (first), (__m128i*) (last), (__m128i*) (mask), sse2_sub) 

#define VECT_INVERT_ARR(first, last) \
    sse2_invert_arr(first, last);

#define VECT_AND_ARR(dst, src, src_end) \
    sse2_and_arr((__m128i*) dst, (__m128i*) (src), (__m128i*) (src_end))

#define VECT_OR_ARR(dst, src, src_end) \
    sse2_or_arr((__m128i*) dst, (__m128i*) (src), (__m128i*) (src_end))

#define VECT_SUB_ARR(dst, src, src_end) \
    sse2_sub_arr((__m128i*) dst, (__m128i*) (src), (__m128i*) (src_end))

#define VECT_XOR_ARR(dst, src, src_end) \
    sse2_xor_arr((__m128i*) dst, (__m128i*) (src), (__m128i*) (src_end))

#define VECT_COPY_BLOCK(dst, src, src_end) \
    sse2_copy_block((__m128i*) dst, (__m128i*) (src), (__m128i*) (src_end))

#define VECT_SET_BLOCK(dst, dst_end, value) \
    sse2_set_block((__m128i*) dst, (__m128i*) (dst_end), (value))

} // namespace

#endif
////////////////////////////////////////////////////////////////////////////////
#endif


////////////////////////////////////////////////////////////////////////////////
//#include "bmfwd.h"
/*
Copyright(c) 2002-2005 Anatoliy Kuznetsov(anatoliy_kuznetsov at yahoo.com)

Permission is hereby granted, free of charge, to any person 
obtaining a copy of this software and associated documentation 
files (the "Software"), to deal in the Software without restriction, 
including without limitation the rights to use, copy, modify, merge, 
publish, distribute, sublicense, and/or sell copies of the Software, 
and to permit persons to whom the Software is furnished to do so, 
subject to the following conditions:

The above copyright notice and this permission notice shall be included 
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR 
OTHER DEALINGS IN THE SOFTWARE.

For more information please visit:  http://bmagic.sourceforge.net

*/

#ifndef BMFWD__H__INCLUDED__
#define BMFWD__H__INCLUDED__

//#include "bmconst.h"

namespace FerrisBitMagic
{

class block_allocator;
class ptr_allocator;

template<class BA = block_allocator, class PA = ptr_allocator> class mem_alloc;

template <class A, size_t N> class miniset;
template<size_t N> class bvmini;

typedef FerrisBitMagic::bvmini<FerrisBitMagic::set_total_blocks> standard_miniset;
typedef mem_alloc<block_allocator, ptr_allocator> standard_allocator;

template<class A = FerrisBitMagic::standard_allocator,  
         class MS = FerrisBitMagic::standard_miniset> 
class bvector;


} // namespace

#endif
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//#include "bmfunc.h"
/*
// Copyright(c) 2002-2005 Anatoliy Kuznetsov(anatoliy_kuznetsov at yahoo.com)

Permission is hereby granted, free of charge, to any person 
obtaining a copy of this software and associated documentation 
files (the "Software"), to deal in the Software without restriction, 
including without limitation the rights to use, copy, modify, merge, 
publish, distribute, sublicense, and/or sell copies of the Software, 
and to permit persons to whom the Software is furnished to do so, 
subject to the following conditions:

The above copyright notice and this permission notice shall be included 
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR 
OTHER DEALINGS IN THE SOFTWARE.

For more information please visit:  http://bmagic.sourceforge.net

*/

#ifndef BMFUNC__H__INCLUDED__
#define BMFUNC__H__INCLUDED__

#include <memory.h>


#ifdef _MSC_VER
# pragma warning( disable: 4146 )
#endif

namespace FerrisBitMagic
{


/*! @defgroup gapfunc GAP functions
 *  GAP functions implement different opereations on GAP compressed blocks
 *  and serve as a minimal building blocks.
 *  @ingroup bmagic
 *
 */

/*! @defgroup bitfunc BIT functions
 *  Bit functions implement different opereations on bit blocks
 *  and serve as a minimal building blocks.
 *  @ingroup bmagic
 */


/*! @brief Default GAP lengths table.
    @ingroup gapfunc
*/
template<bool T> struct gap_len_table
{
    static const gap_word_t _len[FerrisBitMagic::gap_levels];
};

template<bool T>
const gap_word_t gap_len_table<T>::_len[FerrisBitMagic::gap_levels] = 
                { 128, 256, 512, FerrisBitMagic::gap_max_buff_len }; 


/*! @brief Alternative GAP lengths table. 
    Good for for memory saver mode and very sparse bitsets.

    @ingroup gapfunc
*/
template<bool T> struct gap_len_table_min
{
    static const gap_word_t _len[FerrisBitMagic::gap_levels];
};

template<bool T>
const gap_word_t gap_len_table_min<T>::_len[FerrisBitMagic::gap_levels] = 
                                { 32, 96, 128, 512 }; 


//---------------------------------------------------------------------

/** Structure to aid in counting bits
    table contains count of bits in 0-255 diapason of numbers

   @ingroup bitfunc
*/
template<bool T> struct bit_count_table 
{
  static const unsigned char _count[256];
};

template<bool T>
const unsigned char bit_count_table<T>::_count[256] = {
    0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
    1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
    1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
    2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
    1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
    2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
    2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
    3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,4,5,5,6,5,6,6,7,5,6,6,7,6,7,7,8
};

//---------------------------------------------------------------------

/** Structure keeps all-left/right ON bits masks. 
    @ingroup bitfunc 
*/
template<bool T> struct block_set_table
{
    static const unsigned _left[32];
    static const unsigned _right[32];
};

template<bool T>
const unsigned block_set_table<T>::_left[32] = {
    0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff, 0x1ff, 0x3ff, 0x7ff,
    0xfff, 0x1fff, 0x3fff, 0x7fff, 0xffff, 0x1ffff, 0x3ffff, 0x7ffff,
    0xfffff, 0x1fffff, 0x3fffff, 0x7fffff, 0xffffff, 0x1ffffff, 0x3ffffff,
    0x7ffffff, 0xfffffff, 0x1fffffff, 0x3fffffff, 0x7fffffff, 0xffffffff
};

template<bool T>
const unsigned block_set_table<T>::_right[32] = {
    0xffffffff, 0xfffffffe, 0xfffffffc, 0xfffffff8, 0xfffffff0,
    0xffffffe0, 0xffffffc0, 0xffffff80, 0xffffff00, 0xfffffe00,
    0xfffffc00, 0xfffff800, 0xfffff000, 0xffffe000, 0xffffc000,
    0xffff8000, 0xffff0000, 0xfffe0000, 0xfffc0000, 0xfff80000,
    0xfff00000, 0xffe00000, 0xffc00000, 0xff800000, 0xff000000,
    0xfe000000, 0xfc000000, 0xf8000000, 0xf0000000, 0xe0000000,
    0xc0000000, 0x80000000
};



/** Structure keeps index of first ON bit for every byte.  
    @ingroup bitfunc 
*/
template<bool T> struct first_bit_table
{
    static const char _idx[256];
};

template<bool T>
const char first_bit_table<T>::_idx[256] = {
    -1,
    0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,
    1,0,5,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1, 0,2,0,1,0,3,0,
    1,0,2,0,1,0,6,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,
    0,3,0,1,0,2,0,1,0,5,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,
    2,0,1,0,3,0,1,0,2,0,1,0,7,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,
    0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,5,0,1,0,2,0,1,0,3,0,1,0,2,0,
    1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,6,0,1,0,2,0,1,0,3,0,1,
    0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,5,0,1,0,2,0,1,0,
    3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0 
};


/*! 
	Define calculates number of 1 bits in 32-bit word.
    @ingroup bitfunc 
*/

#define BM_INCWORD_BITCOUNT(cnt, w) cnt += \
    FerrisBitMagic::bit_count_table<true>::_count[(unsigned char)(w)] + \
    FerrisBitMagic::bit_count_table<true>::_count[(unsigned char)((w) >> 8)] + \
    FerrisBitMagic::bit_count_table<true>::_count[(unsigned char)((w) >> 16)] + \
    FerrisBitMagic::bit_count_table<true>::_count[(unsigned char)((w) >> 24)];

#ifdef BM64OPT
/*! 
	Function calculates number of 1 bits in 64-bit word.
    @ingroup bitfunc 
*/
inline FerrisBitMagic::id_t word_bitcount64(FerrisBitMagic::id64_t w)
{
    w = (w & 0x5555555555555555ULL) + (w >> 1 & 0x5555555555555555ULL);
    w = (w & 0x3333333333333333ULL) + (w >> 2 & 0x3333333333333333ULL);
    w = w + (w >> 4) & 0x0F0F0F0F0F0F0F0FULL;
    w = w + (w >> 8);
    w = w + (w >> 16);
    w = w + (w >> 32) & 0x0000007F;
    return (FerrisBitMagic::id_t)w;
}
#endif



//---------------------------------------------------------------------

/**
    Bit operations enumeration.
*/
enum operation
{
    BM_AND = 0,
    BM_OR,
    BM_SUB,
    BM_XOR
};

//---------------------------------------------------------------------

/** 
    Structure carries pointer on bit block with all bits 1
    @ingroup bitfunc 
*/
template<bool T> struct all_set
{
    struct all_set_block
    {
        FerrisBitMagic::word_t _p[FerrisBitMagic::set_block_size];

        all_set_block()
        {
            ::memset(_p, 0xFF, sizeof(_p));
        }
    };

    static all_set_block  _block;
};


template<bool T> typename all_set<T>::all_set_block all_set<T>::_block;



//---------------------------------------------------------------------

/*! 
   \brief Lexicographical comparison of two words as bit strings.
   Auxiliary implementation for testing and reference purposes.
   \param buf1 - First word.
   \param buf2 - Second word.
   \return  <0 - less, =0 - equal,  >0 - greater.

   @ingroup bitfunc 
*/
template<typename T> int wordcmp0(T w1, T w2)
{
    while (w1 != w2)
    {
        int res = (w1 & 1) - (w2 & 1);
        if (res != 0) return res;
        w1 >>= 1;
        w2 >>= 1;
    }
    return 0;
}


/*! 
   \brief Lexicographical comparison of two words as bit strings.
   Auxiliary implementation for testing and reference purposes.
   \param buf1 - First word.
   \param buf2 - Second word.
   \return  <0 - less, =0 - equal,  >0 - greater.

   @ingroup bitfunc 
*/
/*
template<typename T> int wordcmp(T w1, T w2)
{
    T diff = w1 ^ w2;
    return diff ? ((w1 & diff & (diff ^ (diff - 1)))? 1 : -1) : 0; 
}
*/

template<typename T> int wordcmp(T a, T b)
{
    T diff = a ^ b;
    return diff? ( (a & diff & -diff)? 1 : -1 ) : 0;
}


// Low bit extraction
// x & (x ^ (x-1))


/**
    Internal structure. Copyright information.
*/
template<bool T> struct _copyright
{
    static const char _p[];
};

template<bool T> const char _copyright<T>::_p[] = 
    "BitMagic Library. v.3.3.0 (c) 2002-2005 Anatoliy Kuznetsov.";


/*! 
   \brief Byte orders recognized by the library.
*/
enum ByteOrder
{
	BigEndian    = 0,
	LittleEndian = 1
};


/**
    Internal structure. Different global settings.
*/
template<bool T> struct globals
{
    struct bo
    {
        ByteOrder  _byte_order;

        bo()
        {
            unsigned x;
	        unsigned char *s = (unsigned char *)&x;
            s[0] = 1;
            s[1] = 2;
            s[2] = 3;
            s[3] = 4;

            if(x == 0x04030201) 
            {
                _byte_order = LittleEndian;
                return;
            }

            if(x == 0x01020304) 
            {
                _byte_order = BigEndian;
                return;
            }

            BM_ASSERT(0); // "Invalid Byte Order\n"
	        _byte_order = LittleEndian;
        }
    };

    static bo  _bo;

    static ByteOrder byte_order() { return _bo._byte_order; }

};

template<bool T> typename globals<T>::bo globals<T>::_bo;






/*
   \brief Binary search for the block where bit = pos located.
   \param buf - GAP buffer pointer.
   \param pos - index of the element.
   \param is_set - output. GAP value (0 or 1). 
   \return GAP index.
   @ingroup gapfunc
*/
template<typename T> 
unsigned gap_bfind(const T* buf, unsigned pos, unsigned* is_set)
{
    BM_ASSERT(pos < FerrisBitMagic::gap_max_bits);
	*is_set = (*buf) & 1;

	register unsigned start = 1;
	register unsigned end = 1 + ((*buf) >> 3);

	while ( start != end )
	{
		unsigned curr = (start + end) >> 1;
		if ( buf[curr] < pos )
			start = curr + 1;
		else
			end = curr;
	}
	*is_set ^= ((start-1) & 1);
	return start; 
}


/*!
   \brief Tests if bit = pos is true.
   \param buf - GAP buffer pointer.
   \param pos - index of the element.
   \return true if position is in "1" gap
   @ingroup gapfunc
*/
template<typename T> unsigned gap_test(const T* buf, unsigned pos)
{
    BM_ASSERT(pos < FerrisBitMagic::gap_max_bits);

	unsigned start = 1;
    unsigned end = 1 + ((*buf) >> 3);

    if (end - start < 10)
    {
        unsigned sv = *buf & 1;
        unsigned sv1= sv ^ 1;
        if (buf[1] >= pos) return sv;
        if (buf[2] >= pos) return sv1;
        if (buf[3] >= pos) return sv;
        if (buf[4] >= pos) return sv1;
        if (buf[5] >= pos) return sv;
        if (buf[6] >= pos) return sv1;
        if (buf[7] >= pos) return sv;
        if (buf[8] >= pos) return sv1;
        if (buf[9] >= pos) return sv;
        BM_ASSERT(0);
    }
    else
	while ( start != end )
	{
		unsigned curr = (start + end) >> 1;
		if ( buf[curr] < pos )
			start = curr + 1;
		else
			end = curr;
	}
	return ((*buf) & 1) ^ ((--start) & 1); 
}


/*! For each non-zero block executes supplied function.
*/
template<class T, class F> 
void for_each_nzblock(T*** root, unsigned size1, unsigned size2, F& f)
{
    unsigned block_idx = 0;

    for (unsigned i = 0; i < size1; ++i)
    {
        T** blk_blk = root[i];

        if (!blk_blk) 
        {
            block_idx += FerrisBitMagic::set_array_size;
            continue;
        }

        for (unsigned j = 0;j < size2; ++j, ++block_idx)
        {
            if (blk_blk[j]) f(blk_blk[j], block_idx);
        }
    }  
}

/*! For each non-zero block executes supplied function-predicate.
    Function returns if function-predicate returns true
*/
template<class T, class F> 
bool for_each_nzblock_if(T*** root, unsigned size1, unsigned size2, F& f)
{
    unsigned block_idx = 0;

    for (unsigned i = 0; i < size1; ++i)
    {
        T** blk_blk = root[i];

        if (!blk_blk) 
        {
            block_idx += FerrisBitMagic::set_array_size;
            continue;
        }

        for (unsigned j = 0;j < size2; ++j, ++block_idx)
        {
            if (blk_blk[j]) 
                if (f(blk_blk[j], block_idx)) return true;
        }
    }
    return false;
}

/*! For each block executes supplied function.
*/
template<class T, class F> 
void for_each_block(T*** root, unsigned size1, unsigned size2, F& f)
{
    unsigned block_idx = 0;

    for (unsigned i = 0; i < size1; ++i)
    {
        T** blk_blk = root[i];

        if (blk_blk)
        {
            for (unsigned j = 0;j < size2; ++j, ++block_idx)
            {
                f(blk_blk[j], block_idx);
            }
        }
        else
        {
            for (unsigned j = 0;j < size2; ++j, ++block_idx)
            {
                f(0, block_idx);
            }
        }
    }  
}



/*! Special BM optimized analog of STL for_each
*/
template<class T, class F> F bmfor_each(T first, T last, F f)
{
    do
    {
        f(*first);
        ++first;
    } while (first < last);
    return f;
}

/*! Computes SUM of all elements of the sequence
*/
template<class T> T sum_arr(T* first, T* last)
{
    T sum = 0;
    while (first < last)
    {
        sum += *first;
        ++first;
    }
    return sum;
}


/*! 
   \brief Calculates number of bits ON in GAP buffer.
   \param buf - GAP buffer pointer.
   \return Number of non-zero bits.
   @ingroup gapfunc
*/
template<typename T> unsigned gap_bit_count(const T* buf) 
{
    register const T* pcurr = buf;
    register const T* pend = pcurr + (*pcurr >> 3);

    register unsigned bits_counter = 0;
    ++pcurr;

    if (*buf & 1)
    {
        bits_counter += *pcurr + 1;
        ++pcurr;
    }
    ++pcurr;  // set GAP to 1

    while (pcurr <= pend)
    {
        bits_counter += *pcurr - *(pcurr-1);
        pcurr += 2; // jump to the next positive GAP
    } 

    return bits_counter;
}

/*!
   \brief Counts 1 bits in GAP buffer in the closed [left, right] diapason.
   \param buf - GAP buffer pointer.
   \param left - leftmost bit index to start from
   \param right- rightmost bit index
   \return Number of non-zero bits.
*/
template<typename T>
unsigned gap_bit_count_range(const T* buf, T left, T right)
{
    BM_ASSERT(left <= right);
    
    const T* pcurr = buf;
    const T* pend = pcurr + (*pcurr >> 3);
    
    unsigned bits_counter = 0;
    unsigned is_set;
    unsigned start_pos = gap_bfind(buf, left, &is_set);

    pcurr = buf + start_pos;
    if (right <= *pcurr) // we are in the target block right now
    {
        if (is_set)
            bits_counter = (right - left + 1);
        return bits_counter;
    }
    if (is_set)
        bits_counter += *pcurr - left + 1;

    unsigned prev_gap = *pcurr++;
    is_set ^= 1;
    while (right > *pcurr)
    {
        if (is_set)
            bits_counter += *pcurr - prev_gap;
        if (pcurr == pend) 
            return bits_counter;
        prev_gap = *pcurr++;
        is_set ^= 1;
    }
    if (is_set)
        bits_counter += right - prev_gap;

    return bits_counter;
}



/*! 
   \brief Lexicographical comparison of GAP buffers.
   \param buf1 - First GAP buffer pointer.
   \param buf2 - Second GAP buffer pointer.
   \return  <0 - less, =0 - equal,  >0 - greater.

   @ingroup gapfunc
*/
template<typename T> int gapcmp(const T* buf1, const T* buf2)
{
    const T* pcurr1 = buf1;
    const T* pend1 = pcurr1 + (*pcurr1 >> 3);
    unsigned bitval1 = *buf1 & 1;
    ++pcurr1;

    const T* pcurr2 = buf2;
    unsigned bitval2 = *buf2 & 1;
    ++pcurr2;

    while (pcurr1 <= pend1)
    {
        if (*pcurr1 == *pcurr2)
        {
            if (bitval1 != bitval2)
            {
                return (bitval1) ? 1 : -1;
            }
        }
        else
        {
            if (bitval1 == bitval2)
            {
                if (bitval1)
                {
                    return (*pcurr1 < *pcurr2) ? -1 : 1;
                }
                else
                {
                    return (*pcurr1 < *pcurr2) ? 1 : -1;
                }
            }
            else
            {
                return (bitval1) ? 1 : -1;
            }
        }

        ++pcurr1; ++pcurr2;

        bitval1 ^= 1;
        bitval2 ^= 1;
    }

    return 0;
}


/*!
   \brief Abstract operation for GAP buffers. 
          Receives functor F as a template argument
   \param dest - destination memory buffer.
   \param vect1 - operand 1 GAP encoded buffer.
   \param vect1_mask - XOR mask for starting bitflag for vector1 
   can be 0 or 1 (1 inverts the vector)
   \param vect2 - operand 2 GAP encoded buffer.
   \param vect2_mask - same as vect1_mask
   \param f - operation functor.
   \note Internal function.

   @ingroup gapfunc
*/
template<typename T, class F> 
void gap_buff_op(T*         BMRESTRICT dest, 
                 const T*   BMRESTRICT vect1,
                 unsigned   vect1_mask, 
                 const T*   BMRESTRICT vect2,
                 unsigned   vect2_mask, 
                 F f)
{
    register const T*  cur1 = vect1;
    register const T*  cur2 = vect2;

    unsigned bitval1 = (*cur1++ & 1) ^ vect1_mask;
    unsigned bitval2 = (*cur2++ & 1) ^ vect2_mask;
    
    unsigned bitval = f(bitval1, bitval2);
    unsigned bitval_prev = bitval;

    register T* res = dest; 
    *res = bitval;
    ++res;

    while (1)
    {
        bitval = f(bitval1, bitval2);

        // Check if GAP value changes and we need to 
        // start the next one.
        if (bitval != bitval_prev)
        {
            ++res;
            bitval_prev = bitval;
        }

        if (*cur1 < *cur2)
        {
            *res = *cur1;
            ++cur1;
            bitval1 ^= 1;
        }
        else // >=
        {
            *res = *cur2;
            if (*cur2 < *cur1)
            {
                bitval2 ^= 1;                
            }
            else  // equal
            {
                if (*cur2 == (FerrisBitMagic::gap_max_bits - 1))
                {
                    break;
                }

                ++cur1;
                bitval1 ^= 1;
                bitval2 ^= 1;
            }
            ++cur2;
        }

    } // while

    unsigned dlen = (unsigned)(res - dest);
    *dest = (*dest & 7) + (dlen << 3);

}


/*!
   \brief Abstract distance(similarity) operation for GAP buffers. 
          Receives functor F as a template argument
   \param vect1 - operand 1 GAP encoded buffer.
   \param vect2 - operand 2 GAP encoded buffer.
   \param f - operation functor.
   \note Internal function.

   @ingroup gapfunc
*/
/*
template<typename T, class F> 
unsigned gap_buff_count_op(const T*  vect1, const T*  vect2, F f)
{
    register const T* cur1 = vect1;
    register const T* cur2 = vect2;

    unsigned bitval1 = (*cur1++ & 1);
    unsigned bitval2 = (*cur2++ & 1);
    unsigned bitval = f(bitval1, bitval2);
    unsigned bitval_prev = bitval;

    unsigned count = 0;
    T res;
    T res_prev;

    while (1)
    {
        bitval = f(bitval1, bitval2);

        // Check if GAP value changes and we need to 
        // start the next one.
        if (bitval != bitval_prev)
        {
            bitval_prev = bitval;
        }

        if (*cur1 < *cur2)
        {
            if (bitval)
                count += *cur1; 
            ++cur1;
            bitval1 ^= 1;
        }
        else // >=
        {
            if (bitval)
                count += *cur2; 
            if (*cur2 < *cur1)
            {
                bitval2 ^= 1;                
            }
            else  // equal
            {
                if (*cur2 == (FerrisBitMagic::gap_max_bits - 1))
                {
                    break;
                }

                ++cur1;
                bitval1 ^= 1;
                bitval2 ^= 1;
            }
            ++cur2;
        }

    } // while

    return count;
}
*/


/*!
   \brief Sets or clears bit in the GAP buffer.

   \param val - new bit value
   \param buf - GAP buffer.
   \param pos - Index of bit to set.
   \param is_set - (OUT) flag if bit was actually set.

   \return New GAP buffer length. 

   @ingroup gapfunc
*/
template<typename T> unsigned gap_set_value(unsigned val, 
                                            T* BMRESTRICT buf, 
                                            unsigned pos, 
                                            unsigned* BMRESTRICT is_set)
{
    BM_ASSERT(pos < FerrisBitMagic::gap_max_bits);
    unsigned curr = gap_bfind(buf, pos, is_set);

    register T end = (*buf >> 3);
	if (*is_set == val)
	{
		*is_set = 0;
		return end;
	}
    *is_set = 1;

    register T* pcurr = buf + curr;
    register T* pprev = pcurr - 1;
    register T* pend = buf + end;

    // Special case, first bit GAP operation. There is no platform beside it.
    // initial flag must be inverted.
    if (pos == 0)
    {
        *buf ^= 1;
        if ( buf[1] ) // We need to insert a 1 bit platform here.
        {
            ::memmove(&buf[2], &buf[1], (end - 1) * sizeof(gap_word_t));
            buf[1] = 0;
            ++end;
        }
        else // Only 1 bit in the GAP. We need to delete the first GAP.
        {
            pprev = buf + 1;
            pcurr = pprev + 1;
            do
            {
                *pprev++ = *pcurr++;
            } while (pcurr < pend);
            --end;
        }
    }
    else if (curr > 1 && ((unsigned)(*pprev))+1 == pos) // Left border bit
	{
 	   ++(*pprev);
	   if (*pprev == *pcurr)  // Curr. GAP to be merged with prev.GAP.
	   {
            --end;
            if (pcurr != pend) // GAP merge: 2 GAPS to be deleted 
            {
                --end;
                ++pcurr;
                do
                {
                    *pprev++ = *pcurr++;
                } while (pcurr < pend);
            }
	   }    
    }
	else if (*pcurr == pos) // Rightmost bit in the GAP. Border goes left.
	{
		--(*pcurr);       
		if (pcurr == pend)
        {
		   ++end;
        }
	}
	else  // Worst case we need to split current block.
	{
        ::memmove(pcurr+2, pcurr,(end - curr + 1)*sizeof(T));
        *pcurr++ = pos - 1;
        *pcurr = pos;
		end+=2;
	}

    // Set correct length word.
    *buf = (*buf & 7) + (end << 3);

    buf[end] = FerrisBitMagic::gap_max_bits - 1;
    return end;
}

//------------------------------------------------------------------------

/**
    \brief Searches for the next 1 bit in the GAP block
    \param buf - GAP buffer
    \param nbit - bit index to start checking from.
    \param prev - returns previously checked value

    @ingroup gapfunc
*/
template<typename T> int gap_find_in_block(const T* buf, 
                                           unsigned nbit, 
                                           FerrisBitMagic::id_t* prev)
{
    BM_ASSERT(nbit < FerrisBitMagic::gap_max_bits);

    unsigned bitval;
    unsigned gap_idx = FerrisBitMagic::gap_bfind(buf, nbit, &bitval);

    if (bitval) // positive block.
    {
       return 1;
    }

    register unsigned val = buf[gap_idx] + 1;
    *prev += val - nbit;
 
    return (val != FerrisBitMagic::gap_max_bits);  // no bug here.
}



/*! 
   \brief Sets bits to 1 in the bitblock.
   \param dest - Bitset buffer.
   \param bitpos - Offset of the start bit.
   \param bitcount - number of bits to set.

   @ingroup bitfunc
*/  
inline void or_bit_block(unsigned* dest, 
                         unsigned bitpos, 
                         unsigned bitcount)
{
    unsigned nbit  = unsigned(bitpos & FerrisBitMagic::set_block_mask); 
    unsigned nword = unsigned(nbit >> FerrisBitMagic::set_word_shift); 
    nbit &= FerrisBitMagic::set_word_mask;

    FerrisBitMagic::word_t* word = dest + nword;

    if (bitcount == 1)  // special case (only 1 bit to set)
    {
        *word |= unsigned(1 << nbit);
        return;
    }

    if (nbit) // starting position is not aligned
    {
        unsigned right_margin = nbit + bitcount;

        // here we checking if we setting bits only in the current
        // word. Example: 00111000000000000000000000000000 (32 bits word)

        if (right_margin < 32) 
        {
            unsigned mask = 
                block_set_table<true>::_right[nbit] & 
                block_set_table<true>::_left[right_margin-1];
            *word |= mask;
            return; // we are done
        }
        else
        {
            *word |= block_set_table<true>::_right[nbit];
            bitcount -= 32 - nbit;
        }
        ++word;
    }

    // now we are word aligned, lets find out how many words we 
    // can now turn ON using loop

    for ( ;bitcount >= 32; bitcount -= 32) 
    {
        *word++ = 0xffffffff;
    }

    if (bitcount) 
    {
        *word |= block_set_table<true>::_left[bitcount-1];
    }
}


/*! 
   \brief SUB (AND NOT) bit interval to 1 in the bitblock.
   \param dest - Bitset buffer.
   \param bitpos - Offset of the start bit.
   \param bitcount - number of bits to set.

   @ingroup bitfunc
*/  
inline void sub_bit_block(unsigned* dest, 
                          unsigned bitpos, 
                          unsigned bitcount)
{
    unsigned nbit  = unsigned(bitpos & FerrisBitMagic::set_block_mask); 
    unsigned nword = unsigned(nbit >> FerrisBitMagic::set_word_shift); 
    nbit &= FerrisBitMagic::set_word_mask;

    FerrisBitMagic::word_t* word = dest + nword;

    if (bitcount == 1)  // special case (only 1 bit to set)
    {
        *word &= ~unsigned(1 << nbit);
        return;
    }

    if (nbit) // starting position is not aligned
    {
        unsigned right_margin = nbit + bitcount;

        // here we checking if we setting bits only in the current
        // word. Example: 00111000000000000000000000000000 (32 bits word)

        if (right_margin < 32) 
        {
            unsigned mask = 
                block_set_table<true>::_right[nbit] & 
                block_set_table<true>::_left[right_margin-1];
            *word &= ~mask;
            return; // we are done
        }
        else
        {
            *word &= ~block_set_table<true>::_right[nbit];
            bitcount -= 32 - nbit;
        }
        ++word;
    }

    // now we are word aligned, lets find out how many words we 
    // can now turn ON using loop

    for ( ;bitcount >= 32; bitcount -= 32) 
    {
        *word++ = 0;
    }

    if (bitcount) 
    {
        *word &= ~block_set_table<true>::_left[bitcount-1];
    }
}


/*! 
   \brief XOR bit interval to 1 in the bitblock.
   \param dest - Bitset buffer.
   \param bitpos - Offset of the start bit.
   \param bitcount - number of bits to set.

   @ingroup bitfunc
*/  
inline void xor_bit_block(unsigned* dest, 
                          unsigned bitpos, 
                          unsigned bitcount)
{
    unsigned nbit  = unsigned(bitpos & FerrisBitMagic::set_block_mask); 
    unsigned nword = unsigned(nbit >> FerrisBitMagic::set_word_shift); 
    nbit &= FerrisBitMagic::set_word_mask;

    FerrisBitMagic::word_t* word = dest + nword;

    if (bitcount == 1)  // special case (only 1 bit to set)
    {
        *word ^= unsigned(1 << nbit);
        return;
    }

    if (nbit) // starting position is not aligned
    {
        unsigned right_margin = nbit + bitcount;

        // here we checking if we setting bits only in the current
        // word. Example: 00111000000000000000000000000000 (32 bits word)

        if (right_margin < 32) 
        {
            unsigned mask = 
                block_set_table<true>::_right[nbit] & 
                block_set_table<true>::_left[right_margin-1];
            *word ^= mask;
            return; // we are done
        }
        else
        {
            *word ^= block_set_table<true>::_right[nbit];
            bitcount -= 32 - nbit;
        }
        ++word;
    }

    // now we are word aligned, lets find out how many words we 
    // can now turn ON using loop

    for ( ;bitcount >= 32; bitcount -= 32) 
    {
        *word++ ^= 0xffffffff;
    }

    if (bitcount) 
    {
        *word ^= block_set_table<true>::_left[bitcount-1];
    }
}


/*!
   \brief SUB (AND NOT) GAP block to bitblock.
   \param dest - bitblock buffer pointer.
   \param buf  - GAP buffer pointer.

   @ingroup gapfunc
*/
template<typename T> 
void gap_sub_to_bitset(unsigned* dest, const T*  buf)
{
    register const T* pcurr = buf;    
    register const T* pend = pcurr + (*pcurr >> 3);
    ++pcurr;

    if (*buf & 1)  // Starts with 1
    {
        sub_bit_block(dest, 0, *pcurr + 1);
        ++pcurr;
    }
    ++pcurr; // now we are in GAP "1" again

    while (pcurr <= pend)
    {
        unsigned bitpos = *(pcurr-1) + 1;
        BM_ASSERT(*pcurr > *(pcurr-1));
        unsigned gap_len = *pcurr - *(pcurr-1);
        sub_bit_block(dest, bitpos, gap_len);
        pcurr += 2;
    }
}


/*!
   \brief XOR GAP block to bitblock.
   \param dest - bitblock buffer pointer.
   \param buf  - GAP buffer pointer.

   @ingroup gapfunc
*/
template<typename T> 
void gap_xor_to_bitset(unsigned* dest, const T*  buf)
{
    register const T* pcurr = buf;    
    register const T* pend = pcurr + (*pcurr >> 3);
    ++pcurr;

    if (*buf & 1)  // Starts with 1
    {
        xor_bit_block(dest, 0, *pcurr + 1);
        ++pcurr;
    }
    ++pcurr; // now we are in GAP "1" again

    while (pcurr <= pend)
    {
        unsigned bitpos = *(pcurr-1) + 1;
        BM_ASSERT(*pcurr > *(pcurr-1));
        unsigned gap_len = *pcurr - *(pcurr-1);
        xor_bit_block(dest, bitpos, gap_len);
        pcurr += 2;
    }
}


/*!
   \brief Adds(OR) GAP block to bitblock.
   \param dest - bitblock buffer pointer.
   \param buf  - GAP buffer pointer.

   @ingroup gapfunc
*/
template<typename T> 
void gap_add_to_bitset(unsigned* dest, const T*  buf)
{
    register const T* pcurr = buf;    
    register const T* pend = pcurr + (*pcurr >> 3);
    ++pcurr;

    if (*buf & 1)  // Starts with 1
    {
        or_bit_block(dest, 0, *pcurr + 1);
        ++pcurr;
    }
    ++pcurr; // now we are in GAP "1" again

    while (pcurr <= pend)
    {
        unsigned bitpos = *(pcurr-1) + 1;
        BM_ASSERT(*pcurr > *(pcurr-1));
        unsigned gap_len = *pcurr - *(pcurr-1);
        or_bit_block(dest, bitpos, gap_len);
        pcurr += 2;
    }
}


/*!
   \brief ANDs GAP block to bitblock.
   \param dest - bitblock buffer pointer.
   \param buf  - GAP buffer pointer.

   @ingroup gapfunc
*/
template<typename T> 
void gap_and_to_bitset(unsigned* dest, const T*  buf)
{
    register const T* pcurr = buf;    
    register const T* pend = pcurr + (*pcurr >> 3);
    ++pcurr;

    if (! (*buf & 1) )  // Starts with 0 
    {
        // Instead of AND we can SUB 0 gaps here 
        sub_bit_block(dest, 0, *pcurr + 1);
        ++pcurr;
    }
    ++pcurr; // now we are in GAP "0" again

    while (pcurr <= pend)
    {
        unsigned bitpos = *(pcurr-1) + 1;
        BM_ASSERT(*pcurr > *(pcurr-1));
        unsigned gap_len = *pcurr - *(pcurr-1);
        sub_bit_block(dest, bitpos, gap_len);
        pcurr += 2;
    }
}


/*!
   \brief Compute bitcount of bit block AND masked by GAP block.
   \param dest - bitblock buffer pointer.
   \param buf  - GAP buffer pointer.

   @ingroup gapfunc bitfunc
*/
template<typename T> 
FerrisBitMagic::id_t gap_bitset_and_count(const unsigned* block, const T*  buf)
{
    BM_ASSERT(block);

    register const T* pcurr = buf;    
    register const T* pend = pcurr + (*pcurr >> 3);
    ++pcurr;

    FerrisBitMagic::id_t count = 0;

    if (*buf & 1)  // Starts with 1
    {
        count += bit_block_calc_count_range(block, 0, *pcurr);
        ++pcurr;
    }
    ++pcurr; // now we are in GAP "1" again

    while (pcurr <= pend)
    {
        FerrisBitMagic::id_t c = bit_block_calc_count_range(block, *(pcurr-1)+1, *pcurr);

        count += c;
        pcurr += 2;
    }
    return count;
}


/*!
   \brief Compute bitcount of bit block SUB masked by GAP block.
   \param dest - bitblock buffer pointer.
   \param buf  - GAP buffer pointer.

   @ingroup gapfunc bitfunc
*/
template<typename T> 
FerrisBitMagic::id_t gap_bitset_sub_count(const unsigned* block, const T*  buf)
{
    BM_ASSERT(block);

    register const T* pcurr = buf;    
    register const T* pend = pcurr + (*pcurr >> 3);
    ++pcurr;

    FerrisBitMagic::id_t count = 0;

    if (!(*buf & 1))  // Starts with 0
    {
        count += bit_block_calc_count_range(block, 0, *pcurr);
        ++pcurr;
    }
    ++pcurr; // now we are in GAP "0" again

    for (;pcurr <= pend; pcurr+=2)
    {
        count += bit_block_calc_count_range(block, *(pcurr-1)+1, *pcurr);
    }
    return count;
}



/*!
   \brief Compute bitcount of bit block XOR masked by GAP block.
   \param dest - bitblock buffer pointer.
   \param buf  - GAP buffer pointer.

   @ingroup gapfunc bitfunc
*/
template<typename T> 
FerrisBitMagic::id_t gap_bitset_xor_count(const unsigned* block, const T*  buf)
{
    BM_ASSERT(block);

    register const T* pcurr = buf;    
    register const T* pend = pcurr + (*pcurr >> 3);
    ++pcurr;

    unsigned bitval = *buf & 1;
    
    register FerrisBitMagic::id_t count = bit_block_calc_count_range(block, 0, *pcurr);
    if (bitval)
    {
        count = *pcurr + 1 - count;
    }
    
    for (bitval^=1, ++pcurr; pcurr <= pend; bitval^=1, ++pcurr)
    {
        T prev = *(pcurr-1)+1;
        FerrisBitMagic::id_t c = bit_block_calc_count_range(block, prev, *pcurr);
        
        if (bitval) // 1 gap; means Result = Total_Bits - BitCount;
        {
            c = (*pcurr - prev + 1) - c;
        }
        
        count += c;
    }
    return count;
}


/*!
   \brief Compute bitcount of bit block OR masked by GAP block.
   \param dest - bitblock buffer pointer.
   \param buf  - GAP buffer pointer.

   @ingroup gapfunc bitfunc
*/
template<typename T> 
FerrisBitMagic::id_t gap_bitset_or_count(const unsigned* block, const T*  buf)
{
    BM_ASSERT(block);

    register const T* pcurr = buf;    
    register const T* pend = pcurr + (*pcurr >> 3);
    ++pcurr;

    unsigned bitval = *buf & 1;
    
    register FerrisBitMagic::id_t count;
    if (bitval)
    {
        count = *pcurr + 1;
    } 
    else
    {
        count = bit_block_calc_count_range(block, 0, *pcurr);
    }
    
    for (bitval^=1, ++pcurr; pcurr <= pend; bitval^=1, ++pcurr)
    {
        T prev = *(pcurr-1)+1;
        FerrisBitMagic::id_t c;
        
        if (bitval)
        {
            c = (*pcurr - prev + 1);
        }
        else
        {
            c = bit_block_calc_count_range(block, prev, *pcurr);
        }
        
        count += c;
    }
    return count;
}

/*!
   \brief Bitblock memset operation. 

   \param dst - destination block.
   \param value - value to set.

   @ingroup bitfunc
*/
inline 
void bit_block_set(FerrisBitMagic::word_t* BMRESTRICT dst, FerrisBitMagic::word_t value)
{
//#ifdef BMVECTOPT
//    VECT_SET_BLOCK(dst, dst + FerrisBitMagic::set_block_size, value);
//#else
    ::memset(dst, value, FerrisBitMagic::set_block_size * sizeof(FerrisBitMagic::word_t));
//#endif
}


/*!
   \brief GAP block to bitblock conversion.
   \param dest - bitblock buffer pointer.
   \param buf  - GAP buffer pointer.

   @ingroup gapfunc
*/
template<typename T> 
void gap_convert_to_bitset(unsigned* dest, const T*  buf)
{
    bit_block_set(dest, 0);
    gap_add_to_bitset(dest, buf);
}


/*!
   \brief GAP block to bitblock conversion.
   \param dest - bitblock buffer pointer.
   \param buf  - GAP buffer pointer.
   \param dest_size - length of the destination buffer.

   @ingroup gapfunc
*/
template<typename T> 
void gap_convert_to_bitset(unsigned* dest, const T*  buf,  unsigned  dest_len)
{
   ::memset(dest, 0, dest_len * sizeof(unsigned));
    gap_add_to_bitset(dest, buf);
}



/*!
   \brief Smart GAP block to bitblock conversion.

    Checks if GAP block is ALL-ZERO or ALL-ON. In those cases returns 
    pointer on special static bitblocks.

   \param dest - bitblock buffer pointer.
   \param buf  - GAP buffer pointer.
   \param set_max - max possible bitset length

   @ingroup gapfunc
*/
template<typename T> 
        unsigned* gap_convert_to_bitset_smart(unsigned* dest,
                                              const T* buf, 
                                              id_t set_max)
{
    if (buf[1] == set_max - 1)
    {
        return (buf[0] & 1) ? FULL_BLOCK_ADDR : 0;
    }

    gap_convert_to_bitset(dest, buf);
    return dest;
}


/*!
   \brief Calculates sum of all words in GAP block. (For debugging purposes)
   \note For debugging and testing ONLY.
   \param buf - GAP buffer pointer.
   \return Sum of all words.

   @ingroup gapfunc
*/
template<typename T> unsigned gap_control_sum(const T* buf)
{
    unsigned end = *buf >> 3;

    register const T* pcurr = buf;    
    register const T* pend = pcurr + (*pcurr >> 3);
    ++pcurr;

    if (*buf & 1)  // Starts with 1
    {
        ++pcurr;
    }
    ++pcurr; // now we are in GAP "1" again

    while (pcurr <= pend)
    {
        BM_ASSERT(*pcurr > *(pcurr-1));
        pcurr += 2;
    }
    return buf[end];

}


/*! 
   \brief Sets all bits to 0 or 1 (GAP)
   \param buf - GAP buffer pointer.
   \param set_max - max possible bitset length

   @ingroup gapfunc
*/
template<class T> void gap_set_all(T* buf, 
                                        unsigned set_max,
                                        unsigned value)
{
    BM_ASSERT(value == 0 || value == 1);
    *buf = (*buf & 6u) + (1u << 3) + value;
    *(++buf) = set_max - 1;
}


/*!
    \brief Init gap block so it has block in it (can be whole block)
    \param buf  - GAP buffer pointer.
    \param from - one block start
    \param to   - one block end
    \param value - (block value)1 or 0
    \param set_max - max possible bitset length
    
   @ingroup gapfunc
*/
template<class T> 
void gap_init_range_block(T*       buf,
                          unsigned from,
                          unsigned to,
                          unsigned value,
                          unsigned set_max)
{
    BM_ASSERT(value == 0 || value == 1);

    unsigned gap_len;
    if (from == 0)
    {
        if (to == set_max - 1)
        {
            gap_set_all(buf, set_max, value);
        }
        else
        {
            gap_len = 2;
            buf[1] = to;
            buf[2] = set_max - 1;
            buf[0] =  (*buf & 6u) + (gap_len << 3) + value;
        }
        return;
    }
    // from != 0

    value = !value;
    if (to == set_max - 1)
    {
        gap_len = 2;
        buf[1] = from - 1;
        buf[2] = set_max - 1;
    }
    else
    {
        gap_len = 3;
        buf[1] = from - 1;
        buf[2] = to;
        buf[3] = set_max - 1;
    }
    buf[0] =  (*buf & 6u) + (gap_len << 3) + value;
}


/*! 
   \brief Inverts all bits in the GAP buffer.
   \param buf - GAP buffer pointer.

   @ingroup gapfunc
*/
template<typename T> void gap_invert(T* buf)
{ 
    *buf ^= 1;
}

/*! 
   \brief Temporary inverts all bits in the GAP buffer.
   
   In this function const-ness of the buffer means nothing.
   Calling this function again restores the status of the buffer.

   \param buf - GAP buffer pointer. (Buffer IS changed) 

   @ingroup gapfunc
*/
/*
template<typename T> void gap_temp_invert(const T* buf)
{
    T* buftmp = const_cast<T*>(buf);
    *buftmp ^= 1;
}
*/

/*!
   \brief Checks if GAP block is all-zero.
   \param buf - GAP buffer pointer.
   \param set_max - max possible bitset length
   \returns true if all-zero.

   @ingroup gapfunc
*/
template<typename T> 
             bool gap_is_all_zero(const T* buf, unsigned set_max)
{
    return (((*buf & 1)==0)  && (*(++buf) == set_max - 1));
}

/*!
   \brief Checks if GAP block is all-one.
   \param buf - GAP buffer pointer.
   \param set_max - max possible bitset length
   \returns true if all-one.

   @ingroup gapfunc
*/
template<typename T> 
         bool gap_is_all_one(const T* buf, unsigned set_max)
{
    return ((*buf & 1)  && (*(++buf) == set_max - 1));
}

/*!
   \brief Returs GAP block length.
   \param buf - GAP buffer pointer.
   \returns GAP block length.

   @ingroup gapfunc
*/
template<typename T> unsigned gap_length(const T* buf)
{
    return (*buf >> 3) + 1;
}


/*!
   \brief Returs GAP block capacity.
   \param buf - GAP buffer pointer.
   \returns GAP block capacity.

   @ingroup gapfunc
*/
template<typename T> 
unsigned gap_capacity(const T* buf, const T* glevel_len)
{
    return glevel_len[(*buf >> 1) & 3];
}


/*!
   \brief Returs GAP block capacity limit.
   \param buf - GAP buffer pointer.
   \param glevel_len - GAP lengths table (gap_len_table)
   \returns GAP block limit.

   @ingroup gapfunc
*/
template<typename T> 
unsigned gap_limit(const T* buf, const T* glevel_len)
{
    return glevel_len[(*buf >> 1) & 3]-4;
}


/*!
   \brief Returs GAP blocks capacity level.
   \param buf - GAP buffer pointer.
   \returns GAP block capacity level.

   @ingroup gapfunc
*/
template<typename T> unsigned gap_level(const T* buf)
{
    return (*buf >> 1) & 3;
}


/*!
   \brief Sets GAP block capacity level.
   \param buf - GAP buffer pointer.
   \param level new GAP block capacity level.

   @ingroup gapfunc
*/
template<typename T> void set_gap_level(T* buf, 
                                        unsigned level)
{
    BM_ASSERT(level < FerrisBitMagic::gap_levels);
    *buf = ((level & 3) << 1) | (*buf & 1) | (*buf & ~7); 
}


/*!
   \brief Calculates GAP block capacity level.
   \param len - GAP buffer length.
   \param glevel_len - GAP lengths table
   \return GAP block capacity level. 
            -1 if block does not fit any level.
   @ingroup gapfunc
*/
template<typename T>
inline int gap_calc_level(int len, const T* glevel_len)
{
    if (len <= (glevel_len[0]-4)) return 0;
    if (len <= (glevel_len[1]-4)) return 1;
    if (len <= (glevel_len[2]-4)) return 2;
    if (len <= (glevel_len[3]-4)) return 3;

    BM_ASSERT(FerrisBitMagic::gap_levels == 4);
    return -1;
}

/*! @brief Returns number of free elements in GAP block array. 
    Difference between GAP block capacity on this level and actual GAP length.
    
    @param buf - GAP buffer pointer
    @parma glevel_len - GAP lengths table
    
    @return Number of free GAP elements
    @ingroup gapfunc
*/
template<typename T>
inline unsigned gap_free_elements(const T* buf, const T* glevel_len)
{
    unsigned len = gap_length(buf);
    unsigned capacity = gap_capacity(buf, glevel_len);
    return capacity - len;
}


/*! 
   \brief Lexicographical comparison of BIT buffers.
   \param buf1 - First buffer pointer.
   \param buf2 - Second buffer pointer.
   \param len - Buffer length in elements (T).
   \return  <0 - less, =0 - equal,  >0 - greater.

   @ingroup bitfunc 
*/
template<typename T> 
int bitcmp(const T* buf1, const T* buf2, unsigned len)
{
    BM_ASSERT(len);

    const T* pend1 = buf1 + len; 
    do
    {
        T w1 = *buf1++;
        T w2 = *buf2++;
        T diff = w1 ^ w2;
    
        if (diff)
        { 
            return (w1 & diff & -diff) ? 1 : -1;
        }

    } while (buf1 < pend1);

    return 0;
}


/*! 
   \brief Converts bit block to GAP. 
   \param dest - Destinatio GAP buffer.
   \param src - Source bitblock buffer.
   \param bits - Number of bits to convert.
   \param dest_len - length of the dest. buffer.
   \return  New ength of GAP block or 0 if conversion failed 
   (insufficicent space).

   @ingroup gapfunc
*/
template<typename T> 
    unsigned bit_convert_to_gap(T* BMRESTRICT dest, 
                                const unsigned* BMRESTRICT src, 
                                FerrisBitMagic::id_t bits, 
                                unsigned dest_len)
{
    register T* BMRESTRICT pcurr = dest;
    T* BMRESTRICT end = dest + dest_len; 
    register int bitval = (*src) & 1;
    *pcurr |= bitval;

    ++pcurr;
    *pcurr = 0;
    register unsigned bit_idx = 0;
    register int bitval_next;

    unsigned val = *src;

    do
    {
        // We can fast pace if *src == 0 or *src = 0xffffffff

        while (val == 0 || val == 0xffffffff)
        {
           bitval_next = val ? 1 : 0;
           if (bitval != bitval_next)
           {
               *pcurr++ = bit_idx-1; 
               BM_ASSERT((pcurr-1) == (dest+1) || *(pcurr-1) > *(pcurr-2));
               if (pcurr >= end)
               {
                   return 0; // OUT of memory
               }
               bitval = bitval_next;
           }
           bit_idx += sizeof(*src) * 8;
           if (bit_idx >= bits)
           {
               goto complete;
           }
           ++src;
           val = *src;
        }


        register unsigned mask = 1;
        while (mask)
        {
            // Now plain bitshifting. Optimization wanted.

            bitval_next = val & mask ? 1 : 0;
            if (bitval != bitval_next)
            {
                *pcurr++ = bit_idx-1;
                BM_ASSERT((pcurr-1) == (dest+1) || *(pcurr-1) > *(pcurr-2));
                bitval = bitval_next;
                if (pcurr >= end)
                {
                    return 0; // OUT of memory
                }
            }

            mask <<= 1;
            ++bit_idx;

        } // while mask

        if (bit_idx >= bits)
        {
            goto complete;
        }

        ++src;
        val = *src;

    } while(1);

complete:
    *pcurr = bit_idx-1;
    unsigned len = (unsigned)(pcurr - dest);
    *dest = (*dest & 7) + (len << 3);
    return len;
}

/*!
    @ingroup bitfunc 
*/
template<typename T> T bit_convert_to_arr(T* BMRESTRICT dest, 
                                          const unsigned* BMRESTRICT src, 
                                          FerrisBitMagic::id_t bits, 
                                          unsigned dest_len)
{
    register T* BMRESTRICT pcurr = dest;
    T* BMRESTRICT end = dest + dest_len; 
    register unsigned bit_idx = 0;

    do
    {
        register unsigned val = *src;
        // We can skip if *src == 0 

        while (val == 0)
        {
            bit_idx += sizeof(*src) * 8;
            if (bit_idx >= bits)
            {
               return (T)(pcurr - dest);
            }
            val = *(++src);
        }

        if (pcurr + sizeof(val)*8 > end) // insufficient space
        {
            return 0;
        }

        for (int i = 0; i < 32; i+=4)
        {
            if (val & 1)
                *pcurr++ = bit_idx;
            val >>= 1; ++bit_idx;
            if (val & 1)
                *pcurr++ = bit_idx;
            val >>= 1; ++bit_idx;
            if (val & 1)
                *pcurr++ = bit_idx;
            val >>= 1; ++bit_idx;
            if (val & 1)
                *pcurr++ = bit_idx;
            val >>= 1; ++bit_idx;
        }
        if (bits <= bit_idx)
            break;

        val = *(++src);

    } while (1);

    return (T)(pcurr - dest);
}



/*! 
    @brief Bitcount for bit string
    
	Function calculates number of 1 bits in the given array of words.
    Make sure the addresses are aligned.

    @ingroup bitfunc 
*/
inline 
FerrisBitMagic::id_t bit_block_calc_count(const FerrisBitMagic::word_t* block, 
							  const FerrisBitMagic::word_t* block_end)
{
    BM_ASSERT(block < block_end);
	FerrisBitMagic::id_t count = 0;

#ifdef BM64OPT

    // 64-bit optimized algorithm.

    const FerrisBitMagic::id64_t* b1 = (FerrisBitMagic::id64_t*) block;
    const FerrisBitMagic::id64_t* b2 = (FerrisBitMagic::id64_t*) block_end;

    FerrisBitMagic::id64_t  acc = *b1++;  // accumulator (sparse vectors optimization)

    do
    {
        FerrisBitMagic::id64_t in = *b1++;
        FerrisBitMagic::id64_t acc_prev = acc;
        acc |= in;

        if (acc_prev &= in)  // counting bits in accumulator
        {
            acc = (acc & 0x5555555555555555ULL) + (acc >> 1 & 0x5555555555555555ULL);
            acc = (acc & 0x3333333333333333ULL) + (acc >> 2 & 0x3333333333333333ULL);
            acc = acc + (acc >> 4) & 0x0F0F0F0F0F0F0F0FULL;
            acc = acc + (acc >> 8);
            acc = acc + (acc >> 16);
            acc = acc + (acc >> 32) & 0x0000007F;
            count += (unsigned)acc;

            acc = acc_prev;
        }
    } while (b1 < b2);
    count += word_bitcount64(acc);  // count-in remaining accumulator 

#else
    // For 32 bit code the fastest method is
    // to use bitcount table for each byte in the block.
    // As optimization for sparse bitsets used bits accumulator
    // to collect ON bits using bitwise OR. 
    FerrisBitMagic::word_t  acc = *block++;
    do
    {
        FerrisBitMagic::word_t in = *block++;
        FerrisBitMagic::word_t acc_prev = acc;
        acc |= in;
        if (acc_prev &= in)  // accumulator miss: counting bits
        {
            BM_INCWORD_BITCOUNT(count, acc);
            acc = acc_prev;
        }
    } while (block < block_end);

    BM_INCWORD_BITCOUNT(count, acc); // count-in remaining accumulator 

#endif
	
    return count;
}

/*!
    Function calculates number of times when bit value changed 
    (1-0 or 0-1).
    
    For 001 result is 2
        010 - 3
        011 - 2
        111 - 1
    
    @ingroup bitfunc 
*/

inline 
FerrisBitMagic::id_t bit_count_change(FerrisBitMagic::word_t w)
{
    unsigned count = 1;
    w ^= (w >> 1);

    BM_INCWORD_BITCOUNT(count, w);
    count -= (w >> ((sizeof(w) * 8) - 1));
    return count;
}


/*!
    Function calculates number of times when bit value changed 
    (1-0 or 0-1) in the bit block.
        
    @ingroup bitfunc 
*/
inline 
FerrisBitMagic::id_t bit_block_calc_count_change(const FerrisBitMagic::word_t* block, 
							         const FerrisBitMagic::word_t* block_end)
{
    BM_ASSERT(block < block_end);
    FerrisBitMagic::id_t count = 1;
    
#ifdef BM64OPT

    // 64-bit optimized algorithm.

    const FerrisBitMagic::id64_t* b1 = (FerrisBitMagic::id64_t*) block;
    const FerrisBitMagic::id64_t* b2 = (FerrisBitMagic::id64_t*) block_end;

    FerrisBitMagic::id64_t w, w0, w_prev, w_l;
    w = w0 = *b1;
    const int w_shift = sizeof(w) * 8 - 1;
    w ^= (w >> 1);
    count += word_bitcount64(w);
    count -= (w_prev = (w0 >> w_shift)); // negative value correction
    
    for (++b1 ;b1 < b2; ++b1)
    {
        w = w0 = *b1;
        ++count;
        
        if (!w)
        {
            count -= !w_prev;
            w_prev = 0;
        }
        else
        {
            w ^= (w >> 1);
            count += word_bitcount64(w);
            
            w_l = w0 & 1;
            count -= (w0 >> w_shift);  // negative value correction
            count -= !(w_prev ^ w_l);  // word border correction
            
            w_prev = (w0 >> w_shift);
        }
    } // for

#else
    
    FerrisBitMagic::word_t  w, w0, w_prev, w_l; 
    
    w = w0 = *block;
    const int w_shift = sizeof(w) * 8 - 1;    
    w ^= (w >> 1);
    BM_INCWORD_BITCOUNT(count, w);
    count -= (w_prev = (w0 >> w_shift)); // negative value correction

    for (++block ;block < block_end; ++block)
    {
        w = w0 = *block;
        ++count;

        if (!w)
        {       
            count -= !w_prev;
            w_prev = 0;
        }
        else
        {
            w ^= (w >> 1);
            BM_INCWORD_BITCOUNT(count, w);
            
            w_l = w0 & 1;
            count -= (w0 >> w_shift);  // negative value correction
            count -= !(w_prev ^ w_l);  // word border correction
            
            w_prev = (w0 >> w_shift);
        }
    } // for
#endif
    return count;
}


/*!
	Function calculates number of 1 bits in the given array of words in
    the range between left anf right bits (borders included)
    Make sure the addresses are aligned.

    @ingroup bitfunc
*/
inline 
FerrisBitMagic::id_t bit_block_calc_count_range(const FerrisBitMagic::word_t* block,
                                    FerrisBitMagic::word_t left,
                                    FerrisBitMagic::word_t right)
{
    BM_ASSERT(left <= right);
    
	FerrisBitMagic::id_t count = 0;

    unsigned nbit  = left; // unsigned(left & FerrisBitMagic::set_block_mask);
    unsigned nword = unsigned(nbit >> FerrisBitMagic::set_word_shift);
    nbit &= FerrisBitMagic::set_word_mask;

    const FerrisBitMagic::word_t* word = block + nword;

    if (left == right)  // special case (only 1 bit to check)
    {
        return (*word >> nbit) & 1;
    }
    unsigned acc;
    unsigned bitcount = right - left + 1;

    if (nbit) // starting position is not aligned
    {
        unsigned right_margin = nbit + (right - left);

        if (right_margin < 32)
        {
            unsigned mask =
                block_set_table<true>::_right[nbit] &
                block_set_table<true>::_left[right_margin];
            acc = *word & mask;
            
            BM_INCWORD_BITCOUNT(count, acc);
            return count;
        }
        else
        {
            acc = *word & block_set_table<true>::_right[nbit];
            BM_INCWORD_BITCOUNT(count, acc);
            bitcount -= 32 - nbit;
        }
        ++word;
    }

    // now when we are word aligned, we can count bits the usual way
    for ( ;bitcount >= 32; bitcount -= 32)
    {
        acc = *word++;
        BM_INCWORD_BITCOUNT(count, acc);
    }

    if (bitcount)  // we have a tail to count
    {
        acc = (*word) & block_set_table<true>::_left[bitcount-1];
        BM_INCWORD_BITCOUNT(count, acc);
    }

    return count;
}


// ----------------------------------------------------------------------

/*! Function inverts block of bits 
    @ingroup bitfunc 
*/
template<typename T> void bit_invert(T* start, T* end)
{
#ifdef BMVECTOPT
    VECT_INVERT_ARR(start, end);
#else
    do
    {
        start[0] = ~start[0];
        start[1] = ~start[1];
        start[2] = ~start[2];
        start[3] = ~start[3];
        start+=4;
    } while (start < end);
#endif
}

// ----------------------------------------------------------------------

/*! @brief Returns "true" if all bits in the block are 1
    @ingroup bitfunc 
*/
inline bool is_bits_one(const FerrisBitMagic::wordop_t* start, 
                        const FerrisBitMagic::wordop_t* end)
{
   do
   {
        FerrisBitMagic::wordop_t tmp = 
            start[0] & start[1] & start[2] & start[3];
        if (tmp != FerrisBitMagic::all_bits_mask) 
            return false;
        start += 4;
   } while (start < end);

   return true;
}

// ----------------------------------------------------------------------


/*! @brief Returns "true" if all bits in the block are 0
    @ingroup bitfunc 
*/
inline bool bit_is_all_zero(const FerrisBitMagic::wordop_t* start, 
                            const FerrisBitMagic::wordop_t* end)
{
   do
   {
        FerrisBitMagic::wordop_t tmp = 
            start[0] | start[1] | start[2] | start[3];
       if (tmp) 
           return false;
       start += 4;
   } while (start < end);

   return true;
}

// ----------------------------------------------------------------------

// GAP blocks manipulation functions:

/*! \brief GAP and functor */
inline unsigned and_op(unsigned v1, unsigned v2)
{
    return v1 & v2;
}


/*! \brief GAP xor functor */
inline unsigned xor_op(unsigned v1, unsigned v2)
{
    return v1 ^ v2;
}


/*!
   \brief GAP AND operation.
   
   Function performs AND logical oparation on gap vectors.
   If possible function put the result into vect1 and returns this
   pointer.  Otherwise result is put into tmp_buf, which should be 
   twice of the vector size.

   \param vect1   - operand 1
   \param vect2   - operand 2
   \param tmp_buf - pointer on temporary buffer
   \return Result pointer (tmp_buf OR vect1)

   @ingroup gapfunc
*/
inline gap_word_t* gap_operation_and(const gap_word_t* BMRESTRICT vect1,
                                     const gap_word_t* BMRESTRICT vect2,
                                     gap_word_t*       BMRESTRICT tmp_buf)
{
    gap_buff_op(tmp_buf, vect1, 0, vect2, 0, and_op);

    return tmp_buf;
}


/*!
   \brief GAP XOR operation.
   
   Function performs XOR logical oparation on gap vectors.
   If possible function put the result into vect1 and returns this
   pointer.  Otherwise result is put into tmp_buf, which should be 
   twice of the vector size.

   \param vect1   - operand 1
   \param vect2   - operand 2
   \param tmp_buf - pointer on temporary buffer
   \return Result pointer (tmp_buf)

   @ingroup gapfunc
*/
inline gap_word_t* gap_operation_xor(const gap_word_t*  BMRESTRICT vect1,
                                     const gap_word_t*  BMRESTRICT vect2,
                                     gap_word_t*        BMRESTRICT tmp_buf)
{
    gap_buff_op(tmp_buf, vect1, 0, vect2, 0, xor_op);

    return tmp_buf;
}



/*!
   \brief GAP OR operation.
   
   Function performs OR logical oparation on gap vectors.
   If possible function put the result into vect1 and returns this
   pointer.  Otherwise result is put into tmp_buf, which should be 
   twice of the vector size.

   \param vect1   - operand 1
   \param vect2   - operand 2
   \param tmp_buf - pointer on temporary buffer
   \return Result pointer (tmp_buf)

   @ingroup gapfunc
*/
inline gap_word_t* gap_operation_or(const gap_word_t*  BMRESTRICT vect1,
                                    const gap_word_t*  BMRESTRICT vect2,
                                    gap_word_t*        BMRESTRICT tmp_buf)
{
//    gap_invert(vect1);
//    gap_temp_invert(vect2);
    gap_buff_op(tmp_buf, vect1, 1, vect2, 1, and_op);
//    gap_word_t* res = gap_operation_and(vect1, vect2, tmp_buf);
//    gap_temp_invert(vect2);

    gap_invert(tmp_buf);
//    gap_invert(vect1);
    
    return tmp_buf;
}


/*!
   \brief GAP SUB (AND NOT) operation.
   
   Function performs SUB logical oparation on gap vectors.
   If possible function put the result into vect1 and returns this
   pointer.  Otherwise result is put into tmp_buf, which should be 
   twice of the vector size.

   \param vect1   - operand 1
   \param vect2   - operand 2
   \param tmp_buf - pointer on temporary buffer
   \return Result pointer (tmp_buf)

   @ingroup gapfunc
*/
inline gap_word_t* gap_operation_sub(const gap_word_t*  BMRESTRICT vect1,
                                     const gap_word_t*  BMRESTRICT vect2,
                                     gap_word_t*        BMRESTRICT tmp_buf)
{
//    gap_temp_invert(vect2);
    
    gap_buff_op(tmp_buf, vect1, 0, vect2, 1, and_op);    
//    gap_word_t* res = gap_operation_and(vect1, vect2, tmp_buf);
//    gap_temp_invert(vect2);

    return tmp_buf;
}


// ----------------------------------------------------------------------

// BIT blocks manipulation functions:


/*!
   \brief Bitblock copy operation. 

   \param dst - destination block.
   \param src - source block.

   @ingroup bitfunc
*/
inline 
void bit_block_copy(FerrisBitMagic::word_t* BMRESTRICT dst, const FerrisBitMagic::word_t* BMRESTRICT src)
{
#ifdef BMVECTOPT
    VECT_COPY_BLOCK(dst, src, src + FerrisBitMagic::set_block_size);
#else
    ::memcpy(dst, src, FerrisBitMagic::set_block_size * sizeof(FerrisBitMagic::word_t));
#endif
}


/*!
   \brief Plain bitblock AND operation. 
   Function does not analyse availability of source and destination blocks.

   \param dst - destination block.
   \param src - source block.

   @ingroup bitfunc
*/
inline 
void bit_block_and(FerrisBitMagic::word_t* BMRESTRICT dst, const FerrisBitMagic::word_t* BMRESTRICT src)
{
#ifdef BMVECTOPT
    VECT_AND_ARR(dst, src, src + FerrisBitMagic::set_block_size);
#else
    const FerrisBitMagic::wordop_t* BMRESTRICT wrd_ptr = (wordop_t*)src;
    const FerrisBitMagic::wordop_t* BMRESTRICT wrd_end = (wordop_t*)(src + FerrisBitMagic::set_block_size);
    FerrisBitMagic::wordop_t* BMRESTRICT dst_ptr = (wordop_t*)dst;

    do
    {
        dst_ptr[0] &= wrd_ptr[0];
        dst_ptr[1] &= wrd_ptr[1];
        dst_ptr[2] &= wrd_ptr[2];
        dst_ptr[3] &= wrd_ptr[3];

        dst_ptr+=4;
        wrd_ptr+=4;
    } while (wrd_ptr < wrd_end);
#endif
}


/*!
   \brief Function ANDs two bitblocks and computes the bitcount. 
   Function does not analyse availability of source blocks.

   \param src1     - first bit block
   \param src1_end - first bit block end
   \param src2     - second bit block

   @ingroup bitfunc
*/
inline 
unsigned bit_block_and_count(const FerrisBitMagic::word_t* src1, 
                             const FerrisBitMagic::word_t* src1_end,
                             const FerrisBitMagic::word_t* src2)
{
    unsigned count;
#ifdef BMVECTOPT
    count = VECT_BITCOUNT_AND(src1, src1_end, src2);
#else  
    count = 0;  
    do
    {
        BM_INCWORD_BITCOUNT(count, src1[0] & src2[0]);
        BM_INCWORD_BITCOUNT(count, src1[1] & src2[1]);
        BM_INCWORD_BITCOUNT(count, src1[2] & src2[2]);
        BM_INCWORD_BITCOUNT(count, src1[3] & src2[3]);

        src1+=4;
        src2+=4;
    } while (src1 < src1_end);
#endif    
    return count;
}


/*!
   \brief Function XORs two bitblocks and computes the bitcount. 
   Function does not analyse availability of source blocks.

   \param src1     - first bit block.
   \param src1_end - first bit block end
   \param src2     - second bit block.

   @ingroup bitfunc
*/
inline 
unsigned bit_block_xor_count(const FerrisBitMagic::word_t* BMRESTRICT src1,
                             const FerrisBitMagic::word_t* BMRESTRICT src1_end, 
                             const FerrisBitMagic::word_t* BMRESTRICT src2)
{
    unsigned count;
#ifdef BMVECTOPT
    count = VECT_BITCOUNT_XOR(src1, src1_end, src2);
#else  
    count = 0;  
    do
    {
        BM_INCWORD_BITCOUNT(count, src1[0] ^ src2[0]);
        BM_INCWORD_BITCOUNT(count, src1[1] ^ src2[1]);
        BM_INCWORD_BITCOUNT(count, src1[2] ^ src2[2]);
        BM_INCWORD_BITCOUNT(count, src1[3] ^ src2[3]);

        src1+=4;
        src2+=4;
    } while (src1 < src1_end);
#endif
    return count;
}


/*!
   \brief Function SUBs two bitblocks and computes the bitcount. 
   Function does not analyse availability of source blocks.

   \param src1     - first bit block.
   \param src1_end - first bit block end
   \param src2     - second bit block.

   @ingroup bitfunc
*/
inline 
unsigned bit_block_sub_count(const FerrisBitMagic::word_t* BMRESTRICT src1, 
                             const FerrisBitMagic::word_t* BMRESTRICT src1_end, 
                             const FerrisBitMagic::word_t* BMRESTRICT src2)
{
    unsigned count;
#ifdef BMVECTOPT
    count = VECT_BITCOUNT_SUB(src1, src1_end, src2);
#else  
    count = 0;  
    do
    {
        BM_INCWORD_BITCOUNT(count, src1[0] & ~src2[0]);
        BM_INCWORD_BITCOUNT(count, src1[1] & ~src2[1]);
        BM_INCWORD_BITCOUNT(count, src1[2] & ~src2[2]);
        BM_INCWORD_BITCOUNT(count, src1[3] & ~src2[3]);

        src1+=4;
        src2+=4;
    } while (src1 < src1_end);
#endif
    return count;
}


/*!
   \brief Function ORs two bitblocks and computes the bitcount. 
   Function does not analyse availability of source blocks.

   \param src1     - first bit block
   \param src1_end - first block end
   \param src2     - second bit block.

   @ingroup bitfunc
*/
inline 
unsigned bit_block_or_count(const FerrisBitMagic::word_t* src1, 
                            const FerrisBitMagic::word_t* src1_end,
                            const FerrisBitMagic::word_t* src2)
{
    unsigned count;
#ifdef BMVECTOPT
    count = VECT_BITCOUNT_OR(src1, src1_end, src2);
#else  
    count = 0;  
    do
    {
        BM_INCWORD_BITCOUNT(count, src1[0] | src2[0]);
        BM_INCWORD_BITCOUNT(count, src1[1] | src2[1]);
        BM_INCWORD_BITCOUNT(count, src1[2] | src2[2]);
        BM_INCWORD_BITCOUNT(count, src1[3] | src2[3]);

        src1+=4;
        src2+=4;
    } while (src1 < src1_end);
#endif
    return count;
}


/*!
   \brief bitblock AND operation. 

   \param dst - destination block.
   \param src - source block.

   \returns pointer on destination block. 
    If returned value  equal to src means that block mutation requested. 
    NULL is valid return value.

   @ingroup bitfunc
*/
inline FerrisBitMagic::word_t* bit_operation_and(FerrisBitMagic::word_t* BMRESTRICT dst, 
                                     const FerrisBitMagic::word_t* BMRESTRICT src)
{
    BM_ASSERT(dst || src);

    FerrisBitMagic::word_t* ret = dst;

    if (IS_VALID_ADDR(dst))  // The destination block already exists
    {

        if (!IS_VALID_ADDR(src))
        {
            if (IS_EMPTY_BLOCK(src))
            {
                //If the source block is zero 
                //just clean the destination block
                return 0;
            }
        }
        else
        {
            // Regular operation AND on the whole block.
            bit_block_and(dst, src);
        }
    }
    else // The destination block does not exist yet
    {
        if(!IS_VALID_ADDR(src))
        {
            if(IS_EMPTY_BLOCK(src)) 
            {
                // The source block is empty.
                // One argument empty - all result is empty.
                return 0;
            }
            // Here we have nothing to do.
            // Src block is all ON, dst block remains as it is
        }
        else // destination block does not exists, src - valid block
        {
            if (IS_FULL_BLOCK(dst))
            {
                return const_cast<FerrisBitMagic::word_t*>(src);
            }
            // Nothng to do.
            // Dst block is all ZERO no combination required.
        }
    }

    return ret;
}


/*!
   \brief Performs bitblock AND operation and calculates bitcount of the result. 

   \param src1     - first bit block.
   \param src1_end - first bit block end
   \param src2     - second bit block.

   \returns bitcount value 

   @ingroup bitfunc
*/
inline 
FerrisBitMagic::id_t bit_operation_and_count(const FerrisBitMagic::word_t* BMRESTRICT src1,
                                 const FerrisBitMagic::word_t* BMRESTRICT src1_end,
                                 const FerrisBitMagic::word_t* BMRESTRICT src2)
{
    if (IS_EMPTY_BLOCK(src1) || IS_EMPTY_BLOCK(src2))
    {
        return 0;
    }
    return bit_block_and_count(src1, src1_end, src2);
}


/*!
   \brief Performs bitblock SUB operation and calculates bitcount of the result. 

   \param src1      - first bit block.
   \param src1_end  - first bit block end
   \param src2      - second bit block

   \returns bitcount value 

   @ingroup bitfunc
*/
inline 
FerrisBitMagic::id_t bit_operation_sub_count(const FerrisBitMagic::word_t* BMRESTRICT src1, 
                                 const FerrisBitMagic::word_t* BMRESTRICT src1_end,
                                 const FerrisBitMagic::word_t* BMRESTRICT src2)
{
    if (IS_EMPTY_BLOCK(src1))
    {
        return 0;
    }
    
    if (IS_EMPTY_BLOCK(src2)) // nothing to diff
    {
        return bit_block_calc_count(src1, src1_end);
    }
    return bit_block_sub_count(src1, src1_end, src2);
}


/*!
   \brief Performs bitblock OR operation and calculates bitcount of the result. 

   \param src1     - first bit block.
   \param src1_end - first bit block end
   \param src2     - second bit block.

   \returns bitcount value 

   @ingroup bitfunc
*/
inline 
FerrisBitMagic::id_t bit_operation_or_count(const FerrisBitMagic::word_t* BMRESTRICT src1,
                                const FerrisBitMagic::word_t* BMRESTRICT src1_end, 
                                const FerrisBitMagic::word_t* BMRESTRICT src2)
{
    if (IS_EMPTY_BLOCK(src1))
    {
        if (!IS_EMPTY_BLOCK(src2))
            return bit_block_calc_count(src2, src2 + (src1_end - src1));
        else
            return 0; // both blocks are empty        
    }
    else
    {
        if (IS_EMPTY_BLOCK(src2))
            return bit_block_calc_count(src1, src1_end);
    }

    return bit_block_or_count(src1, src1_end, src2);
}


/*!
   \brief Plain bitblock OR operation. 
   Function does not analyse availability of source and destination blocks.

   \param dst - destination block.
   \param src - source block.

   @ingroup bitfunc
*/
inline void bit_block_or(FerrisBitMagic::word_t* BMRESTRICT dst, 
                         const FerrisBitMagic::word_t* BMRESTRICT src)
{
#ifdef BMVECTOPT
    VECT_OR_ARR(dst, src, src + FerrisBitMagic::set_block_size);
#else
    const FerrisBitMagic::wordop_t* BMRESTRICT wrd_ptr = (wordop_t*)src;
    const FerrisBitMagic::wordop_t* BMRESTRICT wrd_end = (wordop_t*)(src + set_block_size);
    FerrisBitMagic::wordop_t* BMRESTRICT dst_ptr = (wordop_t*)dst;

    do
    {
        dst_ptr[0] |= wrd_ptr[0];
        dst_ptr[1] |= wrd_ptr[1];
        dst_ptr[2] |= wrd_ptr[2];
        dst_ptr[3] |= wrd_ptr[3];

        dst_ptr+=4;
        wrd_ptr+=4;

    } while (wrd_ptr < wrd_end);
#endif
}


/*!
   \brief Block OR operation. Makes analysis if block is 0 or FULL. 

   \param dst - destination block.
   \param src - source block.

   \returns pointer on destination block. 
    If returned value  equal to src means that block mutation requested. 
    NULL is valid return value.

   @ingroup bitfunc
*/
inline 
FerrisBitMagic::word_t* bit_operation_or(FerrisBitMagic::word_t* BMRESTRICT dst, 
                             const FerrisBitMagic::word_t* BMRESTRICT src)
{
    BM_ASSERT(dst || src);

    FerrisBitMagic::word_t* ret = dst;

    if (IS_VALID_ADDR(dst)) // The destination block already exists
    {
        if (!IS_VALID_ADDR(src))
        {
            if (IS_FULL_BLOCK(src))
            {
                // if the source block is all set 
                // just set the destination block
                ::memset(dst, 0xFF, FerrisBitMagic::set_block_size * sizeof(FerrisBitMagic::word_t));
            }
        }
        else
        {
            // Regular operation OR on the whole block
            bit_block_or(dst, src);
        }
    }
    else // The destination block does not exist yet
    {
        if (!IS_VALID_ADDR(src))
        {
            if (IS_FULL_BLOCK(src)) 
            {
                // The source block is all set, because dst does not exist
                // we can simply replace it.
                return const_cast<FerrisBitMagic::word_t*>(FULL_BLOCK_ADDR);
            }
        }
        else
        {
            if (dst == 0)
            {
                // The only case when we have to allocate the new block:
                // Src is all zero and Dst does not exist
                return const_cast<FerrisBitMagic::word_t*>(src);
            }
        }
    }
    return ret;
}

/*!
   \brief Plain bitblock SUB (AND NOT) operation. 
   Function does not analyse availability of source and destination blocks.

   \param dst - destination block.
   \param src - source block.

   @ingroup bitfunc
*/
inline 
void bit_block_sub(FerrisBitMagic::word_t* BMRESTRICT dst, 
                   const FerrisBitMagic::word_t* BMRESTRICT src)
{
#ifdef BMVECTOPT
    VECT_SUB_ARR(dst, src, src + FerrisBitMagic::set_block_size);
#else
    const FerrisBitMagic::wordop_t* BMRESTRICT wrd_ptr = (wordop_t*) src;
    const FerrisBitMagic::wordop_t* BMRESTRICT wrd_end = 
                     (wordop_t*) (src + FerrisBitMagic::set_block_size);
    FerrisBitMagic::wordop_t* dst_ptr = (wordop_t*)dst;
    
    // Regular operation AND-NOT on the whole block.
    do
    {
        dst_ptr[0] &= ~wrd_ptr[0];
        dst_ptr[1] &= ~wrd_ptr[1];
        dst_ptr[2] &= ~wrd_ptr[2];
        dst_ptr[3] &= ~wrd_ptr[3];

        dst_ptr+=4;
        wrd_ptr+=4;
    } while (wrd_ptr < wrd_end);
#endif
    
}


/*!
   \brief bitblock SUB operation. 

   \param dst - destination block.
   \param src - source block.

   \returns pointer on destination block. 
    If returned value  equal to src means that block mutation requested. 
    NULL is valid return value.

   @ingroup bitfunc
*/
inline 
FerrisBitMagic::word_t* bit_operation_sub(FerrisBitMagic::word_t* BMRESTRICT dst, 
                              const FerrisBitMagic::word_t* BMRESTRICT src)
{
    BM_ASSERT(dst || src);

    FerrisBitMagic::word_t* ret = dst;
    if (IS_VALID_ADDR(dst))  //  The destination block already exists
    {
        if (!IS_VALID_ADDR(src))
        {
            if (IS_FULL_BLOCK(src))
            {
                // If the source block is all set
                // just clean the destination block
                return 0;
            }
        }
        else
        {
            bit_block_sub(dst, src);
        }
    }
    else // The destination block does not exist yet
    {
        if (!IS_VALID_ADDR(src))
        {
            if (IS_FULL_BLOCK(src)) 
            {
                // The source block is full
                return 0;
            }
        }
        else
        {
            if (IS_FULL_BLOCK(dst))
            {
                // The only case when we have to allocate the new block:
                // dst is all set and src exists
                return const_cast<FerrisBitMagic::word_t*>(src);                  
            }
        }
    }
    return ret;
}


/*!
   \brief Plain bitblock XOR operation. 
   Function does not analyse availability of source and destination blocks.

   \param dst - destination block.
   \param src - source block.

   @ingroup bitfunc
*/
inline 
void bit_block_xor(FerrisBitMagic::word_t* BMRESTRICT dst, 
                   const FerrisBitMagic::word_t* BMRESTRICT src)
{
#ifdef BMVECTOPT
    VECT_XOR_ARR(dst, src, src + FerrisBitMagic::set_block_size);
#else
    const FerrisBitMagic::wordop_t* BMRESTRICT wrd_ptr = (wordop_t*) src;
    const FerrisBitMagic::wordop_t* BMRESTRICT wrd_end = 
                            (wordop_t*) (src + FerrisBitMagic::set_block_size);
    FerrisBitMagic::wordop_t* BMRESTRICT dst_ptr = (wordop_t*)dst;

    // Regular XOR operation on the whole block.
    do
    {
        dst_ptr[0] ^= wrd_ptr[0];
        dst_ptr[1] ^= wrd_ptr[1];
        dst_ptr[2] ^= wrd_ptr[2];
        dst_ptr[3] ^= wrd_ptr[3];

        dst_ptr+=4;
        wrd_ptr+=4;
    } while (wrd_ptr < wrd_end);
#endif
    
}


/*!
   \brief bitblock XOR operation. 

   \param dst - destination block.
   \param src - source block.

   \returns pointer on destination block. 
    If returned value  equal to src means that block mutation requested. 
    NULL is valid return value.

   @ingroup bitfunc
*/
inline 
FerrisBitMagic::word_t* bit_operation_xor(FerrisBitMagic::word_t* BMRESTRICT dst, 
                              const FerrisBitMagic::word_t* BMRESTRICT src)
{
    BM_ASSERT(dst || src);
    if (src == dst) return 0;  // XOR rule  

    FerrisBitMagic::word_t* ret = dst;

    if (IS_VALID_ADDR(dst))  //  The destination block already exists
    {           
        if (!src) return dst;
        
        bit_block_xor(dst, src);
    }
    else // The destination block does not exist yet
    {
        if (!src) return dst;      // 1 xor 0 = 1

        // Here we have two cases:
        // if dest block is full or zero if zero we need to copy the source
        // otherwise XOR loop against 0xFF...
        //BM_ASSERT(dst == 0);
        return const_cast<FerrisBitMagic::word_t*>(src);  // src is the final result               
    }
    return ret;
}

/*!
   \brief Performs bitblock XOR operation and calculates bitcount of the result. 

   \param src1 - first bit block.
   \param src2 - second bit block.

   \returns bitcount value 

   @ingroup bitfunc
*/
inline 
FerrisBitMagic::id_t bit_operation_xor_count(const FerrisBitMagic::word_t* BMRESTRICT src1,
                                 const FerrisBitMagic::word_t* BMRESTRICT src1_end,
                                 const FerrisBitMagic::word_t* BMRESTRICT src2)
{
    if (IS_EMPTY_BLOCK(src1) || IS_EMPTY_BLOCK(src2))
    {
        if (IS_EMPTY_BLOCK(src1) && IS_EMPTY_BLOCK(src2))
            return 0;
        const FerrisBitMagic::word_t* block = IS_EMPTY_BLOCK(src1) ? src2 : src1;
        return bit_block_calc_count(block, block + (src1_end - src1));
    }
    return bit_block_xor_count(src1, src1_end, src2);
}



//------------------------------------------------------------------------

/**
    \brief Searches for the next 1 bit in the BIT block
    \param data - BIT buffer
    \param nbit - bit index to start checking from
    \param prev - returns previously checked value

    @ingroup bitfunc
*/
inline 
int bit_find_in_block(const FerrisBitMagic::word_t* data, 
                      unsigned nbit, 
                      FerrisBitMagic::id_t* prev)
{
    register FerrisBitMagic::id_t p = *prev;
    int found = 0;

    for(;;)
    {
        unsigned nword  = nbit >> FerrisBitMagic::set_word_shift;
        if (nword >= FerrisBitMagic::set_block_size) break;

        register FerrisBitMagic::word_t val = data[nword] >> (p & FerrisBitMagic::set_word_mask);

        if (val)
        {
            while((val & 1) == 0)
            {
                val >>= 1;
                ++nbit;
                ++p;
            }
            ++found;
            break;
        }
        else
        {
           p    += (FerrisBitMagic::set_word_mask + 1) - (nbit & FerrisBitMagic::set_word_mask);
           nbit += (FerrisBitMagic::set_word_mask + 1) - (nbit & FerrisBitMagic::set_word_mask);
        }
    }
    *prev = p;
    return found;
}

/*!
   \brief Unpacks word into list of ON bit indexes
   \param w - value
   \param bits - pointer on the result array 
   \return number of bits in the list

   @ingroup bitfunc
*/
template<typename T,typename B> unsigned bit_list(T w, B* bits)
{
    // Note: 4-bit table method works slower than plain check approach
    unsigned octet = 0;
    B*       bp = bits;
    do
    {
        if (w & 1)   *bp++ = octet + 0;
        if (w & 2)   *bp++ = octet + 1;
        if (w & 4)   *bp++ = octet + 2;
        if (w & 8)   *bp++ = octet + 3;
        if (w & 16)  *bp++ = octet + 4;
        if (w & 32)  *bp++ = octet + 5;
        if (w & 64)  *bp++ = octet + 6;
        if (w & 128) *bp++ = octet + 7;

        w >>= 8;
        octet += 8;
    } while (w);
    return bp - bits;
}


/*! @brief Calculates memory overhead for number of gap blocks sharing 
           the same memory allocation table (level lengths table).
    @ingroup gapfunc
*/
template<typename T> 
unsigned gap_overhead(const T* length, 
                      const T* length_end, 
                      const T* glevel_len)
{
    BM_ASSERT(length && length_end && glevel_len);

    unsigned overhead = 0;
    for (;length < length_end; ++length)
    {
        unsigned len = *length;
        int level = gap_calc_level(len, glevel_len);
        BM_ASSERT(level >= 0 && level < (int)FerrisBitMagic::gap_levels);
        unsigned capacity = glevel_len[level];
        BM_ASSERT(capacity >= len);
        overhead += capacity - len;
    }
    return overhead;
}


/*! @brief Finds optimal gap blocks lengths.
    @param length - first element of GAP lengths array
    @param length_end - end of the GAP lengths array
    @param glevel_len - destination GAP lengths array
    @ingroup gapfunc
*/

template<typename T>
bool improve_gap_levels(const T* length,
                        const T* length_end,
                        T*       glevel_len)
{
    BM_ASSERT(length && length_end && glevel_len);

    size_t lsize = length_end - length;

    BM_ASSERT(lsize);
    
    gap_word_t max_len = 0;
    unsigned i;
    for (i = 0; i < lsize; ++i)
    {
        if (length[i] > max_len)
            max_len = length[i];
    }
    if (max_len < 5 || lsize <= FerrisBitMagic::gap_levels)
    {
        glevel_len[0] = max_len + 4;
        for (i = 1; i < FerrisBitMagic::gap_levels; ++i)
        {
            glevel_len[i] = FerrisBitMagic::gap_max_buff_len;
        }
        return true;
    }

    glevel_len[FerrisBitMagic::gap_levels-1] = max_len + 5;

    unsigned min_overhead = gap_overhead(length, length_end, glevel_len);
    bool is_improved = false;
    gap_word_t prev_value = glevel_len[FerrisBitMagic::gap_levels-1];
    //
    // main problem solving loop
    //
    for (i = FerrisBitMagic::gap_levels-2; ; --i)
    {
        unsigned opt_len = 0;
        unsigned j;
        bool imp_flag = false;
        gap_word_t gap_saved_value = glevel_len[i];
        for (j = 0; j < lsize; ++j)
        {
//            if (length[j]+4 > prev_value)
//                continue;
            
            glevel_len[i] = length[j]+4;
            unsigned ov = gap_overhead(length, length_end, glevel_len);
            if (ov <= min_overhead)
            {
                min_overhead = ov;                
                opt_len = length[j]+4;
                imp_flag = true;
            }
        }
        if (imp_flag) {
            glevel_len[i] = opt_len; // length[opt_idx]+4;
            is_improved = true;
        }
        else 
        {
            glevel_len[i] = gap_saved_value;
        }
        if (i == 0) 
            break;
        prev_value = glevel_len[i];
    }
    // 
    // Remove duplicates
    //

    T val = *glevel_len;
    T* gp = glevel_len;
    T* res = glevel_len;
    for (i = 0; i < FerrisBitMagic::gap_levels; ++i)
    {
        if (val != *gp)
        {
            val = *gp;
            *++res = val;
        }
        ++gp;
    }

    // Filling the "unused" part with max. possible value
    while (++res < (glevel_len + FerrisBitMagic::gap_levels)) 
    {
        *res = FerrisBitMagic::gap_max_buff_len;
    }

    return is_improved;

}



} // namespace FerrisBitMagic

#endif

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//#include "bmvmin.h"
/*
Copyright(c) 2002-2005 Anatoliy Kuznetsov(anatoliy_kuznetsov at yahoo.com)

Permission is hereby granted, free of charge, to any person 
obtaining a copy of this software and associated documentation 
files (the "Software"), to deal in the Software without restriction, 
including without limitation the rights to use, copy, modify, merge, 
publish, distribute, sublicense, and/or sell copies of the Software, 
and to permit persons to whom the Software is furnished to do so, 
subject to the following conditions:

The above copyright notice and this permission notice shall be included 
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR 
OTHER DEALINGS IN THE SOFTWARE.

For more information please visit:  http://bmagic.sourceforge.net

*/

#ifndef BMVMIN__H__INCLUDED__
#define BMVMIN__H__INCLUDED__

namespace FerrisBitMagic
{


#define BM_MINISET_GAPLEN (FerrisBitMagic::gap_len_table<true>::_len[0])
#define BM_MINISET_ARRSIZE(x) ((x / 32) + ( (x % 32) && 1 ))

/*! @defgroup mset Small sets functionality
 *  Templates in this group are used to keep block types in BM library.
 *  Classes of this group can tune bvector template (MS parameter)
 *  for best performance or minimal memory usage.
 *  @ingroup bmagic
 *  @{
 */


/*!
    @brief Template class implements memory saving set functionality
    
    Template can be used as template parameter for bvector if we 
    want to tune bvector for minimal memory consumption.

    @sa bvmini
*/
template <class A, size_t N> class miniset
{
public:

    miniset() 
        : m_buf(0),
          m_type(1)
    {}

    miniset(const miniset& mset)
    {
        if (mset.m_buf)
        {
            if (mset.m_type)
                init_gapbuf(mset.m_buf);
            else
                init_bitbuf(mset.m_buf);
        }
        else
        {
            m_type = mset.m_type;
            m_buf = 0;
        }
    }

    ~miniset()
    {
        if (m_buf)
        {
            A::deallocate(m_buf, m_type ? 
                (BM_MINISET_GAPLEN / (sizeof(FerrisBitMagic::word_t) / sizeof(FerrisBitMagic::gap_word_t)))
                : 
                (BM_MINISET_ARRSIZE(N)));
        }
    }

    /// Checks if bit pos 1 or 0. Returns 0 if 0 and non zero otherwise.
    unsigned test(FerrisBitMagic::id_t n) const 
    { 
        return
            !m_buf ? 0 
            :
            m_type ? 
              gap_test((gap_word_t*)m_buf, n)
              : 
              m_buf[n>>FerrisBitMagic::set_word_shift] & (1<<(n & FerrisBitMagic::set_word_mask));
    }

    void set(FerrisBitMagic::id_t n, bool val=true)
    {
        if (m_type == 0)
        {
            if (!m_buf)
            {
                if (!val) return;
                init_bitbuf(0);
            }

            unsigned nword  = n >> FerrisBitMagic::set_word_shift; 
            unsigned mask = unsigned(1) << (n & FerrisBitMagic::set_word_mask);

            val ? (m_buf[nword] |= mask) : (m_buf[nword] &= ~mask);
        }
        else
        {
            if (!m_buf)
            {
                if (!val) return;
                init_gapbuf(0);
            }
            
            unsigned is_set;
            unsigned new_block_len = 
                gap_set_value(val, (gap_word_t*)m_buf, n, &is_set);

            if (new_block_len > unsigned(BM_MINISET_GAPLEN-4))
            {
                convert_buf();
            }
        }
    }

    unsigned mem_used() const
    {
        return sizeof(*this) +
               m_buf ? 
                 (m_type ? (BM_MINISET_GAPLEN * sizeof(gap_word_t))
                        : (BM_MINISET_ARRSIZE(N) * sizeof(FerrisBitMagic::word_t)))
                : 0; 
    }

    void swap(miniset& mset)
    {
        FerrisBitMagic::word_t* buftmp = m_buf;
        m_buf = mset.m_buf;
        mset.m_buf = buftmp;
        unsigned typetmp = m_type;
        m_type = mset.m_type;
        mset.m_type = typetmp;
    }


private:

    void init_bitbuf(FerrisBitMagic::word_t* buf)
    {
        unsigned arr_size = BM_MINISET_ARRSIZE(N);
        m_buf = A::allocate(arr_size, 0);
        if (buf)
        {
            ::memcpy(m_buf, buf, arr_size * sizeof(FerrisBitMagic::word_t));
        }
        else
        {
            ::memset(m_buf, 0, arr_size * sizeof(FerrisBitMagic::word_t));
        }
        m_type = 0;
    }

    void init_gapbuf(FerrisBitMagic::word_t* buf)
    {
        unsigned arr_size = 
            BM_MINISET_GAPLEN / (sizeof(FerrisBitMagic::word_t) / sizeof(FerrisBitMagic::gap_word_t));
        m_buf = A::allocate(arr_size, 0);
        if (buf)
        {
            ::memcpy(m_buf, buf, arr_size * sizeof(FerrisBitMagic::word_t));
        }
        else
        {
            *m_buf = 0;
            gap_set_all((gap_word_t*)m_buf, FerrisBitMagic::gap_max_bits, 0);
        }
        m_type = 1;
    }

    void convert_buf()
    {
        unsigned arr_size = BM_MINISET_ARRSIZE(N);
        FerrisBitMagic::word_t* buf = A::allocate(arr_size, 0);

        gap_convert_to_bitset(buf, (gap_word_t*) m_buf, arr_size);
        arr_size = 
            BM_MINISET_GAPLEN / (sizeof(FerrisBitMagic::word_t) / sizeof(FerrisBitMagic::gap_word_t));
        A::deallocate(m_buf, arr_size);
        m_buf = buf;
        m_type = 0;
    }

private:
    FerrisBitMagic::word_t*   m_buf;      //!< Buffer pointer
    unsigned      m_type;     //!< buffer type (0-bit, 1-gap)
};


/*!
    @brief Mini bitvector used in bvector template to keep block type flags
    
    Template is used as a default template parameter MS for bvector  
    Offers maximum performance comparing to miniset.

    @sa miniset
*/
template<size_t N> class bvmini
{
public:

    bvmini(int start_strategy = 0) 
    {
        ::memset(m_buf, 0, sizeof(m_buf));
    }

    bvmini(const bvmini& mset)
    {
        ::memcpy(m_buf, mset.m_buf, sizeof(m_buf));
    }


    /// Checks if bit pos 1 or 0. Returns 0 if 0 and non zero otherwise.
    unsigned test(FerrisBitMagic::id_t n) const 
    { 
        return m_buf[n>>FerrisBitMagic::set_word_shift] & (1<<(n & FerrisBitMagic::set_word_mask));
    }

    void set(FerrisBitMagic::id_t n, bool val=true)
    {
        unsigned nword  = n >> FerrisBitMagic::set_word_shift; 
        unsigned mask = unsigned(1) << (n & FerrisBitMagic::set_word_mask);

        val ? (m_buf[nword] |= mask) : (m_buf[nword] &= ~mask);
    }

    unsigned mem_used() const
    {
        return sizeof(*this);
    }

    void swap(bvmini& mset)
    {
        for (unsigned i = 0; i < BM_MINISET_ARRSIZE(N); ++i)
        {
            FerrisBitMagic::word_t tmp = m_buf[i];
            m_buf[i] = mset.m_buf[i];
            mset.m_buf[i] = tmp;
        }
    }

private:
    FerrisBitMagic::word_t   m_buf[BM_MINISET_ARRSIZE(N)];
};


/*!@} */

/*!
    @brief Bitvector class with very limited functionality.

    Class implements simple bitset and used for internal 
    and testing purposes. 
*/
template<class A> class bvector_mini
{
public:
    bvector_mini(unsigned size) 
      : m_buf(0),
        m_size(size)
    {
        unsigned arr_size = (size / 32) + 1; 
        m_buf = A::allocate(arr_size, 0);
        ::memset(m_buf, 0, arr_size * sizeof(unsigned));
    }
    bvector_mini(const bvector_mini& bvect)
       : m_size(bvect.m_size)
    {
        unsigned arr_size = (m_size / 32) + 1;
        m_buf = A::allocate(arr_size, 0);
        ::memcpy(m_buf, bvect.m_buf, arr_size * sizeof(unsigned));        
    }

    ~bvector_mini()
    {
        A::deallocate(m_buf, (m_size / 32) + 1); 
    }

    /// Checks if bit pos 1 or 0. Returns 0 if 0 and non zero otherwise.
    int is_bit_true(unsigned pos) const
    {
        unsigned char  mask = (unsigned char)((char)0x1 << (pos & 7));
        unsigned char* offs = (unsigned char*)m_buf + (pos >> 3); // m_buf + (pos/8)

        return (*offs) & mask;
    }

    /// Sets bit number pos to 1
    void set_bit(unsigned pos)
    {
        unsigned char  mask = (unsigned char)(0x1 << (pos & 7));
        unsigned char* offs = (unsigned char*)m_buf + (pos >> 3); 
        *offs |= mask;
    }


    /// Sets bit number pos to 0
    void clear_bit(unsigned pos)
    {
        unsigned char  mask = (unsigned char)(0x1 << (pos & 7));
        unsigned char* offs = (unsigned char*)m_buf + (pos >> 3); 

        *offs &= ~mask;
    }

    /// Counts number of bits ON 
    unsigned bit_count() const
    {
        register unsigned count = 0;
        const unsigned* end = m_buf + (m_size / 32)+1;    

        for (unsigned* start = m_buf; start < end; ++start)
        {
            register unsigned value = *start;
            for (count += (value!=0); value &= value - 1; ++count);
        }
        return count;
    }

    /// Comparison.
    int compare(const bvector_mini& bvect)
    {
        unsigned cnt1 = bit_count();
        unsigned cnt2 = bvect.bit_count();

        if (!cnt1 && !cnt2) return 0;

        unsigned cnt_min = cnt1 < cnt2 ? cnt1 : cnt2;

        if (!cnt_min) return cnt1 ? 1 : -1;

        unsigned idx1 = get_first();
        unsigned idx2 = bvect.get_first();

        for (unsigned i = 0; i < cnt_min; ++i)
        {
            if (idx1 != idx2)
            {
                return idx1 < idx2 ? 1 : -1;
            }
            idx1 = get_next(idx1);
            idx2 = bvect.get_next(idx2);
        }

        BM_ASSERT(idx1==0 || idx2==0);

        if (idx1 != idx2)
        {
            if (!idx1) return -1;
            if (!idx2) return  1;
            return idx1 < idx2 ? 1 : -1;
        }

        return 0;
    }


    /// Returns index of the first ON bit
    unsigned get_first() const
    {
        unsigned pos = 0;
        const unsigned char* ptr = (unsigned char*) m_buf;

        for (unsigned i = 0; i < (m_size/8)+1; ++i)
        {
            register unsigned char w = ptr[i];


            if (w != 0)
            {
                while ((w & 1) == 0)
                {
                    w >>= 1;
                    ++pos;
                }
                return pos;
            }
            pos += sizeof(unsigned char) * 8;
        }
        return 0;
    }


    /// Returns index of next bit, which is ON
    unsigned get_next(unsigned idx) const
    {
        register unsigned i;

        for (i = idx+1; i < m_size; ++i)
        {
            unsigned char* offs = (unsigned char*)m_buf + (i >> 3); 
            if (*offs)
            {
                unsigned char mask = (unsigned char)((char)0x1 << (i & 7));

                if (*offs & mask)
                {
                    return i;
                }
            }
            else
            {
                i += 7;
            }
        }
        return 0;
    }


    void combine_and(const bvector_mini& bvect)
    {
        const unsigned* end = m_buf + (m_size / 32)+1;
    
        const unsigned* src = bvect.get_buf();

        for (unsigned* start = m_buf; start < end; ++start)
        {
            *start &= *src++;
        }
    }

    void combine_xor(const bvector_mini& bvect)
    {
        const unsigned* end = m_buf + (m_size / 32)+1;
    
        const unsigned* src = bvect.get_buf();

        for (unsigned* start = m_buf; start < end; ++start)
        {
            *start ^= *src++;
        }
    }


    void combine_or(const bvector_mini& bvect)
    {
        const unsigned* end = m_buf + (m_size / 32)+1;
    
        const unsigned* src = bvect.get_buf();

        for (unsigned* start = m_buf; start < end; ++start)
        {
            *start |= *src++;
        }
    }

    void combine_sub(const bvector_mini& bvect)
    {
        const unsigned* end = m_buf + (m_size / 32)+1;
    
        const unsigned* src = bvect.get_buf();

        for (unsigned* start = m_buf; start < end; ++start)
        {
            *start &= ~(*src++);
        }
    }

    const unsigned* get_buf() const { return m_buf; }
    unsigned mem_used() const
    {
        return sizeof(bvector_mini) + (m_size / 32) + 1;
    }

    void swap(bvector_mini& bvm)
    {
        BM_ASSERT(m_size == bvm.m_size);
        FerrisBitMagic::word_t* buftmp = m_buf;
        m_buf = bvm.m_buf;
        bvm.m_buf = buftmp;
    }

private:
    FerrisBitMagic::word_t*   m_buf;
    unsigned      m_size;
};



} // namespace FerrisBitMagic

#endif
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//#include "encoding.h"
/*
Copyright(c) 2002-2005 Anatoliy Kuznetsov(anatoliy_kuznetsov at yahoo.com)


Permission is hereby granted, free of charge, to any person 
obtaining a copy of this software and associated documentation 
files (the "Software"), to deal in the Software without restriction, 
including without limitation the rights to use, copy, modify, merge, 
publish, distribute, sublicense, and/or sell copies of the Software, 
and to permit persons to whom the Software is furnished to do so, 
subject to the following conditions:

The above copyright notice and this permission notice shall be included 
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR 
OTHER DEALINGS IN THE SOFTWARE.


For more information please visit:   http://bmagic.sourceforge.net

*/

#ifndef ENCODING_H__INCLUDED__
#define ENCODING_H__INCLUDED__

#include <memory.h>

namespace FerrisBitMagic
{

// ----------------------------------------------------------------
/*!
   \brief Memory encoding.
   
   Class for encoding data into memory. 
   Properly handles aligment issues with integer data types.
*/
class encoder
{
public:

    encoder(unsigned char* buf, unsigned size);
    void put_8(unsigned char c);
    void put_16(FerrisBitMagic::short_t  s);
    void put_16(const FerrisBitMagic::short_t* s, unsigned count);
    void put_32(FerrisBitMagic::word_t  w);
    void put_32(const FerrisBitMagic::word_t* w, unsigned count);
    unsigned size() const;

private:

    unsigned char*  buf_;
    unsigned char*  start_;
    unsigned int    size_;
};

// ----------------------------------------------------------------
/**
   Class for decoding data from memory buffer.
   Properly handles aligment issues with integer data types.
*/
class decoder
{
public:
    decoder(const unsigned char* buf);
    unsigned char get_8();
    FerrisBitMagic::short_t get_16();
    FerrisBitMagic::word_t get_32();
    void get_32(FerrisBitMagic::word_t* w, unsigned count);
    void get_16(FerrisBitMagic::short_t* s, unsigned count);
    unsigned size() const;
private:
   const unsigned char*   buf_;
   const unsigned char*   start_;
};



// ----------------------------------------------------------------
// Implementation details. 
// ----------------------------------------------------------------

/*! 
    \fn encoder::encoder(unsigned char* buf, unsigned size) 
    \brief Construction.
    \param buf - memory buffer pointer.
    \param size - size of the buffer
*/
inline encoder::encoder(unsigned char* buf, unsigned size)
: buf_(buf), start_(buf), size_(size)
{
}

/*!
   \fn void encoder::put_8(unsigned char c) 
   \brief Puts one character into the encoding buffer.
   \param c - character to encode
*/
inline void encoder::put_8(unsigned char c)
{
    *buf_++ = c;
}

/*!
   \fn encoder::put_16(FerrisBitMagic::short_t s)
   \brief Puts short word (16 bits) into the encoding buffer.
   \param s - short word to encode
*/
inline void encoder::put_16(FerrisBitMagic::short_t s)
{
    *buf_++ = (unsigned char) s;
    s >>= 8;
    *buf_++ = (unsigned char) s;
}

/*!
   \brief Method puts array of short words (16 bits) into the encoding buffer.
*/
inline void encoder::put_16(const FerrisBitMagic::short_t* s, unsigned count)
{
    for (unsigned i = 0; i < count; ++i) 
    {
        put_16(s[i]);
    }
}


/*!
   \fn unsigned encoder::size() const
   \brief Returns size of the current encoding stream.
*/
inline unsigned encoder::size() const
{
    return (unsigned)(buf_ - start_);
}

/*!
   \fn void encoder::put_32(FerrisBitMagic::word_t w)
   \brief Puts 32 bits word into encoding buffer.
   \param w - word to encode.
*/
inline void encoder::put_32(FerrisBitMagic::word_t w)
{
    *buf_++ = (unsigned char) w;
    *buf_++ = (unsigned char) (w >> 8);
    *buf_++ = (unsigned char) (w >> 16);
    *buf_++ = (unsigned char) (w >> 24);
}

/*!
    \brief Encodes array of 32-bit words
*/
inline void encoder::put_32(const FerrisBitMagic::word_t* w, unsigned count)
{
    for (unsigned i = 0; i < count; ++i) 
    {
        put_32(w[i]);
    }
}


// ---------------------------------------------------------------------

/*!
   \fn decoder::decoder(const unsigned char* buf) 
   \brief Construction
   \param buf - pointer to the decoding memory. 
*/
inline decoder::decoder(const unsigned char* buf) 
: buf_(buf), start_(buf)
{
}

/*!
   \fn unsigned char decoder::get_8()  
   \brief Reads character from the decoding buffer.
*/
inline unsigned char decoder::get_8() 
{
    return *buf_++;
}

/*!
   \fn FerrisBitMagic::short_t decoder::get_16()
   \brief Reads 16bit word from the decoding buffer.
*/
inline FerrisBitMagic::short_t decoder::get_16() 
{
    FerrisBitMagic::short_t a = (FerrisBitMagic::short_t)(buf_[0] + ((FerrisBitMagic::short_t)buf_[1] << 8));
    buf_ += sizeof(a);
    return a;
}

/*!
   \fn FerrisBitMagic::word_t decoder::get_32()
   \brief Reads 32 bit word from the decoding buffer.
*/
inline FerrisBitMagic::word_t decoder::get_32() 
{
    FerrisBitMagic::word_t a = buf_[0]+ ((unsigned)buf_[1] << 8) +
                   ((unsigned)buf_[2] << 16) + ((unsigned)buf_[3] << 24);
    buf_+=sizeof(a);
    return a;
}


/*!
   \fn void decoder::get_32(FerrisBitMagic::word_t* w, unsigned count)
   \brief Reads block of 32-bit words from the decoding buffer.
   \param w - pointer on memory block to read into.
   \param count - size of memory block in words.
*/
inline void decoder::get_32(FerrisBitMagic::word_t* w, unsigned count)
{
    for (unsigned i = 0; i < count; ++i) 
    {
        w[i] = get_32();
    }
}

/*!
   \fn void decoder::get_16(FerrisBitMagic::short_t* s, unsigned count)
   \brief Reads block of 32-bit words from the decoding buffer.
   \param s - pointer on memory block to read into.
   \param count - size of memory block in words.
*/
inline void decoder::get_16(FerrisBitMagic::short_t* s, unsigned count)
{
    for (unsigned i = 0; i < count; ++i) 
    {
        s[i] = get_16();
    }
}

/*!
   \fn unsigned decoder::size() const
   \brief Returns size of the current decoding stream.
*/
inline unsigned decoder::size() const
{
    return (unsigned)(buf_ - start_);
}


} // namespace FerrisBitMagic

#endif


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//#include "bmalloc.h"
/*
Copyright(c) 2002-2005 Anatoliy Kuznetsov(anatoliy_kuznetsov at yahoo.com)

Permission is hereby granted, free of charge, to any person 
obtaining a copy of this software and associated documentation 
files (the "Software"), to deal in the Software without restriction, 
including without limitation the rights to use, copy, modify, merge, 
publish, distribute, sublicense, and/or sell copies of the Software, 
and to permit persons to whom the Software is furnished to do so, 
subject to the following conditions:

The above copyright notice and this permission notice shall be included 
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR 
OTHER DEALINGS IN THE SOFTWARE.

For more information please visit:  http://bmagic.sourceforge.net

*/

#ifndef BMALLOC__H__INCLUDED__
#define BMALLOC__H__INCLUDED__

#include <stdlib.h>

namespace FerrisBitMagic
{


/*! @defgroup alloc Memory Allocation
 *  Memory allocation related units
 *  @ingroup bmagic
 *
 *  @{
 */


// /*!
//   @brief Default malloc based bitblock allocator class.

//   Functions allocate and deallocate conform to STL allocator specs.
//   @ingroup alloc
// */
// class block_allocator
// {
// public:
//     /**
//     The member function allocates storage for an array of n FerrisBitMagic::word_t
//     elements, by calling malloc.
//     @return pointer to the allocated memory.
//     */
//     static FerrisBitMagic::word_t* allocate(size_t n, const void *)
//     {
// #ifdef BMSSE2OPT
// # ifdef _MSC_VER
//         return (FerrisBitMagic::word_t*) ::_aligned_malloc(n * sizeof(FerrisBitMagic::word_t), 16);
// #else
//         return (FerrisBitMagic::word_t*) ::_mm_malloc(n * sizeof(FerrisBitMagic::word_t), 16);
// # endif

// #else
//         return (FerrisBitMagic::word_t*) ::malloc(n * sizeof(FerrisBitMagic::word_t));
// #endif
//     }

//     /**
//     The member function frees storage for an array of n FerrisBitMagic::word_t
//     elements, by calling free.
//     */
//     static void deallocate(FerrisBitMagic::word_t* p, size_t n)
//     {
// #ifdef BMSSE2OPT
// # ifdef _MSC_VER
//         ::_aligned_free(p);
// #else
//         ::_mm_free(p);
// # endif

// #else
//         ::free(p);
// #endif
//     }

// };



/*! 
  @brief Default malloc based bitblock allocator class.

  Functions allocate and deallocate conform to STL allocator specs.
  @ingroup alloc
*/
class block_allocator
{
    static int getSAMaxSz()
        {
            return 256;
        }
    static FerrisLoki::FerrisSmallObjAllocator& getAlloc()
        {
            static FerrisLoki::FerrisSmallObjAllocator *ret = 0;
            if( !ret )
            {
                ret = new FerrisLoki::FerrisSmallObjAllocator( getSAMaxSz() );
            }
            return *ret;
        }
    
    
public:
    
    /**
    The member function allocates storage for an array of n FerrisBitMagic::word_t 
    elements, by calling malloc. 
    @return pointer to the allocated memory. 
    */
    static FerrisBitMagic::word_t* allocate(size_t n, const void *)
    {
        if( n < getSAMaxSz() )
        {
            return (FerrisBitMagic::word_t*)getAlloc().Allocate( n * sizeof(FerrisBitMagic::word_t) );
        }
        
//        std::cerr << "allocate() n:" << n << std::endl;        
        return (FerrisBitMagic::word_t*) ::malloc(n * sizeof(FerrisBitMagic::word_t));
    }
    /**
    The member function frees storage for an array of n FerrisBitMagic::word_t 
    elements, by calling free. 
    */
    static void deallocate(FerrisBitMagic::word_t* p, size_t n)
    {
        if( n < getSAMaxSz() )
        {
            return getAlloc().Deallocate( p, n * sizeof(FerrisBitMagic::word_t) );
        }
        
//        std::cerr << "deallocate() n:" << n << std::endl;        
        ::free(p);
    }
};
    

/*! 
  @brief Default malloc based bitblock allocator class.

  Functions allocate and deallocate conform to STL allocator specs.
  @ingroup alloc
*/
class my_block_allocator
{
public:
    /**
    The member function allocates storage for an array of n FerrisBitMagic::word_t 
    elements, by calling malloc. 
    @return pointer to the allocated memory. 
    */
    static FerrisBitMagic::word_t* allocate(size_t n, const void *)
    {
//        std::cerr << "allocate() n:" << n << std::endl;
#ifdef BMSSE2OPT
# ifdef _MSC_VER
        return (FerrisBitMagic::word_t*) ::_aligned_malloc(n * sizeof(FerrisBitMagic::word_t), 16);
#else
        return (FerrisBitMagic::word_t*) ::_mm_malloc(n * sizeof(FerrisBitMagic::word_t), 16);
# endif
#else
        return (FerrisBitMagic::word_t*) ::malloc(n * sizeof(FerrisBitMagic::word_t));
#endif
    }
    /**
    The member function frees storage for an array of n FerrisBitMagic::word_t 
    elements, by calling free. 
    */
    static void deallocate(FerrisBitMagic::word_t* p, size_t)
    {
#ifdef BMSSE2OPT
# ifdef _MSC_VER
        ::_aligned_free(p);
#else
        ::_mm_free(p);
# endif
#else
        ::free(p);
#endif
    }
};
    
// -------------------------------------------------------------------------

/*! @brief Default malloc based bitblock allocator class.

  Functions allocate and deallocate conform to STL allocator specs.
*/
// class ptr_allocator
// {
// public:
//     /**
//     The member function allocates storage for an array of n void* 
//     elements, by calling malloc. 
//     @return pointer to the allocated memory. 
//     */
//     static void* allocate(size_t n, const void *)
//     {
//         return ::malloc(n * sizeof(void*));
//     }

//     /**
//     The member function frees storage for an array of n FerrisBitMagic::word_t 
//     elements, by calling free. 
//     */
//     static void deallocate(void* p, size_t)
//     {
//         ::free(p);
//     }
// };


class ptr_allocator
{
    static int getSAMaxSz()
        {
            return 256;
        }
    static FerrisLoki::FerrisSmallObjAllocator& getAlloc()
        {
//             static Loki::SmallObjAllocator ret( 2*4096, getSAMaxSz() );
//             return ret;
            static FerrisLoki::FerrisSmallObjAllocator* ret = 0;
            if( !ret )
            {
                ret = new FerrisLoki::FerrisSmallObjAllocator( getSAMaxSz() );
            }
            return *ret;
        }
    
public:
    static void* allocate(size_t n, const void *)
    {
        if( n < getSAMaxSz() )
        {
            return (FerrisBitMagic::word_t*)getAlloc().Allocate( n * sizeof(void*) );
        }
        
        return ::malloc(n * sizeof(void*));
    }

    static void deallocate(void* p, size_t n)
    {
        if( n < getSAMaxSz() )
        {
            return getAlloc().Deallocate( p, n * sizeof(void*) );
        }
        
        ::free(p);
    }
};
    
    
// -------------------------------------------------------------------------

/*! @brief BM style allocator adapter. 

  Template takes two parameters BA - allocator object for bit blocks and
  PA - allocator object for pointer blocks.
*/
template<class BA, class PA> class mem_alloc
{
public:
    typedef BA  block_allocator_type;
    typedef PA  ptr_allocator_type;

public:

    mem_alloc(const BA& block_alloc = BA(), const PA& ptr_alloc = PA())
    : block_alloc_(block_alloc),
      ptr_alloc_(ptr_alloc)
    {}
    
    /*! @brief Returns copy of the block allocator object
    */
    block_allocator_type get_block_allocator() const 
    { 
        return BA(block_alloc_); 
    }

    /*! @brief Returns copy of the ptr allocator object
    */
    ptr_allocator_type get_ptr_allocator() const 
    { 
       return PA(block_alloc_); 
    }


    /*! @brief Allocates and returns bit block.
    */
    FerrisBitMagic::word_t* alloc_bit_block()
    {
        return block_alloc_.allocate(FerrisBitMagic::set_block_size, 0);
    }

    /*! @brief Frees bit block allocated by alloc_bit_block.
    */
    void free_bit_block(FerrisBitMagic::word_t* block)
    {
        if (IS_VALID_ADDR(block)) 
            block_alloc_.deallocate(block, FerrisBitMagic::set_block_size);
    }

    /*! @brief Allocates GAP block using bit block allocator (BA).

        GAP blocks in BM library belong to levels. Each level has a 
        correspondent length described in FerrisBitMagic::gap_len_table<>.
        
        @param level GAP block level.
    */
    FerrisBitMagic::gap_word_t* alloc_gap_block(unsigned level, 
                                    const gap_word_t* glevel_len)
    {
        BM_ASSERT(level < FerrisBitMagic::gap_levels);
        unsigned len = 
            glevel_len[level] / (sizeof(FerrisBitMagic::word_t) / sizeof(gap_word_t));

        return (FerrisBitMagic::gap_word_t*)block_alloc_.allocate(len, 0);
    }

    /*! @brief Frees GAP block using bot block allocator (BA)
    */
    void free_gap_block(FerrisBitMagic::gap_word_t*   block,
                        const gap_word_t* glevel_len)
    {
        BM_ASSERT(IS_VALID_ADDR((FerrisBitMagic::word_t*)block));
         
        unsigned len = gap_capacity(block, glevel_len);
        len /= sizeof(FerrisBitMagic::word_t) / sizeof(FerrisBitMagic::gap_word_t);
        block_alloc_.deallocate((FerrisBitMagic::word_t*)block, len);        
    }

    /*! @brief Allocates block of pointers.
    */
    void* alloc_ptr(unsigned size = FerrisBitMagic::set_array_size)
    {
        return ptr_alloc_.allocate(size, 0);
    }

    /*! @brief Frees block of pointers.
    */
    void free_ptr(void* p, unsigned size = FerrisBitMagic::set_array_size)
    {
        ptr_alloc_.deallocate(p, size);
    }
private:
    BA            block_alloc_;
    PA            ptr_alloc_;
};

typedef mem_alloc<block_allocator, ptr_allocator> standard_allocator;

/** @} */


} // namespace FerrisBitMagic


#endif
////////////////////////////////////////////////////////////////////////////////

namespace FerrisBitMagic
{


#ifdef BMCOUNTOPT

# define BMCOUNT_INC ++count_;
# define BMCOUNT_DEC --count_;
# define BMCOUNT_VALID(x) count_is_valid_ = x;
# define BMCOUNT_SET(x) count_ = x; count_is_valid_ = true;
# define BMCOUNT_ADJ(x) if (x) ++count_; else --count_;

#else

# define BMCOUNT_INC
# define BMCOUNT_DEC
# define BMCOUNT_VALID(x)
# define BMCOUNT_SET(x)
# define BMCOUNT_ADJ(x)

#endif



typedef FerrisBitMagic::miniset<FerrisBitMagic::block_allocator, FerrisBitMagic::set_total_blocks> mem_save_set;


/** @defgroup bmagic BitMagic C++ Library
 *  For more information please visit:  http://bmagic.sourceforge.net
 *  
 */


/** @defgroup bvector The Main bvector<> Group
 *  This is the main group. It includes bvector template: front end of the bm library.
 *  @ingroup bmagic 
 */


/*!
   @brief Block allocation strategies.
   @ingroup bvector
*/
enum strategy
{
    BM_BIT = 0, //!< No GAP compression strategy. All new blocks are bit blocks.
    BM_GAP = 1  //!< GAP compression is ON.
};


/*!
   @brief bitvector with runtime compression of bits.
   @ingroup bvector
*/

template<class Alloc, class MS> 
class bvector
{
public:

    typedef Alloc  allocator_type;

    /*!
       @brief Structure with statistical information about bitset's memory 
              allocation details. 
       @ingroup bvector
    */
    struct statistics
    {
        /// Number of bit blocks.
        unsigned bit_blocks; 
        /// Number of GAP blocks.
        unsigned gap_blocks;  
        /// Estimated maximum of memory required for serialization.
        unsigned max_serialize_mem;
        /// Memory used by bitvector including temp and service blocks
        unsigned  memory_used;
        /// Array of all GAP block lengths in the bvector.
        gap_word_t   gap_length[FerrisBitMagic::set_total_blocks];
        /// GAP lengths used by bvector
        gap_word_t  gap_levels[FerrisBitMagic::gap_levels];
    };

    /**
        @brief Class reference implements an object for bit assignment.
        Since C++ does not provide with build-in bit type supporting l-value 
        operations we have to emulate it.

        @ingroup bvector
    */
    class reference
    {
    public:
        reference(bvector<Alloc, MS>& bv, FerrisBitMagic::id_t position) 
        : bv_(bv),
          position_(position)
        {}

        reference(const reference& ref)
        : bv_(ref.bv_), 
          position_(ref.position_)
        {
            bv_.set(position_, ref.bv_.get_bit(position_));
        }
        
        operator bool() const
        {
            return bv_.get_bit(position_);
        }

        const reference& operator=(const reference& ref) const
        {
            bv_.set(position_, (bool)ref);
            return *this;
        }

        const reference& operator=(bool value) const
        {
            bv_.set(position_, value);
            return *this;
        }

        bool operator==(const reference& ref) const
        {
            return bool(*this) == bool(ref);
        }

        /*! Bitwise AND. Performs operation: bit = bit AND value */
        const reference& operator&=(bool value) const
        {
            bv_.set(position_, value);
            return *this;
        }

        /*! Bitwise OR. Performs operation: bit = bit OR value */
        const reference& operator|=(bool value) const
        {
            if (value != bv_.get_bit(position_))
            {
                bv_.set_bit(position_);
            }
            return *this;
        }

        /*! Bitwise exclusive-OR (XOR). Performs operation: bit = bit XOR value */
        const reference& operator^=(bool value) const
        {
            bv_.set(position_, value != bv_.get_bit(position_));
            return *this;
        }

        /*! Logical Not operator */
        bool operator!() const
        {
            return !bv_.get_bit(position_);
        }

        /*! Bit Not operator */
        bool operator~() const
        {
            return !bv_.get_bit(position_);
        }

        /*! Negates the bit value */
        reference& flip()
        {
            bv_.flip(position_);
            return *this;
        }

    private:
        bvector<Alloc, MS>& bv_;       //!< Reference variable on the parent.
        FerrisBitMagic::id_t            position_; //!< Position in the parent bitvector.
    };

    typedef bool const_reference;

    /*!
        @brief Base class for all iterators.
        @ingroup bvector
    */
    class iterator_base
    {
    friend class bvector;
    public:
        iterator_base() : block_(0) {}

        bool operator==(const iterator_base& it) const
        {
            return (position_ == it.position_) && (bv_ == it.bv_);
        }

        bool operator!=(const iterator_base& it) const
        {
            return ! operator==(it);
        }

        bool operator < (const iterator_base& it) const
        {
            return position_ < it.position_;
        }

        bool operator <= (const iterator_base& it) const
        {
            return position_ <= it.position_;
        }

        bool operator > (const iterator_base& it) const
        {
            return position_ > it.position_;
        }

        bool operator >= (const iterator_base& it) const
        {
            return position_ >= it.position_;
        }

        /**
           \fn bool FerrisBitMagic::bvector::iterator_base::valid() const
           \brief Checks if iterator is still valid. Analog of != 0 comparison for pointers.
           \returns true if iterator is valid.
        */
        bool valid() const
        {
            return position_ != FerrisBitMagic::id_max;
        }

        /**
           \fn bool FerrisBitMagic::bvector::iterator_base::invalidate() 
           \brief Turns iterator into an invalid state.
        */
        void invalidate()
        {
            position_ = FerrisBitMagic::id_max;
        }

    public:

        /** Information about current bitblock. */
        struct bitblock_descr
        {
            const FerrisBitMagic::word_t*   ptr;      //!< Word pointer.
            unsigned            bits[32]; //!< Unpacked list of ON bits
            unsigned            idx;      //!< Current position in the bit list
            unsigned            cnt;      //!< Number of ON bits
            FerrisBitMagic::id_t            pos;      //!< Last bit position before 
        };

        /** Information about current DGAP block. */
        struct dgap_descr
        {
            const gap_word_t*   ptr;       //!< Word pointer.
            gap_word_t          gap_len;   //!< Current dgap length.
        };

    protected:
        FerrisBitMagic::bvector<Alloc, MS>* bv_;         //!< Pointer on parent bitvector
        FerrisBitMagic::id_t                position_;   //!< Bit position (bit idx)
        const FerrisBitMagic::word_t*       block_;      //!< Block pointer.(NULL-invalid)
        unsigned                block_type_; //!< Type of block. 0-Bit, 1-GAP
        unsigned                block_idx_;  //!< Block index

        /*! Block type dependent information for current block. */
        union block_descr
        {
            bitblock_descr   bit_;  //!< BitBlock related info.
            dgap_descr       gap_;  //!< DGAP block related info.
        } bdescr_;
    };

    /*!
        @brief Output iterator iterator designed to set "ON" bits based on
        their indeces.
        STL container can be converted to bvector using this iterator
        @ingroup bvector
    */
    class insert_iterator
    {
    public:
#ifndef BM_NO_STL
        typedef std::output_iterator_tag  iterator_category;
#endif
        typedef unsigned value_type;
        typedef void difference_type;
        typedef void pointer;
        typedef void reference;

        insert_iterator(bvector<Alloc, MS>& bvect)
         : bvect_(bvect)
        {}
        
        insert_iterator& operator=(FerrisBitMagic::id_t n)
        {
            bvect_.set(n);
            return *this;
        }
        
        /*! Returns *this without doing nothing */
        insert_iterator& operator*() { return *this; }
        /*! Returns *this. This iterator does not move */
        insert_iterator& operator++() { return *this; }
        /*! Returns *this. This iterator does not move */
        insert_iterator& operator++(int) { return *this; }
        
    protected:
        FerrisBitMagic::bvector<Alloc, MS>&   bvect_;
    };

    /*!
        @brief Constant input iterator designed to enumerate "ON" bits
        @ingroup bvector
    */
    class enumerator : public iterator_base
    {
    public:
#ifndef BM_NO_STL
        typedef std::input_iterator_tag  iterator_category;
#endif
        typedef unsigned   value_type;
        typedef unsigned   difference_type;
        typedef unsigned*  pointer;
        typedef unsigned&  reference;

    public:
        enumerator() : iterator_base() {}
        enumerator(const bvector<Alloc, MS>* bvect, int position)
            : iterator_base()
        { 
            this->bv_ = const_cast<bvector<Alloc, MS>*>(bvect);
            if (position == 0)
            {
                go_first();
            }
            else
            {
                this->invalidate();
            }
        }

        FerrisBitMagic::id_t operator*() const
        { 
            return this->position_; 
        }

        enumerator& operator++()
        {
            return this->go_up();
        }

        enumerator operator++(int)
        {
            enumerator tmp = *this;
            this->go_up();
            return tmp;
        }


        void go_first()
        {
            BM_ASSERT(this->bv_);

        #ifdef BMCOUNTOPT
            if (this->bv_->count_is_valid_ && 
                this->bv_->count_ == 0)
            {
                this->invalidate();
                return;
            }
        #endif

            typename FerrisBitMagic::bvector<Alloc, MS>::blocks_manager* bman = 
                                                &(this->bv_->blockman_);
            FerrisBitMagic::word_t*** blk_root = bman->blocks_root();

            this->block_idx_ = this->position_= 0;
            unsigned i, j;

            for (i = 0; i < bman->top_block_size(); ++i)
            {
                FerrisBitMagic::word_t** blk_blk = blk_root[i];

                if (blk_blk == 0) // not allocated
                {
                    this->block_idx_ += FerrisBitMagic::set_array_size;
                    this->position_ += FerrisBitMagic::bits_in_array;
                    continue;
                }


                for (j = 0; j < FerrisBitMagic::set_array_size; ++j,++(this->block_idx_))
                {
                    this->block_ = blk_blk[j];

                    if (this->block_ == 0)
                    {
                        this->position_ += bits_in_block;
                        continue;
                    }

                    if (BM_IS_GAP((*bman), this->block_, this->block_idx_))
                    {
                        this->block_type_ = 1;
                        if (search_in_gapblock())
                        {
                            return;
                        }
                    }
                    else
                    {
                        this->block_type_ = 0;
                        if (search_in_bitblock())
                        {
                            return;
                        }
                    }
            
                } // for j

            } // for i

            this->invalidate();
        }

        enumerator& go_up()
        {
            // Current block search.
            ++this->position_;
            typedef typename iterator_base::block_descr block_descr_type;
            
            block_descr_type* bdescr = &(this->bdescr_);

            switch (this->block_type_)
            {
            case 0:   //  BitBlock
                {

                // check if we can get the value from the 
                // bits cache

                unsigned idx = ++(bdescr->bit_.idx);
                if (idx < bdescr->bit_.cnt)
                {
                    this->position_ = bdescr->bit_.pos + 
                                      bdescr->bit_.bits[idx];
                    return *this; 
                }
                this->position_ += 31 - bdescr->bit_.bits[--idx];

                const FerrisBitMagic::word_t* pend = this->block_ + FerrisBitMagic::set_block_size;

                while (++(bdescr->bit_.ptr) < pend)
                {
                    FerrisBitMagic::word_t w = *(bdescr->bit_.ptr);
                    if (w)
                    {
                        bdescr->bit_.idx = 0;
                        bdescr->bit_.pos = this->position_;
                        bdescr->bit_.cnt = FerrisBitMagic::bit_list(w, bdescr->bit_.bits); 
                        this->position_ += bdescr->bit_.bits[0];
                        return *this;
                    }
                    else
                    {
                        this->position_ += 32;
                    }
                }
    
                }
                break;

            case 1:   // DGAP Block
                {
                    if (--(bdescr->gap_.gap_len))
                    {
                        return *this;
                    }

                    // next gap is "OFF" by definition.
                    if (*(bdescr->gap_.ptr) == FerrisBitMagic::gap_max_bits - 1)
                    {
                        break;
                    }
                    gap_word_t prev = *(bdescr->gap_.ptr);
                    register unsigned val = *(++(bdescr->gap_.ptr));

                    this->position_ += val - prev;
                    // next gap is now "ON"

                    if (*(bdescr->gap_.ptr) == FerrisBitMagic::gap_max_bits - 1)
                    {
                        break;
                    }
                    prev = *(bdescr->gap_.ptr);
                    val = *(++(bdescr->gap_.ptr));
                    bdescr->gap_.gap_len = val - prev;
                    return *this;  // next "ON" found;
                }

            default:
                BM_ASSERT(0);

            } // switch


            // next bit not present in the current block
            // keep looking in the next blocks.
            ++(this->block_idx_);
            unsigned i = this->block_idx_ >> FerrisBitMagic::set_array_shift;
            unsigned top_block_size = this->bv_->blockman_.top_block_size();
            for (; i < top_block_size; ++i)
            {
                FerrisBitMagic::word_t** blk_blk = this->bv_->blockman_.blocks_[i];
                if (blk_blk == 0)
                {
                    this->block_idx_ += FerrisBitMagic::set_array_size;
                    this->position_ += FerrisBitMagic::bits_in_array;
                    continue;
                }

                unsigned j = this->block_idx_ & FerrisBitMagic::set_array_mask;

                for(; j < FerrisBitMagic::set_array_size; ++j, ++(this->block_idx_))
                {
                    this->block_ = blk_blk[j];

                    if (this->block_ == 0)
                    {
                        this->position_ += FerrisBitMagic::bits_in_block;
                        continue;
                    }

                    if (BM_IS_GAP((this->bv_->blockman_), 
                                   this->block_, 
                                   this->block_idx_))
                    {
                        this->block_type_ = 1;
                        if (search_in_gapblock())
                        {
                            return *this;
                        }
                    }
                    else
                    {
                        this->block_type_ = 0;
                        if (search_in_bitblock())
                        {
                            return *this;
                        }
                    }

            
                } // for j

            } // for i


            this->invalidate();
            return *this;
        }


    private:
        bool search_in_bitblock()
        {
            BM_ASSERT(this->block_type_ == 0);
            
            typedef typename iterator_base::block_descr block_descr_type;
            block_descr_type* bdescr = &(this->bdescr_);            

            // now lets find the first bit in block.
            bdescr->bit_.ptr = this->block_;

            const word_t* ptr_end = this->block_ + FerrisBitMagic::set_block_size;

            do
            {
                register FerrisBitMagic::word_t w = *(bdescr->bit_.ptr);

                if (w)  
                {
                    bdescr->bit_.idx = 0;
                    bdescr->bit_.pos = this->position_;
                    bdescr->bit_.cnt = 
                              FerrisBitMagic::bit_list(w, bdescr->bit_.bits);
                    this->position_ += bdescr->bit_.bits[0];

                    return true;
                }
                else
                {
                    this->position_ += 32;
                }

            } 
            while (++(bdescr->bit_.ptr) < ptr_end);

            return false;
        }

        bool search_in_gapblock()
        {
            BM_ASSERT(this->block_type_ == 1);

            typedef typename iterator_base::block_descr block_descr_type;
            block_descr_type* bdescr = &(this->bdescr_);            

            bdescr->gap_.ptr = BMGAP_PTR(this->block_);
            unsigned bitval = *(bdescr->gap_.ptr) & 1;

            ++(bdescr->gap_.ptr);

            do
            {
                register unsigned val = *(bdescr->gap_.ptr);

                if (bitval)
                {
                    gap_word_t* first = BMGAP_PTR(this->block_) + 1;
                    if (bdescr->gap_.ptr == first)
                    {
                        bdescr->gap_.gap_len = val + 1;
                    }
                    else
                    {
                        bdescr->gap_.gap_len = 
                             val - *(bdescr->gap_.ptr-1);
                    }
           
                    return true;
                }
                this->position_ += val + 1;

                if (val == FerrisBitMagic::gap_max_bits - 1)
                {
                    break;
                }

                bitval ^= 1;
                ++(bdescr->gap_.ptr);

            } while (1);

            return false;
        }

    };
    
    /*!
        @brief Constant input iterator designed to enumerate "ON" bits
        counted_enumerator keeps bitcount, ie number of ON bits starting
        from the position 0 in the bit string up to the currently enumerated bit
        
        When increment operator called current position is increased by 1.
        
        @ingroup bvector
    */
    class counted_enumerator : public enumerator
    {
    public:
#ifndef BM_NO_STL
        typedef std::input_iterator_tag  iterator_category;
#endif
        counted_enumerator() : bit_count_(0){}
        
        counted_enumerator(const enumerator& en)
        : enumerator(en)
        {
            if (this->valid())
                bit_count_ = 1;
        }
        
        counted_enumerator& operator=(const enumerator& en)
        {
            enumerator* me = this;
            *me = en;
            if (this->valid())
                this->bit_count_ = 1;
            return *this;
        }
        
        counted_enumerator& operator++()
        {
            this->go_up();
            if (this->valid())
                ++(this->bit_count_);
            return *this;
        }

        counted_enumerator operator++(int)
        {
            counted_enumerator tmp(*this);
            this->go_up();
            if (this->valid())
                ++bit_count_;
            return tmp;
        }
        
        /*! @brief Number of bits ON starting from the .
        
            Method returns number of ON bits fromn the bit 0 to the current bit 
            For the first bit in bitvector it is 1, for the second 2 
        */
        FerrisBitMagic::id_t count() const { return bit_count_; }
        
    private:
        FerrisBitMagic::id_t   bit_count_;
    };

    friend class iterator_base;
    friend class enumerator;

public:

#ifdef BMCOUNTOPT
    bvector(strategy          strat = BM_BIT, 
            const gap_word_t* glevel_len=FerrisBitMagic::gap_len_table<true>::_len,
            FerrisBitMagic::id_t          max_bits = 0,
            const Alloc&      alloc = Alloc()) 
    : count_(0),
      count_is_valid_(true),
      blockman_(glevel_len, max_bits, alloc),
      new_blocks_strat_(strat)
    {}

    bvector(const FerrisBitMagic::bvector<Alloc, MS>& bvect)
     : count_(bvect.count_),
       count_is_valid_(bvect.count_is_valid_),
       blockman_(bvect.blockman_),
       new_blocks_strat_(bvect.new_blocks_strat_)
    {}

#else
    /*!
        \brief Constructs bvector class
        \param strat - operation mode strategy, 
                       BM_BIT - default strategy, bvector use plain bitset blocks,
                       (performance oriented strategy).
                       BM_GAP - memory effitent strategy, bvector allocates blocks
                       as array of intervals(gaps) and convert blocks into plain bitsets
                       only when enthropy grows.
        \param glevel_len - pointer on C-style array keeping GAP block sizes. 
                            (Put FerrisBitMagic::gap_len_table_min<true>::_len for GAP memory saving mode)
        \param max_bits - maximum number of bits addressable by bvector, 0 means "no limits" (recommended)
        \sa FerrisBitMagic::gap_len_table FerrisBitMagic::gap_len_table_min
    */
    bvector(strategy          strat      = BM_BIT,
            const gap_word_t* glevel_len = FerrisBitMagic::gap_len_table<true>::_len,
            FerrisBitMagic::id_t          max_bits   = 0,
            const Alloc&       alloc      = Alloc()) 
    : blockman_(glevel_len, max_bits, alloc),
      new_blocks_strat_(strat)
    {}

    bvector(const bvector<Alloc, MS>& bvect)
        :  blockman_(bvect.blockman_),
           new_blocks_strat_(bvect.new_blocks_strat_)
    {}

#endif

    bvector& operator = (const bvector<Alloc, MS>& bvect)
    {
        if (bvect.blockman_.top_block_size() != blockman_.top_block_size())
        {
            blockman_.set_top_block_size(bvect.blockman_.top_block_size());
        }
        else
        {
            clear(false);
        }
        bit_or(bvect);
        return *this;
    }

    reference operator[](FerrisBitMagic::id_t n)
    {
        assert(n < FerrisBitMagic::id_max);
        return reference(*this, n);
    }


    bool operator[](FerrisBitMagic::id_t n) const
    {
        assert(n < FerrisBitMagic::id_max);
        return get_bit(n);
    }

    void operator &= (const bvector<Alloc, MS>& bvect)
    {
        bit_and(bvect);
    }

    

    
    void operator ^= (const bvector<Alloc, MS>& bvect)
    {
        bit_xor(bvect);
    }

    void operator |= (const bvector<Alloc, MS>& bvect)
    {
        bit_or(bvect);
    }

    void operator -= (const bvector<Alloc, MS>& bvect)
    {
        bit_sub(bvect);
    }

    bool operator < (const bvector<Alloc, MS>& bvect) const
    {
        return compare(bvect) < 0;
    }

    bool operator <= (const bvector<Alloc, MS>& bvect) const
    {
        return compare(bvect) <= 0;
    }

    bool operator > (const bvector<Alloc, MS>& bvect) const
    {
        return compare(bvect) > 0;
    }

    bool operator >= (const bvector<Alloc, MS>& bvect) const
    {
        return compare(bvect) >= 0;
    }

    bool operator == (const bvector<Alloc, MS>& bvect) const
    {
        return compare(bvect) == 0;
    }

    bool operator != (const bvector<Alloc, MS>& bvect) const
    {
        return compare(bvect) != 0;
    }

    bvector<Alloc, MS> operator~() const
    {
        return bvector<Alloc, MS>(*this).invert();
    }
    
    Alloc get_allocator() const
    {
        return blockman_.get_allocator();
    } 


    /*!
        \brief Sets bit n if val is true, clears bit n if val is false
        \param n - index of the bit to be set
        \param val - new bit value
        \return *this
    */
    bvector<Alloc, MS>& set(FerrisBitMagic::id_t n, bool val = true)
    {
        // calculate logical block number
        unsigned nblock = unsigned(n >>  FerrisBitMagic::set_block_shift); 

        int block_type;

        FerrisBitMagic::word_t* blk = 
            blockman_.check_allocate_block(nblock, 
                                           val, 
                                           get_new_blocks_strat(), 
                                           &block_type);

        if (!blk) return *this;

        // calculate word number in block and bit
        unsigned nbit   = unsigned(n & FerrisBitMagic::set_block_mask); 

        if (block_type == 1) // gap
        {
            unsigned is_set;
            unsigned new_block_len;
            
            new_block_len = 
                gap_set_value(val, BMGAP_PTR(blk), nbit, &is_set);
            if (is_set)
            {
                BMCOUNT_ADJ(val)

                unsigned threshold = 
                FerrisBitMagic::gap_limit(BMGAP_PTR(blk), blockman_.glen());

                if (new_block_len > threshold) 
                {
                    extend_gap_block(nblock, BMGAP_PTR(blk));
                }
            }
        }
        else  // bit block
        {
            unsigned nword  = unsigned(nbit >> FerrisBitMagic::set_word_shift); 
            nbit &= FerrisBitMagic::set_word_mask;

            FerrisBitMagic::word_t* word = blk + nword;
            FerrisBitMagic::word_t  mask = (((FerrisBitMagic::word_t)1) << nbit);

            if (val)
            {
                if ( ((*word) & mask) == 0 )
                {
                    *word |= mask; // set bit
                    BMCOUNT_INC;
                }
            }
            else
            {
                if ((*word) & mask)
                {
                    *word &= ~mask; // clear bit
                    BMCOUNT_DEC;
                }
            }
        }
        
        return *this;
    }

    /*!
       \brief Sets every bit in this bitset to 1.
       \return *this
    */
    bvector<Alloc, MS>& set()
    {
        blockman_.set_all_one();
        set(FerrisBitMagic::id_max, false);
        BMCOUNT_SET(id_max);
        return *this;
    }


    /*!
        \brief Sets all bits in the specified closed interval [left,right]
        
        \param left  - interval start
        \param right - interval end (closed interval)
        \param value - value to set interval in
        
        \return *this
    */
    bvector<Alloc, MS>& set_range(FerrisBitMagic::id_t left,
                                  FerrisBitMagic::id_t right,
                                  bool     value = true)
    {
        if (right < left)
        {
            return set_range(right, left, value);
        }

        BMCOUNT_VALID(false)
        BM_SET_MMX_GUARD

        // calculate logical number of start and destination blocks
        unsigned nblock_left  = unsigned(left  >>  FerrisBitMagic::set_block_shift);
        unsigned nblock_right = unsigned(right >>  FerrisBitMagic::set_block_shift);

        FerrisBitMagic::word_t* block = blockman_.get_block(nblock_left);
        bool left_gap = BM_IS_GAP(blockman_, block, nblock_left);

        unsigned nbit_left  = unsigned(left  & FerrisBitMagic::set_block_mask); 
        unsigned nbit_right = unsigned(right & FerrisBitMagic::set_block_mask); 

        unsigned r = 
           (nblock_left == nblock_right) ? nbit_right :(FerrisBitMagic::bits_in_block-1);

        // Set bits in the starting block

        FerrisBitMagic::gap_word_t tmp_gap_blk[5] = {0,};
        gap_init_range_block(tmp_gap_blk, 
                             nbit_left, r, value, FerrisBitMagic::bits_in_block);

        combine_operation_with_block(nblock_left, 
                                     left_gap, 
                                     block,
                                     (FerrisBitMagic::word_t*) tmp_gap_blk,
                                     1,
                                     value ? BM_OR : BM_AND);

        if (nblock_left == nblock_right)  // in one block
            return *this;

        // Set (or clear) all blocks between left and right
        
        unsigned nb_to = nblock_right + (nbit_right ==(FerrisBitMagic::bits_in_block-1));
                
        if (value)
        {
            for (unsigned nb = nblock_left+1; nb < nb_to; ++nb)
            {
                block = blockman_.get_block(nb);
                if (IS_FULL_BLOCK(block)) 
                    continue;

                bool is_gap = BM_IS_GAP(blockman_, block, nb);

                blockman_.set_block(nb, FULL_BLOCK_ADDR);
                blockman_.set_block_bit(nb);
                
                if (is_gap)
                {
                    blockman_.get_allocator().free_gap_block(BMGAP_PTR(block), 
                                                             blockman_.glen());
                }
                else
                {
                    blockman_.get_allocator().free_bit_block(block);
                }
                
            } // for
        }
        else // value == 0
        {
            for (unsigned nb = nblock_left+1; nb < nb_to; ++nb)
            {
                block = blockman_.get_block(nb);
                if (block == 0)  // nothing to do
                    continue;

                blockman_.set_block(nb, 0);
                blockman_.set_block_bit(nb);

                bool is_gap = BM_IS_GAP(blockman_, block, nb);

                if (is_gap) 
                {
                    blockman_.get_allocator().free_gap_block(BMGAP_PTR(block),
                                                             blockman_.glen());
                }
                else
                {
                    blockman_.get_allocator().free_bit_block(block);
                }


            } // for
        } // if value else 

        if (nb_to > nblock_right)
            return *this;

        block = blockman_.get_block(nblock_right);
        bool right_gap = BM_IS_GAP(blockman_, block, nblock_right);

        gap_init_range_block(tmp_gap_blk, 
                             0, nbit_right, value, FerrisBitMagic::bits_in_block);

        combine_operation_with_block(nblock_right, 
                                     right_gap, 
                                     block,
                                     (FerrisBitMagic::word_t*) tmp_gap_blk,
                                     1,
                                     value ? BM_OR : BM_AND);

        return *this;
    }


    /*!
       \brief Sets bit n.
       \param n - index of the bit to be set. 
    */
    void set_bit(FerrisBitMagic::id_t n)
    {
        set(n, true);
    }
    
    /*! Function erturns insert iterator for this bitvector */
    insert_iterator inserter()
    {
        return insert_iterator(*this);
    }


    /*!
       \brief Clears bit n.
       \param n - bit's index to be cleaned.
    */
    void clear_bit(FerrisBitMagic::id_t n)
    {
        set(n, false);
    }


    /*!
       \brief Clears every bit in the bitvector.

       \param free_mem if "true" (default) bvector frees the memory,
       otherwise sets blocks to 0.
    */
    void clear(bool free_mem = false)
    {
        blockman_.set_all_zero(free_mem);
        BMCOUNT_SET(0);
    }

    /*!
       \brief Clears every bit in the bitvector.
       \return *this;
    */
    bvector<Alloc, MS>& reset()
    {
        clear();
        return *this;
    }


    /*!
       \brief Returns count of bits which are 1.
       \return Total number of bits ON. 
    */
    FerrisBitMagic::id_t count() const
    {
    #ifdef BMCOUNTOPT
        if (count_is_valid_) return count_;
    #endif
        word_t*** blk_root = blockman_.get_rootblock();
        typename blocks_manager::block_count_func func(blockman_);
        for_each_nzblock(blk_root, blockman_.top_block_size(),
                                   FerrisBitMagic::set_array_size, func);

        BMCOUNT_SET(func.count());
        return func.count();
    }

    /*! \brief Computes bitcount values for all bvector blocks
        \param arr - pointer on array of block bit counts
        \return Index of the last block counted. 
        This number +1 gives you number of arr elements initialized during the
        function call.
    */
    unsigned count_blocks(unsigned* arr) const
    {
        FerrisBitMagic::word_t*** blk_root = blockman_.get_rootblock();
        typename blocks_manager::block_count_arr_func func(blockman_, &(arr[0]));
        for_each_nzblock(blk_root, blockman_.top_block_size(), 
                                   FerrisBitMagic::set_array_size, func);
        return func.last_block();
    }

    /*!
       \brief Returns count of 1 bits in the given diapason.
       \param left - index of first bit start counting from
       \param right - index of last bit 
       \param block_count_arr - optional parameter (bitcount by bvector blocks)
              calculated by count_blocks method. Used to improve performance of
              wide range searches
       \return Total number of bits ON. 
    */
    FerrisBitMagic::id_t count_range(FerrisBitMagic::id_t left, 
                         FerrisBitMagic::id_t right, 
                         unsigned* block_count_arr=0) const
    {
        assert(left <= right);

        unsigned count = 0;

        // calculate logical number of start and destination blocks
        unsigned nblock_left  = unsigned(left  >>  FerrisBitMagic::set_block_shift);
        unsigned nblock_right = unsigned(right >>  FerrisBitMagic::set_block_shift);

        const FerrisBitMagic::word_t* block = blockman_.get_block(nblock_left);
        bool left_gap = BM_IS_GAP(blockman_, block, nblock_left);

        unsigned nbit_left  = unsigned(left  & FerrisBitMagic::set_block_mask); 
        unsigned nbit_right = unsigned(right & FerrisBitMagic::set_block_mask); 

        unsigned r = 
           (nblock_left == nblock_right) ? nbit_right : (FerrisBitMagic::bits_in_block-1);

        typename blocks_manager::block_count_func func(blockman_);

        if (block)
        {
            if ((nbit_left == 0) && (r == (FerrisBitMagic::bits_in_block-1))) // whole block
            {
                if (block_count_arr)
                {
                    count += block_count_arr[nblock_left];
                }
                else
                {
                    func(block, nblock_left);
                }
            }
            else
            {
                if (left_gap)
                {
                    count += gap_bit_count_range(BMGAP_PTR(block), 
                                                (gap_word_t)nbit_left,
                                                (gap_word_t)r);
                }
                else
                {
                    count += bit_block_calc_count_range(block, nbit_left, r);
                }
            }
        }

        if (nblock_left == nblock_right)  // in one block
        {
            return count + func.count();
        }

        for (unsigned nb = nblock_left+1; nb < nblock_right; ++nb)
        {
            block = blockman_.get_block(nb);
            if (block_count_arr)
            {
                count += block_count_arr[nb];
            }
            else 
            {
                if (block)
                    func(block, nb);
            }
        }
        count += func.count();

        block = blockman_.get_block(nblock_right);
        bool right_gap = BM_IS_GAP(blockman_, block, nblock_right);

        if (block)
        {
            if (right_gap)
            {
                count += gap_bit_count_range(BMGAP_PTR(block),
                                            (gap_word_t)0,
                                            (gap_word_t)nbit_right);
            }
            else
            {
                count += bit_block_calc_count_range(block, 0, nbit_right);
            }
        }

        return count;
    }


    FerrisBitMagic::id_t recalc_count()
    {
        BMCOUNT_VALID(false)
        return count();
    }
    
    /*!
        Disables count cache. Next call to count() or recalc_count()
        restores count caching.
        
        @note Works only if BMCOUNTOPT enabled(defined). 
        Othewise does nothing.
    */
    void forget_count()
    {
        BMCOUNT_VALID(false)    
    }

    /*!
        \brief Inverts all bits.
    */
    bvector<Alloc, MS>& invert()
    {
        BMCOUNT_VALID(false)
        BM_SET_MMX_GUARD

        FerrisBitMagic::word_t*** blk_root = blockman_.get_rootblock();
        typename blocks_manager::block_invert_func func(blockman_);    
        for_each_block(blk_root, blockman_.top_block_size(),
                                 FerrisBitMagic::set_array_size, func);
        set(FerrisBitMagic::id_max, false);
        return *this;
    }


    /*!
       \brief returns true if bit n is set and false is bit n is 0. 
       \param n - Index of the bit to check.
       \return Bit value (1 or 0)
    */
    bool get_bit(FerrisBitMagic::id_t n) const
    {    
        BM_ASSERT(n < FerrisBitMagic::id_max);

        // calculate logical block number
        unsigned nblock = unsigned(n >>  FerrisBitMagic::set_block_shift); 

        const FerrisBitMagic::word_t* block = blockman_.get_block(nblock);

        if (block)
        {
            // calculate word number in block and bit
            unsigned nbit = unsigned(n & FerrisBitMagic::set_block_mask); 
            unsigned is_set;

            if (BM_IS_GAP(blockman_, block, nblock))
            {
                is_set = gap_test(BMGAP_PTR(block), nbit);
            }
            else 
            {
                unsigned nword  = unsigned(nbit >> FerrisBitMagic::set_word_shift); 
                nbit &= FerrisBitMagic::set_word_mask;

                is_set = (block[nword] & (((FerrisBitMagic::word_t)1) << nbit));
            }
            return is_set != 0;
        }
        return false;
    }

    /*!
       \brief returns true if bit n is set and false is bit n is 0. 
       \param n - Index of the bit to check.
       \return Bit value (1 or 0)
    */
    bool test(FerrisBitMagic::id_t n) const 
    { 
        return get_bit(n); 
    }

    /*!
       \brief Returns true if any bits in this bitset are set, and otherwise returns false.
       \return true if any bit is set
    */
    bool any() const
    {
    #ifdef BMCOUNTOPT
        if (count_is_valid_ && count_) return true;
    #endif
        
        word_t*** blk_root = blockman_.get_rootblock();
        typename blocks_manager::block_any_func func(blockman_);
        return for_each_nzblock_if(blk_root, blockman_.top_block_size(),
                                             FerrisBitMagic::set_array_size, func);
    }

    /*!
        \brief Returns true if no bits are set, otherwise returns false.
    */
    bool none() const
    {
        return !any();
    }

    /*!
       \brief Flips bit n
       \return *this
    */
    bvector<Alloc, MS>& flip(FerrisBitMagic::id_t n) 
    {
        set(n, !get_bit(n));
        return *this;
    }

    /*!
       \brief Flips all bits
       \return *this
    */
    bvector<Alloc, MS>& flip() 
    {
        return invert();
    }

    /*! \brief Exchanges content of bv and this bitvector.
    */
    void swap(bvector<Alloc, MS>& bv)
    {
        blockman_.swap(bv.blockman_);
#ifdef BMCOUNTOPT
        BMCOUNT_VALID(false)
        bv.recalc_count();
#endif
    }


    /*!
       \fn FerrisBitMagic::id_t bvector::get_first() const
       \brief Gets number of first bit which is ON.
       \return Index of the first 1 bit.
       \sa get_next
    */
    FerrisBitMagic::id_t get_first() const { return check_or_next(0); }

    /*!
       \fn FerrisBitMagic::id_t bvector::get_next(FerrisBitMagic::id_t prev) const
       \brief Finds the number of the next bit ON.
       \param prev - Index of the previously found bit. 
       \return Index of the next bit which is ON or 0 if not found.
       \sa get_first
    */
    FerrisBitMagic::id_t get_next(FerrisBitMagic::id_t prev) const
    {
        return (++prev == FerrisBitMagic::id_max) ? 0 : check_or_next(prev);
    }

    /*!
       @brief Calculates bitvector statistics.

       @param st - pointer on statistics structure to be filled in. 

       Function fills statistics structure containing information about how 
       this vector uses memory and estimation of max. amount of memory 
       bvector needs to serialize itself.

       @sa statistics
    */
    void calc_stat(struct statistics* st) const;

    /*!
       \brief Logical OR operation.
       \param vect - Argument vector.
    */
    FerrisBitMagic::bvector<Alloc, MS>& bit_or(const  FerrisBitMagic::bvector<Alloc, MS>& vect)
    {
        BMCOUNT_VALID(false);
        combine_operation(vect, BM_OR);
        return *this;
    }

    /*!
       \brief Logical AND operation.
       \param vect - Argument vector.
    */
    FerrisBitMagic::bvector<Alloc, MS>& bit_and(const FerrisBitMagic::bvector<Alloc, MS>& vect)
    {
        BMCOUNT_VALID(false);
        combine_operation(vect, BM_AND);
        return *this;
    }

    /*!
       \brief Logical XOR operation.
       \param vect - Argument vector.
    */
    FerrisBitMagic::bvector<Alloc, MS>& bit_xor(const FerrisBitMagic::bvector<Alloc, MS>& vect)
    {
        BMCOUNT_VALID(false);
        combine_operation(vect, BM_XOR);
        return *this;
    }

    /*!
       \brief Logical SUB operation.
       \param vect - Argument vector.
    */
    FerrisBitMagic::bvector<Alloc, MS>& bit_sub(const FerrisBitMagic::bvector<Alloc, MS>& vect)
    {
        BMCOUNT_VALID(false);
        combine_operation(vect, BM_SUB);
        return *this;
    }


    /*!
       \brief Sets new blocks allocation strategy.
       \param strat - Strategy code 0 - bitblocks allocation only.
                      1 - Blocks mutation mode (adaptive algorithm)
    */
    void set_new_blocks_strat(strategy strat) 
    { 
        new_blocks_strat_ = strat; 
    }

    /*!
       \brief Returns blocks allocation strategy.
       \return - Strategy code 0 - bitblocks allocation only.
                 1 - Blocks mutation mode (adaptive algorithm)
       \sa set_new_blocks_strat
    */
    strategy  get_new_blocks_strat() const 
    { 
        return new_blocks_strat_; 
    }

    void stat(unsigned blocks=0) const;

    /*! 
        \brief Optimization mode
        Every next level means additional checks (better compression vs time)
        \sa optimize
    */
    enum optmode
    {
        opt_free_0    = 1, ///< Free unused 0 blocks
        opt_free_01   = 2, ///< Free unused 0 and 1 blocks
        opt_compress  = 3  ///< compress blocks when possible
    };

    /*!
       \brief Optimize memory bitvector's memory allocation.
   
       Function analyze all blocks in the bitvector, compresses blocks 
       with a regular structure, frees some memory. This function is recommended
       after a bulk modification of the bitvector using set_bit, clear_bit or
       logical operations.
       
       @sa optmode, optimize_gap_size
    */
    void optimize(FerrisBitMagic::word_t* temp_block=0, optmode opt_mode = opt_compress)
    {
        word_t*** blk_root = blockman_.blocks_root();

        if (!temp_block)
            temp_block = blockman_.check_allocate_tempblock();

        typename 
          blocks_manager::block_opt_func  opt_func(blockman_, 
                                                   temp_block, 
                                                   (int)opt_mode);
        for_each_nzblock(blk_root, blockman_.top_block_size(),
                                   FerrisBitMagic::set_array_size, opt_func);
    }

    
    void optimize_gap_size()
    {
        struct bvector<Alloc, MS>::statistics st;
        calc_stat(&st);

        if (!st.gap_blocks)
            return;

        gap_word_t opt_glen[FerrisBitMagic::gap_levels];
        ::memcpy(opt_glen, st.gap_levels, FerrisBitMagic::gap_levels * sizeof(*opt_glen));

        improve_gap_levels(st.gap_length, 
                                st.gap_length + st.gap_blocks, 
                                opt_glen);
        
        set_gap_levels(opt_glen);
    }


    /*!
        @brief Sets new GAP lengths table. All GAP blocks will be reallocated 
        to match the new scheme.

        @param glevel_len - pointer on C-style array keeping GAP block sizes. 
    */
    void set_gap_levels(const gap_word_t* glevel_len)
    {
        word_t*** blk_root = blockman_.blocks_root();

        typename 
            blocks_manager::gap_level_func  gl_func(blockman_, glevel_len);

        for_each_nzblock(blk_root, blockman_.top_block_size(),
                                   FerrisBitMagic::set_array_size, gl_func);

        blockman_.set_glen(glevel_len);
    }

    /*!
        \brief Lexicographical comparison with a bitvector.

        Function compares current bitvector with the provided argument 
        bit by bit and returns -1 if our bitvector less than the argument, 
        1 - greater, 0 - equal.
    */
    int compare(const bvector<Alloc, MS>& bvect) const
    {
        int res;
        unsigned bn = 0;

        for (unsigned i = 0; i < blockman_.top_block_size(); ++i)
        {
            const FerrisBitMagic::word_t* const* blk_blk = blockman_.get_topblock(i);
            const FerrisBitMagic::word_t* const* arg_blk_blk = 
                                   bvect.blockman_.get_topblock(i);

            if (blk_blk == arg_blk_blk) 
            {
                bn += FerrisBitMagic::set_array_size;
                continue;
            }

            for (unsigned j = 0; j < FerrisBitMagic::set_array_size; ++j, ++bn)
            {
                const FerrisBitMagic::word_t* arg_blk = 
                                    arg_blk_blk ? arg_blk_blk[j] : 0;
                const FerrisBitMagic::word_t* blk = blk_blk ? blk_blk[j] : 0;

                if (blk == arg_blk) continue;

                // If one block is zero we check if the other one has at least 
                // one bit ON

                if (!blk || !arg_blk)  
                {
                    const FerrisBitMagic::word_t*  pblk;
                    bool is_gap;

                    if (blk)
                    {
                        pblk = blk;
                        res = 1;
                        is_gap = BM_IS_GAP((*this), blk, bn);
                    }
                    else
                    {
                        pblk = arg_blk;
                        res = -1;
                        is_gap = BM_IS_GAP(bvect, arg_blk, bn);
                    }

                    if (is_gap)
                    {
                        if (!gap_is_all_zero(BMGAP_PTR(pblk), FerrisBitMagic::gap_max_bits))
                        {
                            return res;
                        }
                    }
                    else
                    {
                        FerrisBitMagic::wordop_t* blk1 = (wordop_t*)pblk;
                        FerrisBitMagic::wordop_t* blk2 = 
                            (wordop_t*)(pblk + FerrisBitMagic::set_block_size);
                        if (!bit_is_all_zero(blk1, blk2))
                        {
                            return res;
                        }
                    }

                    continue;
                }

                bool arg_gap = BM_IS_GAP(bvect, arg_blk, bn);
                bool gap = BM_IS_GAP((*this), blk, bn);

                if (arg_gap != gap)
                {
                    FerrisBitMagic::wordop_t temp_blk[FerrisBitMagic::set_block_size_op]; 
                    FerrisBitMagic::wordop_t* blk1;
                    FerrisBitMagic::wordop_t* blk2;

                    if (gap)
                    {
                        gap_convert_to_bitset((FerrisBitMagic::word_t*)temp_blk, 
                                              BMGAP_PTR(blk));

                        blk1 = (FerrisBitMagic::wordop_t*)temp_blk;
                        blk2 = (FerrisBitMagic::wordop_t*)arg_blk;
                    }
                    else
                    {
                        gap_convert_to_bitset((FerrisBitMagic::word_t*)temp_blk, 
                                              BMGAP_PTR(arg_blk));

                        blk1 = (FerrisBitMagic::wordop_t*)blk;
                        blk2 = (FerrisBitMagic::wordop_t*)temp_blk;

                    }                        
                    res = bitcmp(blk1, blk2, FerrisBitMagic::set_block_size_op);  

                }
                else
                {
                    if (gap)
                    {
                        res = gapcmp(BMGAP_PTR(blk), BMGAP_PTR(arg_blk));
                    }
                    else
                    {
                        res = bitcmp((FerrisBitMagic::wordop_t*)blk, 
                                     (FerrisBitMagic::wordop_t*)arg_blk, 
                                      FerrisBitMagic::set_block_size_op);
                    }
                }

                if (res != 0)
                {
                    return res;
                }
            

            } // for j

        } // for i

        return 0;
    }


    /*! @brief Allocates temporary block of memory. 

        Temp block can be passed to bvector functions requiring some temp memory
        for their operation. (like serialize)
        
        @note method is marked const, but it's not quite true, since
        it can in some cases modify the state of the block allocator
        (if it has a state). (Can be important in MT programs).

        @sa free_tempblock
    */
    FerrisBitMagic::word_t* allocate_tempblock() const
    {
        blocks_manager* bm = const_cast<blocks_manager*>(&blockman_);
        return bm->get_allocator().alloc_bit_block();
    }

    /*! @brief Frees temporary block of memory. 

        @note method is marked const, but it's not quite true, since
        it can in some cases modify the state of the block allocator
        (if it has a state). (Can be important in MT programs).

        @sa allocate_tempblock
    */
    void free_tempblock(FerrisBitMagic::word_t* block) const
    {
        blocks_manager* bm = const_cast<blocks_manager*>(&blockman_);
        bm->get_allocator().free_bit_block(block);
    }

    /**
       \brief Returns enumerator pointing on the first non-zero bit.
    */
    enumerator first() const
    {
        typedef typename bvector<Alloc, MS>::enumerator enumerator_type;
        return enumerator_type(this, 0);
    }

    /**
       \fn bvector::enumerator bvector::end() const
       \brief Returns enumerator pointing on the next bit after the last.
    */
    enumerator end() const
    {
        typedef typename bvector<Alloc, MS>::enumerator enumerator_type;
        return enumerator_type(this, 1);
    }


    const FerrisBitMagic::word_t* get_block(unsigned nb) const 
    { 
        return blockman_.get_block(nb); 
    }
    
private:

    FerrisBitMagic::id_t check_or_next(FerrisBitMagic::id_t prev) const
    {
        for (;;)
        {
            unsigned nblock = unsigned(prev >> FerrisBitMagic::set_block_shift); 
            if (nblock >= FerrisBitMagic::set_total_blocks) break;

            if (blockman_.is_subblock_null(nblock >> FerrisBitMagic::set_array_shift))
            {
                prev += (FerrisBitMagic::set_blkblk_mask + 1) -
                              (prev & FerrisBitMagic::set_blkblk_mask);
            }
            else
            {
                unsigned nbit = unsigned(prev & FerrisBitMagic::set_block_mask);

                const FerrisBitMagic::word_t* block = blockman_.get_block(nblock);
    
                if (block)
                {
                    if (IS_FULL_BLOCK(block)) return prev;
                    if (BM_IS_GAP(blockman_, block, nblock))
                    {
                        if (gap_find_in_block(BMGAP_PTR(block), nbit, &prev))
                        {
                            return prev;
                        }
                    }
                    else
                    {
                        if (bit_find_in_block(block, nbit, &prev)) 
                        {
                            return prev;
                        }
                    }
                }
                else
                {
                    prev += (FerrisBitMagic::set_block_mask + 1) - 
                                (prev & FerrisBitMagic::set_block_mask);
                }

            }
            if (!prev) break;
        }

        return 0;
    }
    

    void combine_operation(const FerrisBitMagic::bvector<Alloc, MS>& bvect, 
                           FerrisBitMagic::operation                 opcode)
    {
        typedef void (*block_bit_op)(FerrisBitMagic::word_t*, const FerrisBitMagic::word_t*);
        typedef void (*block_bit_op_next)(FerrisBitMagic::word_t*, 
                                          const FerrisBitMagic::word_t*, 
                                          FerrisBitMagic::word_t*, 
                                          const FerrisBitMagic::word_t*);
        
        block_bit_op      bit_func;
        switch (opcode)
        {
        case BM_AND:
            bit_func = bit_block_and;
            break;
        case BM_OR:
            bit_func = bit_block_or;
            break;
        case BM_SUB:
            bit_func = bit_block_sub;
            break;
        case BM_XOR:
            bit_func = bit_block_xor;
            break;
        }           
       
        
        FerrisBitMagic::word_t*** blk_root = blockman_.blocks_root();
        unsigned block_idx = 0;
        unsigned i, j;

        BM_SET_MMX_GUARD

        for (i = 0; i < blockman_.top_block_size(); ++i)
        {
            FerrisBitMagic::word_t** blk_blk = blk_root[i];

            if (blk_blk == 0) // not allocated
            {
                const FerrisBitMagic::word_t* const* bvbb = 
                                bvect.blockman_.get_topblock(i);
                if (bvbb == 0) 
                {
                    block_idx += FerrisBitMagic::set_array_size;
                    continue;
                }

                for (j = 0; j < FerrisBitMagic::set_array_size; ++j,++block_idx)
                {
                    const FerrisBitMagic::word_t* arg_blk = bvect.blockman_.get_block(i, j);

                    if (arg_blk != 0)
                    {
                       bool arg_gap = BM_IS_GAP(bvect.blockman_, arg_blk, block_idx);
                       
                       combine_operation_with_block(block_idx, 0, 0, 
                                                    arg_blk, arg_gap, 
                                                    opcode);
                    }

                } // for k
                continue;
            }

            for (j = 0; j < FerrisBitMagic::set_array_size; ++j, ++block_idx)
            {
                
                FerrisBitMagic::word_t* blk = blk_blk[j];
                const FerrisBitMagic::word_t* arg_blk = bvect.blockman_.get_block(i, j);

                if (arg_blk || blk)
                {
                   bool arg_gap = BM_IS_GAP(bvect.blockman_, arg_blk, block_idx);
                   bool gap = BM_IS_GAP((*this).blockman_, blk, block_idx);
                   
                   // Optimization branch. Statistically two bit blocks
                   // are bumping into each other quite frequently and
                   // this peace of code tend to be executed often and 
                   // program does not go into nested calls.
                   // But logically this branch can be eliminated without
                   // loosing any functionality.
/*                   
                   if (!gap && !arg_gap)
                   {
                       if (IS_VALID_ADDR(blk) && arg_blk)
                       {
                       
                            if (bit_func2 && (j < FerrisBitMagic::set_array_size-1))
                            {
                                FerrisBitMagic::word_t* blk2 = blk_blk[j+1];
                                const FerrisBitMagic::word_t* arg_blk2 = bvect.get_block(i, j+1);
                                
                                bool arg_gap2 = BM_IS_GAP(bvect, arg_blk2, block_idx + 1);
                                bool gap2 = BM_IS_GAP((*this), blk2, block_idx + 1);
                               
                               if (!gap2 && !arg_gap2 && blk2 && arg_blk2)
                               {
                                    bit_func2(blk, arg_blk, blk2, arg_blk2);
                                    ++j;
                                    ++block_idx;
                                    continue;
                               }
                                
                            }
                            
                            bit_func(blk, arg_blk);
                            continue;
                       }
                   } // end of optimization branch...
*/                   
                   combine_operation_with_block(block_idx, gap, blk, 
                                                arg_blk, arg_gap,
                                                opcode);
                }

            } // for j

        } // for i

    }

    void combine_operation_with_block(unsigned nb,
                                      unsigned gap,
                                      FerrisBitMagic::word_t* blk,
                                      const FerrisBitMagic::word_t* arg_blk,
                                      int arg_gap,
                                      FerrisBitMagic::operation opcode)
    {
         if (gap) // our block GAP-type
         {
             if (arg_gap)  // both blocks GAP-type
             {
                 gap_word_t tmp_buf[FerrisBitMagic::gap_max_buff_len * 3]; // temporary result
             
                 gap_word_t* res;
                 switch (opcode)
                 {
                 case BM_AND:
                     res = gap_operation_and(BMGAP_PTR(blk), 
                                             BMGAP_PTR(arg_blk), 
                                             tmp_buf);
                     break;
                 case BM_OR:
                     res = gap_operation_or(BMGAP_PTR(blk), 
                                            BMGAP_PTR(arg_blk), 
                                            tmp_buf);
                     break;
                 case BM_SUB:
                     res = gap_operation_sub(BMGAP_PTR(blk), 
                                             BMGAP_PTR(arg_blk), 
                                             tmp_buf);
                     break;
                 case BM_XOR:
                     res = gap_operation_xor(BMGAP_PTR(blk), 
                                             BMGAP_PTR(arg_blk), 
                                             tmp_buf);
                     break;
                 default:
                     assert(0);
                     res = 0;
                 }

                 assert(res == tmp_buf);
                 unsigned res_len = FerrisBitMagic::gap_length(res);

                 assert(!(res == tmp_buf && res_len == 0));

                 // if as a result of the operation gap block turned to zero
                 // we can now replace it with NULL
                 if (gap_is_all_zero(res, FerrisBitMagic::gap_max_bits))
                 {
                     blockman_.set_block(nb, 0);
                     blockman_.set_block_bit(nb);
                     blockman_.get_allocator().free_gap_block(BMGAP_PTR(blk), 
                                                             blockman_.glen());
                     return;
                 }

                 // mutation check

                 int level = gap_level(BMGAP_PTR(blk));
                 unsigned threshold = blockman_.glen(level)-4;
                 int new_level = gap_calc_level(res_len, blockman_.glen());

                 if (new_level == -1)
                 {
                     blockman_.convert_gap2bitset(nb, res);
                     return;
                 }

                 if (res_len > threshold)
                 {
                     set_gap_level(res, new_level);
                     gap_word_t* new_blk = 
                         blockman_.allocate_gap_block(new_level, res);

                     FerrisBitMagic::word_t* p = (FerrisBitMagic::word_t*)new_blk;
                     BMSET_PTRGAP(p);

                     blockman_.set_block_ptr(nb, p);
                     blockman_.get_allocator().free_gap_block(BMGAP_PTR(blk), 
                                                             blockman_.glen());
                     return;
                 }

                 // gap opeartion result is in the temporary buffer
                 // we copy it back to the gap_block

                 set_gap_level(tmp_buf, level);
                 ::memcpy(BMGAP_PTR(blk), tmp_buf, res_len * sizeof(gap_word_t));

                 return;
             }
             else // argument is BITSET-type (own block is GAP)
             {
                 // since we can not combine blocks of mixed type
                 // we need to convert our block to bitset
                 
                 if (arg_blk == 0)  // Combining against an empty block
                 {
                    if (opcode == BM_OR  || 
                        opcode == BM_SUB || 
                        opcode == BM_XOR)
                    {
                        return; // nothing to do
                    }
                        
                    if (opcode == BM_AND) // ("Value" AND  0) == 0
                    {
                        blockman_.set_block_ptr(nb, 0);
                        blockman_.set_block_bit(nb);
                        blockman_.get_allocator().
                                       free_gap_block(BMGAP_PTR(blk),
                                                    blockman_.glen());
                        return;
                    }
                 }

                 blk = blockman_.convert_gap2bitset(nb, BMGAP_PTR(blk));
             }
         } 
         else // our block is BITSET-type
         {
             if (arg_gap) // argument block is GAP-type
             {
                if (IS_VALID_ADDR(blk))
                {
                    // special case, maybe we can do the job without 
                    // converting the GAP argument to bitblock
                    switch (opcode)
                    {
                    case BM_OR:
                         gap_add_to_bitset(blk, BMGAP_PTR(arg_blk));
                         return;                         
                    case BM_SUB:
                         gap_sub_to_bitset(blk, BMGAP_PTR(arg_blk));
                         return;
                    case BM_XOR:
                         gap_xor_to_bitset(blk, BMGAP_PTR(arg_blk));
                         return;
                    case BM_AND:
                         gap_and_to_bitset(blk, BMGAP_PTR(arg_blk));
                         return;
                         
                    } // switch
                 }
                 
                 // the worst case we need to convert argument block to 
                 // bitset type.

                 gap_word_t* temp_blk = (gap_word_t*) blockman_.check_allocate_tempblock();
                 arg_blk = 
                     gap_convert_to_bitset_smart((FerrisBitMagic::word_t*)temp_blk, 
                                                 BMGAP_PTR(arg_blk), 
                                                 FerrisBitMagic::gap_max_bits);
             
             }   
         }
     
         // Now here we combine two plain bitblocks using supplied bit function.
         FerrisBitMagic::word_t* dst = blk;

         FerrisBitMagic::word_t* ret; 
         if (dst == 0 && arg_blk == 0)
         {
             return;
         }

         switch (opcode)
         {
         case BM_AND:
             ret = bit_operation_and(dst, arg_blk);
             goto copy_block;
         case BM_XOR:
             ret = bit_operation_xor(dst, arg_blk);
             if (ret && (ret == arg_blk) && IS_FULL_BLOCK(dst))
             {
                 ret = blockman_.get_allocator().alloc_bit_block();
#ifdef BMVECTOPT
                VECT_XOR_ARR_2_MASK(ret, 
                                    arg_blk, 
                                    arg_blk + FerrisBitMagic::set_block_size, 
                                    FerrisBitMagic::all_bits_mask);
#else
                 FerrisBitMagic::wordop_t* dst_ptr = (wordop_t*)ret;
                 const FerrisBitMagic::wordop_t* wrd_ptr = (wordop_t*) arg_blk;
                 const FerrisBitMagic::wordop_t* wrd_end = 
                    (wordop_t*) (arg_blk + FerrisBitMagic::set_block_size);

                 do
                 {
                     dst_ptr[0] = FerrisBitMagic::all_bits_mask ^ wrd_ptr[0];
                     dst_ptr[1] = FerrisBitMagic::all_bits_mask ^ wrd_ptr[1];
                     dst_ptr[2] = FerrisBitMagic::all_bits_mask ^ wrd_ptr[2];
                     dst_ptr[3] = FerrisBitMagic::all_bits_mask ^ wrd_ptr[3];

                     dst_ptr+=4;
                     wrd_ptr+=4;

                 } while (wrd_ptr < wrd_end);
#endif
                 break;
             }
             goto copy_block;
         case BM_OR:
             ret = bit_operation_or(dst, arg_blk);
         copy_block:
             if (ret && (ret == arg_blk) && !IS_FULL_BLOCK(ret))
             {
                ret = blockman_.get_allocator().alloc_bit_block();
                bit_block_copy(ret, arg_blk);
             }
             break;

         case BM_SUB:
             ret = bit_operation_sub(dst, arg_blk);
             if (ret && ret == arg_blk)
             {
                 ret = blockman_.get_allocator().alloc_bit_block();
#ifdef BMVECTOPT
                 VECT_ANDNOT_ARR_2_MASK(ret, 
                                        arg_blk,
                                        arg_blk + FerrisBitMagic::set_block_size,
                                        FerrisBitMagic::all_bits_mask);
#else

                 FerrisBitMagic::wordop_t* dst_ptr = (wordop_t*)ret;
                 const FerrisBitMagic::wordop_t* wrd_ptr = (wordop_t*) arg_blk;
                 const FerrisBitMagic::wordop_t* wrd_end = 
                    (wordop_t*) (arg_blk + FerrisBitMagic::set_block_size);

                 do
                 {
                     dst_ptr[0] = FerrisBitMagic::all_bits_mask & ~wrd_ptr[0];
                     dst_ptr[1] = FerrisBitMagic::all_bits_mask & ~wrd_ptr[1];
                     dst_ptr[2] = FerrisBitMagic::all_bits_mask & ~wrd_ptr[2];
                     dst_ptr[3] = FerrisBitMagic::all_bits_mask & ~wrd_ptr[3];

                     dst_ptr+=4;
                     wrd_ptr+=4;

                 } while (wrd_ptr < wrd_end);
#endif
             }
             break;
         default:
             assert(0);
             ret = 0;
         }

         if (ret != dst) // block mutation
         {
             blockman_.set_block(nb, ret);
             blockman_.get_allocator().free_bit_block(dst);
         }
    }

public:
    void combine_operation_with_block(unsigned nb,
                                      const FerrisBitMagic::word_t* arg_blk,
                                      int arg_gap,
                                      FerrisBitMagic::operation opcode)
    {
        FerrisBitMagic::word_t* blk = const_cast<FerrisBitMagic::word_t*>(get_block(nb));
        bool gap = BM_IS_GAP((*this), blk, nb);
        combine_operation_with_block(nb, gap, blk, arg_blk, arg_gap, opcode);
    }
private:
    void combine_count_operation_with_block(unsigned nb,
                                            const FerrisBitMagic::word_t* arg_blk,
                                            int arg_gap,
                                            FerrisBitMagic::operation opcode)
    {
        const FerrisBitMagic::word_t* blk = get_block(nb);
        bool gap = BM_IS_GAP((*this), blk, nb);
        combine_count_operation_with_block(nb, gap, blk, arg_blk, arg_gap, opcode);
    }


    /**
       \brief Extends GAP block to the next level or converts it to bit block.
       \param nb - Block's linear index.
       \param blk - Blocks's pointer 
    */
    void extend_gap_block(unsigned nb, gap_word_t* blk)
    {
        blockman_.extend_gap_block(nb, blk);
    }


public:

    /** 
        Embedded class managing bit-blocks on very low level.
        Includes number of functor classes used in different bitset algorithms. 
    */
    class blocks_manager
    {
    friend class enumerator;
    friend class block_free_func;

    public:
    
        typedef Alloc allocator_type;
    
    public:

        /** Base functor class */
        class bm_func_base
        {
        public:
            bm_func_base(blocks_manager& bman) : bm_(bman) {}

        protected:
            blocks_manager&  bm_;
        };

        /** Base functor class connected for "constant" functors*/
        class bm_func_base_const
        {
        public:
            bm_func_base_const(const blocks_manager& bman) : bm_(bman) {}

        protected:
            const blocks_manager&  bm_;
        };

        /** Base class for bitcounting functors */
        class block_count_base : public bm_func_base_const
        {
        protected:
            block_count_base(const blocks_manager& bm) 
                : bm_func_base_const(bm) {}

            FerrisBitMagic::id_t block_count(const FerrisBitMagic::word_t* block, unsigned idx) const
            {
                id_t count = 0;
                if (IS_FULL_BLOCK(block))
                {
                    count = FerrisBitMagic::bits_in_block;
                }
                else
                {
                    if (BM_IS_GAP(this->bm_, block, idx))
                    {
                        count = gap_bit_count(BMGAP_PTR(block));
                    }
                    else // bitset
                    {
                        count = 
                            bit_block_calc_count(block, 
                                                 block + FerrisBitMagic::set_block_size);
                    }
                }
                return count;
            }
        };


        /** Bitcounting functor */
        class block_count_func : public block_count_base
        {
        public:
            block_count_func(const blocks_manager& bm) 
                : block_count_base(bm), count_(0) {}

            FerrisBitMagic::id_t count() const { return count_; }

            void operator()(const FerrisBitMagic::word_t* block, unsigned idx)
            {
                count_ += this->block_count(block, idx);
            }

        private:
            FerrisBitMagic::id_t count_;
        };

        /** Bitcounting functor filling the block counts array*/
        class block_count_arr_func : public block_count_base
        {
        public:
            block_count_arr_func(const blocks_manager& bm, unsigned* arr) 
                : block_count_base(bm), arr_(arr), last_idx_(0) 
            {
                arr_[0] = 0;
            }

            void operator()(const FerrisBitMagic::word_t* block, unsigned idx)
            {
                while (++last_idx_ < idx)
                {
                    arr_[last_idx_] = 0;
                }
                arr_[idx] = this->block_count(block, idx);
                last_idx_ = idx;
            }

            unsigned last_block() const { return last_idx_; }

        private:
            unsigned*  arr_;
            unsigned   last_idx_;
        };

        /** bit value change counting functor */
        class block_count_change_func : public bm_func_base_const
        {
        public:
            block_count_change_func(const blocks_manager& bm) 
                : bm_func_base_const(bm),
                  count_(0),
                  prev_block_border_bit_(0)
            {}

            FerrisBitMagic::id_t block_count(const FerrisBitMagic::word_t* block, unsigned idx)
            {
                FerrisBitMagic::id_t count = 0;
                FerrisBitMagic::id_t first_bit;
                
                if (IS_FULL_BLOCK(block) || (block == 0))
                {
                    count = 1;
                    if (idx)
                    {
                        first_bit = block ? 1 : 0;
                        count -= !(prev_block_border_bit_ ^ first_bit);
                    }
                    prev_block_border_bit_ = block ? 1 : 0;
                }
                else
                {
                    if (BM_IS_GAP(this->bm_, block, idx))
                    {
                        gap_word_t* gap_block = BMGAP_PTR(block);
                        count = gap_length(gap_block) - 1;
                        if (idx)
                        {
                            first_bit = gap_test(gap_block, 0);
                            count -= !(prev_block_border_bit_ ^ first_bit);
                        }
                            
                        prev_block_border_bit_ = 
                           gap_test(gap_block, gap_max_bits-1);
                    }
                    else // bitset
                    {
                        count = bit_block_calc_count_change(block,
                                                  block + FerrisBitMagic::set_block_size);
                        if (idx)
                        {
                            first_bit = block[0] & 1;
                            count -= !(prev_block_border_bit_ ^ first_bit);
                        }
                        prev_block_border_bit_ = 
                            block[set_block_size-1] >> ((sizeof(block[0]) * 8) - 1);
                        
                    }
                }
                return count;
            }
            
            FerrisBitMagic::id_t count() const { return count_; }

            void operator()(const FerrisBitMagic::word_t* block, unsigned idx)
            {
                count_ += block_count(block, idx);
            }

        private:
            FerrisBitMagic::id_t   count_;
            FerrisBitMagic::id_t   prev_block_border_bit_;
        };


        /** Functor detects if any bit set*/
        class block_any_func : public bm_func_base_const
        {
        public:
            block_any_func(const blocks_manager& bm) 
                : bm_func_base_const(bm) 
            {}

            bool operator()(const FerrisBitMagic::word_t* block, unsigned idx)
            {
                if (IS_FULL_BLOCK(block)) return true;

                if (BM_IS_GAP(this->bm_, block, idx)) // gap block
                {
                    if (!gap_is_all_zero(BMGAP_PTR(block), FerrisBitMagic::gap_max_bits))
                    {
                        return true;
                    }
                }
                else  // bitset
                {
                    FerrisBitMagic::wordop_t* blk1 = (wordop_t*)block;
                    FerrisBitMagic::wordop_t* blk2 = 
                        (wordop_t*)(block + FerrisBitMagic::set_block_size);
                    if (!bit_is_all_zero(blk1, blk2))
                    {
                        return true;
                    }
                }
                return false;
            }
        };

        /*! Change GAP level lengths functor */
        class gap_level_func : public bm_func_base
        {
        public:
            gap_level_func(blocks_manager& bm, const gap_word_t* glevel_len)
                : bm_func_base(bm),
                  glevel_len_(glevel_len)
            {
                BM_ASSERT(glevel_len);
            }

            void operator()(FerrisBitMagic::word_t* block, unsigned idx)
            {
                blocks_manager& bman = this->bm_;
                
                if (!BM_IS_GAP(bman, block, idx))
                    return;

                gap_word_t* gap_blk = BMGAP_PTR(block);

                // TODO: Use the same code as in the optimize functor
                if (gap_is_all_zero(gap_blk, FerrisBitMagic::gap_max_bits))
                {
                    bman.set_block_ptr(idx, 0);
                    goto free_block;
                }
                else 
                if (gap_is_all_one(gap_blk, FerrisBitMagic::gap_max_bits))
                {
                    bman.set_block_ptr(idx, FULL_BLOCK_ADDR);
                free_block:
                    bman.get_allocator().free_gap_block(gap_blk, 
                                                        bman.glen());
                    bman.set_block_bit(idx);
                    return;
                }

                unsigned len = gap_length(gap_blk);
                int level = gap_calc_level(len, glevel_len_);
                if (level == -1)
                {
                    FerrisBitMagic::word_t* blk = 
                        bman.get_allocator().alloc_bit_block();
                    bman.set_block_ptr(idx, blk);
                    bman.set_block_bit(idx);
                    gap_convert_to_bitset(blk, gap_blk);
                }
                else
                {
                    gap_word_t* gap_blk_new = 
                        bman.allocate_gap_block(level, gap_blk, glevel_len_);

                    FerrisBitMagic::word_t* p = (FerrisBitMagic::word_t*) gap_blk_new;
                    BMSET_PTRGAP(p);
                    bman.set_block_ptr(idx, p);
                }
                bman.get_allocator().free_gap_block(gap_blk, bman.glen());
            }

        private:
            const gap_word_t* glevel_len_;
        };


        /*! Bitblock optimization functor */
        class block_opt_func : public bm_func_base
        {
        public:
            block_opt_func(blocks_manager& bm, 
                           FerrisBitMagic::word_t*     temp_block,
                           int             opt_mode) 
                : bm_func_base(bm),
                  temp_block_(temp_block),
                  opt_mode_(opt_mode)
            {
                BM_ASSERT(temp_block);
            }

            void operator()(FerrisBitMagic::word_t* block, unsigned idx)
            {
                blocks_manager& bman = this->bm_;
                if (IS_FULL_BLOCK(block)) return;

                gap_word_t* gap_blk;

                if (BM_IS_GAP(bman, block, idx)) // gap block
                {
                    gap_blk = BMGAP_PTR(block);

                    if (gap_is_all_zero(gap_blk, FerrisBitMagic::gap_max_bits))
                    {
                        bman.set_block_ptr(idx, 0);
                        goto free_block;
                    }
                    else 
                    if (gap_is_all_one(gap_blk, FerrisBitMagic::gap_max_bits))
                    {
                        bman.set_block_ptr(idx, FULL_BLOCK_ADDR);
                    free_block:
                        bman.get_allocator().free_gap_block(gap_blk, 
                                                            bman.glen());
                        bman.set_block_bit(idx);
                    }
                }
                else // bit block
                {
                    if (opt_mode_ < 3) // free_01 optimization
                    {  
                        FerrisBitMagic::wordop_t* blk1 = (wordop_t*)block;
                        FerrisBitMagic::wordop_t* blk2 = 
                            (wordop_t*)(block + FerrisBitMagic::set_block_size);
                    
                        bool b = bit_is_all_zero(blk1, blk2);
                        if (b)
                        {
                            bman.get_allocator().free_bit_block(block);
                            bman.set_block_ptr(idx, 0);
                            return;
                        }
                        if (opt_mode_ == 2) // check if it is all 1 block
                        {
                            b = is_bits_one(blk1, blk2);
                            if (b) 
                            {
                                bman.get_allocator().free_bit_block(block);
                                bman.set_block_ptr(idx, FULL_BLOCK_ADDR);
                                return;
                            }
                        }
                    }
                
                    // try to compress
                
                    gap_word_t* tmp_gap_blk = (gap_word_t*)temp_block_;
                    *tmp_gap_blk = FerrisBitMagic::gap_max_level << 1;

                    unsigned threashold = bman.glen(FerrisBitMagic::gap_max_level)-4;

                    unsigned len = bit_convert_to_gap(tmp_gap_blk, 
                                                      block, 
                                                      FerrisBitMagic::gap_max_bits, 
                                                      threashold);


                    if (!len) return;
                    
                    // convertion successful
                    
                    bman.get_allocator().free_bit_block(block);

                    // check if new gap block can be eliminated

                    if (gap_is_all_zero(tmp_gap_blk, FerrisBitMagic::gap_max_bits))
                    {
                        bman.set_block_ptr(idx, 0);
                    }
                    else if (gap_is_all_one(tmp_gap_blk, FerrisBitMagic::gap_max_bits))
                    {
                        bman.set_block_ptr(idx, FULL_BLOCK_ADDR);
                    }
                    else
                    {
                        int level = FerrisBitMagic::gap_calc_level(len, bman.glen());

                        gap_blk = 
                           bman.allocate_gap_block(level, tmp_gap_blk);
                        bman.set_block_ptr(idx, (FerrisBitMagic::word_t*)gap_blk);
                        bman.set_block_gap(idx);
                    }
                    
         
                }

            }
        private:
            FerrisBitMagic::word_t*   temp_block_;
            int           opt_mode_;
        };

        /** Bitblock invert functor */
        class block_invert_func : public bm_func_base
        {
        public:
            block_invert_func(blocks_manager& bm) 
                : bm_func_base(bm) {}

            void operator()(FerrisBitMagic::word_t* block, unsigned idx)
            {
                if (!block)
                {
                    this->bm_.set_block(idx, FULL_BLOCK_ADDR);
                }
                else
                if (IS_FULL_BLOCK(block))
                {
                    this->bm_.set_block_ptr(idx, 0);
                }
                else
                {
                    if (BM_IS_GAP(this->bm_, block, idx)) // gap block
                    {
                        gap_invert(BMGAP_PTR(block));
                    }
                    else  // bit block
                    {
                        FerrisBitMagic::wordop_t* wrd_ptr = (wordop_t*) block;
                        FerrisBitMagic::wordop_t* wrd_end = 
                                (wordop_t*) (block + FerrisBitMagic::set_block_size);
                        bit_invert(wrd_ptr, wrd_end);
                    }
                }

            }
        };



    private:


        /** Set block zero functor */
        class block_zero_func : public bm_func_base
        {
        public:
            block_zero_func(blocks_manager& bm, bool free_mem) 
            : bm_func_base(bm),
              free_mem_(free_mem)
            {}

            void operator()(FerrisBitMagic::word_t* block, unsigned idx)
            {
                blocks_manager& bman = this->bm_;
                if (IS_FULL_BLOCK(block))
                {
                    bman.set_block_ptr(idx, 0);
                }
                else
                {
                    if (BM_IS_GAP(bman, block, idx))
                    {
                        gap_set_all(BMGAP_PTR(block), FerrisBitMagic::gap_max_bits, 0);
                    }
                    else  // BIT block
                    {
                        if (free_mem_)
                        {
                            bman.get_allocator().free_bit_block(block);
                            bman.set_block_ptr(idx, 0);
                        }
                        else
                        {
                            bit_block_set(block, 0);
                        }
                    }
                }
            }
        private:
            bool free_mem_; //!< If "true" frees bitblocks memsets to '0'
        };

        /** Fill block with all-one bits functor */
        class block_one_func : public bm_func_base
        {
        public:
            block_one_func(blocks_manager& bm) : bm_func_base(bm) {}

            void operator()(FerrisBitMagic::word_t* block, unsigned idx)
            {
                if (!IS_FULL_BLOCK(block))
                {
                    this->bm_.set_block_all_set(idx);
                }
            }
        };


        /** Block deallocation functor */
        class block_free_func : public bm_func_base
        {
        public:
            block_free_func(blocks_manager& bm) : bm_func_base(bm) {}

            void operator()(FerrisBitMagic::word_t* block, unsigned idx)
            {
                blocks_manager& bman = this->bm_;
                if (BM_IS_GAP(bman, block, idx)) // gap block
                {
                    bman.get_allocator().free_gap_block(BMGAP_PTR(block),
                                                        bman.glen());
                }
                else
                {
                    bman.get_allocator().free_bit_block(block);
                }
            }
        };


        /** Block copy functor */
        class block_copy_func : public bm_func_base
        {
        public:
            block_copy_func(blocks_manager&        bm_target, 
                            const blocks_manager&  bm_src) 
                : bm_func_base(bm_target), 
                  bm_src_(bm_src) 
            {}

            void operator()(FerrisBitMagic::word_t* block, unsigned idx)
            {
                bool gap = bm_src_.is_block_gap(idx);
                FerrisBitMagic::word_t* new_blk;
                
                blocks_manager& bman = this->bm_;

                if (gap)
                {
                    FerrisBitMagic::gap_word_t* gap_block = BMGAP_PTR(block); 
                    unsigned level = gap_level(gap_block);
                    new_blk = (FerrisBitMagic::word_t*)
                       bman.get_allocator().alloc_gap_block(level, 
                                                            bman.glen());
                    int len = gap_length(BMGAP_PTR(block));
                    ::memcpy(new_blk, gap_block, len * sizeof(gap_word_t));
                    BMSET_PTRGAP(new_blk);
                }
                else
                {
                    if (IS_FULL_BLOCK(block))
                    {
                        new_blk = block;
                    }
                    else
                    {
                        new_blk = bman.get_allocator().alloc_bit_block();
                        bit_block_copy(new_blk, block);
                    }
                }
                bman.set_block(idx, new_blk);
            }

        private:
            const blocks_manager&  bm_src_;
        };

    public:

        blocks_manager(const gap_word_t* glevel_len, 
                       FerrisBitMagic::id_t          max_bits,
                       const Alloc&      alloc = Alloc())
            : blocks_(0),
              temp_block_(0),
              alloc_(alloc)
        {
            ::memcpy(glevel_len_, glevel_len, sizeof(glevel_len_));
            if (!max_bits)  // working in full-range mode
            {
                top_block_size_ = FerrisBitMagic::set_array_size;
            }
            else  // limiting the working range
            {
                top_block_size_ = 
                    max_bits / (FerrisBitMagic::set_block_size * sizeof(FerrisBitMagic::word_t) * 
                                                       FerrisBitMagic::set_array_size * 8);
                if (top_block_size_ < FerrisBitMagic::set_array_size) ++top_block_size_;
            }

            // allocate first level descr. of blocks 
            blocks_ = (FerrisBitMagic::word_t***) alloc_.alloc_ptr(top_block_size_); 
            ::memset(blocks_, 0, top_block_size_ * sizeof(FerrisBitMagic::word_t**));
            volatile const char* vp = _copyright<true>::_p;
            char c = *vp;
            c = 0;
        }

        blocks_manager(const blocks_manager& blockman)
            : blocks_(0),
              top_block_size_(blockman.top_block_size_),
         #ifdef BM_DISBALE_BIT_IN_PTR
              gap_flags_(blockman.gap_flags_),
         #endif
              temp_block_(0),
              alloc_(blockman.alloc_)
        {
            ::memcpy(glevel_len_, blockman.glevel_len_, sizeof(glevel_len_));

            blocks_ = (FerrisBitMagic::word_t***)alloc_.alloc_ptr(top_block_size_);
            ::memset(blocks_, 0, top_block_size_ * sizeof(FerrisBitMagic::word_t**));

            blocks_manager* bm = 
               const_cast<blocks_manager*>(&blockman);

            word_t*** blk_root = bm->blocks_root();

            block_copy_func copy_func(*this, blockman);
            for_each_nzblock(blk_root, top_block_size_, 
                                       FerrisBitMagic::set_array_size, copy_func);
        }

        
        void free_ptr(FerrisBitMagic::word_t** ptr)
        {
            if (ptr) alloc_.free_ptr(ptr);
        }

        ~blocks_manager()
        {
            alloc_.free_bit_block(temp_block_);
            deinit_tree();
        }

        /**
           \brief Finds block in 2-level blocks array  
           \param nb - Index of block (logical linear number)
           \return block adress or NULL if not yet allocated
        */
        FerrisBitMagic::word_t* get_block(unsigned nb) const
        {
            unsigned block_idx = nb >> FerrisBitMagic::set_array_shift;
            if (block_idx >= top_block_size_) return 0;
            FerrisBitMagic::word_t** blk_blk = blocks_[block_idx];
            if (blk_blk)
            {
               return blk_blk[nb & FerrisBitMagic::set_array_mask]; // equivalent of %
            }
            return 0; // not allocated
        }

        /**
           \brief Finds block in 2-level blocks array
           \param i - top level block index
           \param j - second level block index
           \return block adress or NULL if not yet allocated
        */
        inline const FerrisBitMagic::word_t* get_block(unsigned i, unsigned j) const
        {
            if (i >= top_block_size_) return 0;
            const FerrisBitMagic::word_t* const* blk_blk = blocks_[i];
            return (blk_blk == 0) ? 0 : blk_blk[j];
        }

        /**
           \brief Function returns top-level block in 2-level blocks array
           \param i - top level block index
           \return block adress or NULL if not yet allocated
        */
        const FerrisBitMagic::word_t* const * get_topblock(unsigned i) const
        {
            if (i >= top_block_size_) return 0;
            return blocks_[i];
        }
        
        /** 
            \brief Returns root block in the tree.
        */
        FerrisBitMagic::word_t*** get_rootblock() const
        {
            blocks_manager* bm = 
               const_cast<blocks_manager*>(this);

            return bm->blocks_root();
        }

        void set_block_all_set(unsigned nb)
        {
            FerrisBitMagic::word_t* block = this->get_block(nb);
            set_block(nb, const_cast<FerrisBitMagic::word_t*>(FULL_BLOCK_ADDR));

          // If we keep block type flag in pointer itself we dp not need 
          // to clear gap bit 
          #ifdef BM_DISBALE_BIT_IN_PTR
            set_block_bit(nb);    
          #endif

            if (BM_IS_GAP((*this), block, nb))
            {
                alloc_.free_gap_block(BMGAP_PTR(block), glevel_len_);
            }
            else
            {
                alloc_.free_bit_block(block);
            }

        }


        /** 
            Function checks if block is not yet allocated, allocates it and sets to
            all-zero or all-one bits. 
    
            If content_flag == 1 (ALLSET block) requested and taken block is already ALLSET,
            function will return NULL

            initial_block_type and actual_block_type : 0 - bitset, 1 - gap
        */
        FerrisBitMagic::word_t* check_allocate_block(unsigned nb, 
                                         unsigned content_flag,
                                         int      initial_block_type,
                                         int*     actual_block_type,
                                         bool     allow_null_ret=true)
        {
            FerrisBitMagic::word_t* block = this->get_block(nb);

            if (!IS_VALID_ADDR(block)) // NULL block or ALLSET
            {
                // if we wanted ALLSET and requested block is ALLSET return NULL
                unsigned block_flag = IS_FULL_BLOCK(block);
                *actual_block_type = initial_block_type;
                if (block_flag == content_flag && allow_null_ret)
                {
                    return 0; // it means nothing to do for the caller
                }

                if (initial_block_type == 0) // bitset requested
                {
                    block = alloc_.alloc_bit_block();

                    // initialize block depending on its previous status

                    bit_block_set(block, block_flag ? 0xFF : 0);

                    set_block(nb, block);
                }
                else // gap block requested
                {
                    FerrisBitMagic::gap_word_t* gap_block = allocate_gap_block(0);
                    gap_set_all(gap_block, FerrisBitMagic::gap_max_bits, block_flag);
                    set_block(nb, (FerrisBitMagic::word_t*)gap_block);

                    set_block_gap(nb);

                    return (FerrisBitMagic::word_t*)gap_block;
                }

            }
            else // block already exists
            {
                *actual_block_type = BM_IS_GAP((*this), block, nb);
            }

            return block;
        }

        /*! @brief Fills all blocks with 0.
            @param free_mem - if true function frees the resources
        */
        void set_all_zero(bool free_mem)
        {
            block_zero_func zero_func(*this, free_mem);
            for_each_nzblock(blocks_, top_block_size_,
                                      FerrisBitMagic::set_array_size, zero_func);
        }

        /*! Replaces all blocks with ALL_ONE block.
        */
        void set_all_one()
        {
            block_one_func func(*this);
            for_each_block(blocks_, top_block_size_, 
                                    FerrisBitMagic::set_array_size, func);
        }

        /**
            Places new block into descriptors table, returns old block's address.
            Old block is not deleted.
        */
        FerrisBitMagic::word_t* set_block(unsigned nb, FerrisBitMagic::word_t* block)
        {
            FerrisBitMagic::word_t* old_block;

            register unsigned nblk_blk = nb >> FerrisBitMagic::set_array_shift;

            // If first level array not yet allocated, allocate it and
            // assign block to it
            if (blocks_[nblk_blk] == 0) 
            {
                blocks_[nblk_blk] = (FerrisBitMagic::word_t**)alloc_.alloc_ptr();
                ::memset(blocks_[nblk_blk], 0, 
                    FerrisBitMagic::set_array_size * sizeof(FerrisBitMagic::word_t*));

                old_block = 0;
            }
            else
            {
                old_block = blocks_[nblk_blk][nb & FerrisBitMagic::set_array_mask];
            }

            // NOTE: block will be replaced without freeing,
            // potential memory leak may lay here....
            blocks_[nblk_blk][nb & FerrisBitMagic::set_array_mask] = block; // equivalent to %

            return old_block;

        }
        
        
        /**
            Places new block into blocks table.
        */
        void set_block_ptr(unsigned nb, FerrisBitMagic::word_t* block)
        {
            blocks_[nb >> FerrisBitMagic::set_array_shift][nb & FerrisBitMagic::set_array_mask] = block;
        }
        

        /** 
           \brief Converts block from type gap to conventional bitset block.
           \param nb - Block's index. 
           \param gap_block - Pointer to the gap block, 
                              if NULL block nb will be taken
           \return new gap block's memory
        */
        FerrisBitMagic::word_t* convert_gap2bitset(unsigned nb, gap_word_t* gap_block=0)
        {
            FerrisBitMagic::word_t* block = this->get_block(nb);
            if (gap_block == 0)
            {
                gap_block = BMGAP_PTR(block);
            }

            BM_ASSERT(IS_VALID_ADDR((FerrisBitMagic::word_t*)gap_block));
            BM_ASSERT(is_block_gap(nb)); // must be GAP type

            FerrisBitMagic::word_t* new_block = alloc_.alloc_bit_block();

            gap_convert_to_bitset(new_block, gap_block);

            // new block will replace the old one(no deletion)
            set_block_ptr(nb, new_block); 

            alloc_.free_gap_block(BMGAP_PTR(block), glen());

          // If GAP flag is in block pointer no need to clean the gap bit twice
          #ifdef BM_DISBALE_BIT_IN_PTR
            set_block_bit(nb);
          #endif

            return new_block;
        }


        /**
           \brief Extends GAP block to the next level or converts it to bit block.
           \param nb - Block's linear index.
           \param blk - Blocks's pointer 
        */
        void extend_gap_block(unsigned nb, gap_word_t* blk)
        {
            unsigned level = FerrisBitMagic::gap_level(blk);
            unsigned len = FerrisBitMagic::gap_length(blk);
            if (level == FerrisBitMagic::gap_max_level || len >= gap_max_buff_len)
            {
                convert_gap2bitset(nb);
            }
            else
            {
                FerrisBitMagic::word_t* new_blk = (FerrisBitMagic::word_t*)allocate_gap_block(++level, blk);

                BMSET_PTRGAP(new_blk);

                set_block_ptr(nb, new_blk);
                alloc_.free_gap_block(blk, glen());
            }
        }

        bool is_block_gap(unsigned nb) const 
        {
         #ifdef BM_DISBALE_BIT_IN_PTR
            return gap_flags_.test(nb)!=0;
         #else
            FerrisBitMagic::word_t* block = get_block(nb);
            return BMPTR_TESTBIT0(block) != 0;
         #endif
        }

        void set_block_bit(unsigned nb) 
        { 
         #ifdef BM_DISBALE_BIT_IN_PTR
            gap_flags_.set(nb, false);
         #else
            FerrisBitMagic::word_t* block = get_block(nb);
            block = (FerrisBitMagic::word_t*) BMPTR_CLEARBIT0(block);
            set_block_ptr(nb, block);
         #endif
        }

        void set_block_gap(unsigned nb) 
        {
         #ifdef BM_DISBALE_BIT_IN_PTR
            gap_flags_.set(nb);
         #else
            FerrisBitMagic::word_t* block = get_block(nb);
            block = (FerrisBitMagic::word_t*)BMPTR_SETBIT0(block);
            set_block_ptr(nb, block);
         #endif
        }

        /**
           \fn bool FerrisBitMagic::bvector::blocks_manager::is_block_zero(unsigned nb, FerrisBitMagic::word_t* blk)
           \brief Checks all conditions and returns true if block consists of only 0 bits
           \param nb - Block's linear index.
           \param blk - Blocks's pointer 
           \returns true if all bits are in the block are 0.
        */
        bool is_block_zero(unsigned nb, const FerrisBitMagic::word_t* blk) const
        {
           if (blk == 0) return true;

           if (BM_IS_GAP((*this), blk, nb)) // GAP
           {
               gap_word_t* b = BMGAP_PTR(blk);
               return gap_is_all_zero(b, FerrisBitMagic::gap_max_bits);
           }
   
           // BIT
           for (unsigned i = 0; i <  FerrisBitMagic::set_block_size; ++i)
           {
               if (blk[i] != 0)
                  return false;
           }
           return true;
        }


        /**
           \brief Checks if block has only 1 bits
           \param nb - Index of the block.
           \param blk - Block's pointer
           \return true if block consists of 1 bits.
        */
        bool is_block_one(unsigned nb, const FerrisBitMagic::word_t* blk) const
        {
           if (blk == 0) return false;

           if (BM_IS_GAP((*this), blk, nb)) // GAP
           {
               gap_word_t* b = BMGAP_PTR(blk);
               return gap_is_all_one(b, FerrisBitMagic::gap_max_bits);
           }
   
           // BIT block

           if (IS_FULL_BLOCK(blk))
           {
              return true;
           }
           return is_bits_one((wordop_t*)blk, 
                              (wordop_t*)(blk + FerrisBitMagic::set_block_size));
        }

        /*! Returns temporary block, allocates if needed. */
        FerrisBitMagic::word_t* check_allocate_tempblock()
        {
            return temp_block_ ? temp_block_ 
                               : (temp_block_ = alloc_.alloc_bit_block());
        }

        /*! Assigns new GAP lengths vector */
        void set_glen(const gap_word_t* glevel_len)
        {
            ::memcpy(glevel_len_, glevel_len, sizeof(glevel_len_));
        }


        FerrisBitMagic::gap_word_t* allocate_gap_block(unsigned level, 
                                           gap_word_t* src = 0,
                                           const gap_word_t* glevel_len = 0)
        {
           if (!glevel_len)
               glevel_len = glevel_len_;
           gap_word_t* ptr = alloc_.alloc_gap_block(level, glevel_len);
           if (src)
           {
                unsigned len = gap_length(src);
                ::memcpy(ptr, src, len * sizeof(gap_word_t));
                // Reconstruct the mask word using the new level code.
                *ptr = ((len-1) << 3) | (level << 1) | (*src & 1);
           }
           else
           {
               *ptr = level << 1;
           }
           return ptr;
        }


        unsigned mem_used() const
        {
           unsigned mem_used = sizeof(*this);
           mem_used += temp_block_ ? sizeof(word_t) * FerrisBitMagic::set_block_size : 0;
           mem_used += sizeof(FerrisBitMagic::word_t**) * top_block_size_;

         #ifdef BM_DISBALE_BIT_IN_PTR
           mem_used += gap_flags_.mem_used() - sizeof(gap_flags_);
         #endif

           for (unsigned i = 0; i < top_block_size_; ++i)
           {
              mem_used += blocks_[i] ? sizeof(void*) * FerrisBitMagic::set_array_size : 0;
           }

           return mem_used;
        }

        /** Returns true if second level block pointer is 0.
        */
        bool is_subblock_null(unsigned nsub) const
        {
           return blocks_[nsub] == NULL;
        }


        FerrisBitMagic::word_t***  blocks_root()
        {
            return blocks_;
        }

        /*! \brief Returns current GAP level vector
        */
        const gap_word_t* glen() const
        {
            return glevel_len_;
        }

        /*! \brief Returns GAP level length for specified level
            \param level - level number
        */
        unsigned glen(unsigned level) const
        {
            return glevel_len_[level];
        }

        /*! \brief Swaps content 
            \param bm  another blocks manager
        */
        void swap(blocks_manager& bm)
        {
            word_t*** btmp = blocks_;
            blocks_ = bm.blocks_;
            bm.blocks_ = btmp;
         #ifdef BM_DISBALE_BIT_IN_PTR
            gap_flags_.swap(bm.gap_flags_);
         #endif
            gap_word_t gltmp[FerrisBitMagic::gap_levels];
            
            ::memcpy(gltmp, glevel_len_, sizeof(glevel_len_));
            ::memcpy(glevel_len_, bm.glevel_len_, sizeof(glevel_len_));
            ::memcpy(bm.glevel_len_, gltmp, sizeof(glevel_len_));
        }

        /*! \brief Returns size of the top block array in the tree 
        */
        unsigned top_block_size() const
        {
            return top_block_size_;
        }

        /*! \brief Sets ne top level block size.
        */
        void set_top_block_size(unsigned value)
        {
            assert(value);
            if (value == top_block_size_) return;

            deinit_tree();
            top_block_size_ = value;
            // allocate first level descr. of blocks 
            blocks_ = (FerrisBitMagic::word_t***)alloc_.alloc_ptr(top_block_size_); 
            ::memset(blocks_, 0, top_block_size_ * sizeof(FerrisBitMagic::word_t**));
        }
        
        /** \brief Returns reference on the allocator
        */
        allocator_type& get_allocator() { return alloc_; }

        /** \brief Returns allocator
        */
        allocator_type get_allocator() const { return alloc_; }

    private:

        void operator =(const blocks_manager&);

        void deinit_tree()
        {
            if (blocks_ == 0) return;

            block_free_func  free_func(*this);
            for_each_nzblock(blocks_, top_block_size_, 
                                      FerrisBitMagic::set_array_size, free_func);
                                      
            for(unsigned i = 0; i <  top_block_size_; ++i)
            {
                FerrisBitMagic::word_t** blk_blk = blocks_[i];
                if (blk_blk) 
                    alloc_.free_ptr(blk_blk);
            }

            alloc_.free_ptr(blocks_, top_block_size_);
        }

    private:
        /// Tree of blocks.
        FerrisBitMagic::word_t***                          blocks_;
        /// Size of the top level block array in blocks_ tree
        unsigned                               top_block_size_;
     #ifdef BM_DISBALE_BIT_IN_PTR
        /// mini bitvector used to indicate gap blocks
        MS                                     gap_flags_;
     #endif
        /// Temp block.
        FerrisBitMagic::word_t*                            temp_block_; 
        /// vector defines gap block lengths for different levels 
        gap_word_t                             glevel_len_[gap_levels];
        /// allocator
        allocator_type                         alloc_;
    };
    
    const blocks_manager& get_blocks_manager() const
    {
        return blockman_;
    }

    blocks_manager& get_blocks_manager()
    {
        return blockman_;
    }


private:

// This block defines two additional hidden variables used for bitcount
// optimization, in rare cases can make bitvector thread unsafe.
#ifdef BMCOUNTOPT
    mutable id_t      count_;            //!< number of bits "ON" in the vector
    mutable bool      count_is_valid_;   //!< actualization flag
#endif

    blocks_manager    blockman_;         //!< bitblocks manager
    strategy          new_blocks_strat_; //!< blocks allocation strategy.
};





//---------------------------------------------------------------------

template<class Alloc, class MS> 
inline bvector<Alloc, MS> operator& (const bvector<Alloc, MS>& v1,
                                     const bvector<Alloc, MS>& v2)
{
#ifdef BM_USE_EXPLICIT_TEMP
    bvector<Alloc, MS> ret(v1);
    ret.bit_and(v2);
    return ret;
#else    
    return bvector<Alloc, MS>(v1).bit_and(v2);
#endif
}

//---------------------------------------------------------------------

template<class Alloc, class MS> 
inline bvector<Alloc, MS> operator| (const bvector<Alloc, MS>& v1,
                                     const bvector<Alloc>& v2)
{
#ifdef BM_USE_EXPLICIT_TEMP
    bvector<Alloc, MS> ret(v1);
    ret.bit_or(v2);
    return ret;
#else    
    return bvector<Alloc, MS>(v1).bit_or(v2);
#endif
}

//---------------------------------------------------------------------

template<class Alloc, class MS> 
inline bvector<Alloc, MS> operator^ (const bvector<Alloc, MS>& v1,
                                     const bvector<Alloc, MS>& v2)
{
#ifdef BM_USE_EXPLICIT_TEMP
    bvector<Alloc, MS> ret(v1);
    ret.bit_xor(v2);
    return ret;
#else    
    return bvector<Alloc, MS>(v1).bit_xor(v2);
#endif
}

//---------------------------------------------------------------------

template<class Alloc, class MS> 
inline bvector<Alloc, MS> operator- (const bvector<Alloc, MS>& v1,
                                     const bvector<Alloc, MS>& v2)
{
#ifdef BM_USE_EXPLICIT_TEMP
    bvector<Alloc, MS> ret(v1);
    ret.bit_sub(v2);
    return ret;
#else    
    return bvector<Alloc, MS>(v1).bit_sub(v2);
#endif
}




// -----------------------------------------------------------------------

template<typename Alloc, typename MS> 
void bvector<Alloc, MS>::calc_stat(typename bvector<Alloc, MS>::statistics* st) const
{
    st->bit_blocks = st->gap_blocks 
                   = st->max_serialize_mem 
                   = st->memory_used = 0;

    ::memcpy(st->gap_levels, 
             blockman_.glen(), sizeof(gap_word_t) * FerrisBitMagic::gap_levels);

    unsigned empty_blocks = 0;
    unsigned blocks_memory = 0;
    gap_word_t* gapl_ptr = st->gap_length;

    st->max_serialize_mem = sizeof(id_t) * 4;

    unsigned block_idx = 0;

    // Walk the blocks, calculate statistics.
    for (unsigned i = 0; i < blockman_.top_block_size(); ++i)
    {
        const FerrisBitMagic::word_t* const* blk_blk = blockman_.get_topblock(i);

        if (!blk_blk) 
        {
            block_idx += FerrisBitMagic::set_array_size;
            st->max_serialize_mem += sizeof(unsigned) + 1;
            continue;
        }

        for (unsigned j = 0;j < FerrisBitMagic::set_array_size; ++j, ++block_idx)
        {
            const FerrisBitMagic::word_t* blk = blk_blk[j];
            if (IS_VALID_ADDR(blk))
            {
                st->max_serialize_mem += empty_blocks << 2;
                empty_blocks = 0;

                if (BM_IS_GAP(blockman_, blk, block_idx)) // gap block
                {
                    ++(st->gap_blocks);

                    FerrisBitMagic::gap_word_t* gap_blk = BMGAP_PTR(blk);

                    unsigned mem_used = 
                        FerrisBitMagic::gap_capacity(gap_blk, blockman_.glen()) 
                        * sizeof(gap_word_t);

                    *gapl_ptr = gap_length(gap_blk);

                    st->max_serialize_mem += *gapl_ptr * sizeof(gap_word_t);
                    blocks_memory += mem_used;

                    ++gapl_ptr;
                }
                else // bit block
                {
                    ++(st->bit_blocks);
                    unsigned mem_used = sizeof(FerrisBitMagic::word_t) * FerrisBitMagic::set_block_size;
                    st->max_serialize_mem += mem_used;
                    blocks_memory += mem_used;
                }
            }
            else
            {
                ++empty_blocks;
            }
        }
    }  


    st->max_serialize_mem += st->max_serialize_mem / 10; // 10% increment

    // Calc size of different odd and temporary things.

    st->memory_used += sizeof(*this) - sizeof(blockman_);
    st->memory_used += blockman_.mem_used();
    st->memory_used += blocks_memory;
}


// -----------------------------------------------------------------------



template<class Alloc, class MS> 
void bvector<Alloc, MS>::stat(unsigned blocks) const
{
    register FerrisBitMagic::id_t count = 0;
    int printed = 0;

    if (!blocks)
    {
        blocks = FerrisBitMagic::set_total_blocks;
    }

    unsigned nb;
    for (nb = 0; nb < blocks; ++nb)
    {
        register const FerrisBitMagic::word_t* blk = blockman_.get_block(nb);

        if (blk == 0)
        {
           continue;
        }

        if (IS_FULL_BLOCK(blk))
        {
           if (blockman_.is_block_gap(nb)) // gap block
           {
               printf("[Alert!%i]", nb);
               assert(0);
           }
           
           unsigned start = nb; 
           for(unsigned i = nb+1; i < FerrisBitMagic::set_total_blocks; ++i, ++nb)
           {
               blk = blockman_.get_block(nb);
               if (IS_FULL_BLOCK(blk))
               {
                 if (blockman_.is_block_gap(nb)) // gap block
                 {
                     printf("[Alert!%i]", nb);
                     assert(0);
                     --nb;
                     break;
                 }

               }
               else
               {
                  --nb;
                  break;
               }
           }


printf("{F.%i:%i}",start, nb);
            ++printed;
/*
            count += FerrisBitMagic::SET_BLOCK_MASK + 1;

            register const FerrisBitMagic::word_t* blk_end = blk + FerrisBitMagic::SET_BLOCK_SIZE;
            unsigned count2 = ::bit_block_calc_count(blk, blk_end);
            assert(count2 == FerrisBitMagic::SET_BLOCK_MASK + 1);
*/
        }
        else
        {
            if (blockman_.is_block_gap(nb)) // gap block
            {
               unsigned bc = gap_bit_count(BMGAP_PTR(blk));
               unsigned sum = gap_control_sum(BMGAP_PTR(blk));
               unsigned level = gap_level(BMGAP_PTR(blk));
                count += bc;
printf("[ GAP %i=%i:%i ]", nb, bc, level);
//printf("%i", count);
               if (sum != FerrisBitMagic::gap_max_bits)
               {
                    printf("<=*");
               }
                ++printed;
            }
            else // bitset
            {
                const FerrisBitMagic::word_t* blk_end = blk + FerrisBitMagic::set_block_size;
                unsigned bc = bit_block_calc_count(blk, blk_end);

                count += bc;
printf("( BIT %i=%i )", nb, bc);
//printf("%i", count);
                ++printed;
                
            }
        }
        if (printed == 10)
        {
            printed = 0;
            printf("\n");
        }
    } // for nb
//    printf("\nCOUNT=%i\n", count);
    printf("\n");

}

//---------------------------------------------------------------------


} // namespace


#endif






/*
Copyright(c) 2002-2005 Anatoliy Kuznetsov(anatoliy_kuznetsov at yahoo.com)

Permission is hereby granted, free of charge, to any person 
obtaining a copy of this software and associated documentation 
files (the "Software"), to deal in the Software without restriction, 
including without limitation the rights to use, copy, modify, merge, 
publish, distribute, sublicense, and/or sell copies of the Software, 
and to permit persons to whom the Software is furnished to do so, 
subject to the following conditions:

The above copyright notice and this permission notice shall be included 
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR 
OTHER DEALINGS IN THE SOFTWARE.

For more information please visit:  http://bmagic.sourceforge.net

*/

#ifndef BMSERIAL__H__INCLUDED__
#define BMSERIAL__H__INCLUDED__

/*! \defgroup bvserial bvector serialization  
 *  bvector serialization
 *  \ingroup bmagic 
 *
 */

#ifndef BM__H__INCLUDED__
#define BM__H__INCLUDED__

//#include "bm.h"

#endif

//#include "encoding.h"
//#include "bmdef.h"


// Serialization related constants


const unsigned char set_block_end   = 0;   //!< End of serialization
const unsigned char set_block_1zero = 1;   //!< One all-zero block
const unsigned char set_block_1one  = 2;   //!< One block all-set (1111...)
const unsigned char set_block_8zero = 3;   //!< Up to 256 zero blocks
const unsigned char set_block_8one  = 4;   //!< Up to 256 all-set blocks
const unsigned char set_block_16zero= 5;   //!< Up to 65536 zero blocks
const unsigned char set_block_16one = 6;   //!< UP to 65536 all-set blocks
const unsigned char set_block_32zero= 7;   //!< Up to 4G zero blocks
const unsigned char set_block_32one = 8;   //!< UP to 4G all-set blocks
const unsigned char set_block_azero = 9;   //!< All other blocks zero
const unsigned char set_block_aone  = 10;  //!< All other blocks one

const unsigned char set_block_bit     = 11;  //!< Plain bit block
const unsigned char set_block_sgapbit = 12;  //!< SGAP compressed bitblock
const unsigned char set_block_sgapgap = 13;  //!< SGAP compressed GAP block
const unsigned char set_block_gap     = 14;  //!< Plain GAP block
const unsigned char set_block_gapbit  = 15;  //!< GAP compressed bitblock 
const unsigned char set_block_arrbit  = 16;  //!< List of bits ON






#define SER_NEXT_GRP(enc, nb, B_1ZERO, B_8ZERO, B_16ZERO, B_32ZERO) \
   if (nb == 1) \
      enc.put_8(B_1ZERO); \
   else if (nb < 256) \
   { \
      enc.put_8(B_8ZERO); \
      enc.put_8((unsigned char)nb); \
   } \
   else if (nb < 65536) \
   { \
      enc.put_8(B_16ZERO); \
      enc.put_16((unsigned short)nb); \
   } \
   else \
   {\
      enc.put_8(B_32ZERO); \
      enc.put_32(nb); \
   }


#define BM_SET_ONE_BLOCKS(x) \
    {\
         unsigned end_block = i + x; \
         for (;i < end_block; ++i) \
            bman.set_block_all_set(i); \
    } \
    --i


    namespace FerrisBitMagic
{
    


/*!
   \brief Saves bitvector into memory.

   Function serializes content of the bitvector into memory.
   Serialization adaptively uses compression(variation of GAP encoding) 
   when it is benefitial. 

   \param buf - pointer on target memory area. No range checking in the
   function. It is responsibility of programmer to allocate sufficient 
   amount of memory using information from calc_stat function.

   \param temp_block - pointer on temporary memory block. Cannot be 0; 
   If you want to save memory across multiple bvectors
   allocate temporary block using allocate_tempblock and pass it to 
   serialize.
   (Of course serialize does not deallocate temp_block.)

   \return Size of serialization block.
   \sa calc_stat
*/
/*
 Serialization format:
 <pre>

 | HEADER | BLOCKS |

 Header structure:
   BYTE : Serialization type (0x1)
   BYTE : Byte order ( 0 - Big Endian, 1 - Little Endian)
   INT16: Reserved (0)
   INT16: Reserved Flags (0)

 </pre>
*/
template<class BV>
unsigned serialize(const BV& bv, unsigned char* buf, FerrisBitMagic::word_t* temp_block)
{
//    BM_ASSERT(temp_block);
    
    typedef typename BV::blocks_manager blocks_manager_type;
    const blocks_manager_type& bman = bv.get_blocks_manager();

    gap_word_t*  gap_temp_block = (gap_word_t*) temp_block;
    
    
    FerrisBitMagic::encoder enc(buf, 0);

    // Header

    ByteOrder bo = globals<true>::byte_order();
        
    enc.put_8(1);
    enc.put_8((unsigned char)bo);

    unsigned i,j;

    // keep GAP levels information
    enc.put_16(bman.glen(), FerrisBitMagic::gap_levels);
/*     
    for (i = 0; i < FerrisBitMagic::gap_levels; ++i)
    {
        enc.put_16(bman.glen()[i]);
    }
*/
    // Blocks.

    for (i = 0; i < FerrisBitMagic::set_total_blocks; ++i)
    {
        FerrisBitMagic::word_t* blk = bman.get_block(i);

        // -----------------------------------------
        // Empty or ONE block serialization

        bool flag;
        flag = bman.is_block_zero(i, blk);
        if (flag)
        {
            // Look ahead for similar blocks
            for(j = i+1; j < FerrisBitMagic::set_total_blocks; ++j)
            {
               FerrisBitMagic::word_t* blk_next = bman.get_block(j);
               if (flag != bman.is_block_zero(j, blk_next))
                   break;
            }
            if (j == FerrisBitMagic::set_total_blocks)
            {
                enc.put_8(set_block_azero);
                break; 
            }
            else
            {
               unsigned nb = j - i;
               SER_NEXT_GRP(enc, nb, set_block_1zero, 
                                     set_block_8zero, 
                                     set_block_16zero, 
                                     set_block_32zero) 
            }
            i = j - 1;
            continue;
        }
        else
        {
            flag = bman.is_block_one(i, blk);
            if (flag)
            {
                // Look ahead for similar blocks
                for(j = i+1; j < FerrisBitMagic::set_total_blocks; ++j)
                {
                   FerrisBitMagic::word_t* blk_next = bman.get_block(j);
                   if (flag != bman.is_block_one(j, blk_next))
                       break;
                }
                if (j == FerrisBitMagic::set_total_blocks)
                {
                    enc.put_8(set_block_aone);
                    break;
                }
                else
                {
                   unsigned nb = j - i;
                   SER_NEXT_GRP(enc, nb, set_block_1one, 
                                         set_block_8one, 
                                         set_block_16one, 
                                         set_block_32one) 
                }
                i = j - 1;
                continue;
            }
        }

        // ------------------------------
        // GAP serialization

        if (BM_IS_GAP(bman, blk, i))
        {
            gap_word_t* gblk = BMGAP_PTR(blk);
            unsigned len = gap_length(gblk);
            enc.put_8(set_block_gap);
            enc.put_16(gblk, len-1);
            /*
            for (unsigned k = 0; k < (len-1); ++k)
            {
                enc.put_16(gblk[k]);
            }
            */
            continue;
        }

        // -------------------------------
        // BIT BLOCK serialization

        // Try to reduce the size up to the reasonable limit.
/*
        unsigned len = FerrisBitMagic::bit_convert_to_gap(gap_temp_block, 
                                              blk, 
                                              FerrisBitMagic::GAP_MAX_BITS, 
                                              FerrisBitMagic::GAP_EQUIV_LEN-64);
*/
        gap_word_t len;

        len = bit_convert_to_arr(gap_temp_block, 
                                 blk, 
                                 FerrisBitMagic::gap_max_bits, 
                                 FerrisBitMagic::gap_equiv_len-64);

        if (len)  // reduced
        {
//            len = gap_length(gap_temp_block);
//            enc.put_8(SET_BLOCK_GAPBIT);
//            enc.memcpy(gap_temp_block, sizeof(gap_word_t) * (len - 1));
            enc.put_8(set_block_arrbit);
            if (sizeof(gap_word_t) == 2)
            {
                enc.put_16(len);
            }
            else
            {
                enc.put_32(len);
            }
            enc.put_16(gap_temp_block, len);
        }
        else
        {
            enc.put_8(set_block_bit);
            enc.put_32(blk, FerrisBitMagic::set_block_size);
            //enc.memcpy(blk, sizeof(FerrisBitMagic::word_t) * FerrisBitMagic::set_block_size);
        }

    }

    enc.put_8(set_block_end);
    return enc.size();

}

/*!
   @brief Saves bitvector into memory.
   Allocates temporary memory block for bvector.
*/

template<class BV>
unsigned serialize(BV& bv, unsigned char* buf)
{
    typename BV::blocks_manager& bman = bv.get_blocks_manager();

    return serialize(bv, buf, bman.check_allocate_tempblock());
}



/*!
    @brief Bitvector deserialization from memory.

    @param buf - pointer on memory which keeps serialized bvector
    @param temp_block - pointer on temporary block, 
            if NULL bvector allocates own.
    @return Number of bytes consumed by deserializer.

    Function desrializes bitvector from memory block containig results
    of previous serialization. Function does not remove bits 
    which are currently set. Effectively it means OR logical operation 
    between current bitset and previously serialized one.
*/
template<class BV>
unsigned deserialize(BV& bv, const unsigned char* buf, FerrisBitMagic::word_t* temp_block=0)
{
    typedef typename BV::blocks_manager blocks_manager_type;
    blocks_manager_type& bman = bv.get_blocks_manager();

    typedef typename BV::allocator_type allocator_type;

    FerrisBitMagic::wordop_t* tmp_buf = 
        temp_block ? (FerrisBitMagic::wordop_t*) temp_block 
                   : (FerrisBitMagic::wordop_t*)bman.check_allocate_tempblock();

    temp_block = (word_t*)tmp_buf;

    gap_word_t   gap_temp_block[set_block_size*2+10];


    ByteOrder bo_current = globals<true>::byte_order();
    FerrisBitMagic::decoder dec(buf);

    bv.forget_count();

    BM_SET_MMX_GUARD

    // Reading header

    // unsigned char stype =  
    dec.get_8();
    ByteOrder bo = (FerrisBitMagic::ByteOrder)dec.get_8();

    assert(bo == bo_current); // TO DO: Add Byte-Order convertions here

    unsigned i;

    gap_word_t glevels[FerrisBitMagic::gap_levels];
    // keep GAP levels information
    for (i = 0; i < FerrisBitMagic::gap_levels; ++i)
    {
        glevels[i] = dec.get_16();
    }

    // Reading blocks

    unsigned char btype;
    unsigned nb;

    for (i = 0; i < FerrisBitMagic::set_total_blocks; ++i)
    {
        // get the block type

        btype = dec.get_8();


        // In a few next blocks of code we simply ignoring all coming zero blocks.

        if (btype == set_block_azero || btype == set_block_end)
        {
            break;
        }

        if (btype == set_block_1zero)
        {
            continue;
        }

        if (btype == set_block_8zero)
        {
            nb = dec.get_8();
            i += nb-1;
            continue;
        }

        if (btype == set_block_16zero)
        {
            nb = dec.get_16();
            i += nb-1;
            continue;
        }

        if (btype == set_block_32zero)
        {
            nb = dec.get_32();
            i += nb-1;
            continue;
        }

        // Now it is turn of all-set blocks (111)

        if (btype == set_block_aone)
        {
            for (unsigned j = i; j < FerrisBitMagic::set_total_blocks; ++j)
            {
                bman.set_block_all_set(j);
            }
            break;
        }

        if (btype == set_block_1one)
        {
            bman.set_block_all_set(i);
            continue;
        }

        if (btype == set_block_8one)
        {
            BM_SET_ONE_BLOCKS(dec.get_8());
            continue;
        }

        if (btype == set_block_16one)
        {
            BM_SET_ONE_BLOCKS(dec.get_16());
            continue;
        }

        if (btype == set_block_32one)
        {
            BM_SET_ONE_BLOCKS(dec.get_32());
            continue;
        }

        FerrisBitMagic::word_t* blk = bman.get_block(i);


        if (btype == set_block_bit) 
        {
            if (blk == 0)
            {
                blk = bman.get_allocator().alloc_bit_block();
                bman.set_block(i, blk);
                dec.get_32(blk, FerrisBitMagic::set_block_size);
                //dec.memcpy(blk, sizeof(FerrisBitMagic::word_t) * FerrisBitMagic::set_block_size);
                continue;                
            }
            dec.get_32(temp_block, FerrisBitMagic::set_block_size);
            //dec.memcpy(temp_block, sizeof(FerrisBitMagic::word_t) * FerrisBitMagic::set_block_size);
            bv.combine_operation_with_block(i, 
                                            temp_block, 
                                            0, BM_OR);
            continue;
        }

        if (btype == set_block_gap || btype == set_block_gapbit)
        {
            gap_word_t gap_head = 
                sizeof(gap_word_t) == 2 ? dec.get_16() : dec.get_32();

            unsigned len = gap_length(&gap_head);
            int level = gap_calc_level(len, bman.glen());
            --len;
            if (level == -1)  // Too big to be GAP: convert to BIT block
            {
                *gap_temp_block = gap_head;
                dec.get_16(gap_temp_block+1, len - 1);
                //dec.memcpy(gap_temp_block+1, (len-1) * sizeof(gap_word_t));
                gap_temp_block[len] = gap_max_bits - 1;

                if (blk == 0)  // block does not exist yet
                {
                    blk = bman.get_allocator().alloc_bit_block();
                    bman.set_block(i, blk);
                    gap_convert_to_bitset(blk, gap_temp_block);                
                }
                else  // We have some data already here. Apply OR operation.
                {
                    gap_convert_to_bitset(temp_block, 
                                          gap_temp_block);

                    bv.combine_operation_with_block(i, 
                                                    temp_block, 
                                                    0, 
                                                    BM_OR);
                }

                continue;
            } // level == -1

            set_gap_level(&gap_head, level);

            if (blk == 0)
            {
                gap_word_t* gap_blk = 
                  bman.get_allocator().alloc_gap_block(level, bman.glen());
                gap_word_t* gap_blk_ptr = BMGAP_PTR(gap_blk);
                *gap_blk_ptr = gap_head;
                set_gap_level(gap_blk_ptr, level);
                bman.set_block(i, (FerrisBitMagic::word_t*)gap_blk);
                bman.set_block_gap(i);
                for (unsigned k = 1; k < len; ++k) 
                {
                     gap_blk[k] = dec.get_16();
                }
                gap_blk[len] = FerrisBitMagic::gap_max_bits - 1;
            }
            else
            {
                *gap_temp_block = gap_head;
                for (unsigned k = 1; k < len; ++k) 
                {
                     gap_temp_block[k] = dec.get_16();
                }
                gap_temp_block[len] = FerrisBitMagic::gap_max_bits - 1;

                bv.combine_operation_with_block(i, 
                                               (FerrisBitMagic::word_t*)gap_temp_block, 
                                                1, 
                                                BM_OR);
            }

            continue;
        }

        if (btype == set_block_arrbit)
        {
            gap_word_t len = 
                sizeof(gap_word_t) == 2 ? dec.get_16() : dec.get_32();

            // check the block type.
            if (bman.is_block_gap(i))
            {
                // Here we most probably does not want to keep
                // the block GAP since generic bitblock offers better
                // performance.
                blk = bman.convert_gap2bitset(i);
            }
            else
            {
                if (blk == 0)  // block does not exists yet
                {
                    blk = bman.get_allocator().alloc_bit_block();
                    bit_block_set(blk, 0);
                    bman.set_block(i, blk);
                }
            }

            // Get the array one by one and set the bits.
            for (unsigned k = 0; k < len; ++k)
            {
                gap_word_t bit_idx = dec.get_16();
                or_bit_block(blk, bit_idx, 1);
            }

            continue;
        }
/*
        if (btype == set_block_gapbit)
        {
            gap_word_t gap_head = 
                sizeof(gap_word_t) == 2 ? dec.get_16() : dec.get_32();

            unsigned len = gap_length(&gap_head)-1;

            *gap_temp_block = gap_head;
            dec.memcpy(gap_temp_block+1, (len-1) * sizeof(gap_word_t));
            gap_temp_block[len] = GAP_MAX_BITS - 1;

            if (blk == 0)  // block does not exists yet
            {
                blk = A::alloc_bit_block();
                blockman_.set_block(i, blk);
                gap_convert_to_bitset(blk, 
                                      gap_temp_block, 
                                      FerrisBitMagic::SET_BLOCK_SIZE);                
            }
            else  // We have some data already here. Apply OR operation.
            {
                gap_convert_to_bitset(temp_block, 
                                      gap_temp_block, 
                                      FerrisBitMagic::SET_BLOCK_SIZE);

                combine_operation_with_block(i, 
                                             temp_block, 
                                             0, 
                                             BM_OR);
            }

            continue;
        }
*/
        BM_ASSERT(0); // unknown block type


    } // for i

    return dec.size();

}




} // namespace bm

//#include "bmundef.h"

#endif




////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////
//#include "bmundef.h"

// Copyright(c) 2002-2005 Anatoliy Kuznetsov(anatoliy_kuznetsov at yahoo.com)
//
// BM library internal header
//
// Cleaning the BM related preprocessor defines to eliminate the risks of collision
// with other programs. Please let me know if something is missing.
//

#undef BMCOUNT_INC
#undef BMCOUNT_DEC
#undef BMCOUNT_VALID
#undef BMCOUNT_SET
#undef BMCOUNT_ADJ
#undef BMCOUNT_INC
#undef BMCOUNT_DEC
#undef BMCOUNT_VALID
#undef BMCOUNT_SET
#undef BMCOUNT_ADJ
#undef BMRESTRICT
#undef BMGAP_PTR
#undef BMSET_PTRGAP
#undef BM_IS_GAP
#undef BMPTR_SETBIT0
#undef BMPTR_CLEARBIT0
#undef BMPTR_TESTBIT0
#undef BM_SET_MMX_GUARD
#undef SER_NEXT_GRP
#undef BM_SET_ONE_BLOCKS
#undef DECLARE_TEMP_BLOCK
#undef BM_MM_EMPTY
#undef BM_ASSERT
#undef FULL_BLOCK_ADDR
#undef IS_VALID_ADDR
#undef IS_FULL_BLOCK
#undef IS_EMPTY_BLOCK
#undef BM_INCWORD_BITCOUNT
#undef BM_MINISET_GAPLEN
#undef BM_MINISET_ARRSIZE

#undef BMVECTOPT
#undef VECT_XOR_ARR_2_MASK
#undef VECT_ANDNOT_ARR_2_MASK
#undef VECT_INVERT_ARR
#undef VECT_AND_ARR
#undef VECT_OR_ARR
#undef VECT_SUB_ARR
#undef VECT_XOR_ARR

#undef VECT_COPY_BLOCK
#undef VECT_SET_BLOCK



/////////////////////////////////////////////////////////////////////////////////
