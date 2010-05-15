/****************************************
*										*
*	Implementation of rint for MSVC		*
*	Date: March 12, 2009				*
*	Author: David Yu					*
*										*
****************************************/
#include <cmath>
#include "os/rint.h"

#ifdef _MSC_VER
double rint( double value )
  {
	return floor( value + 0.5 );
  }
#endif
