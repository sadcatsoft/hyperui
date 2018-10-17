#include "stdafx.h"

namespace HyperCore
{
/*****************************************************************************/
template < class T > const TRect2D<T> TRect2D<T>::Zero(0.0, 0.0, 0.0, 0.0);
template struct TRect2D<FLOAT_TYPE>;
/*****************************************************************************/
}