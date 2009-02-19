#include <windows.h>
#include "Crypt.h"

extern "C" {
	void *BF_Init(byte *key, unsigned keylen);
	void BF_Enc(void *handle, byte *chipher, byte *plain, int len);
	void BF_Dec(void *handle, byte *plain, byte *chipher);
	void BF_Free(void *handle);
};


Crypt::Crypt() : handle(NULL)
{
}

Crypt::~Crypt()
{
	if (handle != NULL) {
		BF_Free(handle);
	}
}

BOOL Crypt::ResetStream(byte *key, unsigned int len)
{
	if (handle != NULL) {
		BF_Free(handle);
		handle = NULL;
	}

	handle = BF_Init(key, len);
	if (handle == NULL) return FALSE;

	return TRUE;
}

BOOL Crypt::Encrypt(byte *chipher, byte *plain, int len)
{
	if (handle == NULL) {
		SetLastError(ERROR_INVALID_DATA);
		return FALSE;
	}
	BF_Enc(handle, chipher, plain, len);
	return TRUE;
}

BOOL Crypt::Decrypt(byte *plain, byte *chipher)
{
	if (handle == NULL) {
		SetLastError(ERROR_INVALID_DATA);
		return FALSE;
	}
	BF_Dec(handle, plain, chipher);
	return TRUE;
}
