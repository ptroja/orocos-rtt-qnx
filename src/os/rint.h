/****************************************
*										*
*	Header file of rint for MSVC		*
*	Date: March 12, 2009				*
*	Author: David Yu					*
*										*
****************************************/

#ifdef _MSC_VER
#include "../rtt-config.h"
RTT_API double rint( double value );
#elif defined(__QNXNTO__)
using std::rint;
#endif
