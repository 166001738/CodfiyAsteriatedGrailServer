#include "XianZhe.h"
#include "..\GameGrail.h"
#include "..\UserTask.h"
#include <set>

enum {
	ELEMENT_ALL_DIFFERENT = 1,
	ELEMENT_ALL_THE_SAME = 2,
	ELEMENT_INVALID = 3,
};

XianZhe::XianZhe(GameGrail *engine, int id, int color): PlayerEntity(engine, id, color)
{
	energyMax = 4;
}

bool XianZhe::cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto)
{
	switch(type)
	{
	case MSG_RESPOND:
		Respond* respond = (Respond*)proto;
		switch(respond->respond_id())
		{
		case FA_SHU_FAN_TAN:
			//tryNotify��������Ϸ���̴߳���Ϣ��ֻ��id���ڵ�ǰ�ȴ�id������state���ڵ�ǰstate������step���ڵ�ǰstep����Ϸ���̲߳Ż����
			session->tryNotify(id, ::STATE_TIMELINE_6_DRAWN, FA_SHU_FAN_TAN, respond);
			return true;
		}
	}
	//ûƥ���򷵻�false
	return false;
}
int XianZhe::v_magic_skill(Action* action)
{
	int actionID = action->action_id();
	vector<int> cards;
	int playerID = action->src_id();
	PlayerEntity* dst;
	int i;

	if(playerID != id){
		return GE_INVALID_PLAYERID;
	}

	switch (actionID)
	{
	case MO_DAO_FA_DIAN:
		for (i = 0; i < action->card_ids_size(); ++i)
			cards.push_back(action->card_ids(i));
		if (GE_SUCCESS != checkHandCards(cards.size(), cards))
			return GE_INVALID_CARDID;
		if (cards.size() < 2 || elementCheck(cards) != ELEMENT_ALL_DIFFERENT || gem == 0)
			return GE_INVALID_ACTION;
		dst = engine->getPlayerEntity(action->dst_ids(0));
		break;
	case SHENG_JIE_FA_DIAN:
		for (i = 0; i < action->card_ids_size(); ++i)
			cards.push_back(action->card_ids(i));
		if (GE_SUCCESS != checkHandCards(cards.size(), cards))
			return GE_INVALID_CARDID;
		if (elementCheck(cards) != ELEMENT_ALL_DIFFERENT || gem == 0 || cards.size()-2 < action->dst_ids_size() || cards.size() < 3)
			return GE_INVALID_ACTION;
		
		for (i = 0; i < action->dst_ids_size(); ++i)
		{
			dst = engine->getPlayerEntity(action->dst_ids(i));
			for (int j = i+1; j < action->dst_ids_size(); ++j)
				if (action->dst_ids(i) == action->dst_ids(j))
					// Ŀ�겻���ظ�
					return GE_INVALID_ACTION;
		}
		break;
	default:
		return GE_INVALID_ACTION;
	}

	return GE_SUCCESS;
}

int XianZhe::p_magic_skill(int &step, Action *action)
{
	int ret;
	switch(action->action_id())
	{
	case MO_DAO_FA_DIAN:
		ret = MoDaoFaDian(action);
		if(GE_SUCCESS == ret){
			step = STEP_DONE;
		}
		break;
	case SHENG_JIE_FA_DIAN:
		ret = ShengJieFaDian(step, action);
		// ������û��State�����Ҫ����step
		if (GE_URGENT == ret) {
			step = SHENG_JIE_FA_DIAN;
		}
		else if (GE_SUCCESS == ret){
			step = STEP_DONE;
		}
		break;
	default:
		return GE_INVALID_ACTION;
	}
	
	return ret;
}

int XianZhe::p_timeline_6_drawn(int &step, CONTEXT_TIMELINE_6_DRAWN *con)
{
	if (con->dstID != id || con->harm.type != HARM_MAGIC)
		return GE_SUCCESS;
	int ret = GE_SUCCESS;
	if (con->harm.point > 3)
		// �˺����ڵ���4,
		ZhiHuiFaDian();
	else if (con->harm.point == 1 && canFanTan())
	{
		// �˺�Ϊ1
		step = FA_SHU_FAN_TAN;
		ret = FaShuFanTan();
		if (toNextStep(ret) || ret == GE_URGENT) {
			step = STEP_DONE;
		}
	}	
	return ret;
}

int XianZhe::ZhiHuiFaDian()
{
	SkillMsg skill_msg;
	Coder::skillNotice(id, dst->getID(), ZHI_HUI_FA_DIAN, skill_msg);
	engine->sendMessage(-1, MSG_SKILL, skill_msg);

	setGem(gem+2);
	GameInfo game_info;
	Coder::energyNotice(id, gem, crystal, game_info);
	engine->sendMessage(-1, MSG_GAME, game_info);

	return GE_SUCCESS;
}

