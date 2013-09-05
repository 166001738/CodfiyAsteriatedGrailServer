/**
 * \file
 * \version  $Id: zService.cpp 9 2005-06-28 12:03:37Z song $
 * \author  Songsiliang,songsiliang@netease.com
 * \date 2004��11��25�� 10ʱ34��12�� CST
 * \brief ʵ�ַ����������
 *
 * 
 */


#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <assert.h>
#include <signal.h>

#include "zService.h"
  
/**
 * \brief CTRL + C���źŵĴ���������������
 *
 * \param signum �źű��
 */
static void ctrlcHandler(int signum)
{
	//���û�г�ʼ��zServiceʵ������ʾ����
	zService *instance = zService::serviceInstance();
	instance->Terminate();
}

/**
 * \brief HUP�źŴ�����
 *
 * \param signum �źű��
 */
static void hupHandler(int signum)
{
	//���û�г�ʼ��zServiceʵ������ʾ����
	zService *instance = zService::serviceInstance();
	instance->reloadConfig();
}

zService *zService::serviceInst = NULL;

/**
 * \brief ��ʼ������������������Ҫʵ���������
 *
 * \return �Ƿ�ɹ�
 */
bool zService::init()
{
 
	return true;
}

/**
 * \brief ��������ܵ�������
 */
void zService::main()
{
	//��ʼ�����򣬲�ȷ�Ϸ����������ɹ�
	if (init()
			&& validate())
	{
		//�������ص��߳�
		while(!isTerminate())
		{
			if (!serviceCallback())
			{
				break;
			}
		}
	}

	//���������ͷ���Ӧ����Դ
	final();
}

