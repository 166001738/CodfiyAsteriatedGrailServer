#include "GrailState.h"
#include "GameGrail.h"
#include <Windows.h>

int StateWaitForEnter::handle(GameGrail* engine)
{
	//ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateWaitForEnter", engine->getGameId());
	if(engine->getGameNowPlayers() >= engine->getGameMaxPlayers())
	{
		engine->popGameState();
		engine->pushGameState(new StateSeatArrange);
	}
	Sleep(1000);
	return GE_SUCCESS;
}

int StateSeatArrange::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateSeatArrange", engine->getGameId());
	
	int m_maxPlayers = engine->getGameMaxPlayers();
	
	// ֱ�ӽ����������浽engine��
	GameInfo& game_info = engine->game_info;

	if(!isSet)
	{
		if(engine->m_seatMode == 0)
		{
			// ���λ����ұ�ţ�������Deckֻ��Ϊ��ʹ�����Ĵ��ҹ���
			Deck ids(m_maxPlayers);
			ids.init(0, m_maxPlayers - 1);
			//FIXME: disable random for debug
			//ids.randomize();

			// �����Ҷ��飬������Deckֻ��Ϊ��ʹ�����Ĵ��ҹ���
			Deck colors(m_maxPlayers);
			int temp[8];
			memset(temp, 0, m_maxPlayers/2 * sizeof(int));
			for (int i = m_maxPlayers/2; i < m_maxPlayers; ++i)
				temp[i] = 1;
			colors.push(m_maxPlayers, temp);
			//FIXME: disable random for debug
			//colors.randomize();
			
			SinglePlayerInfo *player_info;
			int it;
			for(int i = 0; i < m_maxPlayers; i++){
				game_info.add_player_infos();
				player_info = (SinglePlayerInfo*)&(game_info.player_infos().Get(i));

				player_info->set_seat(i);

				ids.pop(1, &it);
				player_info->set_id(it);

				colors.pop(1, &it);
				player_info->set_team(it);
			}

			for(int i = 0; i < m_maxPlayers; i++){
				messages[i] = &game_info;
			}
			engine->resetReady();
		}
		isSet = true;
	}
	if(engine->waitForAll(MSG_GAME, (void**)messages, false)){
		engine->popGameState();	  
		return engine->setStateRoleStrategy();
	}
	else{
		return GE_TIMEOUT;
	}
}

int StateRoleStrategyRandom::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateRoleStrategyRandom", engine->getGameId());
	int out;
	Deck* roles = engine->initRoles();
	int ret;
	// ֱ�ӽ����������浽engine��
	GameInfo& game_info = engine->game_info;

	for(int i = 0; i < engine->getGameMaxPlayers(); i++){
		// iΪ��ұ�ţ���������		
		if(GE_SUCCESS == (ret=roles->pop(1, &out))){
			//FIXME: ȫ��ӡʱ��
			Coder::roleNotice(i, 8, game_info);
			engine->sendMessage(-1, MSG_GAME, game_info);
		}
		else{
			return ret;
		}
	}
	SAFE_DELETE(roles);
	engine->initPlayerEntities();
	engine->popGameState();
	engine->pushGameState(new StateGameStart);
	return GE_SUCCESS;
}

int StateGameStart::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateGameStart", engine->getGameId());
	if(!isSet){
		isSet=true;
		engine->initDecks();
	}
	int ret;
	vector< int > cards;
	HARM harm;
	harm.type = HARM_NONE;
	harm.point = 4;
	harm.srcID = -1;
	harm.cause = CAUSE_DEFAULT;
	//LIFO
	for(int i = iterator; i < engine->getGameMaxPlayers(); i++){
		ret = engine->setStateMoveCardsToHand(-1, DECK_PILE, i, DECK_HAND, 4, cards, harm);
		iterator++;
		return ret;
	}
	engine->popGameState();	
	return engine->setStateCurrentPlayer(engine->game_info.player_infos().begin()->id());
}

int StateBeforeTurnBegin::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateBeforeTurnBegin", engine->getGameId());
	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();
	while(iterator < engine->getGameMaxPlayers()){	    
		ret = engine->getPlayerEntity(iterator)->p_before_turn_begin(step, m_currentPlayerID);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}
	if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_BEFORE_TURN_BEGIN))){
		engine->pushGameState(new StateTurnBegin);
	}
	return ret;
}

int StateTurnBegin::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateTurnBegin", engine->getGameId());
	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID= engine->getCurrentPlayerID();
	while(iterator < engine->getGameMaxPlayers()){	    
		ret = engine->getPlayerEntity(iterator)->p_turn_begin(step, m_currentPlayerID);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}
	if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_TURN_BEGIN))){
		return engine->setStateCheckBasicEffect();
	}
	return ret;
}

