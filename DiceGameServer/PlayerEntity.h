#ifndef PLAYERENTITY_H
#define PLAYERENTITY_H
#include "CardEntity.h"
#include "GameGrailCommon.h"
#include "GrailState.h"
#include <list>
#include <vector>

using namespace std;

class TeamArea;
class GameGrail;
//�����


class PlayerEntity
{
public:
    PlayerEntity(GameGrail *engine, int id, int color);
	//�ڸ����ǰ���ӻ���Ч��
    int addBasicEffect(int effectCard,int srcUserID=-1);
    //�Ƴ�����Ч��
    int removeBasicEffect(int card);
	//������Ч��
	int checkBasicEffect(int card);
	//���ָ�����ƵĻ���Ч�����Ƿ����,cardID��src�����ҵ�����ID��ʩ����
	int checkBasicEffectName(int name,int* cardID = NULL, int* src = NULL);
    //�������Ʋ���
    int addHandCards(int howMany, vector< int > newCard);
    //�Ƴ����Ʋ���
    int removeHandCards(int howMany, vector<int> oldCard);
	//�������
	int checkHandCards(int howMany, vector<int> oldCard);
	int checkOneHandCard(int cardID);
	//����
	int addCoverCards(int howMany, vector< int > cards);
	int removeCoverCards(int howMany, vector< int > cards);
    //�趨���������Ƿ�����
    void setHandCardsMaxFixed(bool fixed, int howmany=6);
    //�������Ʊ仯
    void addHandCardsRange(int howMany);
    //��������
    void addCrossNum(int howMany, int atMost=-1);
    void subCrossNum(int howMany);

    void setGem(int howMany);
    void setCrystal(int howMany);
    //���õ�ǰ�غ��Ƿ�Ϊ����һغ�
    void setYourTurn(bool yes);
    void setSeatNum(int num){this->seatNum = num;}
    int getSeatNum(){return this->seatNum;}
    //���ú���
    void setTap(bool tap){this->tap = tap;}
    void setToken(int id,int howMany){if(howMany<0)howMany=0;token[id]=howMany<=tokenMax[id]?howMany:tokenMax[id];}
    //������һ�����
    void setPost(PlayerEntity* nextPlayer){this->postPlayer = nextPlayer;}
	void setPre(PlayerEntity* pre){ prePlayer = pre; }
    int getID();
    string getName();
    int getHandCardMax();
    int getHandCardNum();
    int getCrossNum();
    int getCrossMax();
    int getGem();
    int getCrystal();
    int getEnergy();
    int getColor();
    list< int >  getCoverCards(){return this->coverCards;}
    int getToken(int id){return token[id];}
    //��ȡ�������ǰ�Ļ���Ч����
    list< struct BasicEffect > getBasicEffect(){return this->basicEffects;}

    PlayerEntity* getPost(){ return this->postPlayer;}
	PlayerEntity* getPre(){ return prePlayer; }
    list< int > getHandCards(){return this->handCards;}
    int getRoleID(){return characterID;}
    bool tapped(){return this->tap;}
    bool isHandCardsMaxFixed(){return this->handCardsMaxFixed;}
    bool getYourturn();
	bool hasAdditionalAction() {return !additionalActions.empty();}
	void clearAdditionalAction() { additionalActions.clear(); }
	//�غ��޶���ͳһ�������ʼ��
	virtual int p_before_turn_begin(int currentPlayerID) { return GE_SUCCESS; }
	virtual int p_turn_begin(int currentPlayerID) { return GE_SUCCESS; }
	virtual int p_before_action(int currentPlayerID) { return GE_SUCCESS; }
	virtual int p_boot(int currentPlayerID) { return GE_SUCCESS; }
	virtual int p_before_attack(int dstID, int srcID) { return GE_SUCCESS; }
	virtual int p_after_attack(int playerID) { return GE_SUCCESS; }
	virtual int p_before_magic(int srcID) { return GE_SUCCESS; }
	virtual int p_after_magic(int srcID) { return GE_SUCCESS; }
	virtual int p_before_special(int srcID) { return GE_SUCCESS; }
	virtual int p_after_special(int srcID) { return GE_SUCCESS; }
	virtual int p_turn_end(int playerID) { return GE_SUCCESS; }
	virtual int p_timeline_1(CONTEXT_TIMELINE_1 *con) { return GE_SUCCESS; }
	virtual int p_timeline_2_hit(CONTEXT_TIMELINE_2_HIT *con) { return GE_SUCCESS; }
	virtual int p_timeline_2_miss(CONTEXT_TIMELINE_2_MISS *con) { return GE_SUCCESS; }
	virtual int p_timeline_3(CONTEXT_TIMELINE_3 *con) { return GE_SUCCESS; }
	virtual int p_timeline_4(CONTEXT_TIMELINE_4 *con) { return GE_SUCCESS; }
	virtual int p_timeline_5(CONTEXT_TIMELINE_5 *con) { return GE_SUCCESS; }
	virtual int p_timeline_6(CONTEXT_TIMELINE_6 *con) { return GE_SUCCESS; }
	virtual int p_timeline_6_drawn(CONTEXT_TIMELINE_6_DRAWN *con) { return GE_SUCCESS; }
	virtual int p_before_lose_morale(CONTEXT_LOSE_MORALE *con) { return GE_SUCCESS; }
	virtual int p_lose_morale(CONTEXT_LOSE_MORALE *con) { return GE_SUCCESS; }
	virtual int p_fix_morale(CONTEXT_LOSE_MORALE *con) { return GE_SUCCESS; }
	virtual int p_true_lose_morale(CONTEXT_LOSE_MORALE *con) { return GE_SUCCESS; }
	virtual int p_hand_change(int playerID) { return GE_SUCCESS; }
	virtual int p_basic_effect_change(int dstID, int card, int doerID, int cause)  { return GE_SUCCESS; }
	virtual int p_show_hand(int playerID, int howMany, vector<int> cards) { return GE_SUCCESS; }

	virtual int v_allow_action(int claim, int allow, bool canGiveUp);
	virtual int v_attack(int cardID, int dstID, bool realCard = true);
	virtual int v_reattack(int cardID, int orignCardID, int dstID, int orignID, int rate, bool realCard = true);
	virtual int v_missile(int cardID, int dstID, bool realCard = true);
	virtual int v_remissile(int cardID, bool realCard = true);
	virtual int v_block(int cardID);
	virtual int v_shield(int cardID, PlayerEntity* dst);
	virtual int v_weaken(int cardID, PlayerEntity* dst);
	virtual int v_buy(Action *action);
	virtual int v_synthesize(Action *action, TeamArea* team);
	virtual int v_extract(Action *action, TeamArea* team);
protected:
    int id;//���id
    int characterID;
    string name;
    int handCardsMax;
    int handCardsRange;
    int handCardsMin;//��������֮��ʹ��
    int crossNum;
    int crossMax;
    int gem;
    int crystal;
    int energyMax;
    int color;
    float star;
    bool tap;//����״̬
    bool handCardsMaxFixed;//�Ƿ�������������
    bool yourTurn;
    int seatNum;
    int token[3];
    int tokenMax[3];
	PlayerEntity* prePlayer;//�ϼ�
    PlayerEntity* postPlayer;//�¼�
    TeamArea* teamArea;
    list< int > handCards;//����
    list< BasicEffect > basicEffects;//����Ч����
    list< int > exclusiveEffect;//ר��Ч��
    list< int > coverCards;//������
	list< ACTION_QUOTA > additionalActions;
	GameGrail *engine;
};

#endif // PLAYERENTITY_H
