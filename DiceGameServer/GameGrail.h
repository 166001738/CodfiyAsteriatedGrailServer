#pragma once

#include <iostream>
#include <list>
#include <stack>
#include "Game.h"
#include "GameGrailPlayerContext.h"
#include "PlayerEntity.h"
#include "GameGrailCommon.h"
#include <boost/random.hpp>
#include <boost/thread/condition_variable.hpp>
#include "codec.h"
using namespace std;



class GrailState;
class PlayerEntity;
class Deck
{
public:
	Deck(int s){
		size = s;
		items = new int[size];
		iterator = -1;
		rng.seed(time(0));
	}
	~Deck(){
		delete[] items;
	}
	int init(int min, int max){
		if(size < max - min + 1)
			return GE_DECK_OVERFLOW;
		for(int i = 0; i <= max - min; i++)
			items[i] = i + min;
		iterator = max - min;
		return GE_SUCCESS;
	}
	int pop(int howMany, int* out){
		if (iterator + 1 < howMany)
			return GE_DECK_OVERFLOW;
		memcpy(out, items + iterator - howMany + 1, howMany*sizeof(int));
		iterator -= howMany;
		return GE_SUCCESS;
	}
	int push(int howMany, const int* in){
		if (size - iterator - 1 < howMany)
			return GE_DECK_OVERFLOW;
		int begin = iterator+1;
		memcpy(items + begin, in, howMany*sizeof(int));
		iterator += howMany;
		return GE_SUCCESS;
	}
	int popAll(int *out){
		int howMany = iterator+1;
		memcpy(out, items, howMany*sizeof(int));
		iterator = -1;
		return howMany;
	}
	void randomize(){
		int chosen;
		int temp;
		
		for(int i = 0; i <= iterator; i++){
			chosen = rng() % (iterator + 1);
			temp = items[i];
			items[i] = items[chosen];
			items[chosen] = temp;
		}
	}
	int get_size() {
		return iterator+1;
	}
private:
	boost::random::rand48 rng;
	int *items;
	int iterator;
	int size;
};

class TeamArea
{
public:
    TeamArea(){this->initialTeam();}
    void initialTeam();
    void setMorale(int color,int value);
    void setGem(int color,int value);
    void setCrystal(int color,int value);
    void setCup(int color,int value);
    int getMorale(int color){return (color == RED)?moraleRED:moraleBLUE;}
    int getGem(int color){return (color == RED)?gemRED:gemBLUE;}
    int getCrystal(int color){return (color == RED)?crystalRED:crystalBLUE;}
    int getCup(int color){return (color == RED)?cupRED:cupBLUE;}
	int getEnergy(int color) { return (color == RED) ? gemRED+crystalRED : gemBLUE+crystalBLUE; }
private:

    int moraleRED,moraleBLUE;
    int gemRED,gemBLUE;
    int crystalRED,crystalBLUE;
    int cupRED,cupBLUE;
};

class GameGrailConfig : public GameConfig
{
public:
	GameGrailConfig(int maxPlayers, int roleStrategy=1): maxPlayers(maxPlayers), roleStrategy(roleStrategy)
	{}
	~GameGrailConfig() {}
	int maxPlayers;
	int roleStrategy;
};

class GameGrail : public Game
{
public:
	int m_roleStrategy;
	int m_seatMode;
	int m_maxAttempts;
	GameInfo game_info;
protected:
	int m_roundId;
	int m_maxPlayers;
	int m_token;
	int m_responseTime;
	int m_actionTime;
	int m_currentPlayerID;
	time_t m_roundEndTime;
	boost::mutex m_mutex_for_wait;
	boost::mutex m_mutex_for_notify;
	boost::condition_variable m_condition_for_wait;	
	typedef map< int, GameGrailPlayerContext* > PlayerContextList;
	PlayerContextList m_playerContexts;
	typedef map< int, PlayerEntity* > PlayerEntityList;
    PlayerEntityList m_playerEntities;
	typedef stack< GrailState* > StateStack;
	StateStack  m_states;
	TeamArea* m_teamArea;
	Deck *pile, *discard;
	bool m_ready[MAXPLAYER];
public:
	GameGrail(GameGrailConfig *config);
	~GameGrail();
	void sendMessage(int id, uint16_t proto_type, google::protobuf::Message& proto);

	int playerEnterIntoTable(GameGrailPlayerContext *player);
	bool isCanSitIntoTable() const { return m_playerContexts.size() < m_maxPlayers; }	

	GrailState* topGameState() { return m_states.empty()? NULL: m_states.top(); }
	void pushGameState(GrailState* state) { m_states.push(state); }
	void popGameState() { 
		 SAFE_DELETE(m_states.top()); 
		 m_states.pop(); 
	}
	int popGameState_if(int state);	

	int getGameMaxPlayers() const { return m_maxPlayers; }
	int getGameNowPlayers() const { return m_playerContexts.size();}
	int getCurrentPlayerID() const { return m_currentPlayerID; }
	PlayerEntity* getPlayerEntity(int id);
	TeamArea* getTeamArea() { return m_teamArea; }

