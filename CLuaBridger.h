#ifndef _CLUABRIDGER_H
#define _CLUABRIDGER_H
#include "CLuaMacros.h"
#include "CTemplateTypes.h"
namespace cpplua{

	void traceStack(lua_State* L, int n);
	int error_log(lua_State *L);
	void doFile(lua_State *L, const char *fileName);
	lua_State* openLua();
	void closeLua(lua_State* L);
	template<class T>
	LUABRIDGER_FORCEINLINE T readfromlua(lua_State* m_Ptr, int iStackIndex);

	template<class T>
	LUABRIDGER_FORCEINLINE void puttolua(lua_State* L, T value);


	template<class T>
	struct stClassName{
		LUABRIDGER_FORCEINLINE static void setName(const char* p_Str){m_pClassName = p_Str;}
		LUABRIDGER_FORCEINLINE static const char* getName(){return m_pClassName;}
		static const char* m_pClassName;
	};



	struct stCLuaString{
		const char* p_Str;
		size_t sLen;
	};
	typedef struct stCLuaString  CLuaString;
	typedef struct stCLuaString * LPCLuaString;

	//引用计数
	struct  stRefCount{
		stRefCount():m_iRefCount(1){}
		LUABRIDGER_FORCEINLINE int incRef(){ return ++m_iRefCount;}
		LUABRIDGER_FORCEINLINE int decRef(){return --m_iRefCount;}
		int m_iRefCount;
	};

	typedef struct stRefCount RefCount;
	typedef struct stRefCount*  LPRefCount;



	struct stCLuaTable{
		stCLuaTable():m_pLuaState(NULL),m_pSelfAddress(NULL),m_pRefCount(NULL){}
		~stCLuaTable(){
			release();
			m_pLuaState = NULL;
			m_pSelfAddress = NULL;
		}

		stCLuaTable(lua_State* m_Ptr)
		{
			lua_newtable(m_Ptr);
			m_pLuaState = m_Ptr;
			m_iLuaStackPos = lua_gettop(m_pLuaState);
			m_pSelfAddress = lua_topointer(m_pLuaState,m_iLuaStackPos);
			m_pRefCount = new RefCount();
		}

		stCLuaTable(const stCLuaTable& rhs)
		{
			m_pLuaState = rhs.m_pLuaState;
			m_pSelfAddress = rhs.m_pSelfAddress;
			m_iLuaStackPos = rhs.m_iLuaStackPos;
			m_pRefCount = rhs.m_pRefCount;
			if (m_pRefCount)
			{
				m_pRefCount->incRef();
			}
		}

		stCLuaTable&  operator=(const stCLuaTable& rhs)
		{
			if (this!=&rhs)
			{
				release();
				m_pLuaState = rhs.m_pLuaState;
				m_pSelfAddress = rhs.m_pSelfAddress;
				m_iLuaStackPos = rhs.m_iLuaStackPos;
				m_pRefCount = rhs.m_pRefCount;
				if (m_pRefCount)
				{
					m_pRefCount->incRef();
				}
			}
			return *this;
		}

		stCLuaTable(lua_State* m_Ptr, int iStackIndex){
			if (iStackIndex <0)
			{
				iStackIndex = lua_gettop(m_Ptr)+iStackIndex+1;
			}
			m_pLuaState = m_Ptr;
			m_pSelfAddress = lua_topointer(m_pLuaState,iStackIndex);
			m_iLuaStackPos = iStackIndex;
			m_pRefCount = new RefCount();
		}

		stCLuaTable(lua_State* m_Ptr, const char* p_StrName){
			lua_getglobal(m_Ptr,p_StrName);
			if (lua_istable(m_Ptr,-1))
			{
				m_pLuaState = m_Ptr;
				m_pSelfAddress = lua_topointer(m_pLuaState,-1);
				m_iLuaStackPos = lua_gettop(m_pLuaState);
				m_pRefCount = new RefCount();
			}else{
				assert(0 && "the stack top is not a lua table");
			}
		}
		//引用计数-1
		LUABRIDGER_FORCEINLINE void release(){
			if (m_pRefCount)
			{
				if (m_pRefCount->decRef()==0)
				{
					if (isValid())
					{
						lua_remove(m_pLuaState,m_iLuaStackPos);
					}
				}
			}
			m_pRefCount = NULL;
		}

		//当前对象是否有效
		LUABRIDGER_FORCEINLINE bool isValid(){
			if (m_pSelfAddress)
			{
				if (lua_topointer(m_pLuaState,m_iLuaStackPos) == m_pSelfAddress)
				{
					return true;
				}
			}
			return false;
		}

		template<class K,class V>
		LUABRIDGER_FORCEINLINE void put(const K& key,const V& value){
			if ( isValid() ) {
				puttolua(m_pLuaState, key); 
				puttolua(m_pLuaState, value);
				lua_settable(m_pLuaState, m_iLuaStackPos);
			}
		}

		template<class V>
		LUABRIDGER_FORCEINLINE void put(const int iKey, const V& value){
			if ( isValid() ) {
				lua_pushnumber(m_pLuaState, iKey); 
				puttolua(m_pLuaState, value);
				lua_settable(m_pLuaState, m_iLuaStackPos);
			}
		}

		template<class V>
		LUABRIDGER_FORCEINLINE void put(const char* pKey,const V& value){
			if ( isValid() ) {
				lua_pushstring(m_pLuaState, pKey);
				push2lua(m_pLuaState, value);
				lua_settable(m_pLuaState, m_iLuaStackPos);
			}
		}

		template<class K,class V>
		LUABRIDGER_FORCEINLINE V get(const K& key){
			if ( isValid() ) {
				puttolua(m_pLuaState, key);
				lua_gettable(m_pLuaState, m_iLuaStackPos);
			} else {
				lua_pushnil(m_pLuaState);
			}

			V result = readfromlua<V>(m_pLuaState, -1);
			lua_pop(m_pLuaState, 1);
			return result;
		}

		template<class V>
		LUABRIDGER_FORCEINLINE V get(int iKey){
			if(isValid()) {
				lua_pushnumber(m_pLuaState, iKey);
				lua_gettable(m_pLuaState, m_iLuaStackPos);
			} else {
				lua_pushnil(m_pLuaState);
			}

			V result = readfromlua<V>(m_pLuaState, -1);
			lua_pop(m_pLuaState, 1);
			return result;
		}

		template<class V>
		LUABRIDGER_FORCEINLINE V get(const char* pKey){
			if(isValid()) {
				lua_pushstring(m_pLuaState, pKey);
				lua_gettable(m_pLuaState, m_iLuaStackPos);
			} else {
				lua_pushnil(m_pLuaState);
			}

			V result = readfromlua<V>(m_pLuaState, -1);
			lua_pop(m_pLuaState, 1);
			return result;
		}

		template<class K>
		LUABRIDGER_FORCEINLINE stCLuaTable get(const K& key){
			if ( isValid() ) {
				puttolua(m_pLuaState, key);
				lua_gettable(m_pLuaState, m_iLuaStackPos);
			} else {
				lua_pushnil(m_pLuaState);
			}

			return stCLuaTable(m_pLuaState, -1);
		}

		int              m_iLuaStackPos;
		RefCount   * m_pRefCount;
		const void * m_pSelfAddress;
		lua_State   *  m_pLuaState;
	};


	template<class T>
	struct stUserData{
		stUserData():m_Ptr(NULL),m_DelWhenGC(true){}
		stUserData(T* pObj, bool bDelWhenGC = true):m_Ptr(pObj),m_DelWhenGC(bDelWhenGC){}
		~stUserData(){
			if (m_DelWhenGC)
			{
				delete m_Ptr;
			}
		}
		T* m_Ptr;
		bool m_DelWhenGC;
	};




	//在c++ 中直接调用lua函数的封装对象
	template<class R>
	struct stCLuaFunction
	{
	public:
		~stCLuaFunction() {
			if (m_pStatue)
			{
				luaL_unref(m_pStatue, LUA_REGISTRYINDEX, m_iRefIndex);
				m_pStatue = NULL;
			}
		}

		stCLuaFunction(lua_State* L, const char* psFuncName): m_pStatue(L) ,pFuncName(psFuncName){
			lua_getglobal(L, pFuncName);

			if (lua_isfunction(L, -1)) {
				m_iRefIndex = luaL_ref(L, LUA_REGISTRYINDEX);
			} else {
				assert(0 && "you can not ref a unexist function in lua!!!");
			}
		};

		R operator()() {
			lua_pushcclosure(m_pStatue, error_log, 0);                
			int iErrorFunctIndex = lua_gettop(m_pStatue);

			lua_rawgeti(m_pStatue, LUA_REGISTRYINDEX, m_iRefIndex);
			lua_pcall(m_pStatue, 0, 1, iErrorFunctIndex);                // pcall 之后会主动清楚 function和arguements，所以只剩下 errorfunc和返回值     
			R result = readfromlua<R>(m_pStatue, -1);
			lua_settop(m_pStatue, -3);                                
			return result;
		}

		template<class P1>
		R operator()(P1 p1) {
			lua_pushcclosure(m_pStatue, error_log, 0);
			int iErrorFunctIndex = lua_gettop(m_pStatue);

			lua_rawgeti(m_pStatue, LUA_REGISTRYINDEX, m_iRefIndex);
			push2lua(m_pStatue, p1);
			lua_pcall(m_pStatue, 1, 1, iErrorFunctIndex);

			R result = readfromlua<R>(m_pStatue, -1);
			lua_settop(m_pStatue, -3);
			return result;
		}

		template<class P1, class P2>
		R operator()(P1 p1, P2 p2) {
			lua_pushcclosure(m_pStatue, error_log, 0);
			int iErrorFunctIndex = lua_gettop(m_pStatue);

			lua_rawgeti(m_pStatue, LUA_REGISTRYINDEX, m_iRefIndex);
			puttolua(m_pStatue, p1); 
			puttolua(m_pStatue, p2);
			lua_pcall(m_pStatue, 2, 1, iErrorFunctIndex);

			R result = readfromlua<R>(m_pStatue, -1);
			lua_settop(m_pStatue, -3);
			return result;
		}

		template<class P1, class P2,class P3>
		R operator()(P1 p1, P2 p2,P3 p3) {
			lua_pushcclosure(m_pStatue, error_log, 0);
			int iErrorFunctIndex = lua_gettop(m_pStatue);

			lua_rawgeti(m_pStatue, LUA_REGISTRYINDEX, m_iRefIndex);
			puttolua(m_pStatue, p1); 
			puttolua(m_pStatue, p2);
			puttolua(m_pStatue, p3);
			lua_pcall(m_pStatue,3, 1, iErrorFunctIndex);

			R result = readfromlua<R>(m_pStatue, -1);
			lua_settop(m_pStatue, -3);
			return result;
		}


		template<class P1, class P2,class P3,class P4>
		R operator()(P1 p1, P2 p2,P3 p3,P4 p4) {
			lua_pushcclosure(m_pStatue, error_log, 0);
			int iErrorFunctIndex = lua_gettop(m_pStatue);

			lua_rawgeti(m_pStatue, LUA_REGISTRYINDEX, m_iRefIndex);
			puttolua(m_pStatue, p1); 
			puttolua(m_pStatue, p2);
			puttolua(m_pStatue, p3);
			puttolua(m_pStatue, p4);
			lua_pcall(m_pStatue,4, 1, iErrorFunctIndex);

			R result = readfromlua<R>(m_pStatue, -1);
			lua_settop(m_pStatue, -3);
			return result;
		}

		template<class P1, class P2,class P3,class P4,class P5>
		R operator()(P1 p1, P2 p2,P3 p3,P4 p4,P5 p5) {
			lua_pushcclosure(m_pStatue, error_log, 0);
			int iErrorFunctIndex = lua_gettop(m_pStatue);

			lua_rawgeti(m_pStatue, LUA_REGISTRYINDEX, m_iRefIndex);
			puttolua(m_pStatue, p1); 
			puttolua(m_pStatue, p2);
			puttolua(m_pStatue, p3);
			puttolua(m_pStatue, p4);
			puttolua(m_pStatue, p5);
			lua_pcall(m_pStatue,5, 1, iErrorFunctIndex);

			R result = readfromlua<R>(m_pStatue, -1);
			lua_settop(m_pStatue, -3);
			return result;
		}

		template<class P1, class P2,class P3,class P4,class P5,class P6>
		R operator()(P1 p1, P2 p2,P3 p3,P4 p4,P5 p5,P6 p6) {
			lua_pushcclosure(m_pStatue, error_log, 0);
			int iErrorFunctIndex = lua_gettop(m_pStatue);

			lua_rawgeti(m_pStatue, LUA_REGISTRYINDEX, m_iRefIndex);
			puttolua(m_pStatue, p1); 
			puttolua(m_pStatue, p2);
			puttolua(m_pStatue, p3);
			puttolua(m_pStatue, p4);
			puttolua(m_pStatue, p5);
			puttolua(m_pStatue, p6);
			lua_pcall(m_pStatue,6, 1, iErrorFunctIndex);

			R result = readfromlua<R>(m_pStatue, -1);
			lua_settop(m_pStatue, -3);
			return result;
		}

