#ifndef _CLUAMACROS_H
#define _CLUAMACROS_H
#include <stdio.h>
#include <assert.h>
extern "C" 
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "lstate.h"
}
#include <string>
using std::string;
namespace luacpp{


#ifdef _MSC_VER
#include "stdint.h"
#include "inttypes.h"
#else
	// Other compilers should have this.
#include <stdint.h>
#include <inttypes.h>
#endif

#ifndef ToString
#define ToString( T ) #T
#endif

#ifndef LUABRIDGER_FORCEINLINE
#if defined(_MSC_VER) && !defined(NDEBUG)
#define LUABRIDGER_FORCEINLINE __forceinline
#endif
#elif defined(__GNUC__) && __GNUC__ >= 4 && !defined(NDEBUG)
#define LUABRIDGER_FORCEINLINE __attribute__((always_inline))
#else
#define LUABRIDGER_FORCEINLINE
#endif


}
#endif