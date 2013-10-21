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
			Coder::roleNotice(i, out, game_info);
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
	for(int i = iterator; i< engine->getGameMaxPlayers(); i++){	    
		if(GE_SUCCESS != (ret = engine->getPlayerEntity(i)->p_before_turn_begin(m_currentPlayerID))){
			if(ret==GE_DONE_AND_URGENT){
				iterator++;
			}
			return ret;
		}
		iterator++;
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
	for(int i = iterator; i< engine->getGameMaxPlayers(); i++){
		if(GE_SUCCESS != (ret = engine->getPlayerEntity(i)->p_turn_begin(m_currentPlayerID))){
			if(ret==GE_DONE_AND_URGENT){
				iterator++;
			}
			return ret;
		}
		iterator++;
	}
	if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_TURN_BEGIN))){
		return engine->setStateCheckBasicEffect();
	}

	network::TurnBegin turn_begin;
	turn_begin.set_id(m_currentPlayerID);
	engine->sendMessage(-1, network::MSG_TURN_BEGIN, turn_begin);
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
		//TODO set nextState based on reply
		void* temp;
		int ret;
		if(GE_SUCCESS == (ret=engine->getReply(m_currentPlayerID, temp)))
		{
			Respond *weak_respond = (Respond*)temp;
			weak_respond->set_src_id(m_currentPlayerID);

			if(weak_respond->args().Get(0) == 1){
				// ��д��������
				if (weak_respond->args_size() < 2) {
					weak_respond->add_args(howMany);
				}
				else {
					weak_respond->set_args(1, howMany);
				}

				engine->sendMessage(-1, MSG_RESPOND, *weak_respond);
				engine->popGameState();
				engine->pushGameState(new StateBeforeAction);
				HARM harm;
				harm.type = HARM_NONE;
				harm.point = howMany;
				harm.srcID = srcID;
				harm.cause = CAUSE_WEAKEN;
				vector< int > cards;
				return engine->setStateMoveCardsToHand(-1, DECK_PILE, m_currentPlayerID, DECK_HAND, howMany, cards, harm);
			}
			else{
				engine->sendMessage(-1, MSG_RESPOND, *weak_respond);
				engine->popGameState();
				engine->pushGameState(new StateBeforeAction(false));
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
		engine->pushGameState(new StateBeforeAction(false));
		return GE_TIMEOUT;
	}

}

int StateBeforeAction::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter handleBeforeAction", engine->getGameId());
	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();
	for(int i = iterator; i< engine->getGameMaxPlayers(); i++){
		if(GE_SUCCESS != (ret = engine->getPlayerEntity(i)->p_before_action(m_currentPlayerID))){
			if(ret==GE_DONE_AND_URGENT){
				iterator++;
			}
			return ret;
		}
		iterator++;
	}
	bool toAction = this->toAction;
	if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_BEFORE_ACTION))){
		if(toAction){
			engine->pushGameState(new StateBoot);
		}
		else{
			engine->pushGameState(new StateTurnEnd);
		}
	}
	return ret;
}

int StateBoot::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateBoot", engine->getGameId());
	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();
	for(int i = iterator; i< engine->getGameMaxPlayers(); i++){
		if(GE_SUCCESS != (ret = engine->getPlayerEntity(i)->p_boot(m_currentPlayerID))){
			if(ret==GE_DONE_AND_URGENT){
				iterator++;
			}
			return ret;
		}
		iterator++;
	}
	if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_BOOT))){
		engine->pushGameState(new StateActionPhase);
	}
	return ret;
}

