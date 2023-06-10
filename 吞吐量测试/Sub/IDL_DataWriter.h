#ifndef _IDL_DATAWRITER_H_INCLUDED_
#define _IDL_DATAWRITER_H_INCLUDED_
#include "ReturnCode.h"
#include "BasicTypes.h"
#include "DDS/DomainParticipantFactory.h"
#include "DDS/Topic.h"
#include "DDS/Publisher.h"
#include "DDS/DataWriter.h"
#include "IDL_TestDataType.h"
class DataWriterImpl;

class UserDataTypeDataWriter : public DataWriter
{
public:
	UserDataTypeDataWriter(DataWriterImpl* pDataWriterImpl);

	~UserDataTypeDataWriter();

	static UserDataTypeDataWriter* narrow(DataWriter* pDataWriter);

	ReturnCode_t write(UserDataType& data,const InstanceHandle_t & handle);

};

#endif
