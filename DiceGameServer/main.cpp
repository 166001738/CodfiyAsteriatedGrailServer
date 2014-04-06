// main.cpp ���������������
//

#include "stdafx.h"
#include <Windows.h>
#include <libs/system/src/error_code.cpp>
#include "GameGrailCommon.h"
#include "DiceGameServer.h"
#include "GMCommand.h"

int _tmain(int argc, _TCHAR* argv[])
{
	// ��ʼ��gmָ��
	initialize_gm_command();

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