int StateActionPhase::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateActionPhase", engine->getGameId());
	if(!isSet)
	{
		//FIXME ����
		allowAction = ACTION_ANY;
		canGiveUp = false;
		isSet = true;
	}
	int m_currentPlayerID = engine->getCurrentPlayerID();

	CommandRequest cmd_req;
	Coder::askForAction(m_currentPlayerID, allowAction, canGiveUp, cmd_req);
	if(engine->waitForOne(m_currentPlayerID, MSG_CMD_REQ, cmd_req))
	{
		//TODO set nextState based on reply
		void* temp;
		int ret;
		if(GE_SUCCESS == (ret=engine->getReply(m_currentPlayerID, temp))){
			Action *action = (Action*) temp;
			switch(action->action_id())
			{				
			case ACTION_ATTACK:
				//FIXME verify card type
				int card_id = action->args().Get(0);
				if(GE_SUCCESS == (ret=engine->getPlayerEntity(m_currentPlayerID)->checkOneHandCard(card_id))){
					action->set_src_id(m_currentPlayerID);
					engine->sendMessage(-1, MSG_ACTION, *action);

					engine->popGameState();
					return engine->setStateAttackAction(card_id, action->dst_ids().Get(0), action->src_id());
				}
				break;
			}
		}
		return ret;
	}
	else{
		return GE_TIMEOUT;
	}
}

int StateBeforeAttack::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateBeforeAttack", engine->getGameId());
	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();
	for(int i = iterator; i< engine->getGameMaxPlayers(); i++){
		if(GE_SUCCESS != (ret = engine->getPlayerEntity(i)->p_before_attack(dstID, srcID))){
			if(ret==GE_DONE_AND_URGENT){
				iterator++;
			}
			return ret;
		}
		iterator++;
	}
	return engine->popGameState_if(STATE_BEFORE_ATTACK);
}

int StateAttacked::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateAttacked", engine->getGameId());
	//FIXME: NOMISS
	CommandRequest cmd_req;
	Coder::askForReBat(context->hitRate, context->attack.cardID, context->attack.dstID, context->attack.srcID, cmd_req);

	if(engine->waitForOne(context->attack.dstID, MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		int ret;

		CONTEXT_TIMELINE_1 temp = *context;
		if(GE_SUCCESS == (ret = engine->getReply(context->attack.dstID, reply))){
			Respond *respond_attack = (Respond*) reply;
			if (respond_attack->respond_id() != RESPOND_REPLY_ATTACK)
			{
				// TODO: ��Ӧս�ظ��Ĵ����������߱�ѡΪ����Ŀ��ʱ������Ӧ���ܴ������޴��༼�ܣ�
			}

			int ra = respond_attack->args(0);
			int card_id = 100000;
			if (ra != RA_GIVEUP)
				card_id = respond_attack->args(1);

			engine->sendMessage(-1, MSG_RESPOND, *respond_attack);
			switch(ra)
			{
				//FIXME: verify
			case RA_ATTACK:
				if(GE_SUCCESS == (ret=engine->getPlayerEntity(context->attack.dstID)->checkOneHandCard(card_id))){
					// ��������ж�
					respond_attack->set_src_id(context->attack.dstID);
					
					engine->popGameState();
					return engine->setStateReattack(temp.attack.cardID, card_id, temp.attack.srcID, temp.attack.dstID, respond_attack->dst_ids().Get(0), temp.attack.isActive, true);
				}
				break;
			case RA_BLOCK:
				if(GE_SUCCESS == (ret=engine->getPlayerEntity(context->attack.dstID)->checkOneHandCard(card_id))){
					// ��������ж�
					respond_attack->set_src_id(context->attack.dstID);

					engine->popGameState();
					engine->setStateTimeline2Miss(temp.attack.cardID, temp.attack.dstID, temp.attack.srcID, temp.attack.isActive);
					return engine->setStateUseCard(card_id, temp.attack.dstID, temp.attack.srcID);				
				}
				break;
			case RA_GIVEUP:
				engine->popGameState();
				return engine->setStateAttackGiveUp(temp.attack.cardID, temp.attack.dstID, temp.attack.srcID, temp.harm, temp.attack.isActive);
				break;
			}
		}
		return ret;
	}
	else{
		CONTEXT_TIMELINE_1 temp = *context;
		engine->popGameState();
		engine->setStateAttackGiveUp(temp.attack.cardID, temp.attack.dstID, temp.attack.srcID, temp.harm, temp.attack.isActive);
		return GE_TIMEOUT;
	}
}