	//return true when notified, false when timeout
	void resetReady(int id = -1){
		if(id<-1 || id>m_maxPlayers){
			return;
		}
		if(id == -1){
			memset(m_ready, 0, sizeof(m_ready));
		}
		else{
			memset(m_ready,1,sizeof(m_ready));
			m_ready[id] = 0;
		}
	}	
	bool waitForOne(int id, uint16_t proto_type, google::protobuf::Message& proto, int timeout, bool resetReady = true);
	bool waitForOne(int id, uint16_t proto_type, google::protobuf::Message& proto, bool toResetReady = true) { return waitForOne(id, proto_type, proto, m_responseTime, toResetReady); }
	bool waitForAll(uint16_t proto_type, void** proto_ptrs, int timeout, bool toResetReady = true);
	bool waitForAll(uint16_t proto_type, void** proto_ptrs, bool toResetReady = true) { return waitForAll(proto_type, proto_ptrs, m_responseTime, toResetReady); }
	bool tryNotify(int id, int state, int step = 0, void* reply = NULL);
	int getReply(int id, void* &reply);
	
	int drawCardsFromPile(int howMany, vector< int > &cards);
	//setStateǰ׺�ĺ����漰����״̬
	//�ײ�APIԭ���ϲ�ֱ�ӵ���
	int setStateMoveCards(int srcOwner, int srcArea, int dstOwner, int dstArea, int howMany, vector< int > cards, HARM harm, bool isShown = false);
	//���������ϣ����ṩHARM���������ƶ����Ƴ���cards���ø�ֵ
	int setStateMoveCardsToHand(int srcOwner, int srcArea, int dstOwner, int dstArea, int howMany, vector< int > cards, HARM harm, bool isShown = false);
	//����ļ򻯣���1�������ϣ����ṩHARM���������ƶ����Ƴ���cards���ø�ֵ
	int setStateMoveOneCardToHand(int srcOwner, int srcArea, int dstOwner, int dstArea, int cardID, HARM harm, bool isShown = false);
	//��������������ĵط������ṩԭ���������ƶ����Ƴ���cards���ø�ֵ
	int setStateMoveCardsNotToHand(int srcOwner, int srcArea, int dstOwner, int dstArea, int howMany, vector< int > cards, int doerID, int cause, bool isShown = false);
	//����ļ򻯣���1������������ĵط������ṩԭ���������ƶ����Ƴ���cards���ø�ֵ
	int setStateMoveOneCardNotToHand(int srcOwner, int srcArea, int dstOwner, int dstArea, int cardID, int doerID, int cause, bool isShown = false);

	int setStateUseCard(int cardID, int dstID, int srcID, bool stay = false, bool realCard = true);
	int setStateChangeMaxHand(int dstID, bool using_fixed, bool fixed = false, int howmany = 6, int handCardsRange = 0);
	int setStateHandOverLoad(int dstID, HARM harm);
	int setStateCoverOverLoad(int dstID);
	int setStateCheckBasicEffect();
	int setStateAttackAction(int cardID, int dstID, int srcID, bool realCard = true);
	int setStateReattack(int attackFromCard, int attackToCard, int attackFrom, int attacked , int attackTo, bool isActive, bool realCard = true);
	int setStateAttackGiveUp(int cardID, int dstID, int srcID, HARM harm, bool isActive, bool checkSheild = true);
	int setStateMissileGiveUp(int dstID, int srcID, int harmPoint);
	int setStateTimeline1(int cardID, int dstID, int srcID, bool isActive);
	int setStateTimeline2Miss(int cardID, int dstID, int srcID, bool isActive);
	int setStateTimeline2Hit(int cardID, int dstID, int srcID, HARM harm, bool isActive);
	int setStateTimeline3(int dstID, HARM harm);
	int setStateTimeline6(int dstID, HARM harm); //added by Tony
	int setStateStartLoseMorale(int howMany, int dstID, HARM harm);
	int setStateCheckTurnEnd();
	Deck* initRoles();
	void setRole(int playerID, int roleID){
		if(playerID < 0 || playerID >= m_maxPlayers){
			throw GE_INVALID_PLAYERID;
		}
		if(!isValidRoleID(roleID)){
			throw GE_INVALID_ROLEID;
		}
		SinglePlayerInfo* player_it;
		for(int i = 0; i < m_maxPlayers; i++){
			player_it = (SinglePlayerInfo*)&(game_info.player_infos(i));
			if(player_it->id() == playerID){
				player_it->set_role_id(roleID);
				break;
			}
		}
	}
	PlayerEntity* createRole(int playerID, int roleID, int color);
	void initPlayerEntities();
	void initDecks(){
		pile = new Deck(CARDSUM);
		pile->init(0,CARDSUM-1);
		pile->randomize();
		discard = new Deck(CARDSUM);
	}
	int setStateRoleStrategy();
	int setStateCurrentPlayer(int playerID);
protected:
	bool isReady(int id);	
	void GameRun();
	void kickOffNotConnectedPlayers();
	void updateTableStatusMessage();
};
