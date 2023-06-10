/**********************************************************
*****************订阅端程序subscriber.cpp*******************
***********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <unistd.h>
#include <openssl/md5.h>


/* IDL_TypeSupport.h中包含所有依赖的头文件 */
#include "IDL_TypeSupport.h"

std::string calculateMD5(const char* data, size_t size) {
    unsigned char md[MD5_DIGEST_LENGTH];
    MD5(reinterpret_cast<const unsigned char*>(data), size, md);

    char mdString[33];
    for(int i = 0; i < MD5_DIGEST_LENGTH; i++)
        sprintf(&mdString[i * 2], "%02x", (unsigned int)md[i]);

    return std::string(mdString);
}


/* UserDataTypeListener继承于DataReaderListener，
   需要重写其继承过来的方法on_data_available()，在其中进行数据监听读取操作 */
class UserDataTypeListener : public DataReaderListener {
public:
	virtual void on_data_available(DataReader* reader);
};

/* 重写继承过来的方法on_data_available()，在其中进行数据监听读取操作 */
void UserDataTypeListener::on_data_available(DataReader* reader)
{
	UserDataTypeDataReader *UserDataType_reader = NULL;
	UserDataTypeSeq data_seq;
	SampleInfoSeq info_seq;
	ReturnCode_t retcode;
	int i;

	/* 利用reader，创建一个读取UserDataType类型的UserDataType_reader*/
	UserDataType_reader = UserDataTypeDataReader::narrow(reader);
	if (UserDataType_reader == NULL) {
		fprintf(stderr, "DataReader narrow error\n");
		return;
	}

	/* 获取数据，存放至data_seq，data_seq是一个队列 */
	retcode = UserDataType_reader->read(
		data_seq, info_seq, 10, 0, 0, 0);

	if (retcode == RETCODE_NO_DATA) {
		return;
	}
	else if (retcode != RETCODE_OK) {
		fprintf(stderr, "take error %d\n", retcode);
		return;
	}
	

	/* 打印数据 */
	/* 建议1：避免在此进行复杂数据处理 */
	/* 建议2：将数据传送到其他数据处理线程中进行处理 *
	/* 建议3：假如数据结构中有string类型，用完后需手动释放 */

	for (i = 0; i < data_seq.length(); ++i) {
	    //UserDataTypeTypeSupport::print_data(&data_seq[i]);
	    UserDataType* pUserDataType = &data_seq[i];
	    string receivedData = pUserDataType->data;
            string receivedMD5 = pUserDataType->md5;

            // Calculate MD5 hash of received data
            std::string calculatedMD5 = calculateMD5(receivedData.c_str(), receivedData.size());

            std::cout << "Data received:\n";
            std::cout << "  Received data size: " << receivedData.size() << " bytes\n";
            std::cout << "  Received data: " << receivedData << "\n";
            std::cout << "  Received MD5: " << receivedMD5 << "\n";
            std::cout << "  Calculated MD5: " << calculatedMD5 << "\n";
            

            // Compare calculated MD5 with received MD5
            if (receivedMD5 != calculatedMD5) {
                std::cout << "MD5 mismatch! Data integrity compromised.\n";
            }
            
			

	}
}

/* 删除所有实体 */
static int subscriber_shutdown(
	DomainParticipant *participant)
{
	ReturnCode_t retcode;
	int status = 0;

	if (participant != NULL) {
		retcode = participant->delete_contained_entities();
		if (retcode != RETCODE_OK) {
			fprintf(stderr, "delete_contained_entities error %d\n", retcode);
			status = -1;
		}

		retcode = DomainParticipantFactory::get_instance()->delete_participant(participant);
		if (retcode != RETCODE_OK) {
			fprintf(stderr, "delete_participant error %d\n", retcode);
			status = -1;
		}
	}
	return status;
}

