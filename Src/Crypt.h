#ifndef CRYPT_H
#define CRYPT_H

class Crypt {
	void *handle;
public:
	Crypt();
	~Crypt();

	BOOL ResetStream(byte *key, unsigned int len);
	BOOL Encrypt(byte *chipher, byte *plain, int len);
	BOOL Decrypt(byte *plain, byte *chipher);
};

#endif
