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

int signal = 0;

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
	    if (data_seq[i].sender_id == 1)signal = 1;
	    //UserDataTypeTypeSupport::print_data(&data_seq[i]);

	}
}



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
extern "C" int publisher_main(int domainId, int sample_count, std::string randomString,int data_size,int send_count)
{
	DomainParticipant *participant = NULL;
	Publisher *publisher = NULL;
	Subscriber *subscriber = NULL;
	Topic *topic = NULL;
	DataWriter *writer = NULL;
	UserDataTypeDataWriter * UserDataType_writer = NULL;
	UserDataType *instance = NULL;
	UserDataTypeListener *reader_listener = NULL;
	DataReader *reader = NULL;
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
	
	
	/* 创建一个subscriber，可以在创建subscriber的同时定制其QoS  */
	/* 建议1：在程序启动后优先创建subscriber*/
	/* 建议2：一个participant下创建一个subscriber即可，无需重复创建 */
	subscriber = participant->create_subscriber(
		SUBSCRIBER_QOS_DEFAULT/* 默认QoS */,
		NULL /* listener */, STATUS_MASK_NONE);
	if (subscriber == NULL) {
		fprintf(stderr, "create_subscriber error\n");
		publisher_shutdown(participant);
		return -1;
	}
	
	
	/*  创建一个监听器 */
	reader_listener = new UserDataTypeListener();
	
	/*  创建datareader，并定制datareader的QoS */
	/* 建议1：在程序启动后优先创建datareader*/
	/* 建议2：创建一次即可，无需重复创建 */
	/* 建议3：在程序退出时再进行释放 */
	/* 建议4：避免打算接收数据时创建datareader，接收数据后删除，该做法消耗资源，影响性能 */
	reader = subscriber->create_datareader(
		topic, DATAREADER_QOS_DEFAULT/* 默认QoS */,
		reader_listener/* listener */, STATUS_MASK_ALL);
	if (reader == NULL) {
		fprintf(stderr, "create_datareader error\n");
		publisher_shutdown(participant);
		delete reader_listener;
		return -1;
	}
	while(true){
		strcpy(instance->data, randomString.c_str());
	        instance->sender_id = 0;
	        retcode = UserDataType_writer->write(*instance, instance_handle);
	        if (retcode != RETCODE_OK || signal == 0) {
	           fprintf(stderr, "write error %d\n", retcode);
	        }  
	        else if(signal == 1){
	            fprintf(stderr, "A端与B端成功连接 . . \n");
	            break;
	
	        }
	        sleep(1);//沉睡1秒
	}


	/* 7. 主循环 ，发送数据 */
        std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();
        int count_ = 0;
	for (count = 0; (sample_count == 0) || (count < sample_count); ++count) {
	    
	    if(signal == 1){ 
	        signal =0;
	        strcpy(instance->data, randomString.c_str());
	        instance->sender_id = 0;
	        retcode = UserDataType_writer->write(*instance, instance_handle);
	        if (retcode != RETCODE_OK) {
	           fprintf(stderr, "write error %d\n", retcode);
	        }  
	        else{
	            fprintf(stderr, "count %d: A端发送成功 . . \n",count_+1);
	            count_++;
	            if(count_>=send_count)break;
	            }
	            
	       }
	}	       	    
        std::chrono::system_clock::time_point end_time = std::chrono::system_clock::now();
        double elapsed_time = std::chrono::duration<double>(end_time - start_time).count();
        
        std::cout<<"传输时延:"<<elapsed_time/send_count/2/10e6<<"us"<<endl;

	/* 8. 删除数据样本 ,监听器*/
	retcode = UserDataTypeTypeSupport::delete_data(instance);
	if (retcode != RETCODE_OK) {
		fprintf(stderr, "UserDataTypeTypeSupport::delete_data error %d\n", retcode);
	}
	delete reader_listener;

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
	int send_count =10000;
        cout<<"指定循环次数"<<endl;
        cin>>send_count;
	
	return publisher_main(domain_id, sample_count,randomString,data_size,send_count);
}
