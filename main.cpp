
#include <iostream>
#include "CLuaBridger.h"  // #include <iostream>放它下面全报错！！！！
#include "c_svm.h"
using namespace std;
using namespace cpplua;


class Test3
{
public:
	Test3(int p1, int p2, int p3) { 
		printf(" Test3 Constructor! %d %d %d\n", p1, p2, p3);
	}

	void say(){
		printf("hahaha");
	}
	~Test3() { printf(" Test3 Destructor!\n");}
};
int main()
{
	CLuaSVM* pLua = new CLuaSVM();
	lua_State* L = pLua->mLS;

	Test3 * p = new Test3(1,2,3);
	cpplua::justRegisterPointerForLua<Test3>(L,"Test3",p,"psay");
	cpplua::registerClassMethod<Test3>(L,"say",&Test3::say);
	cpplua::doFile(L,"test.lua");
	system("pause");
	return 0;
}