		template<class P1, class P2,class P3,class P4,class P5,class P6,class P7>
		R operator()(P1 p1, P2 p2,P3 p3,P4 p4,P5 p5,P6 p6,P7 p7) {
			lua_pushcclosure(m_pStatue, error_log, 0);
			int iErrorFunctIndex = lua_gettop(m_pStatue);

			lua_rawgeti(m_pStatue, LUA_REGISTRYINDEX, m_iRefIndex);
			puttolua(m_pStatue, p1); 
			puttolua(m_pStatue, p2);
			puttolua(m_pStatue, p3);
			puttolua(m_pStatue, p4);
			puttolua(m_pStatue, p5);
			puttolua(m_pStatue, p6);
			puttolua(m_pStatue, p7);
			lua_pcall(m_pStatue,7, 1, iErrorFunctIndex);

			R result = readfromlua<R>(m_pStatue, -1);
			lua_settop(m_pStatue, -3);
			return result;
		}

		template<class P1, class P2,class P3,class P4,class P5,class P6,class P7,class P8>
		R operator()(P1 p1, P2 p2,P3 p3,P4 p4,P5 p5,P6 p6,P7 p7,P8 p8) {
			lua_pushcclosure(m_pStatue, error_log, 0);
			int iErrorFunctIndex = lua_gettop(m_pStatue);

			lua_rawgeti(m_pStatue, LUA_REGISTRYINDEX, m_iRefIndex);
			puttolua(m_pStatue, p1); 
			puttolua(m_pStatue, p2);
			puttolua(m_pStatue, p3);
			puttolua(m_pStatue, p4);
			puttolua(m_pStatue, p5);
			puttolua(m_pStatue, p6);
			puttolua(m_pStatue, p7);
			puttolua(m_pStatue, p8);
			lua_pcall(m_pStatue,8, 1, iErrorFunctIndex);

			R result = readfromlua<R>(m_pStatue, -1);
			lua_settop(m_pStatue, -3);
			return result;
		}
		template<class P1, class P2,class P3,class P4,class P5,class P6,class P7,class P8,class P9>
		R operator()(P1 p1, P2 p2,P3 p3,P4 p4,P5 p5,P6 p6,P7 p7,P8 p8,P9 p9) {
			lua_pushcclosure(m_pStatue, error_log, 0);
			int iErrorFunctIndex = lua_gettop(m_pStatue);

			lua_rawgeti(m_pStatue, LUA_REGISTRYINDEX, m_iRefIndex);
			puttolua(m_pStatue, p1); 
			puttolua(m_pStatue, p2);
			puttolua(m_pStatue, p3);
			puttolua(m_pStatue, p4);
			puttolua(m_pStatue, p5);
			puttolua(m_pStatue, p6);
			puttolua(m_pStatue, p7);
			puttolua(m_pStatue, p8);
			puttolua(m_pStatue, p9);
			lua_pcall(m_pStatue,9, 1, iErrorFunctIndex);

			R result = readfromlua<R>(m_pStatue, -1);
			lua_settop(m_pStatue, -3);
			return result;
		}


		template<class P1, class P2,class P3,class P4,class P5,class P6,class P7,class P8,class P9,class P10>
		R operator()(P1 p1, P2 p2,P3 p3,P4 p4,P5 p5,P6 p6,P7 p7,P8 p8,P9 p9,P10 p10) {
			lua_pushcclosure(m_pStatue, error_log, 0);
			int iErrorFunctIndex = lua_gettop(m_pStatue);

			lua_rawgeti(m_pStatue, LUA_REGISTRYINDEX, m_iRefIndex);
			puttolua(m_pStatue, p1); 
			puttolua(m_pStatue, p2);
			puttolua(m_pStatue, p3);
			puttolua(m_pStatue, p4);
			puttolua(m_pStatue, p5);
			puttolua(m_pStatue, p6);
			puttolua(m_pStatue, p7);
			puttolua(m_pStatue, p8);
			puttolua(m_pStatue, p9);
			puttolua(m_pStatue, p10);
			lua_pcall(m_pStatue,10, 1, iErrorFunctIndex);

			R result = readfromlua<R>(m_pStatue, -1);
			lua_settop(m_pStatue, -3);
			return result;
		}
	private:
		int m_iRefIndex;
		lua_State* m_pStatue;
		const char* pFuncName;
	};



	template<>
	struct stCLuaFunction<void>
	{
	public:
		~stCLuaFunction() {
			if (m_pStatue)
			{
				luaL_unref(m_pStatue, LUA_REGISTRYINDEX, m_iRefIndex);
				m_pStatue = NULL;
			}
		}

		stCLuaFunction(lua_State* L, const char* psFuncName): m_pStatue(L) ,pFuncName(psFuncName){
			lua_getglobal(L, pFuncName);

			if (lua_isfunction(L, -1)) {
				m_iRefIndex = luaL_ref(L, LUA_REGISTRYINDEX);
			} else {
				assert(0 && "you can not ref a unexist function in lua!!!");
			}
		};

		void operator()() {
			lua_pushcclosure(m_pStatue, error_log, 0);               
			int iErrorFunctIndex = lua_gettop(m_pStatue);

			lua_rawgeti(m_pStatue, LUA_REGISTRYINDEX, m_iRefIndex);
			lua_pcall(m_pStatue, 0, 0, iErrorFunctIndex);                   
			lua_settop(m_pStatue, -2);                                
		}

		template<class P1>
		void operator()(P1 p1) {
			lua_pushcclosure(m_pStatue, error_log, 0);
			int iErrorFunctIndex = lua_gettop(m_pStatue);

			lua_rawgeti(m_pStatue, LUA_REGISTRYINDEX, m_iRefIndex);
			push2lua(m_pStatue, p1);
			lua_pcall(m_pStatue, 1, 0, iErrorFunctIndex);
			lua_settop(m_pStatue, -2);
		}

		template<class P1, class P2>
		void operator()(P1 p1, P2 p2) {
			lua_pushcclosure(m_pStatue, error_log, 0);
			int iErrorFunctIndex = lua_gettop(m_pStatue);

			lua_rawgeti(m_pStatue, LUA_REGISTRYINDEX, m_iRefIndex);
			puttolua(m_pStatue, p1); 
			puttolua(m_pStatue, p2);
			lua_pcall(m_pStatue, 2, 0, iErrorFunctIndex);
			lua_settop(m_pStatue, -2);
		}

		template<class P1, class P2,class P3>
		void operator()(P1 p1, P2 p2,P3 p3) {
			lua_pushcclosure(m_pStatue, error_log, 0);
			int iErrorFunctIndex = lua_gettop(m_pStatue);

			lua_rawgeti(m_pStatue, LUA_REGISTRYINDEX, m_iRefIndex);
			puttolua(m_pStatue, p1); 
			puttolua(m_pStatue, p2);
			puttolua(m_pStatue, p3);
			lua_pcall(m_pStatue,3, 0, iErrorFunctIndex);
			lua_settop(m_pStatue, -2);
		}


		template<class P1, class P2,class P3,class P4>
		void operator()(P1 p1, P2 p2,P3 p3,P4 p4) {
			lua_pushcclosure(m_pStatue, error_log, 0);
			int iErrorFunctIndex = lua_gettop(m_pStatue);

			lua_rawgeti(m_pStatue, LUA_REGISTRYINDEX, m_iRefIndex);
			puttolua(m_pStatue, p1); 
			puttolua(m_pStatue, p2);
			puttolua(m_pStatue, p3);
			puttolua(m_pStatue, p4);
			lua_pcall(m_pStatue,4,0, iErrorFunctIndex);
			lua_settop(m_pStatue, -2);
		}

		template<class P1, class P2,class P3,class P4,class P5>
		void operator()(P1 p1, P2 p2,P3 p3,P4 p4,P5 p5) {
			lua_pushcclosure(m_pStatue, error_log, 0);
			int iErrorFunctIndex = lua_gettop(m_pStatue);

			lua_rawgeti(m_pStatue, LUA_REGISTRYINDEX, m_iRefIndex);
			puttolua(m_pStatue, p1); 
			puttolua(m_pStatue, p2);
			puttolua(m_pStatue, p3);
			puttolua(m_pStatue, p4);
			puttolua(m_pStatue, p5);
			lua_pcall(m_pStatue,5, 0, iErrorFunctIndex);
			lua_settop(m_pStatue, -2);
		}

		template<class P1, class P2,class P3,class P4,class P5,class P6>
		void operator()(P1 p1, P2 p2,P3 p3,P4 p4,P5 p5,P6 p6) {
			lua_pushcclosure(m_pStatue, error_log, 0);
			int iErrorFunctIndex = lua_gettop(m_pStatue);

			lua_rawgeti(m_pStatue, LUA_REGISTRYINDEX, m_iRefIndex);
			puttolua(m_pStatue, p1); 
			puttolua(m_pStatue, p2);
			puttolua(m_pStatue, p3);
			puttolua(m_pStatue, p4);
			puttolua(m_pStatue, p5);
			puttolua(m_pStatue, p6);
			lua_pcall(m_pStatue,6, 0, iErrorFunctIndex);
			lua_settop(m_pStatue, -2);
		}

		template<class P1, class P2,class P3,class P4,class P5,class P6,class P7>
		void operator()(P1 p1, P2 p2,P3 p3,P4 p4,P5 p5,P6 p6,P7 p7) {
			lua_pushcclosure(m_pStatue, error_log, 0);
			int iErrorFunctIndex = lua_gettop(m_pStatue);

			lua_rawgeti(m_pStatue, LUA_REGISTRYINDEX, m_iRefIndex);
			puttolua(m_pStatue, p1); 
			puttolua(m_pStatue, p2);
			puttolua(m_pStatue, p3);
			puttolua(m_pStatue, p4);
			puttolua(m_pStatue, p5);
			puttolua(m_pStatue, p6);
			puttolua(m_pStatue, p7);
			lua_pcall(m_pStatue,7, 0, iErrorFunctIndex);
			lua_settop(m_pStatue, -2);
		}

		template<class P1, class P2,class P3,class P4,class P5,class P6,class P7,class P8>
		void operator()(P1 p1, P2 p2,P3 p3,P4 p4,P5 p5,P6 p6,P7 p7,P8 p8) {
			lua_pushcclosure(m_pStatue, error_log, 0);
			int iErrorFunctIndex = lua_gettop(m_pStatue);

			lua_rawgeti(m_pStatue, LUA_REGISTRYINDEX, m_iRefIndex);
			puttolua(m_pStatue, p1); 
			puttolua(m_pStatue, p2);
			puttolua(m_pStatue, p3);
			puttolua(m_pStatue, p4);
			puttolua(m_pStatue, p5);
			puttolua(m_pStatue, p6);
			puttolua(m_pStatue, p7);
			puttolua(m_pStatue, p8);
			lua_pcall(m_pStatue,8, 0, iErrorFunctIndex);
			lua_settop(m_pStatue, -2);
		}

		template<class P1, class P2,class P3,class P4,class P5,class P6,class P7,class P8,class P9>
		void operator()(P1 p1, P2 p2,P3 p3,P4 p4,P5 p5,P6 p6,P7 p7,P8 p8,P9 p9) {
			lua_pushcclosure(m_pStatue, error_log, 0);
			int iErrorFunctIndex = lua_gettop(m_pStatue);

			lua_rawgeti(m_pStatue, LUA_REGISTRYINDEX, m_iRefIndex);
			puttolua(m_pStatue, p1); 
			puttolua(m_pStatue, p2);
			puttolua(m_pStatue, p3);
			puttolua(m_pStatue, p4);
			puttolua(m_pStatue, p5);
			puttolua(m_pStatue, p6);
			puttolua(m_pStatue, p7);
			puttolua(m_pStatue, p8);
			puttolua(m_pStatue, p9);
			lua_pcall(m_pStatue,9, 0, iErrorFunctIndex);
			lua_settop(m_pStatue, -2);
		}


		template<class P1, class P2,class P3,class P4,class P5,class P6,class P7,class P8,class P9,class P10>
		void operator()(P1 p1, P2 p2,P3 p3,P4 p4,P5 p5,P6 p6,P7 p7,P8 p8,P9 p9,P10 p10) {
			lua_pushcclosure(m_pStatue, error_log, 0);
			int iErrorFunctIndex = lua_gettop(m_pStatue);

			lua_rawgeti(m_pStatue, LUA_REGISTRYINDEX, m_iRefIndex);
			puttolua(m_pStatue, p1); 
			puttolua(m_pStatue, p2);
			puttolua(m_pStatue, p3);
			puttolua(m_pStatue, p4);
			puttolua(m_pStatue, p5);
			puttolua(m_pStatue, p6);
			puttolua(m_pStatue, p7);
			puttolua(m_pStatue, p8);
			puttolua(m_pStatue, p9);
			puttolua(m_pStatue, p10);
			lua_pcall(m_pStatue,10, 0, iErrorFunctIndex);
			lua_settop(m_pStatue, -2);
		}
	private:
		int m_iRefIndex;
		lua_State* m_pStatue;
		const char* pFuncName;
	};



