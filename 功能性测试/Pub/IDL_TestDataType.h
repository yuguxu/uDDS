// Don't modify this file as it will be overwritten.
//
#include "CDR/CDR.h"
#include "ReturnCode.h"
#include "BasicTypes.h"

#ifndef IDL_TestDataType_hh
#define IDL_TestDataType_hh

#ifndef UserDataType_defined
#define UserDataType_defined

struct UserDataType {
  UserDataType()
	{
		data = new char[255];
		data[0]= '\0';
		md5 = new char[255];
		md5[0]= '\0';
	}

  UserDataType(const UserDataType  &IDL_s);

  ~UserDataType(){
		delete data;
		data = NULL;
		delete md5;
		md5 = NULL;
	}

  	int StructSize()
	{
		int strSize = 0;
		strSize += sizeof(UserDataType);
		strSize += strlen(data);
		strSize += strlen(md5);
		return strSize;
	}
  UserDataType& operator= (const UserDataType &IDL_s);

  void Marshal(CDR *cdr) const;
  void UnMarshal(CDR *cdr);

  char* data;
char* md5;
  
};

typedef sequence<UserDataType> UserDataTypeSeq;

#endif




#endif /*IDL_TestDataType_hh */
