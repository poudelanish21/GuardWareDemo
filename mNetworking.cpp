#include "mNetworking.h"
#pragma comment(lib, "shlwapi.lib")

bool IsPrime(ULONG Number) {

	if (Number & 1) {

		ULONG ulSquareRoot = static_cast<ULONG>(sqrt(Number));

		for (ULONG i = 3; i < ulSquareRoot; i += 2) {

			if (Number % i == 0) {

				return false;

			}

		}

		return true;

	}

	if (Number != 2) {

		return false;

	}
	return true;

}

//Pipe Name With The \\.pipe\ Prefix
HANDLE InitializeNamedPipe(PWSTR szPipeName) {

	HANDLE hNamedPipe = CreateNamedPipe(szPipeName, PIPE_ACCESS_DUPLEX,
		PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_NOWAIT, 2, 4096,				//Changing Read Mode To Byte Changes Pipe From Memory Sharing To Message Sharing Form Of IPC.
		4096, 50, NULL);

	if (hNamedPipe == INVALID_HANDLE_VALUE) {

		throw(GetLastError());

	}

	return hNamedPipe;


}

bool CheckConnection(HANDLE hNamedPipe) {

	if (!ConnectNamedPipe(hNamedPipe, NULL)) {

		DWORD Error = GetLastError();

		if (Error == ERROR_IO_PENDING) {

			WaitForSingleObject(hNamedPipe, TIME_OUT);
			return true;

		}

		return false;

	}

	return true;

}

void SendNumberToPipe(HANDLE hNamedPipe, ULONG ulNumber) {

	std::string sNumber = std::to_string(ulNumber) + "\n";

	DWORD dwBytesWritten = 0;
	if (!WriteFile(hNamedPipe, sNumber.c_str(),
		static_cast<DWORD>(sNumber.size()), &dwBytesWritten,
		NULL)) {

		throw(GetLastError());

	}

}

LPWSTR GetPythonExecutableLocation() {

	//Better Alternative Would Be Using AssocQueryString function

	LPWSTR Path = static_cast<LPWSTR>(new wchar_t[MAX_PATH]);
	HINSTANCE hInstance = FindExecutable(L"python.exe", NULL, Path);

	return Path;

}

DWORD SendDataToPipe(HANDLE hNamedPipe, PUCHAR pszData, ULONG ulDataSize) {

	if (hNamedPipe == NULL) {

		return static_cast<ULONG>(GetLastError());

	}
	DWORD dwBytesWritten = 0;

	if (!WriteFile(hNamedPipe, (const unsigned char*)pszData, ulDataSize, &dwBytesWritten, 0)) {

		return static_cast<ULONG>(GetLastError());

	}

	return ERROR_SUCCESS;

}

DWORD SendDataToPythonFile(wchar_t* PythonFileName) {

	STARTUPINFO s_startUpInfo;
	ZeroMemory(&s_startUpInfo, sizeof(s_startUpInfo));
	s_startUpInfo.wShowWindow = SW_SHOWDEFAULT;
	PROCESS_INFORMATION s_processInfo;
	ZeroMemory(&s_processInfo, sizeof(s_processInfo));
	LPWSTR PythonExePath = GetPythonExecutableLocation();
	if (!CreateProcess(PythonExePath,
		PythonFileName, NULL, NULL, false,
		NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE,
		NULL, NULL, &s_startUpInfo, &s_processInfo)) {

		throw(GetLastError());

	}

	try {

		HANDLE hNamedPipe = InitializeNamedPipe((PWSTR)PIPE_NAME);

		if (!CheckConnection(hNamedPipe)) {

			CloseHandle(s_processInfo.hProcess);
			CloseHandle(s_processInfo.hThread);
			throw(GetLastError());

		}

		for (ULONG i = 0; i < 1000000; i++) {

			if (IsPrime(i)) {

				SendNumberToPipe(hNamedPipe, i);

			}

		}

		CloseHandle(hNamedPipe);
		CloseHandle(s_processInfo.hProcess);
		CloseHandle(s_processInfo.hThread);
	}
	catch (DWORD dwErrorCode) {

		std::cout << "Error with code:" << dwErrorCode << "\n";

		if (s_processInfo.hProcess == nullptr) {
			CloseHandle(s_processInfo.hProcess);
		}
		if (s_processInfo.hThread == nullptr) {
			CloseHandle(s_processInfo.hThread);
		}
	}
	return 0;

}