int StateAfterAttack::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateAfterAttack", engine->getGameId());
	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();
	for(int i = iterator; i< engine->getGameMaxPlayers(); i++){
		if(GE_SUCCESS != (ret = engine->getPlayerEntity(i)->p_after_attack(srcID))){
			if(ret==GE_DONE_AND_URGENT){
				iterator++;
			}
			return ret;
		}
		iterator++;
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
	for(int i = iterator; i< engine->getGameMaxPlayers(); i++){
		if(GE_SUCCESS != (ret = engine->getPlayerEntity(i)->p_before_magic(srcID))){
			if(ret==GE_DONE_AND_URGENT){
				iterator++;
			}
			return ret;
		}
		iterator++;
	}
	return engine->popGameState_if(STATE_BEFORE_MAGIC);
}

StateMissiled* StateMissiled::create(GameGrail* engine, int cardID, int dstID, int srcID)
{
	int ret;
	PlayerEntity* src = engine->getPlayerEntity(srcID);
	if(GE_SUCCESS != (ret = src->v_missile(cardID, dstID))){
		throw ret;
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
//change
	if(engine->waitForOne(dstID, MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		int ret;
		REPLY_ATTACKED* reAttack;
		if(GE_SUCCESS == (ret = engine->getReply(dstID, reply)))
		{
			reAttack = (REPLY_ATTACKED*)reply;
			switch(reAttack->flag)
			{				
				//FIXME: verify card type
			case RA_ATTACK:
				if(GE_SUCCESS == (ret=engine->getPlayerEntity(dstID)->v_remissile(reAttack->cardID))){
					srcID = dstID;
					dstID = nextTargetID;
					harmPoint++;
					hasMissiled[srcID] = true;
					return engine->setStateUseCard(reAttack->cardID, dstID, srcID, true);
				}
				break;
			case RA_BLOCK:
				if(GE_SUCCESS == (ret=engine->getPlayerEntity(dstID)->v_block(reAttack->cardID))){
					engine->popGameState();
					return engine->setStateUseCard(reAttack->cardID, -1, reAttack->srcID);				
				}
				break;
			case RA_GIVEUP:
				engine->popGameState();
				return engine->setStateMissileGiveUp(dstID, srcID, harmPoint);
				break;
			}
		}
		return ret;
	}
	else{
		engine->popGameState();
		engine->setStateMissileGiveUp(dstID, srcID, harmPoint);
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

int StateAfterMagic::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateAfterMagic", engine->getGameId());
	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();
	for(int i = iterator; i< engine->getGameMaxPlayers(); i++){
		if(GE_SUCCESS != (ret = engine->getPlayerEntity(i)->p_after_magic(srcID))){
			if(ret==GE_DONE_AND_URGENT){
				iterator++;
			}
			return ret;
		}
		iterator++;
	}
	if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_AFTER_MAGIC))){
		return engine->setStateCheckTurnEnd();
	}
	return ret;
}

int StateTurnEnd::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateTurnEnd", engine->getGameId());
	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();
	for(int i = iterator; i< engine->getGameMaxPlayers(); i++){
		if(GE_SUCCESS != (ret = engine->getPlayerEntity(i)->p_turn_end(m_currentPlayerID))){
			if(ret==GE_DONE_AND_URGENT){
				iterator++;
			}
			return ret;
		}
		iterator++;
	}
	if(GE_SUCCESS == (ret = engine->popGameState_if(STATE_TURN_END))){
		PlayerEntity* player = engine->getPlayerEntity(m_currentPlayerID);
		if(!player){
			return GE_INVALID_PLAYERID;
		}
		return engine->setStateCurrentPlayer(player->getPost()->getID());
	}
	return ret;	
}

