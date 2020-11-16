#include "serialport.h"

//HANDLE hCom;
unsigned char write_buff[10] = { 0 };		//≤‚ ‘”√
unsigned char read_buffer[1024] = { 0 };

int main()
{
	int i, j;	//test
	for (i = 0; i < 10; i++)
		write_buff[i] = rand() % 10;

	HANDLE hCom = (void*)0;
	if (!serial_open(&hCom, _T("\\\\.\\COM21"), 115200))
	{
		return 0;
	}

	serial_Read(hCom, read_buffer, 200);
	for (j = 0; j < 200; j++)
	{
		printf("%x\t", read_buffer[j]);
	}
	serial_write(hCom, write_buff, 10);//∑¢ÀÕ◊÷∑˚

	serial_close(hCom);//πÿ±’¥Æø⁄
	return 0;
}

