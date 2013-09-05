#ifndef __zSocket__H__
#define  __zSocket__H__
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <queue>

#include "zType.h"
#include "zMutex.h"
#include "zLogger.h"
#include "zCommonDefine.h"

#define CHECK_TASK_INTER  1

/********************************************************
  protobuf ����Э��������ʽΪ 
   
   4���ֽڰ�����(����������4���ֽ�) + ������(protobuf)     

*********************************************************/

using namespace boost;
class zProcessor
{
public:
	//virtual bool msgParse(const Cmd::t_NullCmd *, const unsigned int) = 0;
	virtual bool msgParse(const void *pstrMsg, const uint32_t) = 0;

	virtual void OnQuit() = 0;

	virtual void OnCheck() = 0;

	virtual ~zProcessor(){}
};

/**
 * \brief ��Ż���ָ��Ľṹ��
 *
 */
struct t_BufferCmd  : private boost::noncopyable
{
	uint32_t nCmdLen;										/**< ʵ�ʴ�ŵ�ָ��� */
	uint32_t offset;										/**< ��Ч���ݵ�ƫ�� */
	uint32_t bitmask;										/**< ��־λ����ʾ���ָ���״̬ */
	uint8_t  pstrCmd[MAX_DATABUFFERSIZE];					/**< ���ָ�����ݵĻ����� */
 
	t_BufferCmd() { nCmdLen = 0; offset = 0; bitmask = 0; };	/**< ���캯����ʼ���ṹ�� */
	~t_BufferCmd() 
	{
	}
};
typedef std::queue<t_BufferCmd *> t_BufferCmdQueue;				/**< ����ָ��������� */

class zSocket :  public boost::noncopyable//,public boost::enable_shared_from_this<zSocket>
{
public:
	zSocket(asio::io_service& service);

	void SetProcessor(zProcessor* pTask)
	{
		m_pBasicTask = pTask;
	}

	virtual ~zSocket()
	{
		//Close();
		Final();
	}

	virtual void Start();

	virtual void CheckSelf(){};

	//����Task�˳���־
	void SetQuitFlag()
	{
		m_bQuit = true;
	}

	bool GetQuitFlag()
	{
		return m_bQuit;
	}

	void Final()
	{
		Close();
		ResetRecvBuffer();
	
		/*
		m_lock.lock();
		while(!recvQueue.empty())
		{
			t_BufferCmd *ptCmd = recvQueue.front();
			recvQueue.pop();
			SAFE_DELETE(ptCmd);
		}
		m_lock.unlock();
		*/

		m_sendLock.lock();
		while(!sendQueue.empty())
		{
			t_BufferCmd *ptCmd = sendQueue.front();
			sendQueue.pop();
			SAFE_DELETE(ptCmd);
		}
		m_sendLock.unlock();
	}

	void BeginReadNewMessage();

	void HandleReadHead(const system::error_code& error);

	void HandleReadBody(const system::error_code& error);

	void HandleWrite(const system::error_code& error);

	asio::ip::tcp::socket& socket()
	{
		return socket_;
	}

	void Close()
	{
		m_bQuit = true;
		socket_.close();
	}

	void Check(const system::error_code& e);
 

	// cmd ���뵽sendQueue��
	bool SendCmd(const void* pstrCmd,const int32_t nCmdLen)
	{
		SendCmdToSQueue(pstrCmd,nCmdLen);
		NotityWriteEvent();
		return true;
	}

protected:

private:
	//֪ͨд����
	void NotityWriteEvent()
	{
		m_sendLock.lock();
		if (wstate != WS_On)
		{
			if (!sendQueue.empty())
			{
				t_BufferCmd* ptCmd = sendQueue.front();
				asio::async_write(socket_,
					asio::buffer(&ptCmd->pstrCmd[ptCmd->offset],
					ptCmd->nCmdLen - ptCmd->offset),
					//strand_.wrap(boost::bind(&zSocket::HandleWrite, shared_from_this(),
					strand_.wrap(boost::bind(&zSocket::HandleWrite, this,
					asio::placeholders::error)));
				sendQueue.pop();
				SAFE_DELETE(ptCmd);   // ����deleteô
				wstate = WS_On;
				
			}
		}
		m_sendLock.unlock();

	}

