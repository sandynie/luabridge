#ifndef _C_SVM_H
#define _C_SVM_H

#include "CLuaMacros.h"
#include "CAutoReleaseFile.h"
#include "c_singleton.h"
#include <map>
using std::map;
enum
{
	FR_FAILED,
	FR_OK,
	FR_INDOING,
	FR_BREAK,

	FR_EXIT_EXE,
	FR_STOP_DOFRAME,
	FR_SYNC_BLOCK
};


enum SCRIPT_VALUE_TYPE
{
	SVT_EMPTY,
	SVT_DOUBLE,
	SVT_FLOAT,
	SVT_BOOL,
	SVT_PTR_STRING,
	SVT_SCRIPT,
	SVT_INT,
	SVT_USERD
};

struct ValueArgument
{
	union
	{
		double				mNumber;
		float				mFloat;
		int					mInt;
		unsigned int		mUint;
		unsigned long		mDword;
		const char*		mpString;
		void*				mpUserData;
	};
	SCRIPT_VALUE_TYPE	mType;
	ValueArgument( double Number ):mNumber( Number),mType( SVT_DOUBLE )
	{}
	ValueArgument( int Int ):mInt( Int),mType( SVT_INT )
	{}
	ValueArgument( const char* S ):mpString( S ),mType( SVT_PTR_STRING )
	{}
	ValueArgument( void* UD ):mpUserData( UD),mType( SVT_USERD )
	{}
	ValueArgument( SCRIPT_VALUE_TYPE T ):mType( T )
	{}
	ValueArgument( ):mType( SVT_EMPTY )
	{}
};


enum SCRIPT_GLOBAL_ARGUMENT
{
	arg1=0,
	arg2,
	arg3,
	arg4,
	arg5,
	arg6,
	arg7,
	arg8,
	event,
	arg_num
};

//arg设置数据
enum emArgDataValueType
{
	ADVT_NONE = 0,
	ADVT_INT,
	ADVT_DOUBLE,
	ADVT_STRING,
	ADVT_PTR,
};

struct stArgSetData 
{
	int nArg;
	int nValueType;        
	int nIntValue;
	double dDoubleValue;
	string strStingValue;
	void* pPtrValue;

	stArgSetData()
	{
		nArg = -1;
		nValueType = ADVT_NONE;
		nIntValue = 0;
		dDoubleValue = 0.0f;
		strStingValue = "";
		pPtrValue = NULL;
	}
};




class CLuaSVM : public Singleton<CLuaSVM>
{
		enum{ MAX_LUA_ERROR_DESC_LEN = 512 };
public:
	CLuaSVM();
	~CLuaSVM();
	/*static CLuaSVM& getInstance()
	{
		static CLuaSVM cs;
		return cs;
	}*/
	static lua_State*	mLS;
	void (* mpFuncErrorHandle )(  const char* Info );
	bool mbShutScript;
	char	mLastErrorInfo[ MAX_LUA_ERROR_DESC_LEN ];
	//保存当前设置arg值
	std::map<SCRIPT_GLOBAL_ARGUMENT, stArgSetData> m_mapArgSetData;
public:
	void ErrorInfoOut( lua_State* L,const char* Format,... );
	void ScriptInfoOut( const char* FormatString,... );

	int ExecuteString( const char* Str );   // 0失败， 1成功
	int LoadBuffer( const char* pbuf, size_t len );
	int ExecuteFile(const char* path);
	int DirectExecuteFile(const char* path);
	int ExecuteScriptFileFunc(const char* path, const char* funcname,const char* Format,... );
protected:

	int mGlobalLuaRef[ arg_num ];
	inline bool PushArgumet( SCRIPT_GLOBAL_ARGUMENT Arg )
	{
		switch( Arg )
		{
		case arg1:
			lua_getref( mLS,mGlobalLuaRef[arg1] );
			return true;
		case arg2:
			lua_getref( mLS,mGlobalLuaRef[arg2] );
			return true;
		case arg3:
			lua_getref( mLS,mGlobalLuaRef[arg3] );
			return true;
		case arg4:
			lua_getref( mLS,mGlobalLuaRef[arg4] );
			return true;
		case arg5:
			lua_getref( mLS,mGlobalLuaRef[arg5] );
			return true;
		case arg6:
			lua_getref( mLS,mGlobalLuaRef[arg6] );
			return true;
		case arg7:
			lua_getref( mLS,mGlobalLuaRef[arg7] );
			return true;
		case arg8:
			lua_getref( mLS,mGlobalLuaRef[arg8] );
			return true;
		case event:
			lua_getref( mLS,mGlobalLuaRef[event] );
			return true;
		default:
			return false;
		}
	}
public:
	//m_mapArgSetData
	void InsertArgSetData(SCRIPT_GLOBAL_ARGUMENT Arg, stArgSetData data);

