#pragma once
#include "..\PlayerEntity.h"

class MaoXian : public PlayerEntity
{
public:
	MaoXian(GameGrail *engine, int id, int color): PlayerEntity(engine, id, color){}
	//�ҳ���ʵ��������Ҫ��p_xxxx�麯������Щp_xxxx�������ڶ�Ӧ��StateXXXXX�ﱻ����
	//ÿ��p_xxxx������в�ֻһ�����ܣ�step��������������Щ��
	int p_before_turn_begin(int &step, int currentPlayerID);
	int p_attack_skill(int &step, Action* action);
	int p_magic_skill(int &step, Action* action);
	int p_special_skill(int &step, Action* action);
	//v_xxxx��������⣬����State���ã�һ������ʵ�֣�������ʥ�������������Ƿ�ն����������⣬ֻ�ܸ�дv_xxxx
	int v_buy(Action *action);
	int v_attack_skill(Action *action);
	int v_magic_skill(Action *action);
	int v_special_skill(Action *action);
private:
	//���弼��
	int QiZha(int step, Action *action);
	int TouTianHuanRi(Action *action);
	int TeShuJiaGong(Action *action);
	int MaoXianZheTianTang(Action *action);
	//��Ӧ������٣�͵�컻�ո�����ӹ�����һ��flag
	bool used_TouTianHuanRi;
};
