#pragma once
#define PLATFORM_WIN32 0
#define PLATFORM_UNIX  1
#define PLATFORM_APPLE 2

#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
#  define PLATFORM__ PLATFORM_WIN32
#elif defined( __APPLE_CC__ )
#  define PLATFORM__ PLATFORM_APPLE
#else
#  define PLATFORM__ PLATFORM_UNIX
#endif

#ifdef _MSC_VER // MS VC++

	typedef __int64				int64;
	typedef unsigned __int64	uint64;
	typedef __int32				int32;
	typedef unsigned __int32	uint32;
	typedef __int16				int16;
	typedef unsigned __int16	uint16;
	typedef __int8				int8;
	typedef unsigned __int8		uint8;
	typedef float				real32;
	typedef double				real64;
	typedef const char*			pcstr;
	typedef char*				pstr;

#else

	typedef long long			int64;
	typedef unsigned long long	uint64;
	typedef long				int32;
	typedef unsigned long		uint32;
	typedef short				int16;
	typedef unsigned short		uint16;
	typedef char				int8;
	typedef unsigned char		uint8;
	typedef float				real32;
	typedef double				real64;
	typedef const char*			pcstr;
	typedef char*				pstr;

#endif

template<int S> struct aligned2
{
	short values[((S + 1) & ~1) >> 1];
	void* operator &()
	{
		return reinterpret_cast<void*>(&values[0]);
	}

	const void* operator &() const
	{
		return reinterpret_cast<const void*>(&values[0]);
	}
};

template<int S> struct aligned4
{
	int values[((S + 3) & ~3) >> 2];
	void* operator &()
	{
		return reinterpret_cast<void*>(&values[0]);
	}

	const void* operator &() const
	{
		return reinterpret_cast<const void*>(&values[0]);
	}
};

template<int S> struct aligned8
{
	double values[((S + 7) & ~7) >> 3];
	void* operator &()
	{
		return reinterpret_cast<void*>(&values[0]);
	}

	const void* operator &() const
	{
		return reinterpret_cast<const void*>(&values[0]);
	}
};

#define STATIC_ASSERT(expr)	typedef char CC_##__LINE__ [(expr) ? 1 : -1]