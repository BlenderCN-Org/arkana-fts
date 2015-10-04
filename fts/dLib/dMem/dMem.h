/**
 * \file mymem.h
 * \author Pompei2
 * \date 01 May 2006
 * \brief This file contains memory manipulation functions that wrap the standard ones.
 **/

#ifndef FTS_MYMEM_H
#define FTS_MYMEM_H

#include <memory.h>

    /// Correctly frees a pointer by using MyFree.
#define SAFE_FREE( p )        { if( (p) )    { free(p); (p) = NULL; } }
    /// Correctly deletes an array of objects.
#define SAFE_DELETE_ARR( p ) { if( (p) ) { delete [] (p); (p) = NULL; } }
    /// Correctly deletes an object.
#define SAFE_DELETE( p ) { if( (p) ) { delete (p); (p) = NULL; } }

#endif /* FTS_MYMEM_H */

/* EOF */
