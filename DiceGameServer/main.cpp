// main.cpp ���������������
//

#include "stdafx.h"
#include <Windows.h>
#include <libs/system/src/error_code.cpp>
#include "GameGrailCommon.h"
#include "DiceGameServer.h"
CardEntity* cardList[150];
int _tmain(int argc, _TCHAR* argv[])
{
	DiceGameServer::newInstance();
	if(!DiceGameServer::getInstance().serverInit())
	{
		printf("GameServer init fail...\n");
		return -1;
	}
 
	ztLoggerWrite(ZONE, e_Information, "GameServer Start ");
	DiceGameServer::getInstance().main();
	DiceGameServer::delInstance();  
	return 0;
}



