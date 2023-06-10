#include "IDL_DataReader.h"

UserDataTypeDataReader::UserDataTypeDataReader(DataReaderImpl * pDataReaderImpl)
	:DataReader(pDataReaderImpl)
{
}

UserDataTypeDataReader::~UserDataTypeDataReader() 
{
}

UserDataTypeDataReader* UserDataTypeDataReader::narrow(DataReader* pDataReader) 
{
	if (!pDataReader)
		return NULL;

	UserDataTypeDataReader *aUserDataTypeDataReader = (UserDataTypeDataReader*)pDataReader;
	return aUserDataTypeDataReader;
}

ReturnCode_t UserDataTypeDataReader::take(UserDataTypeSeq & data_values, SampleInfoSeq & sample_infos, int max_samples, SampleStateKind sample_states, ViewStateKind view_states, InstanceStateKind instance_states)
{
	UserDataVector userDataVector;
	DataReader::take(userDataVector, sample_infos, max_samples, sample_states, view_states, instance_states);

	data_values.ensure_length(userDataVector.size(), userDataVector.size());
	for (int i = 0; i < userDataVector.size(); i++)
	{
		CDR* CDRData = new CDR((unsigned char *)userDataVector[i].pData + 4, userDataVector[i].ilength - 4, MY_BYTE_SEX, 1);
		data_values[i].UnMarshal(CDRData);
		if (CDRData)
			delete CDRData;
		delete userDataVector[i].pData;
	}
	return ReturnCode_t();
}

ReturnCode_t UserDataTypeDataReader::read(UserDataTypeSeq & data_values, SampleInfoSeq & sample_infos, int max_samples, SampleStateKind sample_states, ViewStateKind view_states, InstanceStateKind instance_states)
{
	UserDataVector userDataVector;
	DataReader::read(userDataVector, sample_infos, max_samples, sample_states, view_states, instance_states);

	data_values.ensure_length(userDataVector.size(), userDataVector.size());
	for (int i = 0; i < userDataVector.size(); i++)
	{
		CDR* CDRData = new CDR((unsigned char *)userDataVector[i].pData + 4, userDataVector[i].ilength - 4, MY_BYTE_SEX, 1);
		data_values[i].UnMarshal(CDRData);
		if (CDRData)
			delete CDRData;
		delete userDataVector[i].pData;
	}
	return ReturnCode_t();
}

ReturnCode_t UserDataTypeDataReader::take_next_sample(UserDataType& receivedData, SampleInfo& sample_info)
{
	UserData userData;
	DataReader::take_next_sample(userData, sample_info);
	CDR* CDRData = new CDR((unsigned char *)userData.pData + 4, userData.ilength - 4, MY_BYTE_SEX, 1);
	receivedData.UnMarshal(CDRData);
	if (CDRData)
	{
		delete CDRData;
	}
	return ReturnCode_t();
}

ReturnCode_t UserDataTypeDataReader::return_loan(UserDataTypeSeq& data_values, SampleInfoSeq& sample_infos)
{
	return ReturnCode_t();
}

