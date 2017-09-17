#ifndef _LCC_WITH_VC_H
#define _LCC_WITH_VC_H

#define __LCC_WITH_VC__

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
/*#define __STDC__	1*/
#define __midl
#define _CRT_SECURE_NO_WARNINGS
#undef RC_INVOKED
#define _USE_DECLSPECS_FOR_SAL	0
#undef _CRT_INSECURE_DEPRECATE
#define _PREFAST_

#endif	/* _LCC_WITH_VC_H */