	template<>
	struct stCLuaFunction<stCLuaTable>
	{
	public:
		~stCLuaFunction() {
			if (m_pStatue)
			{
				luaL_unref(m_pStatue, LUA_REGISTRYINDEX, m_iRefIndex);
				m_pStatue = NULL;
			}
		}

		stCLuaFunction(lua_State* L, const char* psFuncName): m_pStatue(L),pFuncName(psFuncName) {
			lua_getglobal(L, psFuncName);

			if (lua_isfunction(L, -1)) {
				m_iRefIndex = luaL_ref(L, LUA_REGISTRYINDEX);
			} else {
				assert(0  && " you can not ref an unexisted method in lua");
			}
		};

		stCLuaTable operator()() {
			lua_pushcclosure(m_pStatue, error_log, 0); 
			int stackTop = lua_gettop(m_pStatue);

			lua_rawgeti(m_pStatue, LUA_REGISTRYINDEX, m_iRefIndex);
			lua_pcall(m_pStatue, 0, 1, stackTop);                     // stack top +1

			lua_remove(m_pStatue, -2);
			return stCLuaTable(m_pStatue, -1);
		}

		template<class P1>
		stCLuaTable operator()(P1 p1) {
			lua_pushcclosure(m_pStatue, error_log, 0);
			int iErrorFuncIndex = lua_gettop(m_pStatue);

			lua_rawgeti(m_pStatue, LUA_REGISTRYINDEX, m_iRefIndex);
			puttolua(m_pStatue, p1);
			lua_pcall(m_pStatue, 1, 1, iErrorFuncIndex);

			lua_remove(m_pStatue, -2);
			return LuaTable(m_pStatue, -1);
		}

		template<class P1, class P2>
		stCLuaTable operator()(P1 p1, P2 p2) {
			lua_pushcclosure(m_pStatue, error_log, 0);
			int iErrorFuncIndex = lua_gettop(m_pStatue);

			lua_rawgeti(m_pStatue, LUA_REGISTRYINDEX, m_iRefIndex);
			puttolua(m_pStatue, p1); 
			puttolua(m_pStatue, p2);
			lua_pcall(m_pStatue, 2, 1, iErrorFuncIndex);

			lua_remove(m_pStatue, -2);
			return LuaTable(m_pStatue, -1);
		}

		template<class P1, class P2, class P3>
		stCLuaTable operator()(P1 p1, P2 p2, P3 p3) {
			lua_pushcclosure(m_pStatue, error_log, 0);
			int iErrorFuncIndex = lua_gettop(m_pStatue);

			lua_rawgeti(m_pStatue, LUA_REGISTRYINDEX, m_iRefIndex);
			puttolua(m_pStatue, p1); 
			puttolua(m_pStatue, p2);
			puttolua(m_pStatue, p3);
			lua_pcall(m_pStatue, 3, 1, iErrorFuncIndex);

			lua_remove(m_pStatue, -2);
			return LuaTable(m_pStatue, -1);
		}

		template<class P1, class P2, class P3, class P4>
		stCLuaTable operator()(P1 p1, P2 p2, P3 p3, P4 p4) {
			lua_pushcclosure(m_pStatue, error_log, 0);
			int iErrorFuncIndex = lua_gettop(m_pStatue);

			lua_rawgeti(m_pStatue, LUA_REGISTRYINDEX, m_iRefIndex);
			puttolua(m_pStatue, p1);
			puttolua(m_pStatue, p2);
			puttolua(m_pStatue, p3); 
			puttolua(m_pStatue, p4);
			lua_pcall(m_pStatue, 4, 1, iErrorFuncIndex);

			lua_remove(m_pStatue, -2);
			return LuaTable(m_pStatue, -1);
		}

		template<class P1, class P2, class P3, class P4, class P5>
		stCLuaTable operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) {
			lua_pushcclosure(m_pStatue, error_log, 0);
			int iErrorFuncIndex = lua_gettop(m_pStatue);

			lua_rawgeti(m_pStatue, LUA_REGISTRYINDEX, m_iRefIndex);
			puttolua(m_pStatue, p1);
			puttolua(m_pStatue, p2);
			puttolua(m_pStatue, p3); 
			puttolua(m_pStatue, p4);
			puttolua(m_pStatue, p5);
			puttolua(m_pStatue, 5, 1, iErrorFuncIndex);

			lua_remove(m_pStatue, -2);
			return LuaTable(m_pStatue, -1);
		}

