#ifndef BF01_FILE_H
#define BF01_FILE_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

/* in-memory encrypt/decrypt; returns malloc'd buffer, sets *out_len; returns NULL on error */
unsigned char *bf01_encrypt_mem(const unsigned char *plaintext, size_t plaintext_len, const char *password, size_t *out_len, int hard_exit);
unsigned char *bf01_decrypt_mem(const unsigned char *cipherdata, size_t cipherdata_len, const char *password, size_t *out_len, int hard_exit);

/* stream wrappers */
/* encrypt stream; if salt is NULL (default and recommended usage), random salt is generated - if not NULL, needs point to 8 byte buffer */
int bf01_encrypt_stream(FILE *fin, FILE *fout, const char *password, int brave, const unsigned char *salt, int hard_exit);
int bf01_decrypt_stream(FILE *fin, FILE *fout, const char *password, int hard_exit);

void bf01_derive_key(unsigned char md5key[16], const char *password, int passlen);
int bf01_rand_bytes(unsigned char *buf, int n, int hard_exit);

#endif
