#pragma once
#define PIPE_NAME L"\\\\.\\pipe\\m_GuardWareDemoPipe"

#include <iostream>
#include <Windows.h>
#include <math.h>
#include <string>

#include <Shlwapi.h>
#define TIME_OUT 400L
#ifndef _UNICODE
pragma error("Please enable the Unicode Character Set")
#endif


DWORD SendDataToPipe(HANDLE hNamedPipe, PUCHAR pszData, ULONG ulDataSize);

bool CheckConnection(HANDLE hNamedPipe);

bool IsPrime(ULONG Number);

LPWSTR GetPythonExecutableLocation();

//Pipe Name With The \\.pipe\ Prefix
HANDLE InitializeNamedPipe(PWSTR szPipeName);