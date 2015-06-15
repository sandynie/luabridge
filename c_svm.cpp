#include "c_svm.h"
#include <malloc.h>
lua_State* CLuaSVM::mLS	= NULL;

#define  AppFree free
#define AppRealloc realloc


int static DIGV( lua_State* L )
{
	const char* tValue = lua_tostring( L,1 );
	lua_getglobal( L, tValue );
	if( lua_isnil( L ,-1 ) )
	{
		lua_pushvalue( L,2 );
		lua_setglobal( L,tValue );
	}
	return 0;
}


int getglobal(lua_State *ls)
{
	if( lua_isstring(ls,1) == false )
	{
		lua_pushnil( ls );
	}
	else 
		lua_getglobal( ls,lua_tostring( ls,1 ) );
	return 1;
}

CLuaSVM::CLuaSVM():mpFuncErrorHandle( NULL ),mbShutScript( false )
{
	printf("CLuaSVM");
	LuaInit();
	lua_pushstring( mLS,ToString( arg1 ) );
	this->mGlobalLuaRef[ arg1 ]		= lua_ref( mLS,-1 );

	lua_pushstring( mLS,ToString( arg2 ) );
	this->mGlobalLuaRef[ arg2 ]		= lua_ref( mLS,-1 );

	lua_pushstring( mLS,ToString( arg3 ) );
	this->mGlobalLuaRef[ arg3 ]		= lua_ref( mLS,-1 );

	lua_pushstring( mLS,ToString( arg4 ) );
	this->mGlobalLuaRef[ arg4 ]		= lua_ref( mLS,-1 );

	lua_pushstring( mLS,ToString( arg5 ) );
	this->mGlobalLuaRef[ arg5 ]		= lua_ref( mLS,-1 );

	lua_pushstring( mLS,ToString( arg6 ) );
	this->mGlobalLuaRef[ arg6 ]		= lua_ref( mLS,-1 );

	lua_pushstring( mLS,ToString( arg7 ) );
	this->mGlobalLuaRef[ arg7 ]		= lua_ref( mLS,-1 );

	lua_pushstring( mLS,ToString( arg8 ) );
	this->mGlobalLuaRef[ arg8 ]		= lua_ref( mLS,-1 );

	lua_pushstring( mLS,ToString( event ) );
	this->mGlobalLuaRef[ event ]	= lua_ref( mLS,-1 );
}

static void *l_alloc (void *ud, void *ptr, size_t osize, size_t nsize) 
{
	(void)ud;
	(void)osize;

	if(nsize == 0)
	{
		AppFree(ptr);
		return NULL;
	}
	else
	{
		return AppRealloc(ptr, nsize);
	}
}

void CLuaSVM::LuaInit()
{
	mLS = lua_newstate(l_alloc,0);
		
		//lua_checkstack( mLS,100);
		/*luaopen_base(mLS);
		luaopen_table(mLS);
		luaopen_string(mLS);
		luaopen_math(mLS);
		luaopen_debug(mLS);*/

		luaL_openlibs( mLS );

		//mLS = luaL_newstate();

		lua_register( mLS,"DIGV",DIGV );
		lua_register( mLS,"getglobal",getglobal );
		lua_gc(mLS, 1, 0);
}

CLuaSVM::~CLuaSVM()
{
	if( mLS )
		lua_close( mLS );
	mLS	= NULL;
}

void CLuaSVM::ErrorInfoOut( lua_State* L,const char* Format,... )
{
	va_list tArgs;
	va_start( tArgs, Format ); _vsnprintf( mLastErrorInfo,MAX_LUA_ERROR_DESC_LEN, Format, tArgs ); va_end( tArgs );
	if( mpFuncErrorHandle != NULL )
		(*mpFuncErrorHandle)( mLastErrorInfo );
	if( L != NULL )
		lua_pushnumber( L,FR_FAILED );

	fprintf( stderr,"%s\n",mLastErrorInfo );
	fflush( stderr );
}

void CLuaSVM::ScriptInfoOut( const char* FormatString,... )
{
	va_list tArgs;
	va_start( tArgs, FormatString ); _vsnprintf( mLastErrorInfo,MAX_LUA_ERROR_DESC_LEN, FormatString, tArgs ); 
	va_end( tArgs );
	if( mpFuncErrorHandle != NULL )
		(*mpFuncErrorHandle)( mLastErrorInfo );
}

int CLuaSVM::ExecuteString( const char* Str )
{
	if( mbShutScript )
		return 0;
	int status	= luaL_dostring( mLS,Str );
	if(status != 0)
	{
		ErrorInfoOut( NULL,"ExecuteString():%s, error: %s\n", Str, lua_tostring(mLS, -1));
		return 1;
	}
	return 1;
}

int CLuaSVM::LoadBuffer( const char* pbuf, size_t len )
{
	if( mbShutScript )
		return 0;
	int status = luaL_loadbuffer( mLS, pbuf, len, "" );

	if( status == 0 ) status = lua_pcall( mLS, 0, 0, 0 );

	if(status != 0)
	{
		ErrorInfoOut( NULL,"loadbuffer():%s, error: %s\n", pbuf, lua_tostring(mLS, -1));
		return 0;
	}
	return 1;
}
int CLuaSVM::DirectExecuteFile(const char* path)
{
	if (mbShutScript)
	{
		return 1;
	}
	int ret= luaL_dofile(mLS, path); 
	if (ret !=0)
	{
		printf("Error occurs when calling luaL_dofile() Hint Machine 0x%x\n",ret);
		printf("Error: %s", lua_tostring(CLuaSVM::GetSingleton().mLS,-1));
		return 1;
	}
	return 0;
}
int CLuaSVM::ExecuteFile(const char* path)  //这里的path必须是经过luac -o test.out test.lua 之后的文件，不然它读进内存的都是不带转义的字符，导致编译出错.....
{
	if( mbShutScript )
		return 0;

	CAutoReleaseFile pAutoFile(path);
	if(LoadBuffer(pAutoFile&0 , pAutoFile.GetSize()) != FR_OK)
	{
		ErrorInfoOut( mLS,"load script failed:%s, error msg :%s\n", path, lua_tostring(mLS, -1) );
	}
	else
	{
		//ErrorInfoOut(NULL,"load script file:%s\n", Path );
	}

	return 1;


	return 0;

}