int StateWeaken::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateWeaken", engine->getGameId());
	int m_currentPlayerID = engine->getCurrentPlayerID();

	CommandRequest weaken_proto;
	Coder::askForWeak(m_currentPlayerID, howMany, weaken_proto);
	if(engine->waitForOne(m_currentPlayerID, network::MSG_CMD_REQ, weaken_proto))
	{
		void* reply;
		int ret;
		if(GE_SUCCESS == (ret=engine->getReply(m_currentPlayerID, reply)))
		{
			Respond *weak_respond = (Respond*)reply;

			if(weak_respond->args().Get(0) == 1){
				int srcID_t = srcID;
				int howMany_t = howMany;
				engine->popGameState();
				engine->pushGameState(new StateBeforeAction);
				HARM harm;
				harm.type = HARM_NONE;
				harm.point = howMany_t;
				harm.srcID = srcID_t;
				harm.cause = CAUSE_WEAKEN;
				vector< int > cards;
				return engine->setStateMoveCardsToHand(-1, DECK_PILE, m_currentPlayerID, DECK_HAND, howMany_t, cards, harm);
			}
			else{
				engine->popGameState();
				engine->pushGameState(new StateTurnEnd);
				return GE_SUCCESS;
			}
		}
		return ret;
	}
	else
	{
		//Timeout, skip to nextState
		Respond weak_respond;
		weak_respond.set_src_id(m_currentPlayerID);
		weak_respond.add_args(0);

		engine->sendMessage(-1, MSG_RESPOND, weak_respond);
		engine->popGameState();
		engine->pushGameState(new StateTurnEnd);
		return GE_TIMEOUT;
	}

}

int StateBeforeAction::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter handleBeforeAction", engine->getGameId());
	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();

	while(iterator < engine->getGameMaxPlayers()){	    
		ret = engine->getPlayerEntity(iterator)->p_before_action(step, m_currentPlayerID);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}
	if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_BEFORE_ACTION))){
		engine->pushGameState(new StateBoot);
	}
	return ret;
}

int StateBoot::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateBoot", engine->getGameId());
	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();

	while(iterator < engine->getGameMaxPlayers()){	    
		ret = engine->getPlayerEntity(iterator)->p_boot(step, m_currentPlayerID);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}
	if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_BOOT))){
		engine->pushGameState(new StateActionPhase(ACTION_ANY, false));
	}
	return ret;
}

int StateActionPhase::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateActionPhase", engine->getGameId());

	//FIXME: ����

	int m_currentPlayerID = engine->getCurrentPlayerID();

	CommandRequest cmd_req;
	Coder::askForAction(m_currentPlayerID, allowAction, canGiveUp, cmd_req);
	if(engine->waitForOne(m_currentPlayerID, MSG_CMD_REQ, cmd_req))
	{
		void* temp;
		int ret;
		if(GE_SUCCESS == (ret = engine->getReply(m_currentPlayerID, temp))){
			Action *action = (Action*) temp;
			PlayerEntity *src;
			src = engine->getPlayerEntity(m_currentPlayerID);
			if(GE_SUCCESS != (ret = src->v_allow_action(action->action_type(), allowAction, canGiveUp))){
				return ret;
			}
			switch(action->action_type())
			{
			case ACTION_ATTACK:
				return basicAttack(action, engine);
			case ACTION_MAGIC:
				return basicMagic(action, engine);
			case ACTION_SPECIAL:
				return basicSpecial(action, engine);
			case ACTION_MAGIC_SKILL:
				return magicSkill(action, engine);
			case ACTION_NONE:
				engine->popGameState_if(STATE_ACTION_PHASE);
			    return engine->setStateCheckTurnEnd();
			default:
				return GE_INVALID_ACTION;
			}
		}
		return ret;
	}
	else{
		if(canGiveUp){
			engine->popGameState_if(STATE_ACTION_PHASE);
			engine->setStateCheckTurnEnd();
		}
		return GE_TIMEOUT;
	}
}

int StateActionPhase::basicAttack(Action *action, GameGrail* engine)
{
	int ret;
	int card_id = action->args().Get(0);
	int m_currentPlayerID = engine->getCurrentPlayerID();
	PlayerEntity *src = engine->getPlayerEntity(m_currentPlayerID);
	if(GE_SUCCESS == (ret = src->v_attack(card_id, action->dst_ids().Get(0)))){
		engine->popGameState();
		return engine->setStateAttackAction(card_id, action->dst_ids().Get(0), action->src_id());
	}
	return ret;
}

int StateActionPhase::basicMagic(Action *action, GameGrail* engine)
{
	int ret;
	int card_id = action->args().Get(0);
	int m_currentPlayerID = engine->getCurrentPlayerID();
	PlayerEntity *src = engine->getPlayerEntity(m_currentPlayerID);
	PlayerEntity *dst = engine->getPlayerEntity(action->dst_ids(0));
	switch(getCardByID(card_id)->getName())
	{
	case NAME_POISON:
		if (GE_SUCCESS == (ret = src->checkOneHandCard(card_id))){
			engine->popGameState();
			engine->pushGameState(new StateAfterMagic(m_currentPlayerID));
			engine->setStateUseCard(card_id, action->dst_ids(0), m_currentPlayerID, true);
			engine->pushGameState(new StateBeforeMagic(m_currentPlayerID));
			return GE_SUCCESS;
		}
		break;
	case NAME_SHIELD:
		if (GE_SUCCESS == (ret = src->v_shield(card_id, dst))){
			engine->popGameState();
			engine->pushGameState(new StateAfterMagic(m_currentPlayerID));
			engine->setStateUseCard(card_id, action->dst_ids(0), m_currentPlayerID, true);
			engine->pushGameState(new StateBeforeMagic(m_currentPlayerID));
			return GE_SUCCESS;
		}
		break;
	case NAME_MISSILE:
		if (GE_SUCCESS == (ret = src->v_missile(card_id, action->dst_ids(0)))){
			engine->popGameState();
			engine->pushGameState(new StateAfterMagic(m_currentPlayerID));
			engine->pushGameState(StateMissiled::create(engine, card_id, action->dst_ids(0), m_currentPlayerID));
			engine->setStateUseCard(card_id, action->dst_ids(0), m_currentPlayerID);
			engine->pushGameState(new StateBeforeMagic(m_currentPlayerID));
			return GE_SUCCESS;
		}
		break;
	case NAME_WEAKEN:
		if (GE_SUCCESS == (ret = src->v_weaken(card_id, dst))){
			engine->popGameState();
			engine->pushGameState(new StateAfterMagic(m_currentPlayerID));
			engine->setStateUseCard(card_id, action->dst_ids(0), m_currentPlayerID, true);
			engine->pushGameState(new StateBeforeMagic(m_currentPlayerID));
			return GE_SUCCESS;
		}
	}
	return ret;
}

