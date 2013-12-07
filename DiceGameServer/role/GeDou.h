#pragma once
#include "..\PlayerEntity.h"

class GeDou : public PlayerEntity
{
public:
	GeDou(GameGrail *engine, int id, int color): PlayerEntity(engine, id, color){xuLiUsed  = cangYanUsed = baiShiUsed = false; baiShiTarget = -1; token[0]=0; tokenMax[0]=6;}
	//�ҳ���ʵ��������Ҫ��p_xxxx�麯������Щp_xxxx�������ڶ�Ӧ��StateXXXXX�ﱻ����
	//ÿ��p_xxxx������в�ֻһ�����ܣ�step��������������Щ��
	int p_boot(int &step, int currentPlayerID);
	int p_timeline_1(int &step, CONTEXT_TIMELINE_1 *con);
	int p_timeline_2_miss(int &setp, CONTEXT_TIMELINE_2_MISS *con);
	int p_timeline_3(int &step, CONTEXT_TIMELINE_3 *con);
	int p_after_attack(int &step, int currentPlayerID);
	int p_before_magic(int &step, int currentPlayerID);
	int p_after_magic(int &step, int currentPlayerID);
	int p_before_special(int &step, int currentPlayerID);
	//��ɫ����Ϣ��������UserTask���ã�ֻҪʹ����waitForXXX��������Ҫ��д�������
	//return true ��ʾ������
	bool cmdMsgParse(UserTask* session, uint16_t type, ::google::protobuf::Message *proto);
private:
	//���弼��
	int NianQiLiChang(CONTEXT_TIMELINE_3 *con);
	int XuLiCangYan(CONTEXT_TIMELINE_1 *con);
	int XuLiMiss(CONTEXT_TIMELINE_2_MISS *con);
	int NianDan(int step, int playerID);
	int CangYanZiShang(int playerID);
	int BaiShiDouShen(int playerID);
	int BaiShiHarm(CONTEXT_TIMELINE_1 *con);
	int BaiShiQuitMagic(int playerID);
	int BaiShiQuitSpecial(int playerID);
	int baiShiTarget;
	bool xuLiUsed;
	bool cangYanUsed;
	bool baiShiUsed;
};