/* 订阅者函数 */
extern "C" int subscriber_main(int domainId, int sample_count)
{
	DomainParticipant *participant = NULL;
	Subscriber *subscriber = NULL;
	Topic *topic = NULL;
	UserDataTypeListener *reader_listener = NULL;
	DataReader *reader = NULL;
	ReturnCode_t retcode;
	const char *type_name = NULL;
	int count = 0;
	int status = 0;

	/* 1. 创建一个participant，可以在此处定制participant的QoS */
	/* 建议1：在程序启动后优先创建participant，进行资源初始化*/
	/* 建议2：相同的domainId只创建一次participant，重复创建会占用大量资源 */ 
	participant = DomainParticipantFactory::get_instance()->create_participant(
		domainId, PARTICIPANT_QOS_DEFAULT/* participant默认QoS */,
		NULL /* listener */, STATUS_MASK_NONE);
	if (participant == NULL) {
		fprintf(stderr, "create_participant error\n");
		subscriber_shutdown(participant);
		return -1;
	}

	/* 2. 创建一个subscriber，可以在创建subscriber的同时定制其QoS  */
	/* 建议1：在程序启动后优先创建subscriber*/
	/* 建议2：一个participant下创建一个subscriber即可，无需重复创建 */
	subscriber = participant->create_subscriber(
		SUBSCRIBER_QOS_DEFAULT/* 默认QoS */,
		NULL /* listener */, STATUS_MASK_NONE);
	if (subscriber == NULL) {
		fprintf(stderr, "create_subscriber error\n");
		subscriber_shutdown(participant);
		return -1;
	}

	/* 3. 在创建主题之前注册数据类型 */
	/* 建议1：在程序启动后优先进行注册 */
	/* 建议2：一个数据类型注册一次即可 */
	type_name = UserDataTypeTypeSupport::get_type_name();
	retcode = UserDataTypeTypeSupport::register_type(
		participant, type_name);
	if (retcode != RETCODE_OK) {
		fprintf(stderr, "register_type error %d\n", retcode);
		subscriber_shutdown(participant);
		return -1;
	}

	/* 4. 创建主题，并定制主题的QoS  */
	/* 建议1：在程序启动后优先创建Topic */
	/* 建议2：一种主题名创建一次即可，无需重复创建 */
	topic = participant->create_topic(
		"Example UserDataType"/* 主题名，应与发布者主题名一致 */,
		type_name, TOPIC_QOS_DEFAULT/* 默认QoS */, 
		NULL /* listener */, STATUS_MASK_NONE);
	if (topic == NULL) {
		fprintf(stderr, "create_topic error\n");
		subscriber_shutdown(participant);
		return -1;
	}

	/* 5. 创建一个监听器 */
	reader_listener = new UserDataTypeListener();

	/* 6. 创建datareader，并定制datareader的QoS */
	/* 建议1：在程序启动后优先创建datareader*/
	/* 建议2：创建一次即可，无需重复创建 */
	/* 建议3：在程序退出时再进行释放 */
	/* 建议4：避免打算接收数据时创建datareader，接收数据后删除，该做法消耗资源，影响性能 */
	reader = subscriber->create_datareader(
		topic, DATAREADER_QOS_DEFAULT/* 默认QoS */,
		reader_listener/* listener */, STATUS_MASK_ALL);
	if (reader == NULL) {
		fprintf(stderr, "create_datareader error\n");
		subscriber_shutdown(participant);
		delete reader_listener;
		return -1;
	}

	/* 7. 主循环 ，监听器会默认调用on_data_available()监听数据 */
	for (count = 0; (sample_count == 0) || (count < sample_count); ++count) {
		//保持进程一直运行
	}


	/* 8. 删除所有实体和监听器 */
	status = subscriber_shutdown(participant);
	delete reader_listener;

	return status;
}

int main(int argc, char *argv[])
{
	int domain_id = 0;
	int sample_count = 0; /* 无限循环 */

	if (argc >= 2) {
		domain_id = atoi(argv[1]);/* 发送至域domain_id */
	}
	if (argc >= 3) {
		sample_count = atoi(argv[2]);/* 发送sample_count次 */
	}
	return subscriber_main(domain_id, sample_count);
}

