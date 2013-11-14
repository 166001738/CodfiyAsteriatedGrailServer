#include "GMCommand.h"
#include "map"

typedef void (*PTRFUN)(GameGrail*, PlayerEntity*, vector<string>&); 
map<string, PTRFUN> cmd_mapping;

/*
energy: ���ý�ɫ������������2������ʯ��ˮ��
*/
void setEnergy(GameGrail* engine, PlayerEntity* player, vector<string>& ss)
{
	player->setGem(atoi(ss[1].c_str()));
	player->setCrystal(stoi(ss[2].c_str()));

	GameInfo game_info;
	Coder::energyNotice(player->getID(), player->getGem(), player->getCrystal(), game_info);
	engine->sendMessage(-1, MSG_GAME, game_info);
}

/*
card: ����������ƣ���������������Ϊ��ӵ���id
*/
void addCard(GameGrail* engine, PlayerEntity* player, vector<string>& ss)
{
	int howmany;
	vector<int> cards;
	for (int i = 1; i < ss.size(); ++i)
		cards.push_back(atoi(ss[i].c_str()));
	howmany = ss.size() - 1;
	player->addHandCards(howmany, cards);

	GameInfo game_info;
	Coder::handNotice(player->getID(), player->getHandCards(), game_info);
	engine->sendMessage(-1, MSG_GAME, game_info);
}

/*
¼��gmָ���gmָ�����cmd_mapping�У�keyΪgmָ���ʽ���ַ�����value�Ǵ�������ָ��
*/
void initialize_gm_command()
{
	cmd_mapping["!`energy"] = setEnergy;  // ��������
	cmd_mapping["!`card"] = addCard;     // �������
}

/*
����ָ���ַ����и�
*/
void split(const string& src, const string& separator, vector<string>& dest)
{
    string str = src;
    string substring;
    string::size_type start = 0, index;

    do
    {
        index = str.find_first_of(separator,start);
        if (index != string::npos)
        {    
            substring = str.substr(start,index-start);
            dest.push_back(substring);
            start = str.find_first_not_of(separator,index);
            if (start == string::npos) return;
        }
    }while(index != string::npos);
    
    //the last token
    substring = str.substr(start);
    dest.push_back(substring);
}

void gm_cmd(GameGrail* engine, PlayerEntity* player, string cmd)
{
	vector<string> ss;
	split(cmd, " ", ss);
	// ����ָ����ö�Ӧ��gm������û�ж�Ӧָ���ֱ������
	if (cmd_mapping.find(ss[0]) == cmd_mapping.end())
	{
		return;
	}
	PTRFUN func = cmd_mapping[ss[0]];
	func(engine, player, ss);
}
