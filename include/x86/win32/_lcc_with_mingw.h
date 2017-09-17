#ifndef _LCC_WITH_MINGW_H
#define _LCC_WITH_MINGW_H

#define __LCC_WITH_MINGW__
#define inline
#define __inline
#define __inline__
#define __func__	""
#define __int64		long long		/* dummy(lcc's "long long" is int32_t) */
#define __attribute__(x)
#define __declspec(x)
#define __cdecl
#define __fastcall
#define __stdcall
#define __restrict__

#define _M_IX86		1

#define _WIN32		1
//#define __midl


#endif	/* _LCC_WITH_MINGW_H */