int StateActionPhase::basicSpecial(Action *action, GameGrail* engine)
{
	int ret;
	int m_currentPlayerID = engine->getCurrentPlayerID();
	PlayerEntity *src = engine->getPlayerEntity(m_currentPlayerID);
	PlayerEntity *dst;
	switch(action->action_id())
	{
	case SPECIAL_BUY:
		if (GE_SUCCESS == (ret = src->v_buy(action))){
			engine->sendMessage(-1, MSG_ACTION, *action);
			GameInfo update_info;
			TeamArea *team = engine->getTeamArea();
			int color = src->getColor();
			team->setGem(color, team->getGem(color)+1);
			team->setCrystal(color, team->getCrystal(color)+1);
			if (color == RED){
				update_info.set_red_gem(team->getGem(color));
				update_info.set_red_crystal(team->getCrystal(color));
			}
			else{
				update_info.set_blue_gem(team->getGem(color));
				update_info.set_blue_crystal(team->getCrystal(color));
			}
			engine->sendMessage(-1, MSG_GAME, update_info);
			engine->popGameState();
			engine->pushGameState(new StateAfterSpecial(m_currentPlayerID));
			vector<int> cards;
			HARM buy;
			buy.cause = CAUSE_BUY;
			buy.point = 3;
			buy.srcID = m_currentPlayerID;
			buy.type = HARM_NONE;
			engine->setStateMoveCardsToHand(-1, DECK_PILE, m_currentPlayerID, DECK_HAND, 3, cards, buy);
			engine->pushGameState(new StateBeforeSpecial(m_currentPlayerID));
			return GE_SUCCESS;
		}
		break;
	case SPECIAL_SYNTHESIZE:
		if (GE_SUCCESS == (ret = src->v_synthesize(action, engine->getTeamArea()))){
			engine->sendMessage(-1, MSG_ACTION, *action);
			GameInfo update_info;
			TeamArea *team = engine->getTeamArea();
			int color = src->getColor();
			int gem = action->args(0);
			int crystal = action->args(1);
			team->setGem(color, team->getGem(color)-gem);
			team->setCrystal(color, team->getCrystal(color)-crystal);
			team->setCup(color, team->getCup(color)+1);
			if (color == RED){
				update_info.set_red_gem(team->getGem(color));
				update_info.set_red_crystal(team->getCrystal(color));
				update_info.set_red_grail(team->getCup(color));
			}
			else{
				update_info.set_blue_gem(team->getGem(color));
				update_info.set_blue_crystal(team->getCrystal(color));
				update_info.set_blue_grail(team->getCup(color));
			}
			engine->sendMessage(-1, MSG_GAME, update_info);
			engine->popGameState();
			engine->pushGameState(new StateAfterSpecial(m_currentPlayerID));
			CONTEXT_LOSE_MORALE* morale = new CONTEXT_LOSE_MORALE;
			dst = src;
			while(dst->getColor() == src->getColor()){
				dst = dst->getPost();
			}
			if(team->getCup(color) == 5){
				engine->pushGameState(new StateGameOver(color));
			}
			HARM grail;
			grail.cause = CAUSE_SYNTHESIZE;
			grail.point = 1;
			grail.srcID = m_currentPlayerID;
			grail.type = HARM_NONE;
			morale->dstID = dst->getID();
			morale->harm = grail;
			morale->howMany = 1;
			engine->pushGameState(new StateTrueLoseMorale(morale));
			vector<int> cards;
			HARM synthesize;
			synthesize.cause = CAUSE_SYNTHESIZE;
			synthesize.point = 3;
			synthesize.srcID = m_currentPlayerID;
			synthesize.type = HARM_NONE;
			engine->setStateMoveCardsToHand(-1, DECK_PILE, m_currentPlayerID, DECK_HAND, 3, cards, synthesize);
			engine->pushGameState(new StateBeforeSpecial(m_currentPlayerID));
			return GE_SUCCESS;
		}
		break;
	case SPECIAL_EXTRACT:
		if (GE_SUCCESS == (ret = src->v_extract(action, engine->getTeamArea()))){
			engine->sendMessage(-1, MSG_ACTION, *action);
			GameInfo update_info;
			TeamArea *team = engine->getTeamArea();
			int color = src->getColor();
			int gem = action->args(0);
			int crystal = action->args(1);
			team->setGem(color, team->getGem(color)-gem);
			team->setCrystal(color, team->getCrystal(color)-crystal);
			src->setGem(src->getGem()+gem);
			src->setCrystal(src->getCrystal()+crystal);
			if (color == RED){
				update_info.set_red_gem(team->getGem(color));
				update_info.set_red_crystal(team->getCrystal(color));
			}
			else{
				update_info.set_blue_gem(team->getGem(color));
				update_info.set_blue_crystal(team->getCrystal(color));
			}
			SinglePlayerInfo* player_info = update_info.add_player_infos();
		    player_info->set_id(m_currentPlayerID);
			player_info->set_gem(src->getGem());
			player_info->set_crystal(src->getCrystal());
			engine->sendMessage(-1, MSG_GAME, update_info);
			engine->popGameState();
			engine->pushGameState(new StateAfterSpecial(m_currentPlayerID));
			engine->pushGameState(new StateBeforeSpecial(m_currentPlayerID));
		}
		break;
	}
}

