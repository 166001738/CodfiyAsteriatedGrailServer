#pragma once
#include "..\PlayerEntity.h"

class  MoQiang: public PlayerEntity
{
public:
	 MoQiang(GameGrail *engine, int id, int color): PlayerEntity(engine, id, color){};
	 bool cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto);
	 int  p_before_turn_begin(int &step, int currentPlayerID);
	 int  p_boot(int &step, int currentPlayerID);
	 int p_timeline_2_hit(int &step, CONTEXT_TIMELINE_2_HIT * con);
	 int p_timeline_3(int &step, CONTEXT_TIMELINE_3 *con);
	 int p_lose_morale(int &step, CONTEXT_LOSE_MORALE *con);
	 int v_magic_skill(Action *action);
	 int p_magic_skill(int &step, Action* action);
	 int v_request_hand(int howMany, vector<int> cards, HARM harm);
	 int v_missile(int cardID, int dstID, bool realCard);
	 int v_shield(int cardID, PlayerEntity* dst);
	 int v_weaken(int cardID, PlayerEntity* dst);
	 int v_remissile(int cardID, bool realCard);
	 int v_block(int cardID);
	 int AnZhiHuanYing();
	 int ChongYing(Action* action);
	 int AnZhiJieFang_Effect(CONTEXT_TIMELINE_2_HIT *con);
	 int AnZhiZhangBi(CONTEXT_TIMELINE_3 *con);
	 int QiHeiZhiQiang(CONTEXT_TIMELINE_2_HIT *con);
	 int HuanYingXingChen_Effect(CONTEXT_LOSE_MORALE *con);
 private:
	//���弼��
int      cardCount;
int      hurtID;  
bool     using_AnZhiJieFang;      //�ս��롾��Ӱ��̬��
bool     using_HuanYingXingCeng;  //
bool	 availabel_QiHeiZhiQiang;
bool	 availabel_ChongYing;
bool	 used_ChongYing;
};