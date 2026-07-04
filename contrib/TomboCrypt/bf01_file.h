#ifndef BF01_FILE_H
#define BF01_FILE_H

#include <stdint.h>
#include <stdio.h>

/* encrypt stream; if salt is NULL (default and recommended usage), random salt is generated - if not NULL, needs point to 8 byte buffer */
int bf01_encrypt_stream(FILE *fin, FILE *fout, const char *password, int brave, const unsigned char *salt);
int bf01_decrypt_stream(FILE *fin, FILE *fout, const char *password);

void bf01_derive_key(unsigned char md5key[16], const char *password, int passlen);
void bf01_rand_bytes(unsigned char *buf, int n);

#endif
