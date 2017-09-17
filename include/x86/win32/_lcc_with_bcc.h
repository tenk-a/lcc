#ifndef _LCC_WITH_BCC_H
#define _LCC_WITH_BCC_H

#define __LCC_WITH_BCC__
#define inline
#define __inline
#define __func__	""
#define __int64		long long		/* dummy(lcc's "long long" is int32_t) */
#define __declspec(x)
#define __cdecl
#define __fastcall
#define __stdcall
#define __thiscall

#define _M_IX86		1

#define _WIN32		1
//#define __midl


#endif	/* _LCC_WITH_BCC_H */