int StateActionPhase::magicSkill(Action *action, GameGrail* engine)
{
	int ret;
	int m_currentPlayerID = engine->getCurrentPlayerID();
	PlayerEntity *src = engine->getPlayerEntity(m_currentPlayerID);
	if(GE_SUCCESS == (ret = src->v_magic_skill(action))){
		engine->popGameState();
		engine->pushGameState(new StateAfterMagic(m_currentPlayerID));
		engine->pushGameState(new StateMagicSkill(action));
		engine->pushGameState(new StateBeforeMagic(m_currentPlayerID));
		return GE_SUCCESS;
	}
	return ret;
}

int StateBeforeAttack::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateBeforeAttack", engine->getGameId());
	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();

	while(iterator < engine->getGameMaxPlayers()){	    
		ret = engine->getPlayerEntity(iterator)->p_before_attack(step, dstID, srcID);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}
	return engine->popGameState_if(STATE_BEFORE_ATTACK);
}

int StateAttacked::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateAttacked", engine->getGameId());
	if(RATE_NOMISS == context->hitRate){
		CONTEXT_TIMELINE_1 temp = *context;
		engine->popGameState();
		return engine->setStateTimeline2Hit(temp.attack.cardID, temp.attack.dstID, temp.attack.srcID, temp.harm, temp.attack.isActive);
	}
	CommandRequest cmd_req;
	Coder::askForReBat(context->hitRate, context->attack.cardID, context->attack.dstID, context->attack.srcID, cmd_req);

	if(engine->waitForOne(context->attack.dstID, MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		int ret;

		CONTEXT_TIMELINE_1 temp = *context;
		if(GE_SUCCESS == (ret = engine->getReply(context->attack.dstID, reply))){
			Respond *respond_attack = (Respond*) reply;

			int ra = respond_attack->args(0);
			int card_id;

			switch(ra)
			{
				//FIXME: verify
			case RA_ATTACK:
				card_id = respond_attack->args(1);
				if(GE_SUCCESS == (ret=engine->getPlayerEntity(context->attack.dstID)->v_reattack(card_id, temp.attack.cardID, respond_attack->dst_ids().Get(0), temp.attack.srcID, context->hitRate))){
					// ��������ж�
					respond_attack->set_src_id(context->attack.dstID);					
					engine->popGameState();
					return engine->setStateReattack(temp.attack.cardID, card_id, temp.attack.srcID, temp.attack.dstID, respond_attack->dst_ids().Get(0), temp.attack.isActive, true);
				}
				break;
			case RA_BLOCK:
				card_id = respond_attack->args(1);
				if(GE_SUCCESS == (ret=engine->getPlayerEntity(context->attack.dstID)->v_block(card_id))){
					// ��������ж�
					respond_attack->set_src_id(context->attack.dstID);

					engine->popGameState();
					engine->setStateTimeline2Miss(temp.attack.cardID, temp.attack.dstID, temp.attack.srcID, temp.attack.isActive);
					return engine->setStateUseCard(card_id, temp.attack.srcID, temp.attack.dstID);				
				}
				break;
			case RA_GIVEUP:
				engine->popGameState();
				return engine->setStateAttackGiveUp(temp.attack.cardID, temp.attack.dstID, temp.attack.srcID, temp.harm, temp.attack.isActive, temp.checkShield);
				break;
			}
		}
		return ret;
	}
	else{
		CONTEXT_TIMELINE_1 temp = *context;
		engine->popGameState();
		engine->setStateAttackGiveUp(temp.attack.cardID, temp.attack.dstID, temp.attack.srcID, temp.harm, temp.attack.isActive, temp.checkShield);
		return GE_TIMEOUT;
	}
}

int StateAfterAttack::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateAfterAttack", engine->getGameId());
	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();

	while(iterator < engine->getGameMaxPlayers()){	    
		ret = engine->getPlayerEntity(iterator)->p_after_attack(step, srcID);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}
	if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_AFTER_ATTACK))){
		return engine->setStateCheckTurnEnd();
	}
	return ret;
}

int StateBeforeMagic::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateBeforeMagic", engine->getGameId());
	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();

	while(iterator < engine->getGameMaxPlayers()){	    
		ret = engine->getPlayerEntity(iterator)->p_before_magic(step, srcID);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}
	return engine->popGameState_if(STATE_BEFORE_MAGIC);
}

