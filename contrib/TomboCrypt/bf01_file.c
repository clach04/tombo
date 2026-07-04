/*
 * bf01_file - BF01 format encrypt/decrypt routines (Blowfish-CBC, MD5 KDF)
 *
 * Reusable library extracted from chi_crypt.c.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf01_file.h"

#define ERR_EXIT(...) do { fprintf(stderr, __VA_ARGS__); exit(1); } while (0)

/* C API from blowfish.c and md5.c FIXME header files needed or inline */
extern void *BF_Init(unsigned char *key, unsigned keylen);
extern void  BF_Enc(void *handle, unsigned char *cipher, unsigned char *plain, int len);
extern void  BF_Dec(void *handle, unsigned char *plain, unsigned char *cipher);
extern void  BF_Free(void *handle);
extern void  getMD5Sum(unsigned char *md5sum, unsigned char *in, int len);

#define PASS_MAX 1024

/* --- random bytes --- */

int bf01_rand_bytes(unsigned char *buf, int n, int hard_exit)
{
#ifdef _WIN32
    int i;
    for (i = 0; i < n; i++)
        buf[i] = (unsigned char)(rand() & 0xFF);
    return 0;
#else
    FILE *f = fopen("/dev/urandom", "rb");
    if (!f) { if (hard_exit) ERR_EXIT("cannot open /dev/urandom\n"); return -1; }
    if ((int)fread(buf, 1, n, f) != n) { fclose(f); if (hard_exit) ERR_EXIT("failed to read /dev/urandom\n"); return -1; }
    fclose(f);
    return 0;
#endif
}


/* --- read all of stdin or a file into malloc'd buffer --- */

static unsigned char *read_all(FILE *f, uint32_t *out_size, int hard_exit)
{
    size_t cap = 32768;
    #define READ_BUFF_SIZE 4096
    size_t size = 0;
    unsigned char *data = malloc(cap);
    if (!data) { if (hard_exit) ERR_EXIT("out of memory\n"); return NULL; }
    while (!feof(f)) {
        if (size + READ_BUFF_SIZE > cap) {
            cap *= 2;
            data = realloc(data, cap);
            if (!data) { if (hard_exit) ERR_EXIT("out of memory\n"); return NULL; }
        }
        size += fread(data + size, 1, READ_BUFF_SIZE, f);
    }
    *out_size = (uint32_t)size;
    return data;
}

/* --- key derivation --- */

void bf01_derive_key(unsigned char md5key[16], const char *password, int passlen)
{
    getMD5Sum(md5key, (unsigned char *)password, passlen);
}

/* --- encrypt stream --- */

