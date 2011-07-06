/**
 * \file Math.h
 * \author Pompei2
 * \date 2 February 2010
 * \brief This file contains some mathematical utility functions.
 **/

#ifndef D_FTS_MATH_H
#define D_FTS_MATH_H

#include <algorithm>
#include <stdlib.h>

namespace FTS {

/// Get a random number between two numbers.
/** Returns a random number between \a in_iMin and \a in_iMax.
 *
 * \param in_dMin The minimum numeric value of the number.
 * \param in_dMax The maximum numeric value of the number.
 *
 * \return An valid number between \a in_dMin and \a in_dMax.
 *
 * \note both values may be negative (and real) numbers, no problem.
 *
 * \author Pompei2
 */
template<typename T>
T random(T in_dMin, T in_dMax)
{
    double min = std::min(in_dMin, in_dMax);
    double max = std::max(in_dMin, in_dMax);

    // between 0 and 1.
    double dFact = ((double)rand() / (double)RAND_MAX);

    // between 0 and max-min (for example min=-5, max=10, this is between 0 and 15)
    long double posval = (max - min)*dFact;

    // now between min and max, following the example above: between 0-5=-5 and 15-5=10
    return static_cast<T>(posval + min);
}

} // namespace FTS

#endif // D_FTS_MATH_H

 /* EOF */