StateMissiled* StateMissiled::create(GameGrail* engine, int cardID, int dstID, int srcID)
{
	int ret;
	PlayerEntity* src = engine->getPlayerEntity(srcID);
	if(GE_SUCCESS != (ret = src->v_missile(cardID, dstID))){
		throw (GrailError)ret;
	}
	StateMissiled* probe = new StateMissiled(dstID, srcID, false);
	if(dstID == probe->getNextTargetID(engine, srcID)){
		return probe;
	}
	else{
		probe->isClockwise = true;
		return probe;
	}
}

int StateMissiled::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateMissiled", engine->getGameId());
	int nextTargetID = getNextTargetID(engine, dstID);

	CommandRequest cmd_req;
	Coder::askForMissile(dstID, srcID, harmPoint, nextTargetID, cmd_req);
	int dstID_t = dstID;
	int srcID_t = srcID;
	int harmPoint_t = harmPoint;
//change
	if(engine->waitForOne(dstID, MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		int ret;
		Respond* reAttack;
		if(GE_SUCCESS == (ret = engine->getReply(dstID, reply)))
		{
			reAttack = (Respond*)reply;
			int cardID;			
			switch(reAttack->args(0))
			{				
			case RA_ATTACK:
				cardID = reAttack->args(1);
				if(GE_SUCCESS == (ret=engine->getPlayerEntity(dstID)->v_remissile(cardID))){
					srcID = dstID;
					dstID = nextTargetID;
					harmPoint++;
					hasMissiled[srcID] = true;
					return engine->setStateUseCard(cardID, dstID, srcID);
				}
				break;
			case RA_BLOCK:
				cardID = reAttack->args(1);
				if(GE_SUCCESS == (ret=engine->getPlayerEntity(dstID)->v_block(cardID))){
					engine->popGameState();
					return engine->setStateUseCard(cardID, srcID_t, dstID_t);				
				}
				break;
			case RA_GIVEUP:				
				int harmPoint_t = harmPoint;
				engine->popGameState();
				return engine->setStateMissileGiveUp(dstID_t, srcID_t, harmPoint_t);
				break;
			}
		}
		return ret;
	}
	else{
		engine->popGameState();
		engine->setStateMissileGiveUp(dstID_t, srcID_t, harmPoint_t);
		return GE_TIMEOUT;
	}
}

int StateMissiled::getNextTargetID(GameGrail* engine ,int startID)
{
	int nextTargetID;
	PlayerEntity* start = engine->getPlayerEntity(startID);
	int color = start->getColor();
	PlayerEntity* it = start;
	while(true)
	{
		while(start != (it = isClockwise ? it->getPre() : it->getPost()))
		{
			nextTargetID = it->getID();
			if(!hasMissiled[nextTargetID] && it->getColor() != color){								
				return nextTargetID;
			}
		}
		memset(hasMissiled, 0, sizeof(hasMissiled));
	}
}

int StateMagicSkill::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateMagicSkill", engine->getGameId());
	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();
	PlayerEntity *src = engine->getPlayerEntity(m_currentPlayerID);
	if(step != STEP_DONE){
		ret = src->p_magic_skill(step, action);
		if(GE_SUCCESS != ret){
			return ret;		
		}
	}
	return engine->popGameState_if(STATE_MAGIC_SKILL);
}

int StateAfterMagic::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateAfterMagic", engine->getGameId());
	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();
	
	while(iterator < engine->getGameMaxPlayers()){	    
		ret = engine->getPlayerEntity(iterator)->p_after_magic(step, srcID);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}
	if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_AFTER_MAGIC))){
		return engine->setStateCheckTurnEnd();
	}
	return ret;
}

int StateBeforeSpecial::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateBeforeSpecial", engine->getGameId());
	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();

	while(iterator < engine->getGameMaxPlayers()){	    
		ret = engine->getPlayerEntity(iterator)->p_before_special(step, srcID);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}
	return engine->popGameState_if(STATE_BEFORE_SPECIAL);
}

int StateAfterSpecial::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateAfterSpecial", engine->getGameId());
	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();

	while(iterator < engine->getGameMaxPlayers()){	    
		ret = engine->getPlayerEntity(iterator)->p_after_special(step, srcID);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}
	if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_AFTER_SPECIAL))){
		return engine->setStateCheckTurnEnd();
	}
	return ret;
}

int StateAdditionalAction::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateAdditionalAction", engine->getGameId());
	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();
	PlayerEntity *dst = engine->getPlayerEntity(m_currentPlayerID);
	list<ACTION_QUOTA> quota = dst->getAdditionalAction();

	CommandRequest cmd_req;
	Coder::askForAdditionalAction(m_currentPlayerID, quota, cmd_req);

	if(engine->waitForOne(m_currentPlayerID, MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		int ret;
		if(GE_SUCCESS == (ret=engine->getReply(m_currentPlayerID, reply)))
		{
			Respond *respond = (Respond*)reply;
			int chosen = respond->args(0);
			//���������ж�
			if(chosen == ACTION_NONE){
				engine->popGameState();
				engine->pushGameState(new StateTurnEnd);
			}
			else if(GE_SUCCESS == dst->v_additional_action(chosen)){
				return dst->p_additional_action(chosen);
			}
		}
		return ret;
	}
	else 
	{
		engine->popGameState();
		engine->pushGameState(new StateTurnEnd);
		return GE_TIMEOUT;
	}
}

