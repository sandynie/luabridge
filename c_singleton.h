#ifndef _C_SINGLETON_H
#define _C_SINGLETON_H
#include <assert.h>
#include <stdio.h>
template <typename T> 
class  Singleton
{
	static T *m_pSingleton;

public:
	Singleton()
	{
		assert( !m_pSingleton );
		printf("Singleton");
		m_pSingleton = static_cast<T*>(this);
	}

	~Singleton()
	{
		assert( m_pSingleton );
		m_pSingleton = 0;
	}

	static T &GetSingleton(void)
	{ 
		assert( m_pSingleton );
		return (*m_pSingleton); 
	}

	static T *GetSingletonPtr(void)
	{ 
		return (m_pSingleton);
	}
};

template <typename T> T *Singleton<T>::m_pSingleton = 0;

#endif