int XianZhe::MoDaoFaDian(Action *action)
{
	vector<int> cards;
	int i;

	for (i = 0; i < action->card_ids_size(); ++i)
		cards.push_back(action->card_ids(i));
	
	SkillMsg skill_msg;
	Coder::skillNotice(id, action->dst_ids(0), MO_DAO_FA_DIAN, skill_msg);
	engine->sendMessage(-1, MSG_SKILL, skill_msg);

	setGem(gem-1);
	GameInfo game_info;
	Coder::energyNotice(id, gem, crystal, game_info);
	engine->sendMessage(-1, MSG_GAME, game_info);

	HARM MoDao;
	MoDao.cause = MO_DAO_FA_DIAN;
	MoDao.point = cards.size()-1;
	MoDao.srcID = id;
	MoDao.type = HARM_MAGIC;
	// �����Լ�����˺�
	engine->setStateTimeline3(id, MoDao);
	engine->setStateTimeline3(action->dst_ids(0), MoDao);
	// �ȶ���
	engine->setStateMoveCardsNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cards.size(), cards, id, MO_DAO_FA_DIAN, true);

	return GE_SUCCESS;
}

int XianZhe::ShengJieFaDian(int &step, Action *action)
{
	if (step != SHENG_JIE_FA_DIAN)
	{
		vector<int> cards;
		int i;

		for (i = 0; i < action->card_ids_size(); ++i)
			cards.push_back(action->card_ids(i));
	
		SkillMsg skill_msg;
		Coder::skillNotice(id, action->dst_ids(0), SHENG_JIE_FA_DIAN, skill_msg);
		engine->sendMessage(-1, MSG_SKILL, skill_msg);

		setGem(gem-1);
		GameInfo game_info;
		Coder::energyNotice(id, gem, crystal, game_info);
		engine->sendMessage(-1, MSG_GAME, game_info);
		// �ȶ���
		engine->setStateMoveCardsNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cards.size(), cards, id, SHENG_JIE_FA_DIAN, true);

		return GE_URGENT;
	}
	else
	{
		HARM ShengJie;
		ShengJie.cause = SHENG_JIE_FA_DIAN;
		ShengJie.point = action->card_ids_size()-1;
		ShengJie.srcID = id;
		ShengJie.type = HARM_MAGIC;
		// �����Լ�����˺�
		engine->setStateTimeline3(id, ShengJie);

		PlayerEntity* dst;
		GameInfo game_info;
		for (int i = 0; i < action->dst_ids_size(); ++i)
		{
			dst = engine->getPlayerEntity(action->dst_ids(i));
			dst->addCrossNum(2);
			Coder::crossNotice(dst->getID(), dst->getCrossNum(), game_info);
		}
		engine->sendMessage(-1, MSG_GAME, game_info);

		return GE_SUCCESS;
	}
}

int XianZhe::FaShuFanTan()
{
	CommandRequest cmd_req;
	Coder::askForSkill(id, FA_SHU_FAN_TAN, cmd_req);
	//���޵ȴ�����UserTask����tryNotify����
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		int ret;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*) reply;
			if (respond->card_ids_size() == 0)
				return GE_SUCCESS;

			vector<int> cards;
			for (int i = 0; i < respond->card_ids_size(); ++i)
				cards.push_back(respond->card_ids(i));
			if (GE_SUCCESS != checkHandCards(cards.size(), cards) || elementCheck(cards) != ELEMENT_ALL_THE_SAME)
				return GE_INVALID_CARDID;
			PlayerEntity* dst = engine->getPlayerEntity(respond->dst_ids(0));

			SkillMsg skill_msg;
			Coder::skillNotice(id, dst->getID(), FA_SHU_FAN_TAN, skill_msg);
			engine->sendMessage(-1, MSG_SKILL, skill_msg);

			HARM FanTan1;
			FanTan1.cause = FA_SHU_FAN_TAN;
			FanTan1.point = cards.size();
			FanTan1.srcID = id;
			FanTan1.type = HARM_MAGIC;
			engine->setStateTimeline3(id, FanTan1);

			HARM FanTan2;
			FanTan2.cause = FA_SHU_FAN_TAN;
			FanTan2.point = cards.size()-1;
			FanTan2.srcID = id;
			FanTan2.type = HARM_MAGIC;
			engine->setStateTimeline3(dst->getID(), FanTan2);

			// �ȶ���
			engine->setStateMoveCardsNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cards.size(), cards, id, FA_SHU_FAN_TAN, true);
		}
		return ret;
	}
	else{
		//��ʱɶ��������
		return GE_TIMEOUT;
	}
	return GE_SUCCESS;
}

// ��⿨��Ϊ��Ϊ��ͬϵ��ͬϵ
int XianZhe::elementCheck(vector<int> cards)
{
	vector<int>::iterator card_it;
	CardEntity* card;
	set<int> elements;
	for (card_it = cards.begin(); card_it != cards.end(); ++card_it)
	{
		card = getCardByID(*card_it);
		elements.insert(card->getElement());
	}
	if (elements.size() == 1)
		return ELEMENT_ALL_THE_SAME;
	if (elements.size() == cards.size())
		return ELEMENT_ALL_DIFFERENT;
	return ELEMENT_INVALID;
}

bool XianZhe::canFanTan()
{
	int element_nums[8];
	int e;
	memset(element_nums, 0, 8);

	list<int>::iterator hand_it;
	for (hand_it = handCards.begin(); hand_it != handCards.end(); ++hand_it)
	{
		e = getCardByID(*hand_it)->getElement();
		if (element_nums[e] == 1)
			return true;
		else
			element_nums[e] = 1;
	}
	return false;
}