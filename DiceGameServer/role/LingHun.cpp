#include "LingHun.h"
#include "..\GameGrail.h"
#include "..\UserTask.h"

LingHun::LingHun(GameGrail *engine, int id, int color): PlayerEntity(engine, id, color)
{
  tokenMax[0]=6;
  tokenMax[1]=6;
  used_LING_HUN_LIAN_JIE=false;
  using_LING_HUN_LIAN_JIE=false;
  connectID=-1;
}


bool LingHun::cmdMsgParse(UserTask *session, uint16_t type, ::google::protobuf::Message *proto)
{
	switch(type)
	{
	case MSG_RESPOND:
		Respond* respond = (Respond*)proto;
		switch(respond->respond_id())
		{
		case LING_HUN_ZHUAN_HUAN:
			//tryNotify��������Ϸ���̴߳���Ϣ��ֻ��id���ڵ�ǰ�ȴ�id������state���ڵ�ǰstate������step���ڵ�ǰstep����Ϸ���̲߳Ż����
			session->tryNotify(id,STATE_TIMELINE_1,LING_HUN_ZHUAN_HUAN, respond);
			return true;

		case LING_HUN_LIAN_JIE:
			//tryNotify��������Ϸ���̴߳���Ϣ��ֻ��id���ڵ�ǰ�ȴ�id������state���ڵ�ǰstate������step���ڵ�ǰstep����Ϸ���̲߳Ż����
			session->tryNotify(id,STATE_BOOT,LING_HUN_LIAN_JIE, respond);  //��ʲô״̬������
			return true;
        
		case LING_HUN_ZENG_FU:
			//tryNotify��������Ϸ���̴߳���Ϣ��ֻ��id���ڵ�ǰ�ȴ�id������state���ڵ�ǰstate������step���ڵ�ǰstep����Ϸ���̲߳Ż����
			session->tryNotify(id,STATE_BOOT,LING_HUN_ZENG_FU, respond);  //��ʲô״̬������
		case LING_HUN_LIAN_JIE_REACT:
			session->tryNotify(id,STATE_TIMELINE_6,LING_HUN_LIAN_JIE_REACT, respond);
			return true;
		}
	}
	//ûƥ���򷵻�false
	return false;
}


int LingHun::p_boot(int &step, int currentPlayerID)
{
	int ret =GE_INVALID_STEP;
	if (currentPlayerID != id )
		return GE_SUCCESS;

	  while(STEP_DONE != step)
	{
		switch(step)
		{
		case STEP_INIT:
			if(gem>0)
	      {
	        step =LING_HUN_ZENG_FU;
	       
	      }
			else 
			 {	
				 if(token[0]>0&&token[1]>0&&!used_LING_HUN_LIAN_JIE)
				   step=LING_HUN_LIAN_JIE;
				 else 
				   step=STEP_DONE;
			}
			break;
		case  LING_HUN_ZENG_FU:	  
			  ret = LingHunZengFu();
			if(toNextStep(ret) || ret == GE_URGENT){
				if(token[0]>0&&token[1]>0&&!used_LING_HUN_LIAN_JIE)
					 step=LING_HUN_LIAN_JIE;
				else
				     step = STEP_DONE;
			}	
			break;	
		case  LING_HUN_LIAN_JIE:	  
			  ret =LingHunLianJie();
			if(toNextStep(ret) || ret == GE_URGENT){
				step = STEP_DONE;
			}	
		    break;	
		default:
			return GE_INVALID_STEP;
		}
	}
	return ret;
}

int LingHun::p_timeline_1(int &step, CONTEXT_TIMELINE_1 *con)
{
	int ret = GE_INVALID_STEP;
	//��ǹ���������ɷ���
	if(con->attack.srcID != id){
		return GE_SUCCESS;
	}
	//���ɹ�����������ߣ�ʧ���򷵻أ�step�ᱣ�����´��ٽ����Ͳ�������
	//һ�㳬ʱҲ�������һ��
	while(STEP_DONE != step)
	{
		switch(step)
		{
		case STEP_INIT:
			//��ʼ��step
			 if(token[0]>0||token[1]>0)   //���ƻ꼰���궼Ϊ0��ʱ�򣬲���ʾ�����ת����
		   {
			  step = LING_HUN_ZHUAN_HUAN;
			}
			else 
				step=STEP_DONE;
			break;
		case  LING_HUN_ZHUAN_HUAN:	  
			ret =LingHunZhuanHuan(con);
			if(toNextStep(ret) || ret == GE_URGENT){
				step = STEP_DONE;
			}	
			break;	
		default:
			return GE_INVALID_STEP;
		}
	}
	return ret;
}


