/**
 * \file
 * \version  $Id: zNetService.cpp 3203 2005-10-03 03:56:05Z fangq $
 * \author  Songsiliang,songsiliang@netease.com
 * \date 2004��11��29�� 17ʱ19��12�� CST
 * \brief ʵ�����������
 *
 * 
 */

#include <iostream>
#include <string>
 
#include "zSocket.h"
#include "zTCPServer.h"
 
#include "zNetService.h"
 
zNetService *zNetService::instance = NULL;

/**
 * \brief ��ʼ������������
 *
 * ʵ��<code>zService::init</code>���麯��
 *
 * \param port �˿�
 * \return �Ƿ�ɹ�
 */
bool zNetService::init(unsigned short port)
{
	if (!zService::init())
		return false;

	//Zebra::logger->debug("%s", __PRETTY_FUNCTION__);

	//��ʼ��������
	tcpServer = new zTCPServer();
	if (NULL == tcpServer)
		return false;
	tcpServer->init();
	if (!tcpServer->bind("0.0.0.0", port,this))
		return false;

	return true;
}

/**
 * \brief ��������������ص�����
 *
 * ʵ���麯��<code>zService::serviceCallback</code>����Ҫ���ڼ�������˿ڣ��������false���������򣬷���true����ִ�з���
 *
 * \return �ص��Ƿ�ɹ�
 */
bool zNetService::serviceCallback()
{
	//Zebra::logger->debug("%s", __PRETTY_FUNCTION__);

	//struct sockaddr_in addr;
	//int retcode = tcpServer->accept(&addr);
	//if (retcode >= 0) 
	//{
	//	//�������ӳɹ�����������
	//	newTCPTask(retcode, &addr);
	//}

	tcpServer->Check();

	return true;
}

/**
 * \brief �����������������
 *
 * ʵ�ִ��麯��<code>zService::final</code>��������Դ
 *
 */
void zNetService::final()
{
	SAFE_DELETE(tcpServer);
}