		template<class P1, class P2, class P3, class P4, class P5, class P6>
		stCLuaTable operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6) {
			lua_pushcclosure(m_pStatue, error_log, 0);
			int iErrorFuncIndex = lua_gettop(m_pStatue);

			lua_rawgeti(m_pStatue, LUA_REGISTRYINDEX, m_iRefIndex);
			puttolua(m_pStatue, p1); 
			puttolua(m_pStatue, p2);
			puttolua(m_pStatue, p3);
			puttolua(m_pStatue, p4); 
			puttolua(m_pStatue, p5);
			puttolua(m_pStatue, p6);
			lua_pcall(m_pStatue, 6, 1, iErrorFuncIndex);

			lua_remove(m_pStatue, -2);
			return LuaTable(m_pStatue, -1);
		}

		template<class P1, class P2, class P3, class P4, class P5, class P6, class P7>
		stCLuaTable operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7) {
			lua_pushcclosure(m_pStatue, error_log, 0);
			int iErrorFuncIndex = lua_gettop(m_pStatue);

			lua_rawgeti(m_pStatue, LUA_REGISTRYINDEX, m_iRefIndex);
			puttolua(m_pStatue, p1);
			puttolua(m_pStatue, p2); 
			puttolua(m_pStatue, p3); 
			puttolua(m_pStatue, p4); 
			puttolua(m_pStatue, p5);
			puttolua(m_pStatue, p6); 
			puttolua(m_pStatue, p7);
			lua_pcall(m_pStatue, 7, 1, iErrorFuncIndex);

			lua_remove(m_pStatue, -2);
			return LuaTable(m_pStatue, -1);
		}

		template<class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8>
		stCLuaTable operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8) {
			lua_pushcclosure(m_pStatue, error_log, 0);
			int iErrorFuncIndex = lua_gettop(m_pStatue);

			lua_rawgeti(m_pStatue, LUA_REGISTRYINDEX, m_iRefIndex);
			puttolua(m_pStatue, p1); 
			puttolua(m_pStatue, p2);
			puttolua(m_pStatue, p3); 
			puttolua(m_pStatue, p4); 
			puttolua(m_pStatue, p5); 
			puttolua(m_pStatue, p6);
			puttolua(m_pStatue, p7); 
			puttolua(m_pStatue, p8);
			lua_pcall(m_pStatue, 8, 1, iErrorFuncIndex);

			lua_remove(m_pStatue, -2);
			return LuaTable(m_pStatue, -1);
		}

		template<class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9>
		stCLuaTable operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9) {
			lua_pushcclosure(m_pStatue, error_log, 0);
			int iErrorFuncIndex = lua_gettop(m_pStatue);

			lua_rawgeti(m_pStatue, LUA_REGISTRYINDEX, m_iRefIndex);
			puttolua(m_pStatue, p1); 
			puttolua(m_pStatue, p2); 
			puttolua(m_pStatue, p3); 
			puttolua(m_pStatue, p4);
			puttolua(m_pStatue, p5);
			puttolua(m_pStatue, p6); 
			puttolua(m_pStatue, p7); 
			puttolua(m_pStatue, p8);
			puttolua(m_pStatue, p9);
			lua_pcall(m_pStatue, 9, 1, iErrorFuncIndex);

			lua_remove(m_pStatue, -2);
			return LuaTable(m_pStatue, -1);
		}


		template<class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9,class P10>
		stCLuaTable operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9,P10 p10) {
			lua_pushcclosure(m_pStatue, error_log, 0);
			int iErrorFuncIndex = lua_gettop(m_pStatue);

			lua_rawgeti(m_pStatue, LUA_REGISTRYINDEX, m_iRefIndex);
			puttolua(m_pStatue, p1); 
			puttolua(m_pStatue, p2); 
			puttolua(m_pStatue, p3); 
			puttolua(m_pStatue, p4);
			puttolua(m_pStatue, p5);
			puttolua(m_pStatue, p6); 
			puttolua(m_pStatue, p7); 
			puttolua(m_pStatue, p8);
			puttolua(m_pStatue, p9);
			puttolua(m_pStatue, p10);
			lua_pcall(m_pStatue, 10, 1, iErrorFuncIndex);

			lua_remove(m_pStatue, -2);
			return LuaTable(m_pStatue, -1);
		}
	private:
		int m_iRefIndex;
		lua_State* m_pStatue;
		const char* pFuncName;
	};



	template<class T>
	struct stReadTypeFromLua{
		LUABRIDGER_FORCEINLINE static T GetValue(lua_State* m_Ptr,int iStackIndex){
			stUserData<T>* pUD = static_cast<stUserData<T>*>(lua_touserdata(m_Ptr,iStackIndex));
			return *((pUD)->m_Ptr);
		}
	};

	template<class T>
	struct stReadTypeFromLua<T*>{
		LUABRIDGER_FORCEINLINE static T* GetValue(lua_State* m_Ptr,int iStackIndex){
			stUserData<T>* pUD = static_cast<stUserData<T>*>(lua_touserdata(m_Ptr,iStackIndex));
			return (pUD)->m_Ptr;
		}
	};


	template<class T>
	struct stReadTypeFromLua<T&>{
		LUABRIDGER_FORCEINLINE static T& GetValue(lua_State* m_Ptr,int iStackIndex){
			stUserData<T>* pUD = static_cast<stUserData<T>*>(lua_touserdata(m_Ptr,iStackIndex));
			return *((pUD)->m_Ptr);
		}
	};


	template<class T>
	LUABRIDGER_FORCEINLINE T readfromlua(lua_State* m_Ptr, int iStackIndex){
		if (! lua_isuserdata(m_Ptr,iStackIndex))
		{
			lua_pushstring(m_Ptr," it is not a userdata!");
			lua_error(m_Ptr);
		}
		return stReadTypeFromLua<T>::GetValue(m_Ptr,iStackIndex);
	}

	//对于函数模版只有全特化没偏特
	template<>
	LUABRIDGER_FORCEINLINE void readfromlua(lua_State* m_Ptr, int iStackIndex){}

	template<>
	LUABRIDGER_FORCEINLINE bool readfromlua(lua_State* m_Ptr, int iStackIndex){return lua_toboolean(m_Ptr, iStackIndex) ? true : false;}

	template<>
	LUABRIDGER_FORCEINLINE char* readfromlua(lua_State* m_Ptr, int iStackIndex){return (char*)lua_tostring(m_Ptr, iStackIndex);}

	template<>
	LUABRIDGER_FORCEINLINE const char*	readfromlua(lua_State *m_Ptr, int iStackIndex) { return (const char*)lua_tostring(m_Ptr, iStackIndex);}

	template<>
	LUABRIDGER_FORCEINLINE char			readfromlua(lua_State *m_Ptr, int iStackIndex) { return (char)lua_tonumber(m_Ptr, iStackIndex);}

	template<>
	LUABRIDGER_FORCEINLINE unsigned char  readfromlua(lua_State *m_Ptr, int iStackIndex) { return (unsigned char)lua_tonumber(m_Ptr, iStackIndex);}

	template<>	
	LUABRIDGER_FORCEINLINE short		readfromlua(lua_State *m_Ptr, int iStackIndex) { return (short)lua_tonumber(m_Ptr, iStackIndex);}

	template<>
	LUABRIDGER_FORCEINLINE unsigned short readfromlua(lua_State *m_Ptr, int iStackIndex) { return (unsigned short)lua_tonumber(m_Ptr, iStackIndex);}

	template<>
	LUABRIDGER_FORCEINLINE long			readfromlua(lua_State *m_Ptr, int iStackIndex) { return (long)lua_tonumber(m_Ptr, iStackIndex);}

	template<>
	LUABRIDGER_FORCEINLINE unsigned long  readfromlua(lua_State *m_Ptr, int iStackIndex) { return (unsigned long)lua_tonumber(m_Ptr, iStackIndex);}

	template<>
	LUABRIDGER_FORCEINLINE int		           readfromlua(lua_State *m_Ptr, int iStackIndex) { return (int)lua_tonumber(m_Ptr, iStackIndex);}

	template<>
	LUABRIDGER_FORCEINLINE unsigned int	readfromlua(lua_State *m_Ptr, int iStackIndex) { return (unsigned int)lua_tonumber(m_Ptr, iStackIndex);}

	//template<>
	//LUABRIDGER_FORCEINLINE int64_t  readfromlua(lua_State *m_Ptr, int iStackIndex) { return (int64_t)lua_tonumber(m_Ptr, iStackIndex);}

	//template<> 
	//LUABRIDGER_FORCEINLINE uint64_t          readfromlua(lua_State *m_Ptr, int iStackIndex) { return (uint64_t)lua_tonumber(m_Ptr, iStackIndex);}

	template<>
	LUABRIDGER_FORCEINLINE float		          readfromlua(lua_State *m_Ptr, int iStackIndex) { return (float)lua_tonumber(m_Ptr, iStackIndex);}

	template<>
	LUABRIDGER_FORCEINLINE double		       readfromlua(lua_State *m_Ptr, int iStackIndex) { return (double)lua_tonumber(m_Ptr, iStackIndex);}

	template<>
	LUABRIDGER_FORCEINLINE CLuaString	        readfromlua(lua_State *m_Ptr, int iStackIndex) {CLuaString ls; ls.p_Str = (char*)lua_tolstring(m_Ptr, iStackIndex, &ls.sLen); return ls;}

	template<>	
	LUABRIDGER_FORCEINLINE stCLuaTable	    readfromlua(lua_State *m_Ptr, int iStackIndex) { return stCLuaTable(m_Ptr, iStackIndex);}

	template<class T>
	struct stPutTypeToLua{
		LUABRIDGER_FORCEINLINE static void PutValue(lua_State* m_Ptr,T& value){
			stUserData<T>*pUD = static_cast<stUserData<T>*>(lua_newuserdata(m_Ptr, sizeof(stUserData<T>)));
			T* ptr = new T(value);
			pUD->m_Ptr = ptr;
			pUD->m_DelWhenGC = true;
			luaL_getmetatable(L,ClassName<T>::getName());
			lua_setmetatable(m_Ptr,-2);
		}
	};


	template<class T>
	struct stPutTypeToLua<T*>{
		LUABRIDGER_FORCEINLINE static void PutValue(lua_State* m_Ptr,T* value){
			stUserData<T>*pUD = static_cast<stUserData<T>*>(lua_newuserdata(m_Ptr, sizeof(stUserData<T>)));
			pUD->m_Ptr = value;
			pUD->m_DelWhenGC = false;
			luaL_getmetatable(L,ClassName<T>::getName());
			lua_setmetatable(m_Ptr,-2);
		}
	};

	template<class T>
	struct stPutTypeToLua<T&>{
		LUABRIDGER_FORCEINLINE static void PutValue(lua_State* m_Ptr,T& value){
			stUserData<T>*pUD = static_cast<stUserData<T>*>(lua_newuserdata(L, sizeof(stUserData<T>)));
			pUD->m_Ptr = &value;
			pUD->m_DelWhenGC = true;
			luaL_getmetatable(L,ClassName<T>::getName());
			lua_setmetatable(m_Ptr,-2);
		}
	};

	template<class T>
	LUABRIDGER_FORCEINLINE void puttolua(lua_State* L, T value){
		stPutTypeToLua<T>::PutValue(L,value);
	}


	template<> 
	LUABRIDGER_FORCEINLINE void puttolua(lua_State *L, char value) { lua_pushnumber(L, value);}

	template<>
	LUABRIDGER_FORCEINLINE void puttolua(lua_State *L, unsigned char value) { lua_pushnumber(L, value);}

	template<>
	LUABRIDGER_FORCEINLINE void puttolua(lua_State *L, short value) { lua_pushnumber(L, value);}

	template<> 
	LUABRIDGER_FORCEINLINE void puttolua(lua_State *L, unsigned short value) { lua_pushnumber(L, value);}

	template<> 
	LUABRIDGER_FORCEINLINE void puttolua(lua_State *L, long value) { lua_pushnumber(L, value);}

	template<> 
	LUABRIDGER_FORCEINLINE void puttolua(lua_State *L, unsigned long value) { lua_pushnumber(L, value);}

	template<>
	LUABRIDGER_FORCEINLINE void puttolua(lua_State *L, int value) { lua_pushnumber(L, value);}

	template<> 
	LUABRIDGER_FORCEINLINE void puttolua(lua_State *L, unsigned int value) { lua_pushnumber(L, value);}

	template<> 
	LUABRIDGER_FORCEINLINE void puttolua(lua_State *L, float value) { lua_pushnumber(L, value);}

	template<>
	LUABRIDGER_FORCEINLINE void puttolua(lua_State *L, double value) { lua_pushnumber(L, value);}

	template<> 
	LUABRIDGER_FORCEINLINE void puttolua(lua_State *L, char* value) { lua_pushstring(L, value);}

	template<>
	LUABRIDGER_FORCEINLINE void puttolua(lua_State *L, const char* value) { lua_pushstring(L, value);}

	template<>
	LUABRIDGER_FORCEINLINE void puttolua(lua_State *L, bool value) { lua_pushboolean(L, value);}

	/*template<> 
	LUABRIDGER_FORCEINLINE void puttolua(lua_State *L, int64_t value) { return lua_pushnumber(L, (LUA_NUMBER)value);}

	template<>
	LUABRIDGER_FORCEINLINE void puttolua(lua_State *L, uint64_t value) { return lua_pushnumber(L, (LUA_NUMBER)value);}*/

	template<>
	LUABRIDGER_FORCEINLINE void puttolua(lua_State *L, CLuaString value) {lua_pushlstring(L, value.p_Str, value.sLen);}

	template<>
	LUABRIDGER_FORCEINLINE void puttolua(lua_State *L, stCLuaTable value) { if(value.m_pRefCount) lua_pushvalue(L, value.m_iLuaStackPos); else lua_pushnil(L);}





	void traceLuaStack(lua_State* L, int n) {
		lua_Debug ar;
		if(lua_getstack(L, n, &ar)) {
			lua_getinfo(L, "Sln", &ar);

			if(ar.name) {
				printf("\tstack[%d] -> line %d : %s()[%s : line %d]\n", n, ar.currentline, ar.name, ar.short_src, ar.linedefined);
			} else {
				printf("\tstack[%d] -> line %d : unknown[%s : line %d]\n", n, ar.currentline, ar.short_src, ar.linedefined);
			}
			traceLuaStack(L, n+1);
		}
	}

	int error_log(lua_State *L) {
		printf("error : %s\n", lua_tostring(L, -1));
		traceLuaStack(L, 0);
		return 0;
	}

	template<class T>
	int doGc(lua_State* m_Ptr){
		stUserData<T>* pUD = static_cast<stUserData<T>*>(luaL_checkudata(m_Ptr,-1,stClassName<T>::getName()));
		delete pUD;
		return 0;
	}


	lua_State* openLua() {
		lua_State *L = luaL_newstate();

		luaopen_base(L);
		luaL_openlibs(L);
		luaopen_debug(L);

		return L;
	}

	void closeLua(lua_State* L) {
		lua_close(L);
	}



	void doFile(lua_State *L, const char *fileName) {
		lua_pushcclosure(L, error_log, 0);
		int stackTop = lua_gettop(L);

		if(luaL_loadfile(L, fileName) == 0) {
			if(lua_pcall(L, 0, 0, stackTop)) {
				lua_pop(L, 1);
			}
		} else {
			printf("dofile error: %s\n", lua_tostring(L, -1));
			lua_pop(L, 1);
		}

		lua_pop(L, 1);
	}

	template<class T>
	const char* stClassName<T>::m_pClassName;

	struct stClassMethodInterface{

		stClassMethodInterface(const char* m_name):m_pMethodName(m_name){}
		virtual ~stClassMethodInterface(){}
		virtual int doCall(lua_State* L)=0;
		const char* m_pMethodName;
	};


	struct stGlobalMethodInterface{
		stGlobalMethodInterface(const char* m_name):m_pMethodName(m_name){}
		virtual ~stGlobalMethodInterface(){}
		virtual int doCall(lua_State*L)=0;
		const char* m_pMethodName;
	};


	template<class R,class T,class P1>
	struct stClassMethodBase;

	template<class R,class T>
	struct stClassMethodBase<R,T,void>: public stClassMethodInterface{
		typedef R(T::* pFunc) ();
		pFunc m_pFunc;
		stClassMethodBase(const char* m_name, pFunc func):m_pFunc(func),stClassMethodInterface(m_name){}
		~stClassMethodBase(){}
		virtual int doCall(lua_State* L){
			T* pObj = readfromlua<T*>(L,1);
			puttolua(L,(pObj->*m_pFunc)());
			return 1;
		}
	};

	template<class T>
	struct stClassMethodBase<void,T,void>: public stClassMethodInterface{
		typedef void (T::* pFunc) ();
		pFunc m_pFunc;
		stClassMethodBase(const char* m_name, pFunc func):m_pFunc(func),stClassMethodInterface(m_name){}
		~stClassMethodBase(){}
		virtual int doCall(lua_State* L){
			T* pObj = readfromlua<T*>(L,1);
			(pObj->*m_pFunc)();
			return 0;
		}
	};


	template<class R,class T,class P1>
	struct stClassMethodBase<R,T,TYPELIST_1(P1)>: public stClassMethodInterface{
		typedef R(T::* pFunc) (P1);
		pFunc m_pFunc;
		stClassMethodBase(const char* m_name, pFunc func):m_pFunc(func),stClassMethodInterface(m_name){}
		~stClassMethodBase(){}
		virtual int doCall(lua_State* L){
			T* pObj = readfromlua<T*>(L,1);
			puttolua(L,(pObj->*m_pFunc)(readfromlua<P1>(L,2)));
			return 1;
		}
	};

	template<class T,class P1>
	struct stClassMethodBase<void,T,TYPELIST_1(P1)>: public stClassMethodInterface{
		typedef void (T::* pFunc) (P1);
		pFunc m_pFunc;
		stClassMethodBase(const char* m_name, pFunc func):m_pFunc(func),stClassMethodInterface(m_name){}
		~stClassMethodBase(){}
		virtual int doCall(lua_State* L){
			T* pObj = readfromlua<T*>(L,1);
			(pObj->*m_pFunc)(readfromlua<P1>(L,2));
			return 0;
		}
	};

	template<class R,class T,class P1,class P2>
	struct stClassMethodBase<R,T,TYPELIST_2(P1,P2)>: public stClassMethodInterface{
		typedef R(T::* pFunc) (P1,P2);
		pFunc m_pFunc;
		stClassMethodBase(const char* m_name, pFunc func):m_pFunc(func),stClassMethodInterface(m_name){}
		~stClassMethodBase(){}
		virtual int doCall(lua_State* L){
			T* pObj = readfromlua<T*>(L,1);
			puttolua(L,(pObj->*m_pFunc)(readfromlua<P1>(L,2),readfromlua<P2>(L,3)));
			return 1;
		}
	};

	template<class T,class P1,class P2>
	struct stClassMethodBase<void,T,TYPELIST_2(P1,P2)>: public stClassMethodInterface{
		typedef void (T::* pFunc) (P1,P2);
		pFunc m_pFunc;
		stClassMethodBase(const char* m_name, pFunc func):m_pFunc(func),stClassMethodInterface(m_name){}
		~stClassMethodBase(){}
		virtual int doCall(lua_State* L){
			T* pObj = readfromlua<T*>(L,1);
			(pObj->*m_pFunc)(readfromlua<P1>(L,2),readfromlua<P2>(L,3));
			return 0;
		}
	};

	template<class R,class T,class P1,class P2,class P3>
	struct stClassMethodBase<R,T,TYPELIST_3(P1,P2,P3)>: public stClassMethodInterface{
		typedef R(T::* pFunc) (P1,P2,P3);
		pFunc m_pFunc;
		stClassMethodBase(const char* m_name, pFunc func):m_pFunc(func),stClassMethodInterface(m_name){}
		~stClassMethodBase(){}
		virtual int doCall(lua_State* L){
			T* pObj = readfromlua<T*>(L,1);
			puttolua(L,(pObj->*m_pFunc)(readfromlua<P1>(L,2),readfromlua<P2>(L,3),readfromlua<P3>(L,4)));
			return 1;
		}
	};
	template<class T,class P1,class P2,class P3>
	struct stClassMethodBase<void,T,TYPELIST_3(P1,P2,P3)>: public stClassMethodInterface{
		typedef void (T::* pFunc) (P1,P2,P3);
		pFunc m_pFunc;
		stClassMethodBase(const char* m_name, pFunc func):m_pFunc(func),stClassMethodInterface(m_name){}
		~stClassMethodBase(){}
		virtual int doCall(lua_State* L){
			T* pObj = readfromlua<T*>(L,1);
			(pObj->*m_pFunc)(readfromlua<P1>(L,2),readfromlua<P2>(L,3),readfromlua<P3>(L,4));
			return 0;
		}
	};

	template<class R,class T,class P1,class P2,class P3,class P4>
	struct stClassMethodBase<R,T,TYPELIST_4(P1,P2,P3,P4)>: public stClassMethodInterface{
		typedef R(T::* pFunc) (P1,P2,P3,P4);
		pFunc m_pFunc;
		stClassMethodBase(const char* m_name, pFunc func):m_pFunc(func),stClassMethodInterface(m_name){}
		~stClassMethodBase(){}
		virtual int doCall(lua_State* L){
			T* pObj = readfromlua<T*>(L,1);
			puttolua(L,(pObj->*m_pFunc)(readfromlua<P1>(L,2),readfromlua<P2>(L,3),readfromlua<P3>(L,4),readfromlua<P4>(L,5)));
			return 1;
		}
	};
	template<class T,class P1,class P2,class P3,class P4>
	struct stClassMethodBase<void,T,TYPELIST_4(P1,P2,P3,P4)>: public stClassMethodInterface{
		typedef void (T::* pFunc) (P1,P2,P3,P4);
		pFunc m_pFunc;
		stClassMethodBase(const char* m_name, pFunc func):m_pFunc(func),stClassMethodInterface(m_name){}
		~stClassMethodBase(){}
		virtual int doCall(lua_State* L){
			T* pObj = readfromlua<T*>(L,1);
			(pObj->*m_pFunc)(readfromlua<P1>(L,2),readfromlua<P2>(L,3),readfromlua<P3>(L,4),readfromlua<P4>(L,5));
			return 0;
		}
	};


	template<class R,class T,class P1,class P2,class P3,class P4,class P5>
	struct stClassMethodBase<R,T,TYPELIST_5(P1,P2,P3,P4,P5)>: public stClassMethodInterface{
		typedef R(T::* pFunc) (P1,P2,P3,P4,P5);
		pFunc m_pFunc;
		stClassMethodBase(const char* m_name, pFunc func):m_pFunc(func),stClassMethodInterface(m_name){}
		~stClassMethodBase(){}
		virtual int doCall(lua_State* L){
			T* pObj = readfromlua<T*>(L,1);
			puttolua(L,(pObj->*m_pFunc)(readfromlua<P1>(L,2),readfromlua<P2>(L,3),readfromlua<P3>(L,4),readfromlua<P4>(L,5),readfromlua<P5>(L,6)));
			return 1;
		}
	};

	template<class T,class P1,class P2,class P3,class P4,class P5>
	struct stClassMethodBase<void,T,TYPELIST_5(P1,P2,P3,P4,P5)>: public stClassMethodInterface{
		typedef void (T::* pFunc) (P1,P2,P3,P4,P5);
		pFunc m_pFunc;
		stClassMethodBase(const char* m_name, pFunc func):m_pFunc(func),stClassMethodInterface(m_name){}
		~stClassMethodBase(){}
		virtual int doCall(lua_State* L){
			T* pObj = readfromlua<T*>(L,1);
			(pObj->*m_pFunc)(readfromlua<P1>(L,2),readfromlua<P2>(L,3),readfromlua<P3>(L,4),readfromlua<P4>(L,5),readfromlua<P5>(L,6));
			return 0;
		}
	};
	template<class R,class T,class P1,class P2,class P3,class P4,class P5,class P6>
	struct stClassMethodBase<R,T,TYPELIST_6(P1,P2,P3,P4,P5,P6)>: public stClassMethodInterface{
		typedef R(T::* pFunc) (P1,P2,P3,P4,P5,P6);
		pFunc m_pFunc;
		stClassMethodBase(const char* m_name, pFunc func):m_pFunc(func),stClassMethodInterface(m_name){}
		~stClassMethodBase(){}
		virtual int doCall(lua_State* L){
			T* pObj = readfromlua<T*>(L,1);
			puttolua(L,(pObj->*m_pFunc)(readfromlua<P1>(L,2),readfromlua<P2>(L,3),readfromlua<P3>(L,4),readfromlua<P4>(L,5),readfromlua<P5>(L,6),readfromlua<P6>(L,7)));
			return 1;
		}
	};

	template<class T,class P1,class P2,class P3,class P4,class P5,class P6>
	struct stClassMethodBase<void,T,TYPELIST_6(P1,P2,P3,P4,P5,P6)>: public stClassMethodInterface{
		typedef void (T::* pFunc) (P1,P2,P3,P4,P5,P6);
		pFunc m_pFunc;
		stClassMethodBase(const char* m_name, pFunc func):m_pFunc(func),stClassMethodInterface(m_name){}
		~stClassMethodBase(){}
		virtual int doCall(lua_State* L){
			T* pObj = readfromlua<T*>(L,1);
			(pObj->*m_pFunc)(readfromlua<P1>(L,2),readfromlua<P2>(L,3),readfromlua<P3>(L,4),readfromlua<P4>(L,5),readfromlua<P5>(L,6),readfromlua<P6>(L,7));
			return 0;
		}
	};
	template<class R,class T,class P1,class P2,class P3,class P4,class P5,class P6,class P7>
	struct stClassMethodBase<R,T,TYPELIST_7(P1,P2,P3,P4,P5,P6,P7)>: public stClassMethodInterface{
		typedef R(T::* pFunc) (P1,P2,P3,P4,P5,P6,P7);
		pFunc m_pFunc;
		stClassMethodBase(const char* m_name, pFunc func):m_pFunc(func),stClassMethodInterface(m_name){}
		~stClassMethodBase(){}
		virtual int doCall(lua_State* L){
			T* pObj = readfromlua<T*>(L,1);
			puttolua(L,(pObj->*m_pFunc)(readfromlua<P1>(L,2),readfromlua<P2>(L,3),readfromlua<P3>(L,4),readfromlua<P4>(L,5),readfromlua<P5>(L,6),readfromlua<P6>(L,7),readfromlua<P7>(L,8)));
			return 1;
		}
	};

	template<class T,class P1,class P2,class P3,class P4,class P5,class P6,class P7>
	struct stClassMethodBase<void,T,TYPELIST_7(P1,P2,P3,P4,P5,P6,P7)>: public stClassMethodInterface{
		typedef void (T::* pFunc) (P1,P2,P3,P4,P5,P6,P7);
		pFunc m_pFunc;
		stClassMethodBase(const char* m_name, pFunc func):m_pFunc(func),stClassMethodInterface(m_name){}
		~stClassMethodBase(){}
		virtual int doCall(lua_State* L){
			T* pObj = readfromlua<T*>(L,1);
			(pObj->*m_pFunc)(readfromlua<P1>(L,2),readfromlua<P2>(L,3),readfromlua<P3>(L,4),readfromlua<P4>(L,5),readfromlua<P5>(L,6),readfromlua<P6>(L,7),readfromlua<P7>(L,8));
			return 0;
		}
	};
	template<class R,class T,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8>
	struct stClassMethodBase<R,T,TYPELIST_8(P1,P2,P3,P4,P5,P6,P7,P8)>: public stClassMethodInterface{
		typedef R(T::* pFunc) (P1,P2,P3,P4,P5,P6,P7,P8);
		pFunc m_pFunc;
		stClassMethodBase(const char* m_name, pFunc func):m_pFunc(func),stClassMethodInterface(m_name){}
		~stClassMethodBase(){}
		virtual int doCall(lua_State* L){
			T* pObj = readfromlua<T*>(L,1);
			puttolua(L,(pObj->*m_pFunc)(readfromlua<P1>(L,2),readfromlua<P2>(L,3),readfromlua<P3>(L,4),readfromlua<P4>(L,5),readfromlua<P5>(L,6),readfromlua<P6>(L,7),readfromlua<P7>(L,8),readfromlua<P8>(L,9)));
			return 1;
		}
	};
	template<class T,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8>
	struct stClassMethodBase<void,T,TYPELIST_8(P1,P2,P3,P4,P5,P6,P7,P8)>: public stClassMethodInterface{
		typedef void (T::* pFunc) (P1,P2,P3,P4,P5,P6,P7,P8);
		pFunc m_pFunc;
		stClassMethodBase(const char* m_name, pFunc func):m_pFunc(func),stClassMethodInterface(m_name){}
		~stClassMethodBase(){}
		virtual int doCall(lua_State* L){
			T* pObj = readfromlua<T*>(L,1);
			(pObj->*m_pFunc)(readfromlua<P1>(L,2),readfromlua<P2>(L,3),readfromlua<P3>(L,4),readfromlua<P4>(L,5),readfromlua<P5>(L,6),readfromlua<P6>(L,7),readfromlua<P7>(L,8),readfromlua<P8>(L,9));
			return 0;
		}
	};
	template<class R,class T,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8,class P9>
	struct stClassMethodBase<R,T,TYPELIST_9(P1,P2,P3,P4,P5,P6,P7,P8,P9)>: public stClassMethodInterface{
		typedef R(T::* pFunc) (P1,P2,P3,P4,P5,P6,P7,P8,P9);
		pFunc m_pFunc;
		stClassMethodBase(const char* m_name, pFunc func):m_pFunc(func),stClassMethodInterface(m_name){}
		~stClassMethodBase(){}
		virtual int doCall(lua_State* L){
			T* pObj = readfromlua<T*>(L,1);
			puttolua(L,(pObj->*m_pFunc)(readfromlua<P1>(L,2),readfromlua<P2>(L,3),readfromlua<P3>(L,4),readfromlua<P4>(L,5),readfromlua<P5>(L,6),readfromlua<P6>(L,7),readfromlua<P7>(L,8),readfromlua<P8>(L,9),readfromlua<P9>(L,10)));
			return 1;
		}
	};
	template<class T,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8,class P9>
	struct stClassMethodBase<void,T,TYPELIST_9(P1,P2,P3,P4,P5,P6,P7,P8,P9)>: public stClassMethodInterface{
		typedef void (T::* pFunc) (P1,P2,P3,P4,P5,P6,P7,P8,P9);
		pFunc m_pFunc;
		stClassMethodBase(const char* m_name, pFunc func):m_pFunc(func),stClassMethodInterface(m_name){}
		~stClassMethodBase(){}
		virtual int doCall(lua_State* L){
			T* pObj = readfromlua<T*>(L,1);
			(pObj->*m_pFunc)(readfromlua<P1>(L,2),readfromlua<P2>(L,3),readfromlua<P3>(L,4),readfromlua<P4>(L,5),readfromlua<P5>(L,6),readfromlua<P6>(L,7),readfromlua<P7>(L,8),readfromlua<P8>(L,9),readfromlua<P9>(L,10));
			return 0;
		}
	};
	template<class R,class T,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8,class P9,class P10>
	struct stClassMethodBase<R,T,TYPELIST_10(P1,P2,P3,P4,P5,P6,P7,P8,P9,P10)>: public stClassMethodInterface{
		typedef R(T::* pFunc) (P1,P2,P3,P4,P5,P6,P7,P8,P9,P10);
		pFunc m_pFunc;
		stClassMethodBase(const char* m_name, pFunc func):m_pFunc(func),stClassMethodInterface(m_name){}
		~stClassMethodBase(){}
		virtual int doCall(lua_State* L){
			T* pObj = readfromlua<T*>(L,1);
			puttolua(L,(pObj->*m_pFunc)(readfromlua<P1>(L,2),readfromlua<P2>(L,3),readfromlua<P3>(L,4),readfromlua<P4>(L,5),readfromlua<P5>(L,6),readfromlua<P6>(L,7),readfromlua<P7>(L,8),readfromlua<P8>(L,9),readfromlua<P9>(L,10),readfromlua<P10 >(L,11)));
		
			return 1;
		}
	};

	template<class T,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8,class P9,class P10>
	struct stClassMethodBase<void,T,TYPELIST_10(P1,P2,P3,P4,P5,P6,P7,P8,P9,P10)>: public stClassMethodInterface{
		typedef void (T::* pFunc) (P1,P2,P3,P4,P5,P6,P7,P8,P9,P10);
		pFunc m_pFunc;
		stClassMethodBase(const char* m_name, pFunc func):m_pFunc(func),stClassMethodInterface(m_name){}
		~stClassMethodBase(){}
		virtual int doCall(lua_State* L){
			T* pObj = readfromlua<T*>(L,1);
			(pObj->*m_pFunc)(readfromlua<P1>(L,2),readfromlua<P2>(L,3),readfromlua<P3>(L,4),readfromlua<P4>(L,5),readfromlua<P5>(L,6),readfromlua<P6>(L,7),readfromlua<P7>(L,8),readfromlua<P8>(L,9),readfromlua<P9>(L,10),readfromlua<P10 >(L,11));

			return 0;
		}
	};
	//这里只支持到10个参数，想支持更多可以继续进行偏特化



	template<class R,class P>
	struct stGlobalMethodBase;

	template<class R>
	struct stGlobalMethodBase<R,void>: public stGlobalMethodInterface{
		typedef R (* pFunc)();
		pFunc m_pFunc;
		stGlobalMethodBase( const char* name, pFunc func): m_pFunc(func), stGlobalMethodInterface(name) {};
		~stGlobalMethodBase() {};
		virtual int doCall(lua_State *L) {
			puttolua(L, (*m_pFunc)());
			
			return 1;
		};
	};

	template<>
	struct stGlobalMethodBase<void ,void>: public stGlobalMethodInterface{
		typedef void (* pFunc)();
		pFunc m_pFunc;
		stGlobalMethodBase( const char* name, pFunc func): m_pFunc(func), stGlobalMethodInterface(name) {};
		~stGlobalMethodBase() {};
		virtual int doCall(lua_State *L) {
		 (*m_pFunc)();
			return 0;
		};
	};

	template<class R,class P1>
	struct stGlobalMethodBase<R,TYPELIST_1(P1)>: public stGlobalMethodInterface{
		typedef R (* pFunc)(P1);
		pFunc m_pFunc;
		stGlobalMethodBase( const char* name, pFunc func): m_pFunc(func), stGlobalMethodInterface(name) {};
		~stGlobalMethodBase() {};
		virtual int doCall(lua_State *L) {
			puttolua(L, (*m_pFunc)(readfromlua<P1>(L,1)));
		
			return 1;
		};
	};
	template<class P1>
	struct stGlobalMethodBase<void ,TYPELIST_1(P1)>: public stGlobalMethodInterface{
		typedef void  (* pFunc)(P1);
		pFunc m_pFunc;
		stGlobalMethodBase( const char* name, pFunc func): m_pFunc(func), stGlobalMethodInterface(name) {};
		~stGlobalMethodBase() {};
		virtual int doCall(lua_State *L) {
			 (*m_pFunc)(readfromlua<P1>(L,1));

			return 0;
		};
	};


	template<class R,class P1,class P2>
	struct stGlobalMethodBase<R,TYPELIST_2(P1,P2)>: public stGlobalMethodInterface{
		typedef R (* pFunc)(P1,P2);
		pFunc m_pFunc;
		stGlobalMethodBase( const char* name, pFunc func): m_pFunc(func), stGlobalMethodInterface(name) {};
		~stGlobalMethodBase() {};
		virtual int doCall(lua_State *L) {
			puttolua(L, (*m_pFunc)(readfromlua<P1>(L,1),readfromlua<P2>(L,2)));
			
			return 1;
		};
	};

	template<class P1,class P2>
	struct stGlobalMethodBase<void,TYPELIST_2(P1,P2)>: public stGlobalMethodInterface{
		typedef void (* pFunc)(P1,P2);
		pFunc m_pFunc;
		stGlobalMethodBase( const char* name, pFunc func): m_pFunc(func), stGlobalMethodInterface(name) {};
		~stGlobalMethodBase() {};
		virtual int doCall(lua_State *L) {
			(*m_pFunc)(readfromlua<P1>(L,1),readfromlua<P2>(L,2));

			return 0;
		};
	};
	template<class R,class P1,class P2,class P3>
	struct stGlobalMethodBase<R,TYPELIST_3(P1,P2,P3)>: public  stGlobalMethodInterface{
		typedef R (* pFunc)(P1,P2,P3);
		pFunc m_pFunc;
		stGlobalMethodBase( const char* name, pFunc func): m_pFunc(func), stGlobalMethodInterface(name) {};
		~stGlobalMethodBase() {};
		virtual int doCall(lua_State *L) {
			puttolua(L, (*m_pFunc)(readfromlua<P1>(L,1),readfromlua<P2>(L,2),readfromlua<P3>(L,3)));
			
			return 1;
		};
	};

	template<class P1,class P2,class P3>
	struct stGlobalMethodBase<void,TYPELIST_3(P1,P2,P3)>: public  stGlobalMethodInterface{
		typedef void (* pFunc)(P1,P2,P3);
		pFunc m_pFunc;
		stGlobalMethodBase( const char* name, pFunc func): m_pFunc(func), stGlobalMethodInterface(name) {};
		~stGlobalMethodBase() {};
		virtual int doCall(lua_State *L) {
			(*m_pFunc)(readfromlua<P1>(L,1),readfromlua<P2>(L,2),readfromlua<P3>(L,3));

			return 0;
		};
	};


	template<class R,class P1,class P2,class P3,class P4>
	struct stGlobalMethodBase<R,TYPELIST_4(P1,P2,P3,P4)>: public stGlobalMethodInterface{
		typedef R (* pFunc)(P1,P2,P3,P4);
		pFunc m_pFunc;
		stGlobalMethodBase( const char* name, pFunc func): m_pFunc(func), stGlobalMethodInterface(name) {};
		~stGlobalMethodBase() {};
		virtual int doCall(lua_State *L) {
			puttolua(L, (*m_pFunc)(readfromlua<P1>(L,1),readfromlua<P2>(L,2),readfromlua<P3>(L,3),readfromlua<P4>(L,4)));
			
			return 1;
		};
	};

	template<class P1,class P2,class P3,class P4>
	struct stGlobalMethodBase<void,TYPELIST_4(P1,P2,P3,P4)>: public stGlobalMethodInterface{
		typedef void (* pFunc)(P1,P2,P3,P4);
		pFunc m_pFunc;
		stGlobalMethodBase( const char* name, pFunc func): m_pFunc(func), stGlobalMethodInterface(name) {};
		~stGlobalMethodBase() {};
		virtual int doCall(lua_State *L) {
			 (*m_pFunc)(readfromlua<P1>(L,1),readfromlua<P2>(L,2),readfromlua<P3>(L,3),readfromlua<P4>(L,4));

			return 0;
		};
	};

	template<class R,class P1,class P2,class P3,class P4,class P5>
	struct stGlobalMethodBase<R,TYPELIST_5(P1,P2,P3,P4,P5)>: public stGlobalMethodInterface{
		typedef R (* pFunc)(P1,P2,P3,P4,P5);
		pFunc m_pFunc;
		stGlobalMethodBase( const char* name, pFunc func): m_pFunc(func), stGlobalMethodInterface(name) {};
		~stGlobalMethodBase() {};
		virtual int doCall(lua_State *L) {
			puttolua(L, (*m_pFunc)(readfromlua<P1>(L,1),readfromlua<P2>(L,2),readfromlua<P3>(L,3),readfromlua<P4>(L,4),readfromlua<P5>(L,5)));
			
			return 1;
		};
	};

	template<class P1,class P2,class P3,class P4,class P5>
	struct stGlobalMethodBase<void,TYPELIST_5(P1,P2,P3,P4,P5)>: public stGlobalMethodInterface{
		typedef void (* pFunc)(P1,P2,P3,P4,P5);
		pFunc m_pFunc;
		stGlobalMethodBase( const char* name, pFunc func): m_pFunc(func), stGlobalMethodInterface(name) {};
		~stGlobalMethodBase() {};
		virtual int doCall(lua_State *L) {
			 (*m_pFunc)(readfromlua<P1>(L,1),readfromlua<P2>(L,2),readfromlua<P3>(L,3),readfromlua<P4>(L,4),readfromlua<P5>(L,5));

			return 0;
		};
	};

	template<class R,class P1,class P2,class P3,class P4,class P5,class P6>
	struct stGlobalMethodBase<R,TYPELIST_6(P1,P2,P3,P4,P5,P6)>: public stGlobalMethodInterface{
		typedef R (* pFunc)(P1,P2,P3,P4,P5,P6);
		pFunc m_pFunc;
		stGlobalMethodBase( const char* name, pFunc func): m_pFunc(func), stGlobalMethodInterface(name) {};
		~stGlobalMethodBase() {};
		virtual int doCall(lua_State *L) {
			puttolua(L, (*m_pFunc)(readfromlua<P1>(L,1),readfromlua<P2>(L,2),readfromlua<P3>(L,3),readfromlua<P4>(L,4),readfromlua<P5>(L,5),readfromlua<P6>(L,6)));
			
			return 1;
		};
	};

	template<class P1,class P2,class P3,class P4,class P5,class P6>
	struct stGlobalMethodBase<void,TYPELIST_6(P1,P2,P3,P4,P5,P6)>: public stGlobalMethodInterface{
		typedef void (* pFunc)(P1,P2,P3,P4,P5,P6);
		pFunc m_pFunc;
		stGlobalMethodBase( const char* name, pFunc func): m_pFunc(func), stGlobalMethodInterface(name) {};
		~stGlobalMethodBase() {};
		virtual int doCall(lua_State *L) {
			(*m_pFunc)(readfromlua<P1>(L,1),readfromlua<P2>(L,2),readfromlua<P3>(L,3),readfromlua<P4>(L,4),readfromlua<P5>(L,5),readfromlua<P6>(L,6));

			return 0;
		};
	};
	template<class R,class P1,class P2,class P3,class P4,class P5,class P6,class P7>
	struct stGlobalMethodBase<R,TYPELIST_7(P1,P2,P3,P4,P5,P6,P7)>: public stGlobalMethodInterface{
		typedef R (* pFunc)(P1,P2,P3,P4,P5,P6,P7);
		pFunc m_pFunc;
		stGlobalMethodBase( const char* name, pFunc func): m_pFunc(func), stGlobalMethodInterface(name) {};
		~stGlobalMethodBase() {};
		virtual int doCall(lua_State *L) {
			puttolua(L, (*m_pFunc)(readfromlua<P1>(L,1),readfromlua<P2>(L,2),readfromlua<P3>(L,3),readfromlua<P4>(L,4),readfromlua<P5>(L,5),readfromlua<P6>(L,6),readfromlua<P7>(L,7)));
		
			return 1;
		};
	};

	template<class P1,class P2,class P3,class P4,class P5,class P6,class P7>
	struct stGlobalMethodBase<void,TYPELIST_7(P1,P2,P3,P4,P5,P6,P7)>: public stGlobalMethodInterface{
		typedef void (* pFunc)(P1,P2,P3,P4,P5,P6,P7);
		pFunc m_pFunc;
		stGlobalMethodBase( const char* name, pFunc func): m_pFunc(func), stGlobalMethodInterface(name) {};
		~stGlobalMethodBase() {};
		virtual int doCall(lua_State *L) {
			(*m_pFunc)(readfromlua<P1>(L,1),readfromlua<P2>(L,2),readfromlua<P3>(L,3),readfromlua<P4>(L,4),readfromlua<P5>(L,5),readfromlua<P6>(L,6),readfromlua<P7>(L,7));

			return 0;
		};
	};

	template<class R,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8>
	struct stGlobalMethodBase<R,TYPELIST_8(P1,P2,P3,P4,P5,P6,P7,P8)>: public stGlobalMethodInterface{
		typedef R (* pFunc)(P1,P2,P3,P4,P5,P6,P7,P8);
		pFunc m_pFunc;
		stGlobalMethodBase( const char* name, pFunc func): m_pFunc(func), stGlobalMethodInterface(name) {};
		~stGlobalMethodBase() {};
		virtual int doCall(lua_State *L) {
			puttolua(L, (*m_pFunc)(readfromlua<P1>(L,1),readfromlua<P2>(L,2),readfromlua<P3>(L,3),readfromlua<P4>(L,4),readfromlua<P5>(L,5),readfromlua<P6>(L,6),readfromlua<P7>(L,7),readfromlua<P8>(L,8)));
			
			return 1;
		};
	};
	template<class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8>
	struct stGlobalMethodBase<void,TYPELIST_8(P1,P2,P3,P4,P5,P6,P7,P8)>: public stGlobalMethodInterface{
		typedef void (* pFunc)(P1,P2,P3,P4,P5,P6,P7,P8);
		pFunc m_pFunc;
		stGlobalMethodBase( const char* name, pFunc func): m_pFunc(func), stGlobalMethodInterface(name) {};
		~stGlobalMethodBase() {};
		virtual int doCall(lua_State *L) {
			(*m_pFunc)(readfromlua<P1>(L,1),readfromlua<P2>(L,2),readfromlua<P3>(L,3),readfromlua<P4>(L,4),readfromlua<P5>(L,5),readfromlua<P6>(L,6),readfromlua<P7>(L,7),readfromlua<P8>(L,8));

			return 0;
		};
	};
	template<class R,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8,class P9>
	struct stGlobalMethodBase<R,TYPELIST_9(P1,P2,P3,P4,P5,P6,P7,P8,P9)>: public stGlobalMethodInterface{
		typedef R (* pFunc)(P1,P2,P3,P4,P5,P6,P7,P8,P9);
		pFunc m_pFunc;
		stGlobalMethodBase( const char* name, pFunc func): m_pFunc(func), stGlobalMethodInterface(name) {};
		~stGlobalMethodBase() {};
		virtual int doCall(lua_State *L) {
			puttolua(L, (*m_pFunc)(readfromlua<P1>(L,1),readfromlua<P2>(L,2),readfromlua<P3>(L,3),readfromlua<P4>(L,4),readfromlua<P5>(L,5),readfromlua<P6>(L,6),readfromlua<P7>(L,7),readfromlua<P8>(L,8),readfromlua<P9>(L,9)));
			
			return 1;
		};
	};
	template<class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8,class P9>
	struct stGlobalMethodBase<void,TYPELIST_9(P1,P2,P3,P4,P5,P6,P7,P8,P9)>: public stGlobalMethodInterface{
		typedef void (* pFunc)(P1,P2,P3,P4,P5,P6,P7,P8,P9);
		pFunc m_pFunc;
		stGlobalMethodBase( const char* name, pFunc func): m_pFunc(func), stGlobalMethodInterface(name) {};
		~stGlobalMethodBase() {};
		virtual int doCall(lua_State *L) {
			(*m_pFunc)(readfromlua<P1>(L,1),readfromlua<P2>(L,2),readfromlua<P3>(L,3),readfromlua<P4>(L,4),readfromlua<P5>(L,5),readfromlua<P6>(L,6),readfromlua<P7>(L,7),readfromlua<P8>(L,8),readfromlua<P9>(L,9));

			return 0;
		};
	};

	template<class R,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8,class P9,class P10>
	struct stGlobalMethodBase<R,TYPELIST_10(P1,P2,P3,P4,P5,P6,P7,P8,P9,P10)>: public stGlobalMethodInterface{
		typedef R (* pFunc)(P1,P2,P3,P4,P5,P6,P7,P8,P9,P10);
		pFunc m_pFunc;
		stGlobalMethodBase( const char* name, pFunc func): m_pFunc(func), stGlobalMethodInterface(name) {};
		~stGlobalMethodBase() {};
		virtual int doCall(lua_State *L) {
			puttolua(L, (*m_pFunc)(readfromlua<P1>(L,1),readfromlua<P2>(L,2),readfromlua<P3>(L,3),readfromlua<P4>(L,4),readfromlua<P5>(L,5),readfromlua<P6>(L,6),readfromlua<P7>(L,7),readfromlua<P8>(L,8),readfromlua<P9>(L,9),readfromlua<P10>(L,10)));
			
			return 1;
		};
	};

	template<class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8,class P9,class P10>
	struct stGlobalMethodBase<void,TYPELIST_10(P1,P2,P3,P4,P5,P6,P7,P8,P9,P10)>: public stGlobalMethodInterface{
		typedef void (* pFunc)(P1,P2,P3,P4,P5,P6,P7,P8,P9,P10);
		pFunc m_pFunc;
		stGlobalMethodBase( const char* name, pFunc func): m_pFunc(func), stGlobalMethodInterface(name) {};
		~stGlobalMethodBase() {};
		virtual int doCall(lua_State *L) {
			(*m_pFunc)(readfromlua<P1>(L,1),readfromlua<P2>(L,2),readfromlua<P3>(L,3),readfromlua<P4>(L,4),readfromlua<P5>(L,5),readfromlua<P6>(L,6),readfromlua<P7>(L,7),readfromlua<P8>(L,8),readfromlua<P9>(L,9),readfromlua<P10>(L,10));

			return 0;
		};
	};
	template<typename T>
	LUABRIDGER_FORCEINLINE int doConstructorAndRegisterUserDataInLua(lua_State *L, T* pObj) {
		stUserData<T>*pUD = static_cast<stUserData<T>*>(lua_newuserdata(L, sizeof(stUserData<T>)));
		pUD->m_Ptr = pObj;
		luaL_getmetatable(L, stClassName<T>::getName());
		lua_setmetatable(L, -2);

		return 1;
	}

	template<class T>
	LUABRIDGER_FORCEINLINE int constructorUsedInLua(lua_State *L) {
		return doConstructorAndRegisterUserDataInLua<T>(L, new T());
	}

	template<class T, class P1>
	LUABRIDGER_FORCEINLINE int constructorUsedInLua(lua_State *L) {
		return doConstructorAndRegisterUserDataInLua<T>(L, new T(readfromlua<P1>(L,1)));
	}

	template<class T, class P1, class P2>
	LUABRIDGER_FORCEINLINE int constructorUsedInLua(lua_State *L) {
		return doConstructorAndRegisterUserDataInLua<T>(L, new T(readfromlua<P1>(L, 1), readfromlua<P2>(L, 2)));
	}

	template<class T, class P1, class P2, class P3>
	LUABRIDGER_FORCEINLINE int constructorUsedInLua(lua_State *L) {
		return doConstructorAndRegisterUserDataInLua<T>(L, new T(readfromlua<P1>(L, 1), readfromlua<P2>(L, 2), readfromlua<P3>(L, 3)));
	}

	template<class T, class P1, class P2, class P3, class P4>
	LUABRIDGER_FORCEINLINE int constructorUsedInLua(lua_State *L) {
		return doConstructorAndRegisterUserDataInLua<T>(L, new T(readfromlua<P1>(L, 1), readfromlua<P2>(L, 2), readfromlua<P3>(L, 3), readfromlua<P4>(L, 4)));
	}

	template<class T, class P1, class P2, class P3, class P4, class P5>
	LUABRIDGER_FORCEINLINE int constructorUsedInLua(lua_State *L) {
		return doConstructorAndRegisterUserDataInLua<T>(L, new T(readfromlua<P1>(L, 1), readfromlua<P2>(L, 2), readfromlua<P3>(L, 3), readfromlua<P4>(L, 4), readfromlua<P5>(L, 5)));
	}

	template<class T, class P1, class P2, class P3, class P4, class P5, class P6>
	LUABRIDGER_FORCEINLINE int constructorUsedInLua(lua_State *L) {
		return doConstructorAndRegisterUserDataInLua<T>(L, new T(readfromlua<P1>(L, 1), readfromlua<P2>(L, 2), readfromlua<P3>(L, 3), readfromlua<P4>(L, 4), readfromlua<P5>(L, 5), readfromlua<P6>(L, 6)));
	}

	template<class T, class P1, class P2, class P3, class P4, class P5, class P6, class P7>
	LUABRIDGER_FORCEINLINE int constructorUsedInLua(lua_State *L) {
		return doConstructorAndRegisterUserDataInLua<T>(L, new T(readfromlua<P1>(L, 1), readfromlua<P2>(L, 2), readfromlua<P3>(L, 3), readfromlua<P4>(L, 4), readfromlua<P5>(L, 5), readfromlua<P6>(L, 6), readfromlua<P7>(L, 7)));
	}

	template<class T, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8>
	LUABRIDGER_FORCEINLINE int constructorUsedInLua(lua_State *L) {
		return doConstructorAndRegisterUserDataInLua<T>(L, new T(readfromlua<P1>(L, 1), readfromlua<P2>(L, 2), readfromlua<P3>(L, 3), readfromlua<P4>(L, 4), readfromlua<P5>(L, 5), readfromlua<P6>(L, 6), readfromlua<P7>(L, 7), readfromlua<P8>(L, 8)));
	}

	template<class T, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9>
	LUABRIDGER_FORCEINLINE int constructorUsedInLua(lua_State *L) {
		return doConstructorAndRegisterUserDataInLua<T>(L, new T(readfromlua<P1>(L, 1), readfromlua<P2>(L, 2), readfromlua<P3>(L, 3), readfromlua<P4>(L, 4), readfromlua<P5>(L, 5), readfromlua<P6>(L, 6), readfromlua<P7>(L, 7), readfromlua<P8>(L, 8), readfromlua<P9>(L, 9)));
	}


	template<class T, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9,class P10>
	LUABRIDGER_FORCEINLINE int constructorUsedInLua(lua_State *L) {
		return doConstructorAndRegisterUserDataInLua<T>(L, new T(readfromlua<P1>(L, 1), readfromlua<P2>(L, 2), readfromlua<P3>(L, 3), readfromlua<P4>(L, 4), readfromlua<P5>(L, 5), readfromlua<P6>(L, 6), readfromlua<P7>(L, 7), readfromlua<P8>(L, 8), readfromlua<P9>(L, 9),readfromlua<P10>(L,10)));
	}


	LUABRIDGER_FORCEINLINE int doCallClassMethodInLua(lua_State *L) {
		stClassMethodInterface* pMethod = (stClassMethodInterface*)lua_touserdata(L, lua_upvalueindex(1));
		return pMethod->doCall(L); // execute method
	}



	template<class T, class R>
	LUABRIDGER_FORCEINLINE void registerClassMethod(lua_State* L, const char* pMethodName, R (T::*func)()) {
		luaL_getmetatable(L, stClassName<T>::getName());

		if(lua_istable(L, -1)) {
			stClassMethodInterface* pMethodObj = new stClassMethodBase<R,T,void>(pMethodName,func);
			lua_pushstring(L, pMethodName);
			lua_pushlightuserdata(L, pMethodObj);
			lua_pushcclosure(L, doCallClassMethodInLua, 1);
			lua_rawset(L, -3);
		} else {
			printf("please register class %s\n",  stClassName<T>::getName());
		}
		lua_pop(L, 1);
	}


	template<class T, class R, class P1>
	LUABRIDGER_FORCEINLINE void registerClassMethod(lua_State* L, const char* pMethodName, R (T::*func)(P1)) {
		luaL_getmetatable(L, stClassName<T>::getName());

		if(lua_istable(L, -1)) {
			stClassMethodInterface* pMethodObj = new stClassMethodBase<R,T,TYPELIST_1(P1)>(pMethodName,func);
			lua_pushstring(L, pMethodName);
			lua_pushlightuserdata(L, pMethodObj);
			lua_pushcclosure(L, doCallClassMethodInLua, 1);
			lua_rawset(L, -3);
		} else {
			printf("please register class %s\n",  stClassName<T>::getName());
		}
		lua_pop(L, 1);
	}


	template<class T, class R, class P1,class P2>
	LUABRIDGER_FORCEINLINE void registerClassMethod(lua_State* L, const char* pMethodName, R (T::*func)(P1,P2)) {
		luaL_getmetatable(L, stClassName<T>::getName());

		if(lua_istable(L, -1)) {
			stClassMethodInterface* pMethodObj = new stClassMethodBase<R,T,TYPELIST_2(P1,P2)>(pMethodName,func);
			lua_pushstring(L, pMethodName);
			lua_pushlightuserdata(L, pMethodObj);
			lua_pushcclosure(L, doCallClassMethodInLua, 1);
			lua_rawset(L, -3);
		} else {
			printf("please register class %s\n",  stClassName<T>::getName());
		}
		lua_pop(L, 1);
	}

	template<class T, class R, class P1,class P2,class P3>
	LUABRIDGER_FORCEINLINE void registerClassMethod(lua_State* L, const char* pMethodName, R (T::*func)(P1,P2,P3)) {
		luaL_getmetatable(L, stClassName<T>::getName());

		if(lua_istable(L, -1)) {
			stClassMethodInterface* pMethodObj = new stClassMethodBase<R,T,TYPELIST_3(P1,P2,P3)>(pMethodName,func);
			lua_pushstring(L, pMethodName);
			lua_pushlightuserdata(L, pMethodObj);
			lua_pushcclosure(L, doCallClassMethodInLua, 1);
			lua_rawset(L, -3);
		} else {
			printf("please register class %s\n",  stClassName<T>::getName());
		}
		lua_pop(L, 1);
	}

	template<class T, class R, class P1,class P2,class P3,class P4>
	LUABRIDGER_FORCEINLINE void registerClassMethod(lua_State* L, const char* pMethodName, R (T::*func)(P1,P2,P3,P4)) {
		luaL_getmetatable(L, stClassName<T>::getName());

		if(lua_istable(L, -1)) {
			stClassMethodInterface* pMethodObj = new stClassMethodBase<R,T,TYPELIST_4(P1,P2,P3,P4)>(pMethodName,func);
			lua_pushstring(L, pMethodName);
			lua_pushlightuserdata(L, pMethodObj);
			lua_pushcclosure(L, doCallClassMethodInLua, 1);
			lua_rawset(L, -3);
		} else {
			printf("please register class %s\n",  stClassName<T>::getName());
		}
		lua_pop(L, 1);
	}


	template<class T, class R, class P1,class P2,class P3,class P4,class P5>
	LUABRIDGER_FORCEINLINE void registerClassMethod(lua_State* L, const char* pMethodName, R (T::*func)(P1,P2,P3,P4,P5)) {
		luaL_getmetatable(L, stClassName<T>::getName());

		if(lua_istable(L, -1)) {
			stClassMethodInterface* pMethodObj = new stClassMethodBase<R,T,TYPELIST_5(P1,P2,P3,P4,P5)>(pMethodName,func);
			lua_pushstring(L, pMethodName);
			lua_pushlightuserdata(L, pMethodObj);
			lua_pushcclosure(L, doCallClassMethodInLua, 1);
			lua_rawset(L, -3);
		} else {
			printf("please register class %s\n",  stClassName<T>::getName());
		}
		lua_pop(L, 1);
	}


	template<class T, class R, class P1,class P2,class P3,class P4,class P5,class P6>
	LUABRIDGER_FORCEINLINE void registerClassMethod(lua_State* L, const char* pMethodName, R (T::*func)(P1,P2,P3,P4,P5,P6)) {
		luaL_getmetatable(L, stClassName<T>::getName());

		if(lua_istable(L, -1)) {
			stClassMethodInterface* pMethodObj = new stClassMethodBase<R,T,TYPELIST_6(P1,P2,P3,P4,P5,P6)>(pMethodName,func);
			lua_pushstring(L, pMethodName);
			lua_pushlightuserdata(L, pMethodObj);
			lua_pushcclosure(L, doCallClassMethodInLua, 1);
			lua_rawset(L, -3);
		} else {
			printf("please register class %s\n",  stClassName<T>::getName());
		}
		lua_pop(L, 1);
	}


	template<class T, class R, class P1,class P2,class P3,class P4,class P5,class P6,class P7>
	LUABRIDGER_FORCEINLINE void registerClassMethod(lua_State* L, const char* pMethodName, R (T::*func)(P1,P2,P3,P4,P5,P6,P7)) {
		luaL_getmetatable(L, stClassName<T>::getName());

		if(lua_istable(L, -1)) {
			stClassMethodInterface* pMethodObj = new stClassMethodBase<R,T,TYPELIST_7(P1,P2,P3,P4,P5,P6,P7)>(pMethodName,func);
			lua_pushstring(L, pMethodName);
			lua_pushlightuserdata(L, pMethodObj);
			lua_pushcclosure(L, doCallClassMethodInLua, 1);
			lua_rawset(L, -3);
		} else {
			printf("please register class %s\n",  stClassName<T>::getName());
		}
		lua_pop(L, 1);
	}


	template<class T, class R, class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8>
	LUABRIDGER_FORCEINLINE void registerClassMethod(lua_State* L, const char* pMethodName, R (T::*func)(P1,P2,P3,P4,P5,P6,P7,P8)) {
		luaL_getmetatable(L, stClassName<T>::getName());

		if(lua_istable(L, -1)) {
			stClassMethodInterface* pMethodObj = new stClassMethodBase<R,T,TYPELIST_8(P1,P2,P3,P4,P5,P6,P7,P8)>(pMethodName,func);
			lua_pushstring(L, pMethodName);
			lua_pushlightuserdata(L, pMethodObj);
			lua_pushcclosure(L, doCallClassMethodInLua, 1);
			lua_rawset(L, -3);
		} else {
			printf("please register class %s\n",  stClassName<T>::getName());
		}
		lua_pop(L, 1);
	}


	template<class T, class R, class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8,class P9>
	LUABRIDGER_FORCEINLINE void registerClassMethod(lua_State* L, const char* pMethodName, R (T::*func)(P1,P2,P3,P4,P5,P6,P7,P8,P9)) {
		luaL_getmetatable(L, stClassName<T>::getName());

		if(lua_istable(L, -1)) {
			stClassMethodInterface* pMethodObj = new stClassMethodBase<R,T,TYPELIST_9(P1,P2,P3,P4,P5,P6,P7,P8,P9)>(pMethodName,func);
			lua_pushstring(L, pMethodName);
			lua_pushlightuserdata(L, pMethodObj);
			lua_pushcclosure(L, doCallClassMethodInLua, 1);
			lua_rawset(L, -3);
		} else {
			printf("please register class %s\n",  stClassName<T>::getName());
		}
		lua_pop(L, 1);
	}

	template<class T, class R, class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8,class P9,class P10>
	LUABRIDGER_FORCEINLINE void registerClassMethod(lua_State* L, const char* pMethodName, R (T::*func)(P1,P2,P3,P4,P5,P6,P7,P8,P9,P10)) {
		luaL_getmetatable(L, stClassName<T>::getName());

		if(lua_istable(L, -1)) {
			stClassMethodInterface* pMethodObj = new stClassMethodBase<R,T,TYPELIST_10(P1,P2,P3,P4,P5,P6,P7,P8,P9,P10)>(pMethodName,func);
			lua_pushstring(L, pMethodName);
			lua_pushlightuserdata(L, pMethodObj);
			lua_pushcclosure(L, doCallClassMethodInLua, 1);
			lua_rawset(L, -3);
		} else {
			printf("please register class %s\n",  stClassName<T>::getName());
		}
		lua_pop(L, 1);
	}




	LUABRIDGER_FORCEINLINE int doCallGlobalMethodInLua(lua_State *L) {
		stGlobalMethodInterface* pFunction = (stGlobalMethodInterface*)lua_touserdata(L, lua_upvalueindex(1)); //get functionClass pointer
		return pFunction->doCall(L); // execute function
	}

	template<class R>
	LUABRIDGER_FORCEINLINE void registerGlobalFunction(lua_State* L, const char* pMethodName, R (*func)()) {
		stGlobalMethodInterface* pFunction = new stGlobalMethodBase<R,void>(pMethodName, func);
		lua_pushlightuserdata(L, pFunction);
		lua_pushcclosure(L, doCallGlobalMethodInLua, 1);
		lua_setglobal(L, pMethodName);

	}


	template<class R,class P1>
	LUABRIDGER_FORCEINLINE void registerGlobalFunction(lua_State* L, const char* pMethodName, R (*func)(P1)) {
		stGlobalMethodInterface* pFunction = new stGlobalMethodBase<R,TYPELIST_1(P1)>(pMethodName, func);
		lua_pushlightuserdata(L, pFunction);
		lua_pushcclosure(L, doCallGlobalMethodInLua, 1);
		lua_setglobal(L, pMethodName);

	}


	template<typename R,class P1,class P2>
	LUABRIDGER_FORCEINLINE void registerGlobalFunction(lua_State* L, const char* pMethodName, R (*func)(P1,P2)) {
		stGlobalMethodInterface* pFunction = new stGlobalMethodBase<R,TYPELIST_2(P1,P2)>(pMethodName, func);
		lua_pushlightuserdata(L, pFunction);
		lua_pushcclosure(L, doCallGlobalMethodInLua, 1);
		lua_setglobal(L, pMethodName);

	}

	template<typename R,class P1,class P2,class P3>
	LUABRIDGER_FORCEINLINE void registerGlobalFunction(lua_State* L, const char* pMethodName, R (*func)(P1,P2,P3)) {
		stGlobalMethodInterface* pFunction = new stGlobalMethodBase<R,TYPELIST_3(P1,P2,P3)>(pMethodName, func);
		lua_pushlightuserdata(L, pFunction);
		lua_pushcclosure(L, doCallGlobalMethodInLua, 1);
		lua_setglobal(L, pMethodName);

	}


	template<typename R,class P1,class P2,class P3,class P4>
	LUABRIDGER_FORCEINLINE void registerGlobalFunction(lua_State* L, const char* pMethodName, R (*func)(P1,P2,P3,P4)) {
		stGlobalMethodInterface* pFunction = new stGlobalMethodBase<R,TYPELIST_4(P1,P2,P3,P4)>(pMethodName, func);
		lua_pushlightuserdata(L, pFunction);
		lua_pushcclosure(L, doCallGlobalMethodInLua, 1);
		lua_setglobal(L, pMethodName);

	}


	template<typename R,class P1,class P2,class P3,class P4,class P5>
	LUABRIDGER_FORCEINLINE void registerGlobalFunction(lua_State* L, const char* pMethodName, R (*func)(P1,P2,P3,P4,P5)) {
		stGlobalMethodInterface* pFunction = new stGlobalMethodBase<R,TYPELIST_5(P1,P2,P3,P4,P5)>(pMethodName, func);
		lua_pushlightuserdata(L, pFunction);
		lua_pushcclosure(L, doCallGlobalMethodInLua, 1);
		lua_setglobal(L, pMethodName);

	}


	template<typename R,class P1,class P2,class P3,class P4,class P5,class P6>
	LUABRIDGER_FORCEINLINE void registerGlobalFunction(lua_State* L, const char* pMethodName, R (*func)(P1,P2,P3,P4,P5,P6)) {
		stGlobalMethodInterface* pFunction = new stGlobalMethodBase<R,TYPELIST_6(P1,P2,P3,P4,P5,P6)>(pMethodName, func);
		lua_pushlightuserdata(L, pFunction);
		lua_pushcclosure(L, doCallGlobalMethodInLua, 1);
		lua_setglobal(L, pMethodName);

	}


	template<typename R,class P1,class P2,class P3,class P4,class P5,class P6,class P7>
	LUABRIDGER_FORCEINLINE void registerGlobalFunction(lua_State* L, const char* pMethodName, R (*func)(P1,P2,P3,P4,P5,P6,P7)) {
		stGlobalMethodInterface* pFunction = new stGlobalMethodBase<R,TYPELIST_7(P1,P2,P3,P4,P5,P6,P7)>(pMethodName, func);
		lua_pushlightuserdata(L, pFunction);
		lua_pushcclosure(L, doCallGlobalMethodInLua, 1);
		lua_setglobal(L, pMethodName);

	}


	template<typename R,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8>
	LUABRIDGER_FORCEINLINE void registerGlobalFunction(lua_State* L, const char* pMethodName, R (*func)(P1,P2,P3,P4,P5,P6,P7,P8)) {
		stGlobalMethodInterface* pFunction = new stGlobalMethodBase<R,TYPELIST_8(P1,P2,P3,P4,P5,P6,P7,P8)>(pMethodName, func);
		lua_pushlightuserdata(L, pFunction);
		lua_pushcclosure(L, doCallGlobalMethodInLua, 1);
		lua_setglobal(L, pMethodName);

	}

	template<typename R,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8,class P9>
	LUABRIDGER_FORCEINLINE void registerGlobalFunction(lua_State* L, const char* pMethodName, R (*func)(P1,P2,P3,P4,P5,P6,P7,P8,P9)) {
		stGlobalMethodInterface* pFunction = new stGlobalMethodBase<R,TYPELIST_9(P1,P2,P3,P4,P5,P6,P7,P8,P9)>(pMethodName, func);
		lua_pushlightuserdata(L, pFunction);
		lua_pushcclosure(L, doCallGlobalMethodInLua, 1);
		lua_setglobal(L, pMethodName);

	}

	template<class R,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8,class P9,class P10>
	LUABRIDGER_FORCEINLINE void registerGlobalFunction(lua_State* L, const char* pMethodName, R (*func)(P1,P2,P3,P4,P5,P6,P7,P8,P9,P10)) {
		stGlobalMethodInterface* pFunction = new stGlobalMethodBase<R,TYPELIST_10(P1,P2,P3,P4,P5,P6,P7,P8,P9,P10)>(pMethodName, func);
		lua_pushlightuserdata(L, pFunction);
		lua_pushcclosure(L, doCallGlobalMethodInLua, 1);
		lua_setglobal(L, pMethodName);

	}



	template<class T>
	LUABRIDGER_FORCEINLINE void registerClassMetatable(lua_State *L, const char *pClassName) {
		luaL_newmetatable(L, pClassName );  
		lua_pushstring(L, "__index"); 
		lua_pushvalue(L, -2);         
		lua_settable(L, -3);         

		lua_pushstring(L, "__newindex");
		lua_pushvalue(L, -2);
		lua_settable(L, -3);          

		lua_pushstring(L, "__gc");
		lua_pushcfunction(L, &doGc<T>);
		lua_rawset(L, -3);

		lua_pop(L, 1);
	}


	template<class T, class F>
	LUABRIDGER_FORCEINLINE void registerClassForTableInLua(lua_State *L, const char* pClassName, F func) {
		stClassName<T>::setName(pClassName);
		lua_pushcfunction(L, func);
		lua_setglobal(L, name);
		registerMetatable<T>(L, pClassName);
	}


	template<class T>
	LUABRIDGER_FORCEINLINE void justRegisterPointerForLua(lua_State* L, const char* pClassName,T* ptr,const char* pPointerName){
		stClassName<T>::setName(pClassName);
		registerClassMetatable<T>(L, pClassName);
		stUserData<T>*pUD = static_cast<stUserData<T>*>(lua_newuserdata(L, sizeof(stUserData<T>)));
		pUD->m_Ptr = ptr;
		luaL_getmetatable(L, stClassName<T>::getName());
		lua_setmetatable(L, -2);
		lua_setglobal(L,pPointerName);
	}

}
#endif