int LingHun::p_timeline_6(int &step, CONTEXT_TIMELINE_6 *con) 
{    
	//��������ӡ���ʹ��                ����������Ϊ�������Ӷ���           ����������ֻ��ת��һ���˺���                                                       
    if(used_LING_HUN_LIAN_JIE==true&&(con->dstID==id||con->dstID==connectID)&&!using_LING_HUN_LIAN_JIE)
     {
          int ret = GE_INVALID_STEP;
			//��ʼ��step
			step =LING_HUN_LIAN_JIE_REACT;
			if(token[1]>0)                        //�������һ������Է����˺�ת��
		      {
			ret =LingHunLianJieReact(con);
			if(toNextStep(ret)){
				step = STEP_DONE;
			     }	
		      }
			else
				step = STEP_DONE;
			   return GE_SUCCESS;
	  }
		   
  else
  {
	return GE_SUCCESS;
  }
}

 int LingHun::p_timeline_6_drawn(int &step, CONTEXT_TIMELINE_6_DRAWN *con) 
 { 
	 using_LING_HUN_LIAN_JIE=false;     //����ת���˺�����
	 return GE_SUCCESS;
 }

 //�������ʿ������ͨ�������� ��������ɡ������ҷ�ÿ��1��ʿ���½�����+1����ɫ��꡿

int LingHun::p_true_lose_morale(int &step, CONTEXT_LOSE_MORALE *con) 
{ 
    //id-->new_color-->if(new_color=color) 
	int dst_color;
	int current_color;
	int harmPoint=con->howMany;
	     dst_color=engine->getPlayerEntity(con->dstID)->getColor();
		 current_color=engine->getPlayerEntity(id)->getColor();

  if(dst_color==current_color)
	{
	   setToken(0,token[0]+harmPoint);  //+1���ƻ꡿
	   network::GameInfo update;
	   Coder::tokenNotice(id,0,token[0],update);
	   engine->sendMessage(-1, MSG_GAME, update);
	}   
  return  GE_SUCCESS;
}

int LingHun::v_magic_skill(Action *action)
{
	int actionID = action->action_id();
	int cardID;
	int playerID = action->src_id();
	CardEntity* card;
	PlayerEntity* dst;

	if(playerID != id){
		return GE_INVALID_PLAYERID;
	}
	switch(actionID)
	{
	case  LING_HUN_CI_YU:
		cardID = action->card_ids(0);
		card = getCardByID(cardID);
		//�����Լ�������                          || ���Ƕ�Ӧ�ķ�����                  ||��������С��3  
		if(GE_SUCCESS != checkOneHandCard(cardID) || !card->checkSpeciality(actionID)||getToken(1)<3){
			return GE_INVALID_ACTION;
		}
		break;
	case LING_HUN_ZHEN_BAO:
		cardID = action->card_ids(0);
		card = getCardByID(cardID);
		//�����Լ�������                          || ���Ƕ�Ӧ�ķ�����                  ||�ƻ�����С��3  
		if(GE_SUCCESS != checkOneHandCard(cardID) || !card->checkSpeciality(actionID)||getToken(0)<3){
			return GE_INVALID_ACTION;
		}
		break;

	case LING_HUN_JING_XIANG:
		    //��ɫ���---��������
		if(getToken(0)<2){
			return GE_INVALID_ACTION;
		}
		break;

	case LING_HUN_ZHAO_HUAN:
		  return GE_SUCCESS;
		break;

	default:
		return GE_INVALID_ACTION;
	}
	return GE_SUCCESS;
}

int LingHun::p_magic_skill(int &step, Action* action)
{
	//p_magic_skill��ͬ�ڱ�Ĵ����㣬����ֻ��һ��ƥ�䣬���ÿһ���������ʱ����ذ�step��ΪSTEP_DONE
	int ret;
	switch(action->action_id())
	{
	case  LING_HUN_CI_YU:
		ret = LingHunCiYu(action);
		if(GE_URGENT == ret){
			step = STEP_DONE;
		}
		break;
	case LING_HUN_ZHEN_BAO:
		ret=LingHunZhenBao(action);
		if(GE_URGENT == ret){
			step = STEP_DONE;
		}	
		break;
	case LING_HUN_JING_XIANG:
		ret=LingHunJingXiang(action);
		if(GE_URGENT == ret){
			step = STEP_DONE;
		}	
		break;
	case LING_HUN_ZHAO_HUAN:
		   //����ٻ�
		ret=LingHunZhaoHuan(action);
		if(GE_URGENT == ret){
			step = STEP_DONE;
		}	
		break;
	default:
		return GE_INVALID_ACTION;
	}
	return ret;
}

