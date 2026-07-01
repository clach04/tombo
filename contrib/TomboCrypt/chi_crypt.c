/*
 * chi_crypt - Encrypt/decrypt .chi files (Blowfish-CBC, MD5 KDF)
 *
 * Compatible with TomboCrypt .chi format.
 *
 * Compile:
 *   gcc -std=c99 -DTOMBO -o chi_crypt chi_crypt.c blowfish.c md5.c
 *
 * Usage:
 *   chi_crypt -e infile outfile [--password PASS] [-b]
 *   chi_crypt -d infile outfile [--password PASS]
 *   chi_crypt -e - outfile [--password PASS] [-b]    (stdin)
 *   chi_crypt -d infile - [--password PASS]           (stdout)
 *
 * -b / --brave: skip password confirmation on encrypt
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

#define ERR(...) do { fprintf(stderr, __VA_ARGS__); exit(1); } while (0)

/* C API from blowfish.c and md5.c */
extern void *BF_Init(unsigned char *key, unsigned keylen);
extern void  BF_Enc(void *handle, unsigned char *cipher, unsigned char *plain, int len);
extern void  BF_Dec(void *handle, unsigned char *plain, unsigned char *cipher);
extern void  BF_Free(void *handle);
extern void  getMD5Sum(unsigned char *md5sum, unsigned char *in, int len);

#define PASS_MAX 1024

static unsigned char md5key[16];

static void set_binary_stdout(void)
{
#ifdef _WIN32
    _setmode(_fileno(stdout), _O_BINARY);
#endif
}

/* --- random bytes --- */

