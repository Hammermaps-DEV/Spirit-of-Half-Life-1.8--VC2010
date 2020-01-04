#ifndef VMINMAX_H
#define VMINMAX_H

#if defined ( _MSC_VER )
#pragma once
#endif

#define Vmax(a, b)  (((a) > (b)) ? (a) : (b))
#define Vmin(a, b)  (((a) < (b)) ? (a) : (b))
#define Vfabs(x)	((x) > 0 ? (x) : 0 - (x))

#endif // VMINMAX_H