//--------------�����������-------------------------
int LingHun::LingHunZengFu()
{
	CommandRequest cmd_req;
	Coder::askForSkill(id, LING_HUN_ZENG_FU, cmd_req);
	//���޵ȴ�����UserTask����tryNotify����
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		int ret;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*) reply;
			//����
			   if (respond->args(0) == 1)  //�Լ�����
			{
			if (gem > 0)
			{
				network::SkillMsg skill_msg;
				Coder::skillNotice(id, id, LING_HUN_ZENG_FU, skill_msg);
				engine->sendMessage(-1, MSG_SKILL, skill_msg);
					setGem(--gem);
				setToken(0,token[0]+2); //��ɫ���+2
				setToken(1,token[1]+2); //��ɫ���+2
				GameInfo game_info;
				Coder::tokenNotice(id,0,token[0],game_info);
				Coder::tokenNotice(id,1,token[1], game_info);
				Coder::energyNotice(id, gem, crystal, game_info);
				engine->sendMessage(-1, MSG_GAME, game_info);
				return GE_SUCCESS;
			}	
		   }
		}
		return ret;
	}
	else{
		//��ʱɶ��������
		return GE_TIMEOUT;
	}
}
  //-----------�������衿--------------
int LingHun::LingHunCiYu(Action* action)
{
	int dstID = action->dst_ids(0);
	int cardID = action->card_ids(0);
	PlayerEntity * dstPlayer = engine->getPlayerEntity(dstID);
	CardEntity* card = getCardByID(cardID);
	//������Ƿ�Ϊ��Ӧ������
	if( !card->checkSpeciality(LING_HUN_CI_YU)){
		return GE_SUCCESS;
	}
	

	//���漼��
	SkillMsg skill_msg;
	Coder::skillNotice(id, dstID, LING_HUN_CI_YU, skill_msg);
	engine->sendMessage(-1, MSG_SKILL, skill_msg);	
		
	    setToken(1,token[1]-3);              //��ɫ���-3	
		dstPlayer->setGem(dstPlayer->getGem()+2);
		network::GameInfo update;
		Coder::tokenNotice(id,1,token[1],update);
		Coder::energyNotice(dstID, gem, crystal, update);
		engine->sendMessage(-1, MSG_GAME, update);

		//չʾ����
	CardMsg show_card;
	Coder::showCardNotice(id, 1, cardID, show_card);
	engine->sendMessage(-1, MSG_CARD, show_card);
	engine->setStateMoveOneCardNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cardID, id,LING_HUN_CI_YU, true);
		return GE_URGENT;
}
//-------------������𱬡�--------------
int LingHun::LingHunZhenBao(Action *action)
{
	int dstID = action->dst_ids(0);
	int magic_id = action->action_id();
	int cardID = action->card_ids(0);
	PlayerEntity * dstPlayer = engine->getPlayerEntity(dstID);

	CardEntity* card = getCardByID(cardID);
	//������Ƿ�Ϊ��Ӧ������
	if( !card->checkSpeciality(LING_HUN_ZHEN_BAO)){
		return GE_SUCCESS;
	}
	
	//���漼��
	SkillMsg skill_msg;
	Coder::skillNotice(id, dstID, magic_id, skill_msg);
	engine->sendMessage(-1, MSG_SKILL, skill_msg);
	
	//չʾ����
	CardMsg show_card;
	Coder::showCardNotice(id, 1, cardID, show_card);
	engine->sendMessage(-1, MSG_CARD, show_card);
	
	   setToken(0,token[0]-3); //��ɫ���-3	
		network::GameInfo update;
		Coder::tokenNotice(id,0,token[0],update);
	    engine->sendMessage(-1, MSG_GAME, update);

    // �����˺�
	HARM harm;
	harm.cause = magic_id;
	if(dstPlayer->getHandCardNum()<3&&dstPlayer->getHandCardMax()>5)
	    harm.point = 5;
	else 
		harm.point=3;
	harm.srcID = id;
	harm.type = HARM_MAGIC;
	engine->setStateTimeline3(dstID, harm);

	// ��������
	engine->setStateMoveOneCardNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cardID, LING_HUN_ZHEN_BAO, true);//!!!

	return GE_URGENT;
}