int StateTurnEnd::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateTurnEnd", engine->getGameId());
	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();

	while(iterator < engine->getGameMaxPlayers()){	    
		ret = engine->getPlayerEntity(iterator)->p_turn_end(step, m_currentPlayerID);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}

	if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_TURN_END))){
		PlayerEntity* player = engine->getPlayerEntity(m_currentPlayerID);
		return engine->setStateCurrentPlayer(player->getPost()->getID());
	}
	return ret;	
}

int StateTimeline1::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateTimeline1", engine->getGameId());
	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();

	while(iterator < engine->getGameMaxPlayers()){	    
		ret = engine->getPlayerEntity(iterator)->p_timeline_1(step, context);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}

	CONTEXT_TIMELINE_1* con = new CONTEXT_TIMELINE_1;
	*con = *context;
	if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_TIMELINE_1))){		
		engine->pushGameState(new StateAttacked(con));
	}
	else{
		SAFE_DELETE(con);
	}
	return ret;
}

int StateTimeline2Hit::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateTimeline2Hit", engine->getGameId());

	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();

	while(iterator < engine->getGameMaxPlayers()){	    
		ret = engine->getPlayerEntity(iterator)->p_timeline_2_hit(step, context);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}

	CONTEXT_TIMELINE_2_HIT temp = *context;
	if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_TIMELINE_2_HIT))){
		return engine->setStateTimeline3(temp.attack.dstID,temp.harm);
	}
	return ret;
}

int StateTimeline2Miss::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateTimeline2Miss", engine->getGameId());
	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();

	while(iterator < engine->getGameMaxPlayers()){	    
		ret = engine->getPlayerEntity(iterator)->p_timeline_2_miss(step, context);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}
	return engine->popGameState_if(STATE_TIMELINE_2_MISS);
}

int StateTimeline3::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateTimeline3", engine->getGameId());
	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();

	while(iterator < engine->getGameMaxPlayers()){	    
		ret = engine->getPlayerEntity(iterator)->p_timeline_3(step, context);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}

	PlayerEntity* player = engine->getPlayerEntity(context->dstID);
	int cross = player->getCrossNum();
	if(cross > 0){
		CONTEXT_TIMELINE_4* newCon = new CONTEXT_TIMELINE_4;
		newCon->harm = context->harm;
		newCon->dstID = context->dstID;
		newCon->crossAvailable = cross;
		if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_TIMELINE_3))){
			engine->pushGameState(new StateTimeline4(newCon));
		}
		else{
			SAFE_DELETE(newCon);
		}
	}
	else{
		CONTEXT_TIMELINE_5* newCon = new CONTEXT_TIMELINE_5;
		newCon->dstID = context->dstID;
		newCon->harm = context->harm;
		if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_TIMELINE_3))){
			engine->pushGameState(new StateTimeline5(newCon));
		}
		else{
			SAFE_DELETE(newCon);
		}
	}
	return ret;
}

int StateTimeline4::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateTimeline4", engine->getGameId());
	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();

	while(iterator < engine->getGameMaxPlayers()){	    
		ret = engine->getPlayerEntity(iterator)->p_timeline_4(step, context);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}

	if(context->crossAvailable>0){
		CONTEXT_TIMELINE_4 temp = *context;
		if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_TIMELINE_4))){
			engine->pushGameState(new StateAskForCross(temp.dstID, temp.harm, temp.crossAvailable));
		}		
	}
	else{
		CONTEXT_TIMELINE_5* newCon = new CONTEXT_TIMELINE_5;
		newCon->dstID = context->dstID;
		newCon->harm = context->harm;
		if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_TIMELINE_4))){
			engine->pushGameState(new StateTimeline5(newCon));
		}
		else{
			SAFE_DELETE(newCon);
		}
	}
	return ret;
}

int StateTimeline5::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateTimeline5", engine->getGameId());
	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();

	while(iterator < engine->getGameMaxPlayers()){	    
		ret = engine->getPlayerEntity(iterator)->p_timeline_5(step, context);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}

	if(context->harm.point>0){
		CONTEXT_TIMELINE_6 *newCon = new CONTEXT_TIMELINE_6;
		newCon->dstID = context->dstID;
		newCon->harm = context->harm;
		if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_TIMELINE_5))){
			engine->pushGameState(new StateTimeline6(newCon));
		}
		else{
			SAFE_DELETE(newCon);
		}
	}
	else{
		ret = engine->popGameState_if(STATE_TIMELINE_5);
	}
	return ret;
}

int StateTimeline6::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateTimeline6", engine->getGameId());
	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();

	while(iterator < engine->getGameMaxPlayers()){	    
		ret = engine->getPlayerEntity(iterator)->p_timeline_6(step, context);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}

	CONTEXT_TIMELINE_6 temp = *context;
	vector<int> cards;
	if(context->harm.point>0){
		CONTEXT_TIMELINE_6_DRAWN *newCon = new CONTEXT_TIMELINE_6_DRAWN;
		newCon->dstID = context->dstID;
		newCon->harm = context->harm;
		if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_TIMELINE_6))){
			engine->pushGameState(new StateTimeline6Drawn(newCon));
			ret = engine->setStateMoveCardsToHand(-1, DECK_PILE, temp.dstID, DECK_HAND, temp.harm.point, cards, temp.harm);
		}
		else{
			SAFE_DELETE(newCon);
		}
	}
	else{
		ret = engine->popGameState_if(STATE_TIMELINE_6);
	}
	return ret;
}

