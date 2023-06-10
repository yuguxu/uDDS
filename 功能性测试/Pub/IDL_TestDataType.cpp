// Don't modify this file as it will be overwritten.
//
#include "IDL_TestDataType.h"

UserDataType::UserDataType(const UserDataType &IDL_s){
  data = IDL_s.data;
  md5 = IDL_s.md5;
}

UserDataType& UserDataType::operator= (const UserDataType &IDL_s){
  if (this == &IDL_s) return *this;
  data = IDL_s.data;
  md5 = IDL_s.md5;
  return *this;
}

void UserDataType::Marshal(CDR *cdr) const {
  cdr->PutString(data);
  cdr->PutString(md5);
}

void UserDataType::UnMarshal(CDR *cdr){
  {
    char *IDL_str;
    cdr->GetString(IDL_str);
    if(data != NULL )
    {
        delete data;
        data = NULL;
    }
    data = IDL_str;
  }
  {
    char *IDL_str;
    cdr->GetString(IDL_str);
    if(md5 != NULL )
    {
        delete md5;
        md5 = NULL;
    }
    md5 = IDL_str;
  }
}

