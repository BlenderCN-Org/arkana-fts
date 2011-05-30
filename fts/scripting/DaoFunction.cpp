#include "DaoFunction.h"

namespace FTS {
template<> 
int getDValue(DValue d) { return (int)d.v.i;}
template<> 
long getDValue(DValue d) { return d.v.i;}
template<> 
String getDValue(DValue d) { return DString_GetMBS(d.v.s);}
template<> 
float getDValue(DValue d) { return d.v.f;}
template<> 
double getDValue(DValue d) { return d.v.d;}

};