/* encrypt stream; if salt is NULL (default and recommended usage), random salt is generated - if not NULL, needs point to 8 byte buffer */
int bf01_encrypt_stream(FILE *fin, FILE *fout, const char *password, int brave, const unsigned char *salt, int hard_exit)
{
    int passlen;
    uint32_t filesize;
    unsigned char *plaintext;
    unsigned char md5key[16];
    void *bf;
    uint32_t buflen;
    unsigned char *buf;
    uint8_t saltbuf[8];
    const unsigned char *use_salt;
    unsigned char md5sum[16];
    uint32_t nsize;

    /* password */
    if (password) {
        passlen = (int)strlen(password);
        if (passlen >= PASS_MAX) { if (hard_exit) ERR_EXIT("password too long (max %d)\n", PASS_MAX); return -1; }
    } else {
        if (hard_exit) ERR_EXIT("NULL empty password\n");
        return -1;
    }

    /* derive key */
    bf01_derive_key(md5key, password, passlen);

    /* read input */
    plaintext = read_all(fin, &filesize, hard_exit);
    if (!plaintext) return -1;

    /* build encrypt buffer: salt(8) + md5(16) + plaintext, padded to 8-byte */
    buflen = ((filesize >> 3) + 1) * 8 + 24;
    buf = calloc(1, buflen);
    if (!buf) { free(plaintext); if (hard_exit) ERR_EXIT("out of memory\n"); return -1; }

    if (salt) {
        use_salt = salt;
    } else {
        bf01_rand_bytes(saltbuf, 8, hard_exit);
        use_salt = saltbuf;
    }
    memcpy(buf, use_salt, 8);
    getMD5Sum(md5sum, plaintext, filesize);
    memcpy(buf + 8, md5sum, 16);
    memcpy(buf + 24, plaintext, filesize);

    /* encrypt */
    bf = BF_Init(md5key, 16);
    if (!bf) { free(plaintext); free(buf); if (hard_exit) ERR_EXIT("BF_Init failed\n"); return -1; }
    {
        unsigned char *p = buf;
        unsigned char tmp[8];
        int remaining = (int)(buflen);
        while (remaining > 8) {
            memcpy(tmp, p, 8);
            BF_Enc(bf, p, tmp, 8);
            p += 8;
            remaining -= 8;
        }
        if (remaining > 0) {
            memcpy(tmp, p, remaining);
            memset(tmp + remaining, 0, 8 - remaining);
            BF_Enc(bf, p, tmp, remaining);
        }
    }
    BF_Free(bf);

    /* write output */
    nsize = filesize;
    fwrite("BF01", 1, 4, fout);
    fwrite(&nsize, 4, 1, fout);
    fwrite(buf, 1, buflen, fout);

    free(plaintext);
    free(buf);
    return 0;
}

/* --- decrypt stream --- */

int bf01_decrypt_stream(FILE *fin, FILE *fout, const char *password, int hard_exit)
{
    int passlen;
    unsigned char *filedata;
    uint32_t nfilesize;
    char magic[5];
    uint32_t datasize;
    uint32_t cipherlen;
    unsigned char md5key[16];
    void *bf;
    unsigned char md5check[16];

    /* password */
    if (password) {
        passlen = (int)strlen(password);
        if (passlen >= PASS_MAX) { if (hard_exit) ERR_EXIT("password too long (max %d)\n", PASS_MAX); return -1; }
    } else {
        if (hard_exit) ERR_EXIT("NULL empty password\n");
        return -1;
    }

    /* derive key */
    bf01_derive_key(md5key, password, passlen);

    /* read file */
    filedata = read_all(fin, &nfilesize, hard_exit);
    if (!filedata) return -1;

    /* validate header */
    if (nfilesize < 8) { free(filedata); if (hard_exit) ERR_EXIT("file too small\n"); return -1; }
    memcpy(magic, filedata, 4);
    magic[4] = '\0';
    if (strcmp(magic, "BF01") != 0) { free(filedata); if (hard_exit) ERR_EXIT("bad magic: %s\n", magic); return -1; }
    memcpy(&datasize, filedata + 4, 4);

    cipherlen = nfilesize - 8;
    if (cipherlen < 24) { free(filedata); if (hard_exit) ERR_EXIT("ciphertext too short\n"); return -1; }

    /* decrypt */
    bf = BF_Init(md5key, 16);
    if (!bf) { free(filedata); if (hard_exit) ERR_EXIT("BF_Init failed\n"); return -1; }
    {
        unsigned char *p = filedata + 8;
        unsigned char tmp[8];
        int remaining = (int)cipherlen;
        while (remaining >= 8) {
            memcpy(tmp, p, 8);
            BF_Dec(bf, p, tmp);
            p += 8;
            remaining -= 8;
        }
    }
    BF_Free(bf);

    /* verify md5 */
    getMD5Sum(md5check, filedata + 8 + 24, (int)datasize);
    if (memcmp(filedata + 8 + 8, md5check, 16) != 0) {
        free(filedata);
        if (hard_exit) ERR_EXIT("invalid password\n");
        return -1;
    }

    /* write output */
    fwrite(filedata + 8 + 24, 1, datasize, fout);

    free(filedata);
    return 0;
}