int LingHun::LingHunJingXiang(Action* action)
{
	vector<int> cardIDs;
	vector<int> cards;
	int cardNum= action->card_ids_size();
	int  dstID = action->dst_ids(0);
	PlayerEntity *dstPlayer = engine->getPlayerEntity(dstID);
	int max=dstPlayer->getHandCardMax();
	int currentHandCards=dstPlayer->getHandCardNum();

	    setToken(0,token[0]-2); //��ɫ���+2	
		network::GameInfo update;
		Coder::tokenNotice(id,0,token[0],update);
	    Coder::energyNotice(id, gem, crystal, update);
	    engine->sendMessage(-1, MSG_GAME, update);

	if(cardNum>3)      cardNum=3;
	PlayerEntity *self = engine->getPlayerEntity(id);

	for(int i = 0; i < cardNum;i ++)
	{
		cardIDs.push_back(action->card_ids(i));
	}
	//���漼��
	SkillMsg skill_msg;
	Coder::skillNotice(id, dstID, LING_HUN_JING_XIANG, skill_msg);
	engine->sendMessage(-1, MSG_SKILL, skill_msg);

	HARM  harm;
	harm.type = HARM_MAGIC;
	harm.point = (currentHandCards+3)<max?3:(max-currentHandCards);  //!!!!!!
	harm.srcID = id;
	harm.cause = LING_HUN_JING_XIANG;
	engine->setStateTimeline3(dstID, harm);

	CardMsg show_card;
	Coder::showCardNotice(id, cardNum, cardIDs, show_card);
	engine->sendMessage(-1, MSG_CARD, show_card);
	engine->setStateMoveCardsNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cardNum, cardIDs,LING_HUN_JING_XIANG,true);//���ƣ��˺�
   
	//��������״̬����return GE_URGENT
	return GE_URGENT;
}

int LingHun::LingHunZhaoHuan(Action* action)
{
    vector<int> cardIDs;
	vector<int> cards;

	int cardNum= action->card_ids_size();
	setToken(1,token[1]+cardNum+1);  //��ɫ���+��X+1)	
	network::GameInfo update;
	Coder::tokenNotice(id,1,token[1], update);
	engine->sendMessage(-1, MSG_GAME, update);

	for(int i = 0; i < cardNum;i ++)
	{
		cardIDs.push_back(action->card_ids(i));
	}
	//���漼��
	SkillMsg skill_msg;
	Coder::skillNotice(id, id,LING_HUN_ZHAO_HUAN, skill_msg);
	engine->sendMessage(-1, MSG_SKILL, skill_msg);

	//��X����
	CardMsg show_card;
	Coder::showCardNotice(id, cardNum, cardIDs, show_card);
	engine->sendMessage(-1, MSG_CARD, show_card);
	engine->setStateMoveCardsNotToHand(id, DECK_HAND, -1, DECK_DISCARD, cardNum, cardIDs,LING_HUN_ZHAO_HUAN,true);//���ƣ��˺�
   
	//��������״̬����return GE_URGENT
	return GE_URGENT;

}

int LingHun::LingHunZhuanHuan(CONTEXT_TIMELINE_1 *con)
{
	//�ƻ�ת��Ϊ����
	//����ת��Ϊ�ƻ�

	int ret;
	//���㷢��������ѯ�ʿͻ����Ƿ񷢶�
	CommandRequest cmd_req;
	Coder::askForSkill(id, LING_HUN_ZHUAN_HUAN, cmd_req);
	//���޵ȴ�����UserTask����tryNotify����
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*) reply;
			//���� 
			if(respond->args(0) == 0||respond->args(0) == 1){
				if(respond->args(0) == 0)  //�ƻ� ת��Ϊ ����(��������������Ƿ�Ҫ���жϣ����д�������)
				{
				  setToken(0,token[0]-1);
				  setToken(1,token[1]+1);
				}
				else
				{
				  setToken(0,token[0]+1);
				  setToken(1,token[1]-1);	   
				}
				network::GameInfo update_info;
				Coder::tokenNotice(id,0,token[0], update_info);
				Coder::tokenNotice(id,1,token[1], update_info);
				engine->sendMessage(-1, MSG_GAME, update_info);

				network::SkillMsg skill;
				Coder::skillNotice(id, id, LING_HUN_ZHUAN_HUAN, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);
			}
		}
		return ret;
	}
	else{
		//��ʱɶ��������
		return GE_TIMEOUT;
	}
}


