#include "action_respond.pb.h"
#include "base.pb.h"
#include "zType.h"

using namespace std;
using namespace network;

/*
Ŀǰ����Ϣ�ṹ��
0~15bit  | 16~31bit | 32~?bit
��Ϣ���� | Э����� | Э�鴮���ַ���
*/
struct MsgHeader {
	uint16_t len;
	uint16_t type;
};
const int SIZEOF_HEADER = sizeof(MsgHeader);

bool proto_encoder(uint16_t type, ::google::protobuf::Message& body, string& msg);

/*
msg�����յ�����Ϣ
type����Ϣ���ͣ�����ֵ��

return�����������Э��ָ�루��ǵ�ɾ����
*/
void* proto_decoder(const char* msg, uint16_t& type);