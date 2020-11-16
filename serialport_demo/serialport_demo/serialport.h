#ifndef  __SREIALPORT_H__
#define  __SREIALPORT_H__

#include <Windows.h>
#include <iostream>  
#include <tchar.h>   
#include <string.h>

int serial_open(HANDLE* phCom, LPCWSTR COMx, int BaudRate);
int serial_write(HANDLE hCom, unsigned char WriteBuffer[], DWORD WantWriteLen);
int serial_Read(HANDLE hCom, unsigned char read_buffer[], DWORD read_len);
void serial_close(HANDLE hCom);



#endif