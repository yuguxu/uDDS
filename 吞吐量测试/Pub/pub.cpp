/**********************************************************
*****************发布端程序publisher.cpp********************
***********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <unistd.h>
#include <chrono> // 用于计算时间间隔
#include <random>

/* IDL_TypeSupport.h中包含所有依赖的头文件 */
#include "IDL_TypeSupport.h"

std::string generateRandomString(int byteLength) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(0, 255);

    std::string result;
    result.reserve(byteLength);

    while (result.size() < byteLength) {
        result.push_back(static_cast<char>(dis(gen)));
    }

    return result;
}

/* 删除所有实体 */
static int publisher_shutdown(DomainParticipant *participant)
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

/* 发布者函数 */
extern "C" int publisher_main(int domainId, int sample_count, string randomString,int data_size)
{
	DomainParticipant *participant = NULL;
	Publisher *publisher = NULL;
	Topic *topic = NULL;
	DataWriter *writer = NULL;
	UserDataTypeDataWriter * UserDataType_writer = NULL;
	UserDataType *instance = NULL;
	ReturnCode_t retcode;
	InstanceHandle_t instance_handle = HANDLE_NIL;
	const char *type_name = NULL;
	int count = 0;


	/* 1. 创建一个participant，可以在此处定制participant的QoS */
	/* 建议1：在程序启动后优先创建participant，进行资源初始化*/
	/* 建议2：相同的domainId只创建一次participant，重复创建会占用大量资源 */ 
	participant = DomainParticipantFactory::get_instance()->create_participant(
		domainId, PARTICIPANT_QOS_DEFAULT/* participant默认QoS */,
		NULL /* listener */, STATUS_MASK_NONE);
	if (participant == NULL) {
		fprintf(stderr, "create_participant error\n");
		publisher_shutdown(participant);
		return -1;
	}

	/* 2. 创建一个publisher，可以在创建publisher的同时定制其QoS  */
	/* 建议1：在程序启动后优先创建publisher */
	/* 建议2：一个participant下创建一个publisher即可，无需重复创建 */
	publisher = participant->create_publisher(
		PUBLISHER_QOS_DEFAULT /* 默认QoS */, 
		NULL /* listener */, STATUS_MASK_NONE);
	if (publisher == NULL) {
		fprintf(stderr, "create_publisher error\n");
		publisher_shutdown(participant);
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
		publisher_shutdown(participant);
		return -1;
	}

	/* 4. 创建主题，并定制主题的QoS  */
	/* 建议1：在程序启动后优先创建Topic */
	/* 建议2：一种主题名创建一次即可，无需重复创建 */
	topic = participant->create_topic(
		"Example UserDataType"/* 主题名 */,
		type_name /* 类型名 */, TOPIC_QOS_DEFAULT/* 默认QoS */,
		NULL /* listener */, STATUS_MASK_NONE);
	if (topic == NULL) {
		fprintf(stderr, "create_topic error\n");
		publisher_shutdown(participant);
		return -1;
	}

	/* 5. 创建datawriter，并定制datawriter的QoS  */
	/* 建议1：在程序启动后优先创建datawriter */
	/* 建议2：创建一次即可，无需重复创建 */
	/* 建议3：在程序退出时再进行释放 */
	/* 建议4：避免打算发送数据时创建datawriter，发送数据后删除，该做法消耗资源，影响性能 */
	writer = publisher->create_datawriter(
		topic , DATAWRITER_QOS_DEFAULT/* 默认QoS */,
		NULL /* listener */, STATUS_MASK_NONE);
	if (writer == NULL) {
		fprintf(stderr, "create_datawriter error\n");
		publisher_shutdown(participant);
		return -1;
	}
	UserDataType_writer = UserDataTypeDataWriter::narrow(writer);
	if (UserDataType_writer == NULL) {
		fprintf(stderr, "DataWriter narrow error\n");
		publisher_shutdown(participant);
		return -1;
	}

	/* 6. 创建一个数据样本 */
	/* 建议：该数据为new出来的，使用后用户需要调用delete_data进行释放内存*/
	instance = UserDataTypeTypeSupport::create_data();
	if (instance == NULL) {
		fprintf(stderr, "UserDataTypeTypeSupport::create_data error\n");
		publisher_shutdown(participant);
		return -1;
	}

	/* 7. 主循环 ，发送数据 */
	int sent_count = 0;
        auto start_time = std::chrono::steady_clock::now();
	for (count = 0; (sample_count == 0) || (count < sample_count); ++count) {
		/* 在此处修改数据 */
//		instance->color = new char[100];		
//		instance->x =  count;
//		instance->y = 1 + count;
//		strcpy(instance->color, "red");
		
		strcpy(instance->data, randomString.c_str());
		instance->numpackes = count;
		retcode = UserDataType_writer->write(*instance, instance_handle);
		if (retcode != RETCODE_OK) {
			//fprintf(stderr, "write error %d\n", retcode);	
		}
		else
			//fprintf(stderr, "%d : write  successfully . . \n", count);
			sent_count++;
                        auto current_time = std::chrono::steady_clock::now();
                        auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time).count();
                        if (elapsed_time >= 1) {
                             //cout<<sent_count<<endl;
                             double throughput = sent_count * data_size * 8 / 1e6; // Mbps
                       
                             std::cout << "Sent Throughput: " << throughput << " Mbps" << std::endl;
                             sent_count = 0;
                             start_time = std::chrono::steady_clock::now();
                        }

		        //sleep(1);//沉睡1秒
	}


	/* 8. 删除数据样本 */
	retcode = UserDataTypeTypeSupport::delete_data(instance);
	if (retcode != RETCODE_OK) {
		fprintf(stderr, "UserDataTypeTypeSupport::delete_data error %d\n", retcode);
	}

	/* 9. 删除所有实例 */
	return publisher_shutdown(participant);
}

int main(int argc, char *argv[])
{
	int domain_id = 0;
	int sample_count = 0; /* 无限循环 */
	int data_size = 256; /* 默认数据大小 ( 字节) */
	cout<<"指定数据大小（单位：字节）"<<endl;
        cin>>data_size;

	std::string randomString = generateRandomString(data_size);
	cout<<randomString.length()<<endl;

	if (argc >= 2) {
		domain_id = atoi(argv[1]);  /* 发送至域domain_id */
	}
	if (argc >= 3) {
		sample_count = atoi(argv[2]); /* 发送sample_count次 */
	}

	return publisher_main(domain_id, sample_count,randomString,data_size);
}
