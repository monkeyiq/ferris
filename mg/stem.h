/**************************************************************************
 *
 * stem.h -- Lovins' stemmer header
 * Copyright (C) 1994  Linh Huynh, 
 *                     (glued into library by tes@kbs.citri.edu.au)
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
 * $Id: stem.h,v 1.2 2008/06/07 21:30:10 ben Exp $
 *
 **************************************************************************/

/*
   $Log: stem.h,v $
   Revision 1.2  2008/06/07 21:30:10  ben
   auto backup

   Revision 1.1  2005/07/04 08:47:40  ben
   *** empty log message ***

   Revision 1.1  2003/02/02 13:50:12  monkeyiq
   Start of full text indexing

   * Revision 1.2  1994/09/20  04:20:10  tes
   * Changes made for 1.1
   *
   *
 */
#ifndef STEM_H
#define STEM_H
#ifdef GCC_HASCLASSVISIBILITY
    #pragma GCC visibility push(default)
#endif
#ifdef __cplusplus
extern "C" {
#endif

    void ferrismg_stem( unsigned char *word );



#ifdef __cplusplus
};
#endif
#ifdef GCC_HASCLASSVISIBILITY
    #pragma GCC visibility pop
#endif
#endif
