#pragma once
#include "..\PlayerEntity.h"

class KuangZhan : public PlayerEntity
{
public:
	KuangZhan(GameGrail *engine, int id, int color): PlayerEntity(engine, id, color){}
	//�ҳ���ʵ��������Ҫ��p_xxxx�麯������Щp_xxxx�������ڶ�Ӧ��StateXXXXX�ﱻ����
	//ÿ��p_xxxx������в�ֻһ�����ܣ�step��������������Щ��
	int p_before_turn_begin(int &step, int currentPlayerID);
	int p_timeline_1(int &step, CONTEXT_TIMELINE_1 *con);
	int p_timeline_2_hit(int &setp, CONTEXT_TIMELINE_2_HIT *con);
	//��ɫ����Ϣ��������UserTask���ã�ֻҪʹ����waitForXXX��������Ҫ��д�������
	//return true ��ʾ������
	bool cmdMsgParse(UserTask* session, uint16_t type, ::google::protobuf::Message *proto);
private:
	//���弼��
	int XueYingKuangDao(CONTEXT_TIMELINE_1 *con);
	int XueXingPaoXiao(CONTEXT_TIMELINE_1 *con);
	int KuangHua(CONTEXT_TIMELINE_2_HIT *con);
	int SiLie(CONTEXT_TIMELINE_2_HIT *con);
	bool used_XueYingKuangDao;
};
