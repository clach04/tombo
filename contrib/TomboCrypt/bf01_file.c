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

#define ERR(...) do { fprintf(stderr, __VA_ARGS__); exit(1); } while (0)  // TODO / FIXME NOT suitable for GUI

/* C API from blowfish.c and md5.c FIXME header files needed or inline */
extern void *BF_Init(unsigned char *key, unsigned keylen);
extern void  BF_Enc(void *handle, unsigned char *cipher, unsigned char *plain, int len);
extern void  BF_Dec(void *handle, unsigned char *plain, unsigned char *cipher);
extern void  BF_Free(void *handle);
extern void  getMD5Sum(unsigned char *md5sum, unsigned char *in, int len);

#define PASS_MAX 1024

/* --- random bytes --- */

void bf01_rand_bytes(unsigned char *buf, int n)
{
#ifdef _WIN32
    int i;
    for (i = 0; i < n; i++)
        buf[i] = (unsigned char)(rand() & 0xFF);
#else
    FILE *f = fopen("/dev/urandom", "rb");
    if (!f) ERR("cannot open /dev/urandom\n");
    if ((int)fread(buf, 1, n, f) != n) ERR("failed to read /dev/urandom\n");
    fclose(f);
#endif
}


/* --- read all of stdin or a file into malloc'd buffer --- */

static unsigned char *read_all(FILE *f, uint32_t *out_size)
{
    size_t cap = 32768;
    #define READ_BUFF_SIZE 4096
    size_t size = 0;
    unsigned char *data = malloc(cap);
    if (!data) ERR("out of memory\n");
    while (!feof(f)) {
        if (size + READ_BUFF_SIZE > cap) {
            cap *= 2;
            data = realloc(data, cap);
            if (!data) ERR("out of memory\n");
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
int bf01_encrypt_stream(FILE *fin, FILE *fout, const char *password, int brave, const unsigned char *salt)
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
        if (passlen >= PASS_MAX) ERR("password too long (max %d)\n", PASS_MAX);  // TODO empty password check?
    } else {
        ERR("NULL empty password\n");
    }

    /* derive key */
    bf01_derive_key(md5key, password, passlen);

    /* read input */
    plaintext = read_all(fin, &filesize);

    /* build encrypt buffer: salt(8) + md5(16) + plaintext, padded to 8-byte */
    buflen = ((filesize >> 3) + 1) * 8 + 24;
    buf = calloc(1, buflen);
    if (!buf) ERR("out of memory\n");

    if (salt) {
        use_salt = salt;
    } else {
        bf01_rand_bytes(saltbuf, 8);
        use_salt = saltbuf;
    }
    memcpy(buf, use_salt, 8);
    getMD5Sum(md5sum, plaintext, filesize);
    memcpy(buf + 8, md5sum, 16);
    memcpy(buf + 24, plaintext, filesize);

    /* encrypt */
    bf = BF_Init(md5key, 16);
    if (!bf) ERR("BF_Init failed\n");
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

int bf01_decrypt_stream(FILE *fin, FILE *fout, const char *password)
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
        if (passlen >= PASS_MAX) ERR("password too long (max %d)\n", PASS_MAX);  // TODO empty password check?
    } else {
        ERR("NULL empty password\n");
    }

    /* derive key */
    bf01_derive_key(md5key, password, passlen);

    /* read file */
    filedata = read_all(fin, &nfilesize);

    /* validate header */
    if (nfilesize < 8) ERR("file too small\n");
    memcpy(magic, filedata, 4);
    magic[4] = '\0';
    if (strcmp(magic, "BF01") != 0) ERR("bad magic: %s\n", magic);
    memcpy(&datasize, filedata + 4, 4);

    cipherlen = nfilesize - 8;
    if (cipherlen < 24) ERR("ciphertext too short\n");

    /* decrypt */
    bf = BF_Init(md5key, 16);
    if (!bf) ERR("BF_Init failed\n");
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
        ERR("invalid password\n");
    }

    /* write output */
    fwrite(filedata + 8 + 24, 1, datasize, fout);

    free(filedata);
    return 0;
}
