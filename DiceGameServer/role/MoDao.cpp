#include "MoDao.h"
#include "..\GameGrail.h"
#include "..\UserTask.h"
//FIXME: ����Ժ�������ת�����ƣ����޷�ͨ������
int MoDao::v_request_hand(int howMany, vector<int> cards, HARM harm)
{
	if(harm.cause == MO_BAO_CHONG_JI){
		int cardID = cards[0];
		CardEntity* card = getCardByID(cardID);
		if(card->getType() != TYPE_MAGIC){
			return GE_INVALID_CARDID;
		}
	}
	return GE_SUCCESS;
}
//��������
int MoDao::p_request_hand_give_up(int &step, int targetID, int cause)
{
	if(cause == MO_BAO_CHONG_JI){
		HARM moBao;
		moBao.cause = MO_BAO_CHONG_JI;
		moBao.point = 2;
		moBao.srcID = id;
		moBao.type = HARM_MAGIC;
		engine->setStateTimeline3(targetID, moBao);
		if(!isHit){
			isHit = true;
			TeamArea* m_teamArea = engine->getTeamArea();
			m_teamArea->setGem(color, m_teamArea->getGem(color)+1);
			GameInfo update_info;
			if (color == RED)
				update_info.set_red_gem(m_teamArea->getGem(color));
			else
				update_info.set_blue_gem(m_teamArea->getGem(color));
			engine->sendMessage(-1, MSG_GAME, update_info);
		}
		return GE_URGENT;
	}
	return GE_INVALID_ACTION;
}

int MoDao::v_magic_skill(Action *action)
{
	int actionID = action->action_id();
	int cardID;
	int playerID = action->src_id();
	CardEntity* card;

	if(playerID != id){
		return GE_INVALID_PLAYERID;
	}
	switch(action->action_id())
	{
	case MO_BAO_CHONG_JI:
		cardID = action->card_ids(0);
		card = getCardByID(cardID);
		//�����Լ�������                          || ���Ƿ�����                 
		if(GE_SUCCESS != checkOneHandCard(cardID) || TYPE_MAGIC != card->getType()){
			return GE_INVALID_CARDID;
		}
		return checkTwoTarget(action);
	case MO_DAN_RONG_HE:
		cardID = action->card_ids(0);
		card = getCardByID(cardID);
		//�����Լ�������                          || ���ǻ��ϵ��                 
		if(GE_SUCCESS != checkOneHandCard(cardID) || ELEMENT_FIRE != card->getElement() && ELEMENT_EARTH != card->getElement()){
			return GE_INVALID_CARDID;
		}
		return GE_SUCCESS;
	case HUI_MIE_FENG_BAO:
		//����             
		if(gem <= 0){
			return GE_INVALID_ACTION;
		}
		return checkTwoTarget(action);
	default:
		return GE_INVALID_ACTION;
	}
}

int MoDao::checkTwoTarget(Action* action)
{
	int dst1ID;
	int dst2ID;
	PlayerEntity* dst1;
	PlayerEntity* dst2;
	if(action->dst_ids_size()!=2){
		return GE_INVALID_PLAYERID;
	}
	dst1ID = action->dst_ids(0);
	dst2ID = action->dst_ids(1);
	dst1 = engine->getPlayerEntity(dst1ID);
	dst2 = engine->getPlayerEntity(dst2ID);
	//���Ƕ���                   || ���Ƕ���                  || ͬһ��
	if(dst1->getColor() == color || dst2->getColor() == color || dst1ID == dst2ID){
		return GE_INVALID_PLAYERID;
	}
	return GE_SUCCESS;
}

int MoDao::p_magic_skill(int &step, Action* action)
{
	//p_magic_skill��ͬ�ڱ�Ĵ����㣬����ֻ��һ��ƥ�䣬���ÿһ���������ʱ����ذ�step��ΪSTEP_DONE
	int ret;
	switch(action->action_id())
	{
	case MO_BAO_CHONG_JI:
		ret = MoBaoChongJi(action);
		if(GE_URGENT == ret){
			step = STEP_DONE;
		}
		break;
	case MO_DAN_RONG_HE:
		ret = MoDanRongHe(action);
		if(GE_URGENT == ret){
			step = STEP_DONE;
		}
		break;
	case HUI_MIE_FENG_BAO:
		ret = HuiMieFengBao(action);
		if(GE_URGENT == ret){
			step = STEP_DONE;
		}
		break;
	default:
		return GE_INVALID_ACTION;
	}
	return ret;
}