void CLuaSVM::InsertArgSetData(SCRIPT_GLOBAL_ARGUMENT Arg, stArgSetData data)
{
	std::map<SCRIPT_GLOBAL_ARGUMENT, stArgSetData>::iterator iterFind = m_mapArgSetData.find(Arg);       
	if (iterFind != m_mapArgSetData.end())
	{
		iterFind->second = data;
	}
	else
	{
		m_mapArgSetData.insert(std::make_pair(Arg, data));
	}
}

void CLuaSVM::DoGc()
{
	lua_gc(mLS,LUA_GCCOLLECT,0);
}
void CLuaSVM::DoGcStep()
{
	lua_gc(mLS, LUA_GCSTEP, 50);
}
int CLuaSVM::GetMemUsage()
{
	return lua_gc(mLS, LUA_GCCOUNT, 0);
}

int CLuaSVM::ExecuteScriptFileFunc(const char* path, const char* funcname,const char* Format,... )
{
	if (mbShutScript)
	{
		return 0;
	}
	if (DirectExecuteFile(path) != 0)
	{
		ErrorInfoOut(NULL,"Load file error:%s", path );
		return FR_FAILED;
	}

	int tTableIdx	= LUA_GLOBALSINDEX;
	const char* str0	= funcname;
	char word[64];
	int i=0,t=0;
	while( str0 != NULL && str0[i] != 0 )
	{
		if( str0[i] != '.' )
		{
			word[t]=str0[i];
			t++;
		}
		else
		{
			word[t]=0;
			t=0;
			lua_getfield( mLS,tTableIdx,word );
			tTableIdx	= -1;
			if(!lua_istable(mLS, -1))
			{
				ErrorInfoOut(NULL,"invalid table: %s", word);
				return 0;
			}
		}
		i++;
	}
	word[t]=0;
	lua_getfield(mLS, tTableIdx, word);
	if( lua_isfunction( mLS,-1) == false )
	{
		ErrorInfoOut(NULL,"not funcation" );
		return 0;
	}

	va_list vl;
	va_start(vl, Format);
	const char *sig = Format;
	if(Format == NULL) sig = "";
	int narg = 0;
	while(*sig)
	{
		switch(*sig++)
		{
		case 'd': /* double	argument */
			lua_pushnumber(mLS, va_arg(vl, double));
			break;
		case 'i': /* int argument */
			lua_pushnumber(mLS, va_arg(vl, int));
			break;
		case 's': /* char	argument */
			lua_pushstring(mLS, va_arg(vl, char *));
			break;
		case 'b': /* boolean argument */
			lua_pushboolean(mLS, va_arg(vl, bool));
			break;
		case 'u': /* light user data */
			lua_pushlightuserdata(mLS, va_arg(vl, void *));
			break;
		case '>':
			goto endwhile;
		default:
			ErrorInfoOut(NULL,"invalid option (%c)\n",	*(sig -	1));
			return 0;
		}
		narg++;
		luaL_checkstack(mLS, 1, "too many	arguments");
	}

endwhile:
	int nres = (int)strlen(sig);	/* number of expected results */
	if (lua_pcall(mLS, narg, nres, 0)	!= 0) /* do	the	call */
	{
		ErrorInfoOut(mLS,"initialize module error: %s: %s\n", word, lua_tostring(mLS, -1));
		return 0;
	}

	/* retrieve	results	*/
	nres = -nres; /* stack index of	first result */
	while(*sig)
	{
		switch (*sig++)
		{
		case 'd': /* double	result */
			if (!lua_isnumber(mLS, nres))
			{
				ErrorInfoOut(NULL,"wrong	result type");
			}
			*va_arg(vl,	double *) =	lua_tonumber(mLS,	nres);
			break;
		case 'i': /* int result	*/
			if (!lua_isnumber(mLS, nres))
			{
				ErrorInfoOut(NULL,"wrong	result type");
			}
			*va_arg(vl,	int	*) = (int)lua_tonumber(mLS, nres);
			break;
		case 's': /* CString	result */
			if (!lua_isstring(mLS, nres))
			{
				ErrorInfoOut(NULL,"wrong	result type\n");
			}
			*va_arg(vl,	const char **) = lua_tostring(mLS, nres);
			break;
		case 'b': /* boolean argument */
			if (!lua_isboolean(mLS, nres))
			{
				ErrorInfoOut(NULL,"wrong	result type\n");
			}
			*va_arg(vl,	bool *) = ( 0 != lua_toboolean(mLS, nres));
			break;
		case 'u': /* light user data */
			if (!lua_isuserdata(mLS, nres))
			{
				ErrorInfoOut(NULL,"wrong	result type\n");
			}
			*va_arg(vl,	void **) = lua_touserdata(mLS, nres);
			break;
		default:
			ErrorInfoOut(NULL,"invalid option (%c)\n",	*(sig -	1));
			break;
		}
		nres++;
	}
	return 1;
}
