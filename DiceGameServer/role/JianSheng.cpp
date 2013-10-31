#include "JianSheng.h"
#include "..\GameGrail.h"

//��ʼ�����ֻغϱ���
int JianSheng::p_before_turn_begin(int &step, int currentPlayerID) 
{
	used_LianXuJi = false;
	used_JianYing = false;
	attackCount = 0;
	return GE_SUCCESS; 
}

//�ڳ����ʱ��p_xxxx�п���ִ�в�ֹһ�Σ���ÿ�ζ���ͷ�����Ļ�������������Ҫstep��¼ִ�е�����
int JianSheng::p_timeline_1(int &step, CONTEXT_TIMELINE_1 *con)
{
	int ret = GE_INVALID_STEP;
	//���ɹ�����������ߣ�ʧ���򷵻أ�step�ᱣ�����´��ٽ����Ͳ�������
	switch(step)
	{
	case 0:
		if(GE_SUCCESS != (ret=LieFengJi(con))){
			break;
		}
		step++;
	case 1:
		if(GE_SUCCESS != (ret=JiFengJi(con))){
			break;
		}
		step++;
	case 2:
		if(GE_SUCCESS != (ret=ShengJian(con))){
			break;
		}
		step++;
	//Ϊ�˷�ֹstep2��ʱ
	case 3:
		ret = GE_SUCCESS;
	}
	//��ʱ�Ļ���ֱ�ӽ�����һ��
	if(GE_TIMEOUT == ret){
		step++;
	}
	return ret;
}

//���ж����ж������Ǽ��е�һ���ط�ѯ�ʣ�������ÿ������һ��
int JianSheng::p_after_attack(int &step, int playerID)
{
	int ret = GE_INVALID_STEP;
	switch(step)
	{
	case 0:
		if(GE_SUCCESS != (ret=LianXuJi(playerID))){
			break;
		}
		step++;
	case 1:
		if(GE_SUCCESS != (ret=JianYing(playerID))){
			break;
		}
		step++;
	case 2:
		ret = GE_SUCCESS;
	}
	//��ʵ����û��ѯ�ʣ�����û�г�ʱ
	if(GE_TIMEOUT == ret){
		step++;
	}
	return ret;
}

int JianSheng::LieFengJi(CONTEXT_TIMELINE_1 *con)
{
	int srcID = con->attack.srcID;
	int dstID = con->attack.dstID;
	int cardID = con->attack.cardID;
	CardEntity* card = getCardByID(cardID);
	if(srcID != id || !card->checkSpeciality(LIE_FENG_JI)){
		return GE_SUCCESS;
	}
	PlayerEntity* dst = engine->getPlayerEntity(dstID);
	if(!dst->checkBasicEffectName(NAME_SHIELD)){
		return GE_SUCCESS;
	}
	//if(engine->waitForOne(id, network::MSG_CMD_REQ, weaken_proto)){

	//}
	//else{
	//	return GE_TIMEOUT;
	//}
	return GE_SUCCESS;
}

int JianSheng::JiFengJi(CONTEXT_TIMELINE_1 *con)
{
	return GE_SUCCESS;
}

int JianSheng::ShengJian(CONTEXT_TIMELINE_1 *con)
{
	return GE_SUCCESS;
}

int JianSheng::LianXuJi(int playerID)
{
	return GE_SUCCESS;
}

int JianSheng::JianYing(int playerID)
{
	return GE_SUCCESS;
}