int StateTimeline6Drawn::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateTimeline6Drwan", engine->getGameId());
	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();

	while(iterator < engine->getGameMaxPlayers()){	    
		ret = engine->getPlayerEntity(iterator)->p_timeline_6_drawn(step, context);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}

	ret = engine->popGameState_if(STATE_TIMELINE_6_DRAWN);
	return ret;
}

int StateAskForCross::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateAskForCross", engine->getGameId());

	CommandRequest cmd_req;
	Coder::askForCross(dstID, harm.point, harm.type, crossAvailable, cmd_req);
	int ret;
	if(engine->waitForOne(dstID, MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(dstID, reply)))
		{
			Respond* reCross = (Respond*) reply;
			int usedCross = reCross->args(0);
			if(usedCross>0)
			{
				PlayerEntity *dstPlayer = engine->getPlayerEntity(dstID);
				dstPlayer->subCrossNum(usedCross);
				GameInfo update_info;
				Coder::crossNotice(dstID, dstPlayer->getCrossNum(), update_info);
				engine->sendMessage(-1, MSG_GAME, update_info);
				CONTEXT_TIMELINE_5* newCon = new CONTEXT_TIMELINE_5;
				newCon->dstID = dstID;
				HARM newHarm = harm;
				newHarm.point -= usedCross;
				newCon->dstID = dstID;
				newCon->harm = newHarm;
				engine->popGameState();
				engine->pushGameState(new StateTimeline5(newCon));
				return ret;
			}
			else
			{
				CONTEXT_TIMELINE_5* newCon = new CONTEXT_TIMELINE_5;
				newCon->dstID = dstID;
				newCon->harm = harm;
				engine->popGameState();
				engine->pushGameState(new StateTimeline5(newCon));
				return ret;
			}
		}
		else
			return ret;
	}
	else
	{
		//Timeout, skip to nextState
		CONTEXT_TIMELINE_5* newCon = new CONTEXT_TIMELINE_5;
		newCon->dstID = dstID;
		newCon->harm = harm;
		engine->popGameState();
		engine->pushGameState(new StateTimeline5(newCon));
		return GE_TIMEOUT;
	}
}

int StateHandChange::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateHandChange", engine->getGameId());
	int ret;
	if(!isSet){
		PlayerEntity* dst = engine->getPlayerEntity(dstID);
		if(direction == CHANGE_ADD){
			dst->addHandCards(howMany, cards);					
		}
		else{
			dst->removeHandCards(howMany, cards);
		}
		isSet = true;

		GameInfo update_info;
		Coder::handNotice(dstID, dst->getHandCards(), update_info);
	
		engine->sendMessage(-1, MSG_GAME, update_info);
	}
	ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();

	while(iterator < engine->getGameMaxPlayers()){	    
		ret = engine->getPlayerEntity(iterator)->p_hand_change(step, dstID);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}

	int dstID_temp = dstID;
	HARM harm_temp = harm;
	engine->popGameState_if(STATE_HAND_CHANGE);

	return engine->setStateHandOverLoad(dstID_temp, harm_temp);
}

int StateBasicEffectChange::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateBasicEffectChange", engine->getGameId());
	int ret;
	if(!isSet){
		PlayerEntity* dst = engine->getPlayerEntity(dstID);
		if(direction == CHANGE_ADD){
            dst->addBasicEffect(card, doerID);
		}
		else{
			dst->removeBasicEffect(card);	
		}
		isSet = true;

		GameInfo update_info;
		Coder::basicNotice(dstID, dst->getBasicEffect(), update_info);
	
		engine->sendMessage(-1, MSG_GAME, update_info);
	}
	ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();

	while(iterator < engine->getGameMaxPlayers()){	    
		ret = engine->getPlayerEntity(iterator)->p_basic_effect_change(step, dstID, card, doerID, cause);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}
	return engine->popGameState_if(STATE_BASIC_EFFECT_CHANGE);	 
}

