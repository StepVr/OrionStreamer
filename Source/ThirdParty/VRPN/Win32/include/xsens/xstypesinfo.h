#ifndef XSTYPESINFO_H
#define XSTYPESINFO_H

#include "xstypesconfig.h"

struct XsVersion;

#ifdef __cplusplus
extern "C"
#endif
XSTYPES_DLL_API void XsTypesInfoGetVersion(struct XsVersion* version);

#endif // file guard
