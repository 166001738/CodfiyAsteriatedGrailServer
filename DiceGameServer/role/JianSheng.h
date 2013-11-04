#pragma once
#include "..\PlayerEntity.h"

class JianSheng : public PlayerEntity
{
public:
	JianSheng(GameGrail *engine, int id, int color): PlayerEntity(engine, id, color){}
	//�ҳ���ʵ��������Ҫ��p_xxxx�麯������Щp_xxxx�������ڶ�Ӧ��StateXXXXX�ﱻ����
	//ÿ��p_xxxx������в�ֻһ�����ܣ�step��������������Щ��
	int p_before_turn_begin(int &step, int currentPlayerID);
	int p_timeline_1(int &step, CONTEXT_TIMELINE_1 *con);
	int p_after_attack(int &step, int playerID);
	int p_additional_action(int chosen);
	//v_xxxx��������⣬����State���ã�һ������ʵ�֣�������ʥ�������������Ƿ�ն����������⣬ֻ�ܸ�дv_xxxx
	int v_attack(int cardID, int dstID, bool realCard = true);
	int v_additional_action(int chosen);
	//��ɫ����Ϣ��������UserTask���ã�ֻҪʹ����waitForXXX��������Ҫ��д�������
	//return true ��ʾ������
	bool cmdMsgParse(UserTask* session, uint16_t type, ::google::protobuf::Message *proto);
private:
	//���弼��
	int LieFengJi(CONTEXT_TIMELINE_1 *con);
	int JiFengJi(CONTEXT_TIMELINE_1 *con);
	int ShengJian(CONTEXT_TIMELINE_1 *con);
	int LianXuJi(int playerID);
	int JianYing(int playerID);

	bool used_LianXuJi;
	bool used_JianYing;
	bool using_LianXuJi;
	int attackCount;
};
