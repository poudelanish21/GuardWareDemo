#include "mGuardWareDemoCrypt.h"
#include "mNetworking.h"
#include <Windows.h>

#ifdef _DEBUG
#pragma comment(lib, "./x64_Debug_mGuardWareDemoCrypt.lib")
#else
#pragma comment(lib, "./x64_Release_mGuardWareDemoCrypt.lib")
#endif
#define BUFFER_SIZE 4096L

PROCESS_INFORMATION OpenPythonApplication(wchar_t* PythonScriptPath) {

	LPWSTR pszPythonExePath = GetPythonExecutableLocation();

	STARTUPINFO s_startUpInfo;
	ZeroMemory(&s_startUpInfo, sizeof(s_startUpInfo));
	s_startUpInfo.wShowWindow = SW_SHOWDEFAULT;
	PROCESS_INFORMATION s_processInfo;
	ZeroMemory(&s_processInfo, sizeof(s_processInfo));
#pragma warning(push)
#pragma warning(suppress: 6335)
	if (!CreateProcess(pszPythonExePath,
		PythonScriptPath, NULL, NULL, false,
		NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE,
		NULL, NULL, &s_startUpInfo, &s_processInfo)) {

		throw(GetLastError());

	}
#pragma warning(pop)


	return s_processInfo;

}

PUCHAR ReadDataFromPipe(HANDLE hNamedPipe, ULONG& ulDataSize) {

	if (hNamedPipe == NULL) {

		throw(GetLastError());

	}

	PUCHAR pszData = new UCHAR[BUFFER_SIZE];

	Sleep(10);
	if (!ReadFile(hNamedPipe, pszData, BUFFER_SIZE, &ulDataSize, NULL)) {

		printf("Reading Error");
		delete[] pszData;
		throw(GetLastError());

	}

	return pszData;

}

