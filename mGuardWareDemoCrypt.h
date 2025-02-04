#pragma once

#include <windows.h>
#include <stdio.h>
#include <bcrypt.h>
#include <iostream>
#include <Shlwapi.h>
#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)
#endif
#include <chrono>

#ifndef _WIN32
#pragma error("Only Windows supported")
#endif

#ifndef _UNICODE
#pragma error("UNICODE mode only supported")
#endif

void PrintError(DWORD dwErrorCode);

BCRYPT_ALG_HANDLE GetAlgHandle(LPCWSTR pszAlgId, LPCWSTR pszImplementation = NULL);


PUCHAR GeneratePseudoRandomPassword(ULONG SizeInBits);

//Key Size In Bits From 512 To 16384 And Must Be A Multiple Of 64 For RSA
BCRYPT_KEY_HANDLE GenerateAsymmetricKey(BCRYPT_ALG_HANDLE& hRSA, ULONG KeyLength);

BCRYPT_KEY_HANDLE GenerateSymmetricKey(BCRYPT_ALG_HANDLE& hAlgorithm, PUCHAR pzSecret, ULONG SizeOfKeyInBytes);

PUCHAR EncryptRSA(PUCHAR pbPlainText, ULONG ulPlainSize, ULONG& ulCipherSize, BCRYPT_ALG_HANDLE& hRSAKey);

PUCHAR DecryptRSA(PUCHAR pCipherText, ULONG ulCipherSize, ULONG& DigestSize, BCRYPT_KEY_HANDLE& hRSAKey);

PUCHAR DecryptAES(PUCHAR pbCipherText, ULONG ulSizeOfCipherText, ULONG& ulSizeOfPlainText, BCRYPT_KEY_HANDLE hAesKey);

PUCHAR EncryptAES(PUCHAR pbPlainText, ULONG ulSizeOfPlainText, ULONG& ulSizeOfCipherText, BCRYPT_KEY_HANDLE hAesKey);

PUCHAR HashDataSHA256(PUCHAR pData, ULONG ulDataSize, ULONG& ulHashSize);

