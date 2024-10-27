/************************************************************************
*
* xsync.h
* voxelands - 3d voxel world sandbox game
* Author : Pierre Brochard (pierre.brochard.1982@m4x.org)
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>
*
* License updated from GPLv2 or later to GPLv3 or later by Lisa Milne
* for Voxelands.
************************************************************************/

#ifndef XSYNC_HEADER
#define XSYNC_HEADER

#if defined(__GNUC__) || defined(__clang__)
#  define XINLINE inline
#  define XFINLINE inline __attribute__ ((always_inline))
#elif defined(__BORLANDC__) || defined(_MSC_VER) || defined(__LCC__)
#  define XINLINE __inline
#  define XFINLINE __forceinline
#elif defined(__DMC__) || defined(__POCC__) || defined(__WATCOMC__) || \
	defined(__SUNPRO_C)
#  define XINLINE inline
#  define XFINLINE inline
#else
#  define XINLINE inline
#  define XFINLINE inline
#endif

#ifndef __I_IREFERENCE_COUNTED_H_INCLUDED__
# ifdef __GNUC__
    XFINLINE static int X1SyncGet(volatile int* const pval)
    {
	    return __sync_add_and_fetch(pval,0);
    }

    XFINLINE static int X1SyncInc(volatile int* const pval)
    {
	    return __sync_fetch_and_add(pval,1);
    }

    XFINLINE static int X1SyncDec(volatile int* const pval)
    {
	    return __sync_fetch_and_sub(pval,1);
    }
# else
    XFINLINE static int X1SyncGet(volatile int* const pval)
    {
	    return *pval;
    }

    XFINLINE static int X1SyncInc(volatile int* const pval)
    {
	    return (*pval)++;
    }

    XFINLINE static int X1SyncDec(volatile int* const pval)
    {
	    return (*pval)--;
    }
# endif
#endif

#ifdef __GNUC__
    XFINLINE static int X1SyncBReset(volatile int* const pval)
    {
	    return __sync_bool_compare_and_swap(pval,1,0);
    }

    XFINLINE static int X1SyncBSet(volatile int* const pval)
    {
	    return __sync_bool_compare_and_swap(pval,0,1);
    }

    XFINLINE static int X1SyncSet(volatile int* const pval,const int newval)
    {
	    int oldval = X1SyncGet(pval);
	    int oldval2;

	    do {
		oldval2 = oldval;
	    } while((oldval = __sync_val_compare_and_swap(pval,oldval,newval)) !=
			    oldval2);
	    return oldval;
    }
#else
    XFINLINE static int X1SyncBReset(volatile int* const pval)
    {
	    const int old = *pval;

	    *pval = 0;
	    return old;
    }

    XFINLINE static int X1SyncBSet(volatile int* const pval)
    {
	    const int old = *pval;

	    *pval = 1;
	    return !old;
    }

    XFINLINE static int X1SyncSet(volatile int* const pval,const int newval)
    {
	    const int oldval = *pval;

	    *pval = newval;
	    return oldval;
    }
#endif

#endif