void Demostration() {

	PROCESS_INFORMATION pythonApplication = OpenPythonApplication((wchar_t*)L" .\\pythonScript.py");

	Sleep(200);

	HANDLE hNamedPipe = InitializeNamedPipe((wchar_t*)PIPE_NAME);

	WaitForSingleObject(hNamedPipe, INFINITE);

	BCRYPT_ALG_HANDLE hRSA = GetAlgHandle(BCRYPT_RSA_ALGORITHM);

	BCRYPT_KEY_HANDLE hRSAKey = GenerateAsymmetricKey(*(&hRSA), 4096L);

	ULONG ulSizeOfPublicKey = 0;

	NTSTATUS status = BCryptExportKey(hRSAKey, NULL, BCRYPT_RSAPUBLIC_BLOB, NULL, 0, &ulSizeOfPublicKey, 0);

	if (!NT_SUCCESS(status)) {

		CloseHandle(hNamedPipe);
		BCryptDestroyKey(hRSAKey);
		BCryptCloseAlgorithmProvider(hRSA, 0);
		throw(status);

	}

	PUCHAR hPublicKey = new UCHAR[ulSizeOfPublicKey];

	BCRYPT_RSAKEY_BLOB blobPublicKey;

	status = BCryptExportKey(hRSAKey, NULL, BCRYPT_RSAPUBLIC_BLOB, hPublicKey, ulSizeOfPublicKey, &ulSizeOfPublicKey, 0);

	if (ulSizeOfPublicKey <= sizeof(BCRYPT_RSAKEY_BLOB)) {

		delete[] hPublicKey;
		CloseHandle(hNamedPipe);
		BCryptDestroyKey(hRSAKey);
		BCryptCloseAlgorithmProvider(hRSA, 0);
		throw((ULONG)status);

	}

#pragma warning(push)
#pragma warning(suppress : 6385) //Reading Invalid data warning false positive SAL analysis
	memcpy_s(&blobPublicKey, sizeof(BCRYPT_RSAKEY_BLOB), hPublicKey, sizeof(BCRYPT_RSAKEY_BLOB));
#pragma warning(pop)

	Sleep(20);

	if (SendDataToPipe(hNamedPipe, hPublicKey, ulSizeOfPublicKey) != ERROR_SUCCESS) {

		delete[] hPublicKey;
		CloseHandle(hNamedPipe);
		BCryptDestroyKey(hRSAKey);
		BCryptCloseAlgorithmProvider(hRSA, 0);
		throw(GetLastError());

	}

	ULONG ulSizeOfEncryptedKey = 0;
	WaitForSingleObject(hNamedPipe, INFINITE);
	PUCHAR szEncryptedKey = ReadDataFromPipe(hNamedPipe, ulSizeOfEncryptedKey);
	WaitForSingleObject(hNamedPipe, INFINITE);
	ULONG ulSizeOfDecryptedKey = 0;
	PUCHAR DecryptedKey = DecryptRSA(szEncryptedKey, ulSizeOfEncryptedKey, ulSizeOfDecryptedKey, hRSAKey);

	//No Use Of RSA 
	BCryptDestroyKey(hRSAKey);
	BCryptCloseAlgorithmProvider(hRSA, 0);
	delete[] hPublicKey;
	delete[] szEncryptedKey;


	BCRYPT_ALG_HANDLE hAES = GetAlgHandle(BCRYPT_AES_ALGORITHM);
	BCRYPT_ALG_HANDLE hDecryptedKey = GenerateSymmetricKey(hAES, DecryptedKey, ulSizeOfDecryptedKey);

	PUCHAR NewKey = GeneratePseudoRandomPassword(32); // 256 (bit) / 8 (BYTE)

	ULONG ulSizeOfEncryptedNewKey = 0;
	PUCHAR EncryptedNewKey = EncryptAES(NewKey, 32, ulSizeOfEncryptedNewKey, hDecryptedKey);
	if (SendDataToPipe(hNamedPipe, EncryptedNewKey, ulSizeOfEncryptedNewKey) != ERROR_SUCCESS) {

		delete[] EncryptedNewKey;
		delete[] NewKey;
		CloseHandle(hNamedPipe);
		BCryptDestroyKey(hDecryptedKey);
		BCryptCloseAlgorithmProvider(hAES, 0);
		throw(GetLastError());

	}
	delete[] EncryptedNewKey;
	BCryptDestroyKey(hDecryptedKey);

	BCRYPT_ALG_HANDLE hKey = GenerateSymmetricKey(hAES, NewKey, 32);

	std::string sNumber = "";
	for (ULONG ulNumber = 0; ulNumber < 1000000; ulNumber++) {

		if (IsPrime(ulNumber)) {
			sNumber += std::to_string(ulNumber) + "\n";
			if (static_cast<ULONG>(sNumber.size()) > BUFFER_SIZE) {
				ULONG ulSizeOfEncryptedMessage = 0;

				PUCHAR EncryptedMessage = EncryptAES((PUCHAR)sNumber.c_str(), static_cast<ULONG>(sNumber.size()), ulSizeOfEncryptedMessage, hKey);
				sNumber = "";
				if (SendDataToPipe(hNamedPipe, EncryptedMessage, ulSizeOfEncryptedMessage) != ERROR_SUCCESS) {

					delete[] EncryptedMessage;
					CloseHandle(hNamedPipe);
					BCryptDestroyKey(hKey);
					BCryptCloseAlgorithmProvider(hAES, 0);
					throw(GetLastError());

				}
				WaitForSingleObject(hNamedPipe, INFINITE);
				delete[] EncryptedMessage;

			}
		}
	}
	if (sNumber.size() != 0) {

		ULONG ulSizeOfEncryptedMessage = 0;

		PUCHAR EncryptedMessage = EncryptAES((PUCHAR)sNumber.c_str(), static_cast<ULONG>(sNumber.size()), ulSizeOfEncryptedMessage, hKey);
		sNumber = "";
		if (SendDataToPipe(hNamedPipe, EncryptedMessage, ulSizeOfEncryptedMessage) != ERROR_SUCCESS) {

			delete[] EncryptedMessage;
			CloseHandle(hNamedPipe);
			BCryptDestroyKey(hKey);
			BCryptCloseAlgorithmProvider(hAES, 0);
			throw(GetLastError());

		}
		delete[] EncryptedMessage;

	}
	if (!NT_SUCCESS(status)) {

		CloseHandle(hNamedPipe);
		throw(status);

	}

	CloseHandle(hNamedPipe);


	CloseHandle(pythonApplication.hThread);
	CloseHandle(pythonApplication.hProcess);

	return;

}

int main() {

	try {

		Demostration();

	}
	catch (DWORD dwErrorCode) {

		PrintError(dwErrorCode);

	}



}