int MoDao::v_missile(int cardID, int dstID, bool realCard)
{
	int ret;
	CardEntity* card = getCardByID(cardID);
	if(GE_SUCCESS != (ret = checkOneHandCard(cardID) || card->getName() != NAME_MISSILE && card->getElement() != ELEMENT_FIRE && card->getElement() != ELEMENT_EARTH)){
		return GE_INVALID_CARDID; 
	}
	PlayerEntity *it = this;
	while((it = it->getPost())->getColor() == color)
		;
	if(dstID == it->getID()){
		return GE_SUCCESS;
	}
	it = this;
	while((it = it->getPre())->getColor() == color)
		;
	if(dstID == it->getID()){
		return GE_SUCCESS;
	}
	return GE_INVALID_PLAYERID;
}

int MoDao::v_remissile(int cardID, bool realCard)
{
	int ret;
	CardEntity* card = getCardByID(cardID);
	if(GE_SUCCESS != (ret = checkOneHandCard(cardID) || card->getName() != NAME_MISSILE && card->getElement() != ELEMENT_FIRE && card->getElement() != ELEMENT_EARTH)){
		return GE_INVALID_CARDID; 
	}
	return GE_SUCCESS;
}

int MoDao::MoBaoChongJi(Action* action)
{
	isHit = false;
	int cardID = action->card_ids(0);
	int dst1ID = action->dst_ids(0);
	int dst2ID = action->dst_ids(1);
	list<int> dstIDs;
	dstIDs.push_back(dst1ID);
	dstIDs.push_back(dst2ID);
	//���漼��
	network::SkillMsg skill;
	Coder::skillNotice(id, dstIDs, MO_BAO_CHONG_JI, skill);
	engine->sendMessage(-1, MSG_SKILL, skill);
	CardMsg show_card;
	Coder::showCardNotice(id, 1, cardID, show_card);
	engine->sendMessage(-1, MSG_CARD, show_card);
    //�������Ʋ�����Ҫ��setStateMoveXXXX��ToHand�Ļ�Ҫ���HARM�����㲻���˺�
	PlayerEntity* it = this->getPre();
	HARM moBao;
	moBao.cause = MO_BAO_CHONG_JI;
	moBao.point = 1;
	moBao.srcID = id;
	moBao.type = HARM_NONE;
	//�Ƚ���������������˳��ѹ��������ħ���Լ���������
	while(it != this){
		if(it->getID() == dst1ID || it->getID() == dst2ID){
			engine->pushGameState(new StateRequestHand(it->getID(), moBao, -1, DECK_DISCARD, true, true));
		}
		it = it->getPre();
	}
	engine->setStateMoveOneCardNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cardID, id, MO_BAO_CHONG_JI, true);
	//��������״̬����return GE_URGENT
	return GE_URGENT;
}

int MoDao::MoDanRongHe(Action* action)
{
	//���漼��
	network::SkillMsg skill;
	Coder::skillNotice(id, action->dst_ids(0), MO_DAN_RONG_HE, skill);
	engine->sendMessage(-1, MSG_SKILL, skill);
	engine->pushGameState(StateMissiled::create(engine, action->card_ids(0), action->dst_ids(0), id));
	engine->setStateUseCard(action->card_ids(0), action->dst_ids(0), id);
	//��������״̬����return GE_URGENT
	return GE_URGENT;
}

int MoDao::HuiMieFengBao(Action* action)
{
	int dst1ID = action->dst_ids(0);
	int dst2ID = action->dst_ids(1);
	list<int> dstIDs;
	dstIDs.push_back(dst1ID);
	dstIDs.push_back(dst2ID);
	//���漼��
	network::SkillMsg skill;
	Coder::skillNotice(id, dstIDs, HUI_MIE_FENG_BAO, skill);
	engine->sendMessage(-1, MSG_SKILL, skill);
	//������
	network::GameInfo update;
	setGem(--gem);
	Coder::energyNotice(id, gem, crystal, update);
	engine->sendMessage(-1, MSG_GAME, update);
    //��д�˺��ṹ
	PlayerEntity* it = this->getPre();
	HARM huiMie;
	huiMie.cause = HUI_MIE_FENG_BAO;
	huiMie.point = 2;
	huiMie.srcID = id;
	huiMie.type = HARM_MAGIC;
	//�Ƚ���������������˳��ѹ
	while(it != this){
		if(it->getID() == dst1ID || it->getID() == dst2ID){
			engine->setStateTimeline3(it->getID(), huiMie);
		}
		it = it->getPre();
	}
	//��������״̬����return GE_URGENT
	return GE_URGENT;
}