//��������ӡ�
int LingHun::LingHunLianJie(){

	int ret;
	int dstID;
	//���㷢��������ѯ�ʿͻ����Ƿ񷢶�
	CommandRequest cmd_req;
	Coder::askForSkill(id, LING_HUN_LIAN_JIE, cmd_req);
	//���޵ȴ�����UserTask����tryNotify����
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*) reply;
			//���� 
			if(respond->args(0) == 1){
				//����
			dstID=respond->dst_ids(0);
	       PlayerEntity* dst = engine->getPlayerEntity(dstID);
	       dst->addExclusiveEffect(EX_LING_HUN_LIAN_JIE);
		   addExclusiveEffect(EX_LING_HUN_LIAN_JIE);
	       
           setToken(0,token[0]-1);  //��ɫ���-1	
	       setToken(1,token[1]-1);  //��ɫ���-1	
	      
		   network::GameInfo update;
		   Coder::exclusiveNotice(dstID, dst->getExclusiveEffect(), update);
		   Coder::exclusiveNotice(id, dst->getExclusiveEffect(), update);
	       Coder::tokenNotice(id,0,token[0], update);
	       Coder::tokenNotice(id,1,token[1], update);
	       engine->sendMessage(-1, MSG_GAME, update);

           connectID=dstID;
           used_LING_HUN_LIAN_JIE=true;
	}
   }
		return ret;
}
 
else{
		//��ʱɶ��������
		return GE_TIMEOUT;
	}
}

int LingHun::LingHunLianJieReact(CONTEXT_TIMELINE_6 *con)
{
 
	int  ret;
	int  dstID=con->dstID;
	//���㷢��������ѯ�ʿͻ����Ƿ񷢶�
	CommandRequest cmd_req;
	//LingHun *connectPlayer = m_playerEntities[connectID];

	Coder::askForSkill(id,LING_HUN_LIAN_JIE_REACT, cmd_req);
	Command *cmd = (Command*)(&cmd_req.commands(cmd_req.commands_size()-1));
	cmd->add_args(con->harm.point);
	//���޵ȴ�����UserTask����tryNotify����
	if(engine->waitForOne(id, network::MSG_CMD_REQ, cmd_req))
	{
		void* reply;
		if (GE_SUCCESS == (ret = engine->getReply(id, reply)))
		{
			Respond* respond = (Respond*) reply;
			//���� 
			if(respond->args(0)>0){
				//��������
                network::SkillMsg skill;
				Coder::skillNotice(id, id,LING_HUN_LIAN_JIE_REACT, skill);
				engine->sendMessage(-1, MSG_SKILL, skill);

	          // �����˺�
	           HARM harm;
	           harm.cause =LING_HUN_LIAN_JIE;
               harm.point=respond->args(0);
	           harm.srcID =con->dstID;
	           harm.type = HARM_MAGIC;
	           con->harm.point=con->harm.point-respond->args(0);

			   setToken(1,token[1]-respond->args(0));
				network::GameInfo update_info;
				Coder::tokenNotice(id,1,token[1], update_info);
				engine->sendMessage(-1, MSG_GAME, update_info);
				using_LING_HUN_LIAN_JIE=true;
			
				if(dstID==id)                            //�˺�ת�����Ӷ���
				{
				//  setStateTimeline6(id,con->harm);
					//��ȡ��ɫ�ı�� 
				   engine->setStateTimeline6(id,con->harm);    
				   engine->setStateTimeline6(connectID,harm);
		            con->harm.point=0;
				   engine->setStateTimeline6(id,con->harm);
				}
				else
				{
				 // setStateTimeline6Drawn(connectID,con->harm);
				   engine->setStateTimeline6(connectID,con->harm);
				   engine->setStateTimeline6(id,harm);   
				   con->harm.point=0;
				   engine->setStateTimeline6(id,con->harm);     //����״̬
				}	
				 
	     	
				  return GE_SUCCESS;
	  }
			else
			{
				return GE_SUCCESS;
			}
		}
	}
	else{
		//��ʱɶ��������
		return GE_TIMEOUT;
	}
}
