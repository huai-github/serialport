/*
* 串口同步读写函数
* 异步读写参考：
* https://github.com/PengKun-PK/SerialPortCommunication
* https://www.cnblogs.com/jiaochen/p/5899381.html
* @author:zth 2020.11.16
*/

#include "serialport.h"

/*
HANDLE WINAPI CreateFile(
  _In_     LPCTSTR               lpFileName,			//串口名，常见szPort.Format(_T("\\\\.\\COM%d"), nPort)，nPort 是串口号；
  _In_     DWORD                 dwDesiredAccess,		//设置访问权限，一般设置为GENERIC_READ | GENERIC_WRITE，即可读可写；
  _In_     DWORD                 dwShareMode,			//串口不可共享，所以这个值必须是0；
  _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,	//文件安全模式，必须设置为NULL；
  _In_     DWORD                 dwCreationDisposition,	//创建方式，串口必须是OPEN_EXISTING；
  _In_     DWORD                 dwFlagsAndAttributes,	//涉及到同步操作和异步操作的概念，具体可参考MSDN。一般如果同步的话就是设置为0；如果异步设置为FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED；
  _In_opt_ HANDLE                hTemplateFile			//文件句柄，对于串口通信必须设置为NULL。
);

DCB structure:
DCB结构体中包含了许多信息，对于串口而已主要有波特率、数据位数、奇偶校验和停止位数等信息。在查询或配置串口的属性时，都需要使用到DCB 结构体。
在使用SetCommState对端口进行配置前，需要使用BuildCommDCB 先build 好DCB 结构体；或是使用GetCommState 拿到DCB 结构体，然后再相应修改对应数据。
一般在使用SetCommState 配置串口后，还需要使用SetupComm 设置串口的缓冲区大小。

COMMTIMEOUTS structure:
这个结构体和SetCommTimeouts 函数主要是用来设置读写超时的信息的，可以具体参考MSDN。
其中读写串口的超时有两种：间隔超时和总超时。
间隔超时：是指在接收时两个字符之间的最大时延。
总超时：是指读写操作总共花费的最大时间。写操作只支持总超时，而读操作两种超时均支持
*/

int serial_open(HANDLE* phCom, LPCWSTR COMx, int BaudRate)
{ 
	*phCom = CreateFile(COMx,
						GENERIC_READ | GENERIC_WRITE,	//允许读和写    
						0,								//独占方式    
						NULL,
						OPEN_EXISTING,					//打开而不是创建     
						0,								//重叠方式FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED  (同步方式设置为0)
						NULL);
	if (*phCom == INVALID_HANDLE_VALUE)
	{
		printf("Open ERROR!\n");
		return FALSE;
	}

	SetupComm(*phCom, 1024, 1024);		//输入缓冲区和输出缓冲区的大小都是1024 


	COMMTIMEOUTS TimeOuts;							//设定读写超时
	TimeOuts.ReadIntervalTimeout = 1000;
	TimeOuts.ReadTotalTimeoutMultiplier = 500;
	TimeOuts.ReadTotalTimeoutConstant = 5000;		//设定写超时
	TimeOuts.WriteTotalTimeoutMultiplier = 500;
	TimeOuts.WriteTotalTimeoutConstant = 2000;
	SetCommTimeouts(*phCom, &TimeOuts);				//设置超时

	/* 设置串口属性 */
	DCB dcb;
	GetCommState(*phCom, &dcb);
	dcb.BaudRate = BaudRate;		//设置波特率为BaudRate
	dcb.ByteSize = 8;				//每个字节有8位 
	dcb.Parity = NOPARITY;			//无奇偶校验位 
	dcb.StopBits = ONESTOPBIT;		//一个停止位
	SetCommState(*phCom, &dcb);		//设置参数到hCom
	PurgeComm(*phCom, PURGE_TXCLEAR | PURGE_RXCLEAR | PURGE_TXABORT | PURGE_RXABORT);	//清空缓存区
																					   //PURGE_TXABORT 中断所有写操作并立即返回，即使写操作还没有完成。
																					   //PURGE_RXABORT 中断所有读操作并立即返回，即使读操作还没有完成。
																					   //PURGE_TXCLEAR 清除输出缓冲区 
																					   //PURGE_RXCLEAR 清除输入缓冲区  
	return TRUE;
}

/*
BOOL WriteFile(
  HANDLE  hFile,			//文件句柄
  LPCVOID lpBuffer,			//数据缓存区指针
  DWORD   nNumberOfBytesToWrite,	//你要写的字节数
  LPDWORD lpNumberOfBytesWritten,	//用于保存实际写入字节数的存储区域的指针
  LPOVERLAPPED lpOverlapped		//OVERLAPPED结构体指针
) ;
*/
int serial_write(HANDLE hCom, unsigned char WriteBuffer[], DWORD WantWriteLen)	//同步写串口
{
	DWORD dwError;
	DWORD Writed = 0;	//写入的字节数

	if (ClearCommError(hCom, &dwError, NULL))
	{
		PurgeComm(hCom, PURGE_TXABORT | PURGE_TXCLEAR);	// 在读写串口之前，还要用PurgeComm()函数清空缓冲区
														//PURGE_TXABORT	  中断所有写操作并立即返回，即使写操作还没有完成。
														//PURGE_RXABORT	  中断所有读操作并立即返回，即使读操作还没有完成。
														//PURGE_TXCLEAR	  清除输出缓冲区
														//PURGE_RXCLEAR	  清除输入缓冲区
	}
	if (!WriteFile(hCom, WriteBuffer, WantWriteLen, &Writed, NULL))
	{
		printf("Write ERROR\n");
	}	

	return Writed;
}
/*
BOOL ReadFile(
  HANDLE       hFile,			// 串口句柄
  LPVOID       lpBuffer,		// 读缓冲区
  DWORD        nNumberOfBytesToRead,	// 要求读入的字节数
  LPDWORD      lpNumberOfBytesRead,	// 实际读入的字节数
  LPOVERLAPPED lpOverlapped		// 重叠结构
);
*/
int serial_Read(HANDLE hCom, unsigned char read_buffer[], DWORD WantReadLen)
{
	DWORD dwError;
	DWORD Readed = 0;

	PurgeComm(hCom, PURGE_TXCLEAR | PURGE_RXCLEAR | PURGE_TXABORT | PURGE_RXABORT);	// 清空串口缓冲区，终止发送接收异步操作
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

void serial_close(HANDLE hCom)		//关闭串口
{
	CloseHandle(hCom);
}
