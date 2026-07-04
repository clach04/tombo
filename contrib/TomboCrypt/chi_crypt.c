/*
 * chi_crypt - Encrypt/decrypt .chi files (Blowfish-CBC, MD5 KDF)
 *
 * Compatible with TomboCrypt .chi format.
 *
 * Compile:
 *   gcc -std=c99 -DTOMBO -o chi_crypt chi_crypt.c bf01_file.c blowfish.c md5.c
 *
 *   Debug (deterministic salt for testing):
 *   gcc -std=c99 -DTOMBO -DFIXED_VALUES -o chi_crypt_salted chi_crypt.c bf01_file.c blowfish.c md5.c
 *
 * Usage:
 *   chi_crypt -e infile outfile [--password PASS] [-b]
 *   chi_crypt -d infile outfile [--password PASS]
 *   chi_crypt -e - outfile [--password PASS] [-b]    (stdin)
 *   chi_crypt -d infile - [--password PASS]           (stdout)
 *
 * -b / --brave: skip password confirmation on encrypt
 *
 * FIXED_VALUES debug mode adds:
 *   --salt HEX    16 hex digits (8 bytes) for encrypt salt
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

#include "bf01_file.h"

#define ERR(...) do { fprintf(stderr, __VA_ARGS__); exit(1); } while (0)

#define PASS_MAX 1024

static void set_binary_stdout(void)
{
#ifdef _WIN32
    _setmode(_fileno(stdout), _O_BINARY);
#endif
}

static void set_binary_stdin(void)
{
#ifdef _WIN32
    _setmode(_fileno(stdin), _O_BINARY);
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

int read_password(const char *prompt, char *buf, int buflen)
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

#ifdef FIXED_VALUES
static int hex_digit(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static int parse_hex(const char *hex, unsigned char *out, int nbytes)
{
    int i;
    if ((int)strlen(hex) != nbytes * 2)
        return -1;
    for (i = 0; i < nbytes; i++) {
        int hi = hex_digit(hex[i * 2]);
        int lo = hex_digit(hex[i * 2 + 1]);
        if (hi < 0 || lo < 0)
            return -1;
        out[i] = (unsigned char)((hi << 4) | lo);
    }
    return 0;
}
#endif

/* --- encrypt --- */

static void do_encrypt(const char *inname, const char *outname, const char *password, int brave
#ifdef FIXED_VALUES
    , unsigned char *fixed_salt  // yeah, this is nasty and potentially confusing - aim here is to avoid **accidentally** using fixed salt
#endif
)
{
    FILE *fin, *fout;

    /* open input */
    if (strcmp(inname, "-") == 0) {
        set_binary_stdin();
        fin = stdin;
    } else {
        fin = fopen(inname, "rb");
        if (!fin) ERR("cannot open %s\n", inname);
    }

    /* open output */
    if (strcmp(outname, "-") == 0) {
        set_binary_stdout();
        fout = stdout;
    } else {
        fout = fopen(outname, "wb");
        if (!fout) ERR("cannot open %s for writing\n", outname);
    }

#ifdef FIXED_VALUES
    bf01_encrypt_stream(fin, fout, password, brave, fixed_salt, 1);
#else
    bf01_encrypt_stream(fin, fout, password, brave, NULL, 1);
#endif

    if (fin != stdin) fclose(fin);
    if (fout != stdout) fclose(fout);
}

/* --- decrypt --- */

static void do_decrypt(const char *inname, const char *outname,
                       const char *password)
{
    FILE *fin, *fout;

    /* open input */
    if (strcmp(inname, "-") == 0) {
        set_binary_stdin();
        fin = stdin;
    } else {
        fin = fopen(inname, "rb");
        if (!fin) ERR("cannot open %s\n", inname);
    }

    /* open output */
    if (strcmp(outname, "-") == 0) {
        set_binary_stdout();
        fout = stdout;
    } else {
        fout = fopen(outname, "wb");
        if (!fout) ERR("cannot open %s for writing\n", outname);
    }

    bf01_decrypt_stream(fin, fout, password, 1);

    if (fin != stdin) fclose(fin);
    if (fout != stdout) fclose(fout);
}

/* --- main --- */

static void usage(void)
{
    fprintf(stderr, "Usage: chi_crypt -e|-d infile outfile [--password PASS] [-b/--brave]\n");
    fprintf(stderr, "  -e        encrypt\n");
    fprintf(stderr, "  -d        decrypt\n");
    fprintf(stderr, "  -h/--help show this help\n");
    fprintf(stderr, "  -b        skip password confirmation (encrypt only)\n");
    fprintf(stderr, "  --password PASS   supply password on command line, rather than prompt\n");
#ifdef FIXED_VALUES
    fprintf(stderr, "  --salt HEX        16 hex digits (8 bytes) for encrypt salt\n");
#endif
    fprintf(stderr, "  -b/--brave   only prompt for password once\n");
    fprintf(stderr, "  Use - for infile/outfile to use stdin/stdout\n");
}

int main(int argc, char *argv[])
{
    int mode = 0; /* 0=none, 1=encrypt, 2=decrypt */
    int brave = 0;
    char *password = NULL;
    int passlen;
    char pass1[PASS_MAX], pass2[PASS_MAX];
    const char *infile = NULL;
    const char *outfile = NULL;
    int i;
#ifdef FIXED_VALUES
    unsigned char fixed_salt[8];  // uint8_t
    int has_fixed_salt=0;
#endif

    srand((unsigned)time(NULL));
#ifdef FIXED_VALUES
    bf01_rand_bytes(fixed_salt, 8, 1);  // only use a truly fixed salt if command line argument specified
#endif

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-e") == 0) {
            mode = 1;
        } else if (strcmp(argv[i], "-d") == 0) {
            mode = 2;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            usage();
            return 0;
        } else if (strcmp(argv[i], "-b") == 0 || strcmp(argv[i], "--brave") == 0) {
            brave = 1;
        } else if (strcmp(argv[i], "--password") == 0) {
            if (++i >= argc) ERR("--password requires an argument\n");
            password = argv[i];
            if ((int)strlen(password) >= PASS_MAX) ERR("password too long (max %d)\n", PASS_MAX);
#ifdef FIXED_VALUES
        } else if (strcmp(argv[i], "--salt") == 0) {
            if (++i >= argc) ERR("--salt requires 16 hex digits\n");
            if (parse_hex(argv[i], fixed_salt, 8) != 0)
                ERR("--salt must be exactly 16 hex digits\n");
            has_fixed_salt = 1;
#endif
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

    /* password prompt if needed, and sanity checks */
    if (password) {
        passlen = (int)strlen(password);
        if (passlen >= PASS_MAX) ERR("password too long (max %d)\n", PASS_MAX);
        if (passlen == 0) ERR("empty password\n");
    } else {
        passlen = read_password("password: ", pass1, PASS_MAX);
        if (passlen == 0) ERR("empty password\n");
        if (mode == 1 && !brave) {
            read_password("confirm: ", pass2, PASS_MAX);
            if (strcmp(pass1, pass2) != 0) ERR("passwords do not match\n");
        }
        password = pass1;
    }

    if (mode == 1)
        do_encrypt(infile, outfile, password, brave
#ifdef FIXED_VALUES
    , fixed_salt  // TODO use a macro instead
#endif
    );
    else
        do_decrypt(infile, outfile, password);

    return 0;
}