int StateTimeline1::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateTimeline1", engine->getGameId());
	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();
	for(int i = iterator; i< engine->getGameMaxPlayers(); i++){
		if(GE_SUCCESS != (ret = engine->getPlayerEntity(i)->p_timeline_1(context))){
			if(ret==GE_DONE_AND_URGENT){
				iterator++;
			}
			return ret;
		}
		iterator++;
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
	for(int i = iterator; i< engine->getGameMaxPlayers(); i++){
		if(GE_SUCCESS != (ret = engine->getPlayerEntity(i)->p_timeline_2_hit(context))){
			if(ret==GE_DONE_AND_URGENT){
				iterator++;
			}
			return ret;
		}
		iterator++;
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
	for(int i = iterator; i< engine->getGameMaxPlayers(); i++){
		if(GE_SUCCESS != (ret = engine->getPlayerEntity(i)->p_timeline_2_miss(context))){
			if(ret==GE_DONE_AND_URGENT){
				iterator++;
			}
			return ret;
		}
		iterator++;
	}
	return engine->popGameState_if(STATE_TIMELINE_2_MISS);
}

int StateTimeline3::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateTimeline3", engine->getGameId());
	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();
	for(int i = iterator; i< engine->getGameMaxPlayers(); i++){
		if(GE_SUCCESS != (ret = engine->getPlayerEntity(i)->p_timeline_3(context))){
			if(ret==GE_DONE_AND_URGENT){
				iterator++;
			}
			return ret;
		}
		iterator++;
	}
	PlayerEntity* player = engine->getPlayerEntity(context->dstID);
	if(!player){
		return GE_INVALID_PLAYERID;
	}
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
	for(int i = iterator; i< engine->getGameMaxPlayers(); i++){
		if(GE_SUCCESS != (ret = engine->getPlayerEntity(i)->p_timeline_4(context))){
			if(ret==GE_DONE_AND_URGENT){
				iterator++;
			}
			return ret;
		}
		iterator++;
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
	for(int i = iterator; i< engine->getGameMaxPlayers(); i++){
		if(GE_SUCCESS != (ret = engine->getPlayerEntity(i)->p_timeline_5(context))){
			if(ret==GE_DONE_AND_URGENT){
				iterator++;
			}
			return ret;
		}
		iterator++;
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
	for(int i = iterator; i< engine->getGameMaxPlayers(); i++){
		if(GE_SUCCESS != (ret = engine->getPlayerEntity(i)->p_timeline_6(context))){
			if(ret==GE_DONE_AND_URGENT){
				iterator++;
			}
			return ret;
		}
		iterator++;
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
	for(int i = iterator; i< engine->getGameMaxPlayers(); i++){
		if(GE_SUCCESS != (ret = engine->getPlayerEntity(i)->p_timeline_6_drawn(context))){
			if(ret==GE_DONE_AND_URGENT){
				iterator++;
			}
			return ret;
		}
		iterator++;
	}
	ret = engine->popGameState_if(STATE_TIMELINE_6_DRAWN);
	return ret;
}

int StateAskForCross::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateAskForCross", engine->getGameId());

	CommandRequest cmd_req;
	Coder::askForCross(dstID, harm.point, harm.type, crossAvailable, cmd_req);

	if(engine->waitForOne(dstID, MSG_CMD_REQ, cmd_req))
	{
		//TODO cross reply		
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
			dst->addHandCards(howMany,cards);					
		}
		else{
			dst->removeHandCards(howMany,cards);	
		}
		isSet = true;

		GameInfo update_info;
		Coder::handNotice(dstID, dst->getHandCards(), update_info);
	
		engine->sendMessage(-1, MSG_GAME, update_info);
	}
	ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();
	for(int i = iterator; i< engine->getGameMaxPlayers(); i++){
		if(GE_SUCCESS != (ret = engine->getPlayerEntity(i)->p_hand_change(dstID))){
			if(ret==GE_DONE_AND_URGENT){
				iterator++;
			}
			return ret;
		}
		iterator++;
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
	for(int i = iterator; i< engine->getGameMaxPlayers(); i++){
		if(GE_SUCCESS != (ret = engine->getPlayerEntity(i)->p_basic_effect_change(dstID, card, doerID, cause))){
			if(ret==GE_DONE_AND_URGENT){
				iterator++;
			}
			return ret;
		}
		iterator++;
	}
	return engine->popGameState_if(STATE_BASIC_EFFECT_CHANGE);	 
}

int StateDiscardHand::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateDiscardHand, howMany %d", engine->getGameId(), howMany);
	int ret = GE_FATAL_ERROR;

	CommandRequest cmd_req;
	Coder::askForDiscard(dstID, howMany, isShown, cmd_req);

	if(engine->waitForOne(dstID, MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == engine->getReply(dstID, reply))
		{
			Respond* respond = (Respond*) reply;

			int howMany_t = howMany;
			int dstID_t = dstID;
			HARM harm_t = harm;
			bool isShown_t = isShown;
			vector<int> toDiscard(howMany_t);
			list<int> handcards = engine->getPlayerEntity(dstID_t)->getHandCards();
			int card_id;

			if (respond->args_size() != howMany)
			{
				return ret;
			}
			else
			{
				for (int i=0; i<howMany; ++i)
					toDiscard[i] = respond->args(i);
			}
			
			return engine->setStateMoveCardsNotToHand(dstID_t, DECK_HAND, -1, DECK_DISCARD, howMany_t, toDiscard, harm_t.srcID, harm_t.cause, isShown_t);
		}
		else
		{
			return ret;
		}
	}
	else
	{
		//Timeout auto discard
		int howMany_t = howMany;
		int dstID_t = dstID;
		HARM harm_t = harm;
		bool isShown_t = isShown;
		bool toDemoralize_t = toDemoralize;
		engine->popGameState();
		if(toDemoralize_t){
			engine->setStateStartLoseMorale(howMany_t, dstID_t, harm_t);
		}
		vector<int> toDiscard(howMany_t);
		list<int> handcards = engine->getPlayerEntity(dstID_t)->getHandCards();
		list<int>::iterator it = handcards.begin();
		for(int i = 0; i < howMany_t && it != handcards.end(); i++){
			toDiscard[i] = *it;
			it++;
		}
		return engine->setStateMoveCardsNotToHand(dstID_t, DECK_HAND, -1, DECK_DISCARD, howMany_t, toDiscard, harm_t.srcID, harm_t.cause, isShown_t);
	}
}

int StateBeforeLoseMorale::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateBeforeLoseMorale", engine->getGameId());
	int ret = GE_FATAL_ERROR;
	int m_currentPlayerID = engine->getCurrentPlayerID();
	for(int i = iterator;i<engine->getGameMaxPlayers();i++){
		if(GE_SUCCESS !=(ret = engine->getPlayerEntity(i)->p_before_lose_morale(context))){
			if(ret==GE_DONE_AND_URGENT){
				iterator++;
			}
			return ret;
		}
		iterator++;
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
	for(int i = iterator;i<engine->getGameMaxPlayers();i++){
		if(GE_SUCCESS !=(ret = engine->getPlayerEntity(i)->p_lose_morale(context))){
			if(ret==GE_DONE_AND_URGENT){
				iterator++;
			}
			return ret;
		}
		iterator++;
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
	for(int i = iterator;i<engine->getGameMaxPlayers();i++){
		if(GE_SUCCESS !=(ret = engine->getPlayerEntity(i)->p_fix_morale(context))){
			if(ret==GE_DONE_AND_URGENT){
				iterator++;
			}
			return ret;
		}
		iterator++;
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
	for(int i = iterator; i< engine->getGameMaxPlayers(); i++){
		if(GE_SUCCESS != (ret = engine->getPlayerEntity(i)->p_true_lose_morale(context))){
			if(ret==GE_DONE_AND_URGENT){
				iterator++;
			}
			return ret;
		}
		iterator++;
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
	for(int i = iterator; i< engine->getGameMaxPlayers(); i++){
		if(GE_SUCCESS != (ret = engine->getPlayerEntity(i)->p_show_hand(dstID, howMany, cards))){
			if(ret==GE_DONE_AND_URGENT){
				iterator++;
			}
			return ret;
		}
		iterator++;
	}
	return engine->popGameState_if(STATE_SHOW_HAND);
}

int StateGameOver::handle(GameGrail* engine)
{
	ztLoggerWrite(ZONE, e_Debug, "[Table %d] Enter StateGameOver", engine->getGameId());
	Sleep(10000);
	return GE_SUCCESS;
}