int StateRequestHand::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateRequestHand, howMany %d", engine->getGameId(), howMany);
	int ret = GE_FATAL_ERROR;

	CommandRequest cmd_req;
	Coder::askForDiscard(targetID, howMany, cause, isShown, cmd_req);

	int howMany_t = howMany;
	int targetID_t = targetID;
	int dstOwner_t = dstOwner;
	int dstArea_t = dstArea;
	HARM harm_t = harm;
	bool isShown_t = isShown;
	vector<int> toDiscard(howMany);
	PlayerEntity *target = engine->getPlayerEntity(targetID);
	PlayerEntity *causer = engine->getPlayerEntity(harm.srcID);
	int card_id;
	if(engine->waitForOne(targetID, MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(targetID, reply)))
		{
			Respond* respond = (Respond*) reply;

			if(respond->args(0) == 1)
			{
				if (respond->card_ids_size() != howMany){
					return GE_MOVECARD_FAILED;
				}
				else {
					for (int i=0; i<respond->card_ids_size(); ++i)
						toDiscard[i] = respond->card_ids(i);
				}
				if(GE_SUCCESS != (ret = target->checkHandCards(howMany, toDiscard)) ||
				   GE_SUCCESS != (ret = causer->v_request_hand(howMany, toDiscard, harm_t))){
					return ret;
				}
				engine->popGameState();
				if(dstArea_t != DECK_HAND){
					return engine->setStateMoveCardsNotToHand(targetID_t, DECK_HAND, dstOwner_t, dstArea_t, howMany_t, toDiscard, harm_t.srcID, harm_t.cause, isShown_t);
				}
				else{
					return engine->setStateMoveCardsToHand(targetID_t, DECK_HAND, dstOwner_t, dstArea_t, howMany_t, toDiscard, harm_t, isShown_t);
				}
			}
			else if(canGiveUp){	
				engine->popGameState();
				return causer->p_request_hand_give_up(step, targetID_t, harm_t.cause);
			}
			else{
				return GE_INVALID_ACTION;
			}
		}
	    return ret;
	}
	else
	{
		//Timeout auto discard
		if(!canGiveUp)
		{
			engine->popGameState();
			list<int> handcards = engine->getPlayerEntity(targetID_t)->getHandCards();
			list<int>::iterator it = handcards.begin();
			for(int i = 0; i < howMany_t && it != handcards.end(); i++){
				toDiscard[i] = *it;
				it++;
			}
			if(dstArea != DECK_HAND){
				engine->setStateMoveCardsNotToHand(targetID_t, DECK_HAND, dstOwner_t, dstArea_t, howMany_t, toDiscard, harm_t.srcID, harm_t.cause, isShown_t);
			}
			else{
				engine->setStateMoveCardsToHand(targetID_t, DECK_HAND, dstOwner_t, dstArea_t, howMany_t, toDiscard, harm_t, isShown_t);
			}
		}
		else{
			engine->popGameState();
			causer->p_request_hand_give_up(step, targetID_t, harm_t.cause);
		}
		return GE_TIMEOUT;
	}
}

int StateBeforeLoseMorale::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateBeforeLoseMorale", engine->getGameId());
	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();

	while(iterator < engine->getGameMaxPlayers()){	    
		ret = engine->getPlayerEntity(iterator)->p_before_lose_morale(step, context);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}

	CONTEXT_LOSE_MORALE* newCon = new CONTEXT_LOSE_MORALE;
	newCon->howMany = context->howMany;
	newCon->dstID = context->dstID;
	newCon->harm = context->harm;
	if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_BEFORE_LOSE_MORALE))){
		engine->pushGameState(new StateLoseMorale(newCon));
	}
	else{
		SAFE_DELETE(newCon);
	}
	return ret;
}

int StateLoseMorale::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateLoseMorale", engine->getGameId());
	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();

	while(iterator < engine->getGameMaxPlayers()){	    
		ret = engine->getPlayerEntity(iterator)->p_lose_morale(step, context);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}

	CONTEXT_LOSE_MORALE* newCon = new CONTEXT_LOSE_MORALE;
	newCon->howMany = context->howMany;
	newCon->dstID = context->dstID;
	newCon->harm = context->harm;
	if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_LOSE_MORALE))){
		engine->pushGameState(new StateFixMorale(newCon));
	}
	else{
		SAFE_DELETE(newCon);
	}
	return ret;
}

int StateFixMorale::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateFixMorale", engine->getGameId());
	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();

	while(iterator < engine->getGameMaxPlayers()){	    
		ret = engine->getPlayerEntity(iterator)->p_fix_morale(step, context);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}

	CONTEXT_LOSE_MORALE* newCon = new CONTEXT_LOSE_MORALE;
	newCon->howMany = context->howMany;
	newCon->dstID = context->dstID;
	newCon->harm = context->harm;
	if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_FIX_MORALE))){
		engine->pushGameState(new StateTrueLoseMorale(newCon));
	}
	else{
		SAFE_DELETE(newCon);
	}
	return ret;
}

int StateTrueLoseMorale::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateTrueLoseMorale", engine->getGameId());
	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();

	while(iterator < engine->getGameMaxPlayers()){	    
		ret = engine->getPlayerEntity(iterator)->p_true_lose_morale(step, context);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}

	int color = engine->getPlayerEntity(context->dstID)->getColor();
	TeamArea *m_teamArea = engine->getTeamArea();
	int morale = m_teamArea->getMorale(color);
	m_teamArea->setMorale(color, morale - context->howMany);
	morale = m_teamArea->getMorale(color);

	// ����ʿ��
	GameInfo update_info;
	if (color == RED)
		update_info.set_red_morale(morale);
	else
		update_info.set_blue_morale(morale);
	engine->sendMessage(-1, MSG_GAME, update_info);
	if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_TRUE_LOSE_MORALE))){
		if(morale <= 0){
			engine->pushGameState(new StateGameOver(1-color));
		}
	}
	return ret;
}

int StateShowHand::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateShowHand", engine->getGameId());
	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();

	while(iterator < engine->getGameMaxPlayers()){
		ret = engine->getPlayerEntity(iterator)->p_show_hand(step, dstID, howMany, cards, harm);
		moveIterator(ret);
		if(GE_SUCCESS != ret){
			return ret;
		}		
	}

	GameInfo game_info;
	Coder::showCardNotice(dstID, cards, game_info);
	return engine->popGameState_if(STATE_SHOW_HAND);
}

int StateGameOver::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateGameOver", engine->getGameId());
	Sleep(10000);
	return GE_SUCCESS;
}