/**
 *
 * �����Ƚ�ͨ��һ�㣬�ٴ����Ƚ�һ�����������������ʱ���ǱȽ�����
 * 
 */

#ifndef _zNetService_h_
#define _zNetService_h_

#include <iostream>
#include <string>
 

#include "zService.h"
#include "zTCPServer.h"
 

/**
 * \brief �����������
 *
 * ʵ���������������ܴ��룬�����Ƚ�ͨ��һ��
 *
 */
class zNetService : public zService
{

	public:

		/**
		 * \brief ����������
		 *
		 */
		virtual ~zNetService() { instance = NULL; };

		/**
		 * \brief ���ݵõ���TCP/IP���ӻ�ȡһ����������
		 *
		 * \param sock TCP/IP�׽ӿ�
		 * \param addr ��ַ
		 */
		virtual zTCPTask* newTCPTask(const uint16_t usPort) = 0;

		/**
		 * \brief ��ȡ���ӳ��е�������
		 *
		 * \return ������
		 */
		virtual const int getPoolSize() const
		{
			return 0;
		}

		/**
		 * \brief ��ȡ���ӳ�״̬
		 *
		 * \return ���ӳ�״̬
		 */
		virtual const int getPoolState() const
		{
			return 0;
		}

	protected:

		/**
		 * \brief ���캯��
		 * 
		 * �ܱ����Ĺ��캯����ʵ����Singleton���ģʽ����֤��һ��������ֻ��һ����ʵ��
		 *
		 * \param name ����
		 */
		zNetService(const std::string &name) : zService(name)
		{
			instance = this;

			serviceName = name;
			tcpServer = NULL;
		}

		bool init(unsigned short port);
		virtual bool serviceCallback();
		void final();

		asio::io_service& getIOSerivce()
		{
			return tcpServer->getIOService();
		}

	private:

		static zNetService *instance;		/**< ���Ψһʵ��ָ�룬���������࣬��ʼ��Ϊ��ָ�� */
		std::string serviceName;			/**< ������������� */
		zTCPServer *tcpServer;				/**< TCP������ʵ��ָ�� */

};

#endif