	bool SendCmdToSQueue(const void* pstrCmd,const int32_t nCmdLen)
	{
		uint32_t nSendCmdLen;
		m_sendLock.lock();
		if (!sendQueue.empty())
		{
			//�ǿն��У���õ�һ��Ԫ�ص�����
			t_BufferCmd *ptCmd = sendQueue.back();
			nSendCmdLen = MAX_DATABUFFERSIZE - ptCmd->nCmdLen;

			if (packetAppend(pstrCmd, nCmdLen, &ptCmd->pstrCmd[ptCmd->nCmdLen], &nSendCmdLen))
			{
				ptCmd->nCmdLen += nSendCmdLen;
				m_sendLock.unlock();
				return true;
			}
		}

		//ָ������Ϊ�գ�������һ�������Ѿ�����
		t_BufferCmd *ptCmd = new t_BufferCmd;
		if (NULL == ptCmd)
		{
			//���ܷ����ڴ�
			m_sendLock.unlock();
			return false;
		}
		sendQueue.push(ptCmd);
		nSendCmdLen =  MAX_DATABUFFERSIZE - ptCmd->nCmdLen;

		if (packetAppend(pstrCmd, nCmdLen, &ptCmd->pstrCmd[ptCmd->nCmdLen], &nSendCmdLen))
		{
			ptCmd->nCmdLen += nSendCmdLen;
			m_sendLock.unlock();
			return true;
		}
		m_sendLock.unlock();
		return false;
	}




	bool packetAppend(const void *pData, const int32_t nLen, unsigned char *pBuffer, uint32_t *nSendCmdLen)
	{
		uint32_t nSize = nLen;
	    {
			if ((nSize & PACKET_MASK) + PH_LEN > *nSendCmdLen)
				//���ȳ������巶Χ
				return false;
			else
				bcopy(pData, &pBuffer[PH_LEN], (nSize & PACKET_MASK));
		}
		*(uint32_t *)(pBuffer) = nSize;
		//Zebra::logger->debug("д�����ݰ�ͷ���ݣ�%u", nSize);
 
		*nSendCmdLen = (nSize & PACKET_MASK) + PH_LEN;
		//Zebra::logger->debug("packetAppend:����%u�ֽ����ݰ�", nSize+PH_LEN);

		return true;
	}

	/*
	bool MessageToQueue()
	{
		uint32_t nLeftCmdLen;
		m_lock.lock();
		if (!recvQueue.empty())
		{
			//�ǿն��У���õ�һ��Ԫ�ص�����
			t_BufferCmd *ptCmd = recvQueue.back();
			nLeftCmdLen = MAX_DATABUFFERSIZE - ptCmd->nCmdLen;

			if (nLeftCmdLen >= GetMsgLen())
			{
				bcopy(recvBuffer_,&ptCmd->pstrCmd[ptCmd->nCmdLen],GetMsgLen());
				ptCmd->nCmdLen += GetMsgLen();
				m_lock.unlock();
				return true;
			}
		}

		//ָ������Ϊ�գ�������һ�������Ѿ�����
		t_BufferCmd *ptCmd = new t_BufferCmd;
		if (NULL == ptCmd)
		{
			//���ܷ����ڴ�
			m_lock.unlock();
			return false;
		}
		recvQueue.push(ptCmd);
		nLeftCmdLen =  MAX_DATABUFFERSIZE - ptCmd->nCmdLen;

		if (nLeftCmdLen >= GetMsgLen())
		{
			bcopy(recvBuffer_,&ptCmd->pstrCmd[ptCmd->nCmdLen],GetMsgLen());
			ptCmd->nCmdLen += GetMsgLen();
			m_lock.unlock();
			return true;
		}
		m_lock.unlock();
		return false;

	}*/

	inline void ResetRecvBuffer()
	{
		bufflen_ = 0;
	}

	inline bool ValidateMsgLen()
	{
		uint32_t msgLen =  *(uint32_t*)recvBuffer_;
		ztLoggerWrite(ZONE,e_Debug,"decode packet len:%d",msgLen);
		return *(uint32_t*)recvBuffer_ < MAX_MESSAGE_SIZE;
	}

	inline uint32_t GetBodyLen()
	{
		return *(uint32_t*)recvBuffer_;
	}

	inline uint32_t GetMsgLen()
	{
		return *(uint32_t*)recvBuffer_ + PH_LEN;
	}

	inline uint8_t* GetRecvBodyBuf()
	{
		return &recvBuffer_[PH_LEN];
	}

	///////////////////////////////////////////////



protected:
	zProcessor * m_pBasicTask;
	asio::ip::tcp::socket socket_;

	asio::deadline_timer check_timer_;   //���socket��Ϣ

	asio::io_service::strand strand_;

	uint8_t recvBuffer_[MAX_MESSAGE_SIZE];		// ���ջ��� 
	int32_t  bufflen_;							//������Ч����

	//t_BufferCmdQueue recvQueue;
	//zMutex m_lock;

	t_BufferCmdQueue sendQueue;				//���ͻ���
	zMutex m_sendLock;


	volatile bool  m_bQuit;    //��־��Ҫ�Ͽ����� (kicked off)

	enum WriteState
	{
		WS_Init = 0,
		WS_On	= 1,
		WS_Complete = 2,
	};
	WriteState wstate;
};
 


#endif  //__zSocket__H__

