// Don't modify this file as it will be overwritten.
//
#include "IDL_TestDataType.h"

UserDataType::UserDataType(const UserDataType &IDL_s){
  data = IDL_s.data;
  numpackes = IDL_s.numpackes;
}

UserDataType& UserDataType::operator= (const UserDataType &IDL_s){
  if (this == &IDL_s) return *this;
  data = IDL_s.data;
  numpackes = IDL_s.numpackes;
  return *this;
}

void UserDataType::Marshal(CDR *cdr) const {
  cdr->PutString(data);
  cdr->PutLong(numpackes);
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
  cdr->GetLong(numpackes);
}