static void rand_bytes(unsigned char *buf, int n)
{
#ifdef _WIN32
    /* fallback: rand() -- not ideal but matches original TomboCrypt behavior */
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

/* --- no-echo password input --- */

#ifdef _WIN32
#include <windows.h>
static void set_echo(int on)
{
    HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(h, &mode);
    if (on)
        mode |= ENABLE_ECHO_INPUT;
    else
        mode &= ~ENABLE_ECHO_INPUT;
    SetConsoleMode(h, mode);
}
#else
#include <termios.h>
#include <unistd.h>
static struct termios orig_termios;
static int termios_saved;
static void set_echo(int on)
{
    struct termios t;
    if (!on) {
        tcgetattr(STDIN_FILENO, &orig_termios);
        termios_saved = 1;
        t = orig_termios;
        t.c_lflag &= ~(tcflag_t)ECHO;
        tcsetattr(STDIN_FILENO, TCSANOW, &t);
    } else if (termios_saved) {
        tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
        termios_saved = 0;
    }
}
#endif

static int read_password(const char *prompt, char *buf, int buflen)
{
    int len;
    fprintf(stderr, "%s", prompt);
    set_echo(0);
    if (!fgets(buf, buflen, stdin)) {
        set_echo(1);
        ERR("failed to read password\n");
    }
    set_echo(1);
    fprintf(stderr, "\n");
    len = (int)strlen(buf);
    while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r'))
        buf[--len] = '\0';
    return len;
}

/* --- read all of stdin or a file into malloc'd buffer --- */

static unsigned char *read_all(FILE *f, uint32_t *out_size)
{
    size_t cap = 32768;  // TODO review this compared with https://github.com/clach04/chi_io
    #define READ_BUFF_SIZE 4096  // review this, different approach compared with CryptManager.cpp
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

/* --- encrypt --- */

static void do_encrypt(const char *inname, const char *outname,
                       const char *password, int brave)
{
    char pass[PASS_MAX], pass2[PASS_MAX];
    int passlen;
    FILE *fin, *fout;
    uint32_t filesize;
    unsigned char *plaintext;
    void *bf;
    uint32_t buflen;
    unsigned char *buf;
    uint8_t salt[8];
    unsigned char md5sum[16];
    uint32_t nsize;

    /* password */
    if (password) {
        passlen = (int)strlen(password);
        if (passlen >= PASS_MAX) ERR("password too long (max %d)\n", PASS_MAX);
        memcpy(pass, password, passlen + 1);
    } else {
        passlen = read_password("password: ", pass, PASS_MAX);
        if (passlen == 0) ERR("empty password\n");
        if (!brave) {
            read_password("confirm: ", pass2, PASS_MAX);
            if (strcmp(pass, pass2) != 0) ERR("passwords do not match\n");
        }
    }

    /* derive key */
    getMD5Sum(md5key, (unsigned char *)pass, passlen);

    /* read input */
    if (strcmp(inname, "-") == 0) {
        fin = stdin;
    } else {
        fin = fopen(inname, "rb");
        if (!fin) ERR("cannot open %s\n", inname);
    }
    plaintext = read_all(fin, &filesize);
    if (fin != stdin) fclose(fin);

    /* build encrypt buffer: salt(8) + md5(16) + plaintext, padded to 8-byte */
    buflen = ((filesize >> 3) + 1) * 8 + 24;
    buf = calloc(1, buflen);
    if (!buf) ERR("out of memory\n");

    rand_bytes(salt, 8);
    memcpy(buf, salt, 8);
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
    if (strcmp(outname, "-") == 0) {
        set_binary_stdout();
        fout = stdout;
    } else {
        fout = fopen(outname, "wb");
        if (!fout) ERR("cannot open %s for writing\n", outname);
    }
    nsize = filesize;
    fwrite("BF01", 1, 4, fout);
    fwrite(&nsize, 4, 1, fout);
    fwrite(buf, 1, buflen, fout);
    if (fout != stdout) fclose(fout);

    free(plaintext);
    free(buf);
}

/* --- decrypt --- */

static void do_decrypt(const char *inname, const char *outname,
                       const char *password)
{
    char pass[PASS_MAX];
    int passlen;
    FILE *fin, *fout;
    unsigned char *filedata;
    uint32_t nfilesize;
    char magic[5];
    uint32_t datasize;
    uint32_t cipherlen;
    void *bf;
    unsigned char md5check[16];

    /* password */
    if (password) {
        passlen = (int)strlen(password);
        if (passlen >= PASS_MAX) ERR("password too long (max %d)\n", PASS_MAX);
        memcpy(pass, password, passlen + 1);
    } else {
        passlen = read_password("password: ", pass, PASS_MAX);
        if (passlen == 0) ERR("empty password\n");
    }

    /* derive key */
    getMD5Sum(md5key, (unsigned char *)pass, passlen);

    /* read file */
    if (strcmp(inname, "-") == 0) {
        fin = stdin;
    } else {
        fin = fopen(inname, "rb");
        if (!fin) ERR("cannot open %s\n", inname);
    }
    filedata = read_all(fin, &nfilesize);
    if (fin != stdin) fclose(fin);

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

    /* verify md5 - decrypted blob at filedata+8:
     *   [0..7]   salt
     *   [8..23]  md5 of plaintext
     *   [24..]   plaintext
     */
    getMD5Sum(md5check, filedata + 8 + 24, (int)datasize);
    if (memcmp(filedata + 8 + 8, md5check, 16) != 0) {
        ERR("invalid password\n");
    }

    /* write output */
    if (strcmp(outname, "-") == 0) {
        set_binary_stdout();
        fout = stdout;
    } else {
        fout = fopen(outname, "wb");
        if (!fout) ERR("cannot open %s for writing\n", outname);
    }
    fwrite(filedata + 8 + 24, 1, datasize, fout);
    if (fout != stdout) fclose(fout);

    free(filedata);
}

/* --- main --- */

static void usage(void)
{
    fprintf(stderr, "Usage: chi_crypt -e|-d infile outfile [--password PASS] [-b/--brave]\n");
    fprintf(stderr, "  -e        encrypt\n");
    fprintf(stderr, "  -d        decrypt\n");
    fprintf(stderr, "  -b        skip password confirmation (encrypt only)\n");
    fprintf(stderr, "  --password PASS   supply password on command line, rather than prompt\n");
    fprintf(stderr, "  -b/--brave   only prompt for password once\n");
    fprintf(stderr, "  Use - for infile/outfile to use stdin/stdout\n");
}

int main(int argc, char *argv[])
{
    int mode = 0; /* 0=none, 1=encrypt, 2=decrypt */
    int brave = 0;
    const char *password = NULL;
    const char *infile = NULL;
    const char *outfile = NULL;
    int i;

    srand((unsigned)time(NULL));

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-e") == 0) {
            mode = 1;
        } else if (strcmp(argv[i], "-d") == 0) {
            mode = 2;
        } else if (strcmp(argv[i], "-b") == 0 || strcmp(argv[i], "--brave") == 0) {
            brave = 1;
        } else if (strcmp(argv[i], "--password") == 0) {
            if (++i >= argc) ERR("--password requires an argument\n");
            password = argv[i];
            if ((int)strlen(password) >= PASS_MAX) ERR("password too long (max %d)\n", PASS_MAX);
        } else if (strcmp(argv[i], "-") == 0) {
            if (!infile) {
                infile = argv[i];
            } else if (!outfile) {
                outfile = argv[i];
            } else {
                ERR("extra argument: %s\n", argv[i]);
            }
        } else if (argv[i][0] == '-') {
            ERR("unknown option: %s\n", argv[i]);
        } else if (!infile) {
            infile = argv[i];
        } else if (!outfile) {
            outfile = argv[i];
        } else {
            ERR("extra argument: %s\n", argv[i]);
        }
    }

    if (!mode || !infile || !outfile) {
        usage();
        return 1;
    }

    if (mode == 1)
        do_encrypt(infile, outfile, password, brave);
    else
        do_decrypt(infile, outfile, password);

    return 0;
}