	std::map<SCRIPT_GLOBAL_ARGUMENT, stArgSetData>& GetMapArgSetData()
	{
		return m_mapArgSetData;
	}
	inline void SetGlobalValue( SCRIPT_GLOBAL_ARGUMENT Arg,int Var )
	{            
		if( false == PushArgumet( Arg ) )
			return;
		lua_pushinteger( mLS,Var );
		lua_settable( mLS,LUA_GLOBALSINDEX );   

		stArgSetData data;
		data.nArg = Arg;
		data.nValueType = ADVT_INT;
		data.nIntValue = Var;

		InsertArgSetData(Arg, data);
	}

	inline void SetGlobalValueEx( SCRIPT_GLOBAL_ARGUMENT Arg,int Var )
	{
		if( false == PushArgumet( Arg ) )
			return;
		lua_pushinteger( mLS,Var );
		lua_settable( mLS,LUA_GLOBALSINDEX );   
	}

	inline void SetGlobalValue( SCRIPT_GLOBAL_ARGUMENT Arg,const char* Var )
	{
		if( false == PushArgumet( Arg ) )
			return;
		lua_pushstring( mLS,Var );
		lua_settable( mLS,LUA_GLOBALSINDEX );

		stArgSetData data;
		data.nArg = Arg;
		data.nValueType = ADVT_STRING;
		data.strStingValue = Var;

		InsertArgSetData(Arg, data);
	}

	inline void SetGlobalValueEx( SCRIPT_GLOBAL_ARGUMENT Arg,const char* Var )
	{
		if( false == PushArgumet( Arg ) )
			return;
		lua_pushstring( mLS,Var );
		lua_settable( mLS,LUA_GLOBALSINDEX );
	}

	inline void SetGlobalValue( SCRIPT_GLOBAL_ARGUMENT Arg,double Var )
	{
		if( false == PushArgumet( Arg ) )
			return;
		lua_pushnumber( mLS,Var );
		lua_settable( mLS,LUA_GLOBALSINDEX );  

		stArgSetData data;
		data.nArg = Arg;
		data.nValueType = ADVT_DOUBLE;
		data.dDoubleValue = Var;

		InsertArgSetData(Arg, data);            
	}

	inline void SetGlobalValueEx( SCRIPT_GLOBAL_ARGUMENT Arg,double Var )
	{
		if( false == PushArgumet( Arg ) )
			return;
		lua_pushnumber( mLS,Var );
		lua_settable( mLS,LUA_GLOBALSINDEX );         
	}

	inline void SetGlobalValue( SCRIPT_GLOBAL_ARGUMENT Arg, void* Var )
	{
		if( false == PushArgumet( Arg ) )
			return;

		lua_pushlightuserdata( mLS,Var );
		lua_settable( mLS,LUA_GLOBALSINDEX );

		stArgSetData data;
		data.nArg = Arg;
		data.nValueType = ADVT_PTR;
		data.pPtrValue = Var;

		InsertArgSetData(Arg, data);                  
	}

	inline void SetGlobalValueEx( SCRIPT_GLOBAL_ARGUMENT Arg, void* Var )
	{
		if( false == PushArgumet( Arg ) )
			return;


		lua_pushlightuserdata( mLS,Var );
		lua_settable( mLS,LUA_GLOBALSINDEX );
	}

	void SetGlobalValue( SCRIPT_GLOBAL_ARGUMENT Arg,const ValueArgument* Data )
	{
		SetGlobalValueEx(Arg, Data);
	}

	void SetGlobalValueEx( SCRIPT_GLOBAL_ARGUMENT Arg,const ValueArgument* Data );

	void SetEventGlobalValue(const char* Var)
	{
		SetGlobalValueEx(event, Var);
	}
public:
	void DoGc();
	void DoGcStep();
	int GetMemUsage();

private:
	void LuaInit();

};
#endif