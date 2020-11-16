/*
* ����ͬ����д����
* �첽��д�ο���
* https://github.com/PengKun-PK/SerialPortCommunication
* https://www.cnblogs.com/jiaochen/p/5899381.html
* @author:zth 2020.11.16
*/

#include "serialport.h"

/*
HANDLE WINAPI CreateFile(
  _In_     LPCTSTR               lpFileName,			//������������szPort.Format(_T("\\\\.\\COM%d"), nPort)��nPort �Ǵ��ںţ�
  _In_     DWORD                 dwDesiredAccess,		//���÷���Ȩ�ޣ�һ������ΪGENERIC_READ | GENERIC_WRITE�����ɶ���д��
  _In_     DWORD                 dwShareMode,			//���ڲ��ɹ����������ֵ������0��
  _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,	//�ļ���ȫģʽ����������ΪNULL��
  _In_     DWORD                 dwCreationDisposition,	//������ʽ�����ڱ�����OPEN_EXISTING��
  _In_     DWORD                 dwFlagsAndAttributes,	//�漰��ͬ���������첽�����ĸ������ɲο�MSDN��һ�����ͬ���Ļ���������Ϊ0������첽����ΪFILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED��
  _In_opt_ HANDLE                hTemplateFile			//�ļ���������ڴ���ͨ�ű�������ΪNULL��
);

DCB structure:
DCB�ṹ���а����������Ϣ�����ڴ��ڶ�����Ҫ�в����ʡ�����λ������żУ���ֹͣλ������Ϣ���ڲ�ѯ�����ô��ڵ�����ʱ������Ҫʹ�õ�DCB �ṹ�塣
��ʹ��SetCommState�Զ˿ڽ�������ǰ����Ҫʹ��BuildCommDCB ��build ��DCB �ṹ�壻����ʹ��GetCommState �õ�DCB �ṹ�壬Ȼ������Ӧ�޸Ķ�Ӧ���ݡ�
һ����ʹ��SetCommState ���ô��ں󣬻���Ҫʹ��SetupComm ���ô��ڵĻ�������С��

COMMTIMEOUTS structure:
����ṹ���SetCommTimeouts ������Ҫ���������ö�д��ʱ����Ϣ�ģ����Ծ���ο�MSDN��
���ж�д���ڵĳ�ʱ�����֣������ʱ���ܳ�ʱ��
�����ʱ����ָ�ڽ���ʱ�����ַ�֮������ʱ�ӡ�
�ܳ�ʱ����ָ��д�����ܹ����ѵ����ʱ�䡣д����ֻ֧���ܳ�ʱ�������������ֳ�ʱ��֧��
*/

int serial_open(HANDLE* phCom, LPCWSTR COMx, int BaudRate)
{ 
	*phCom = CreateFile(COMx,
						GENERIC_READ | GENERIC_WRITE,	//�������д    
						0,								//��ռ��ʽ    
						NULL,
						OPEN_EXISTING,					//�򿪶����Ǵ���     
						0,								//�ص���ʽFILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED  (ͬ����ʽ����Ϊ0)
						NULL);
	if (*phCom == INVALID_HANDLE_VALUE)
	{
		printf("Open ERROR!\n");
		return FALSE;
	}

	SetupComm(*phCom, 1024, 1024);		//���뻺����������������Ĵ�С����1024 


	COMMTIMEOUTS TimeOuts;							//�趨��д��ʱ
	TimeOuts.ReadIntervalTimeout = 1000;
	TimeOuts.ReadTotalTimeoutMultiplier = 500;
	TimeOuts.ReadTotalTimeoutConstant = 5000;		//�趨д��ʱ
	TimeOuts.WriteTotalTimeoutMultiplier = 500;
	TimeOuts.WriteTotalTimeoutConstant = 2000;
	SetCommTimeouts(*phCom, &TimeOuts);				//���ó�ʱ

	/* ���ô������� */
	DCB dcb;
	GetCommState(*phCom, &dcb);
	dcb.BaudRate = BaudRate;		//���ò�����ΪBaudRate
	dcb.ByteSize = 8;				//ÿ���ֽ���8λ 
	dcb.Parity = NOPARITY;			//����żУ��λ 
	dcb.StopBits = ONESTOPBIT;		//һ��ֹͣλ
	SetCommState(*phCom, &dcb);		//���ò�����hCom
	PurgeComm(*phCom, PURGE_TXCLEAR | PURGE_RXCLEAR | PURGE_TXABORT | PURGE_RXABORT);	//��ջ�����
																					   //PURGE_TXABORT �ж�����д�������������أ���ʹд������û����ɡ�
																					   //PURGE_RXABORT �ж����ж��������������أ���ʹ��������û����ɡ�
																					   //PURGE_TXCLEAR ������������ 
																					   //PURGE_RXCLEAR ������뻺����  
	return TRUE;
}

