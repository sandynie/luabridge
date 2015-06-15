#ifndef _CAUTORELEASEFILE_H
#define _CAUTORELEASEFILE_H
#include <stdio.h>
#include<stdlib.h>
#include <assert.h>

struct  stObjRef{
	stObjRef():m_iRefCount(1){}
	int incRef(){ return ++m_iRefCount;}
	int decRef(){return --m_iRefCount;}
	int m_iRefCount;
};

class CAutoReleaseFile
{
public:
	CAutoReleaseFile(const char* pFilePath):m_pFilePath(pFilePath){
		if ((m_pFile=fopen(m_pFilePath, "r")) ==NULL )
		{
			assert(0 && "can not open file!!!");
		}	
		size_t result;
		fseek (m_pFile , 0 , SEEK_END);  
		m_lSize = ftell (m_pFile);  
		rewind (m_pFile);  
		/* 分配内存存储整个文件 */   
		m_pBuffer = (char*) malloc (sizeof(char)*m_lSize+1);  
		if (m_pBuffer == NULL)  
		{	
			assert(0 && " malloc error");
		}  

		/* 将文件拷贝到buffer中 */  
		result = fread (m_pBuffer,1,m_lSize,m_pFile);  
		if (result != m_lSize)  
		{  
			assert(0 && " read error");
		}  
		m_pBuffer[m_lSize]='\0';
		m_pRef = new stObjRef();
	}

	CAutoReleaseFile(const CAutoReleaseFile& rhs)
	{
		m_pBuffer = rhs.m_pBuffer;
		m_pFile = rhs.m_pFile;
		m_pFilePath = rhs.m_pFilePath;
		m_pRef = rhs.m_pRef;
		m_lSize = rhs.m_lSize;
		if (m_pRef)
		{
			m_pRef->incRef();
		}

	}
	int GetSize()const{return this->m_lSize;}

	CAutoReleaseFile operator=(const CAutoReleaseFile& rhs)
	{
		if (this!=&rhs)
		{
			Release();
			m_lSize = rhs.m_lSize;
			m_pBuffer = rhs.m_pBuffer;
			m_pFilePath = rhs.m_pFilePath;
			m_pRef = rhs.m_pRef;
			if (m_pRef)
			{
				m_pRef->incRef();
			}
		}
		return *this;
	}

	~CAutoReleaseFile(){
		Release();
	}

	char* & operator &(int i){
		return m_pBuffer;
	}

	void Release()
	{
		if (m_pRef)
		{
			if (m_pRef->decRef()==0)
			{
				if (m_pFile)
				{
					fclose(m_pFile);
				}
				if (m_pBuffer)
				{
					free(m_pBuffer);
				}
			}
		}
		m_pRef = NULL;		
	}
protected:
private:
	stObjRef * m_pRef;
	const char* m_pFilePath;
	FILE* m_pFile;
	char*  m_pBuffer;
	long m_lSize;
};
#endif