#pragma once
#include "..\PlayerEntity.h"

class MoGong: public PlayerEntity
{
public:
	MoGong(GameGrail *engine, int id, int color): PlayerEntity(engine, id, color){
	    coverCardsMax = 8;
		tokenMax[2]=8;
	}
	//�ҳ���ʵ��������Ҫ��p_xxxx�麯������Щp_xxxx�������ڶ�Ӧ��StateXXXXX�ﱻ����
	//ÿ��p_xxxx������в�ֻһ�����ܣ�step��������������Щ��
	 bool cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto);
	 int p_before_turn_begin(int &step, int currentPlayerID);
	 int p_boot(int &step, int currentPlayerID);
	 int v_magic_skill(Action *action);
	 int p_magic_skill(int &step, Action* action);

	 int v_attack_skill(Action *action);
	 int p_attack_skill(int &step, Action *action); 

	 int p_timeline_1(int &step, CONTEXT_TIMELINE_1 *con);
	 int p_timeline_2_hit(int &step, CONTEXT_TIMELINE_2_HIT * con);
	 int p_timeline_2_miss(int &step, CONTEXT_TIMELINE_2_MISS *con);
	 int v_attack(int cardID, int dstID, bool realCard);
	 int p_after_attack(int &step, int playerID);
	 int v_additional_action(int chosen);

	 int p_additional_action(int chosen);
	 int ChongNengMoYan(int PlayerID);
	 int MoYan();
	 int ChongNeng();
	 int ChongNengGaiPai();
	 int MoYanGaiPai();

	 int MoGuanChongJi(CONTEXT_TIMELINE_1 *con);
	 int MoGuanChongJi_Hit(CONTEXT_TIMELINE_2_HIT *con);
	 int DuoChongSheJi(int playerID);
	 int LeiGuangSanShe(Action *action);
	 int DuoChongSheJi_QiPai(Action *action);
private:
	int ChongNengNum;
	int lastTarget;   //Ϊ�������������¼�ϴι���Ŀ��
	//�����ܡ�����ħ�ۡ�
	bool using_CHONG_NENG;
	bool using_MO_YAN;
	//���ڡ�ħ�������롾���������
	bool using_MO_GUAN_CHONG_JI;  
	bool using_DUO_CHONG_SHE_JI;
	//�����б𱾻غϡ�ħ�����������׹����䡿�Ƿ����
	bool available_MO_GUAN_CHONG_JI;
	bool available_LEI_GUANG_SAN_SHE;
	bool available_DUO_CHONG_SHE_JI;
};