/*
BOOL WriteFile(
  HANDLE  hFile,			//�ļ����
  LPCVOID lpBuffer,			//���ݻ�����ָ��
  DWORD   nNumberOfBytesToWrite,	//��Ҫд���ֽ���
  LPDWORD lpNumberOfBytesWritten,	//���ڱ���ʵ��д���ֽ����Ĵ洢�����ָ��
  LPOVERLAPPED lpOverlapped		//OVERLAPPED�ṹ��ָ��
) ;
*/
int serial_write(HANDLE hCom, unsigned char WriteBuffer[], DWORD WantWriteLen)	//ͬ��д����
{
	DWORD dwError;
	DWORD Writed = 0;	//д����ֽ���

	if (ClearCommError(hCom, &dwError, NULL))
	{
		PurgeComm(hCom, PURGE_TXABORT | PURGE_TXCLEAR);	// �ڶ�д����֮ǰ����Ҫ��PurgeComm()������ջ�����
														//PURGE_TXABORT	  �ж�����д�������������أ���ʹд������û����ɡ�
														//PURGE_RXABORT	  �ж����ж��������������أ���ʹ��������û����ɡ�
														//PURGE_TXCLEAR	  ������������
														//PURGE_RXCLEAR	  ������뻺����
	}
	if (!WriteFile(hCom, WriteBuffer, WantWriteLen, &Writed, NULL))
	{
		printf("Write ERROR\n");
	}	

	return Writed;
}
/*
BOOL ReadFile(
  HANDLE       hFile,			// ���ھ��
  LPVOID       lpBuffer,		// ��������
  DWORD        nNumberOfBytesToRead,	// Ҫ�������ֽ���
  LPDWORD      lpNumberOfBytesRead,	// ʵ�ʶ�����ֽ���
  LPOVERLAPPED lpOverlapped		// �ص��ṹ
);
*/
int serial_Read(HANDLE hCom, unsigned char read_buffer[], DWORD WantReadLen)
{
	DWORD dwError;
	DWORD Readed = 0;

	PurgeComm(hCom, PURGE_TXCLEAR | PURGE_RXCLEAR | PURGE_TXABORT | PURGE_RXABORT);	// ��մ��ڻ���������ֹ���ͽ����첽����
	if (!ClearCommError(hCom, &dwError, NULL))
	{
		printf("Read ERROR!\n");
		return FALSE;
	}
	memset(read_buffer, 0, sizeof(read_buffer));

	if (ClearCommError(hCom, &dwError, NULL))
	{
		PurgeComm(hCom, PURGE_RXABORT | PURGE_RXCLEAR);
	}
	if (!ReadFile(hCom, read_buffer, WantReadLen, &Readed, NULL))
	{
		printf("Read ERROR\n");
	}

	return Readed;
}

void serial_close(HANDLE hCom)		//�رմ���
{
	CloseHandle(hCom);
}
