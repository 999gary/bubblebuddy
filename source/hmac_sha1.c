
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "sha1.h"
#include "sha1.c"

typedef struct {
    union {
        unsigned char hash[20];
        struct { u32 h0, h1, h2, h3, h4; };
    };
} sha1_hash160;

sha1_hash160 sha1(unsigned char *data, int len) {
    sha1_hash160 result = {0};
    SHA1(result.hash, data, len);
    return result;
}

sha1_hash160 hmac_sha1(unsigned char *key, int key_len, 
                       unsigned char *data, int len) {
    int block_size = 64;
    unsigned char i_key_pad[64] = {
        0x36,0x36,0x36,0x36,0x36,0x36,0x36,0x36,
        0x36,0x36,0x36,0x36,0x36,0x36,0x36,0x36,
        0x36,0x36,0x36,0x36,0x36,0x36,0x36,0x36,
        0x36,0x36,0x36,0x36,0x36,0x36,0x36,0x36,
        0x36,0x36,0x36,0x36,0x36,0x36,0x36,0x36,
        0x36,0x36,0x36,0x36,0x36,0x36,0x36,0x36,
        0x36,0x36,0x36,0x36,0x36,0x36,0x36,0x36,
        0x36,0x36,0x36,0x36,0x36,0x36,0x36,0x36,
    };
    unsigned char temp[64+20] = {
        0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,
        0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,
        0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,
        0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,
        0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,
        0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,
        0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,
        0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,0x5c,
    };
    SHA1_CTX context;
    
    int i;
    for (i = 0; i < key_len; i++) {
        temp[i] ^= key[i];
        i_key_pad[i] ^= key[i];
    }
    
    SHA1Init(&context);
    SHA1Update(&context, i_key_pad, sizeof(i_key_pad));
    SHA1Update(&context, data, len);
    SHA1Final(temp + block_size, &context);
    
    return sha1(temp, sizeof(temp));
}

sha1_hash160 bfbb_hmac_sha1(unsigned char *data, int len) {
    unsigned char bfbb_sig[] = {
        0x06,0x17,0xEE,0x6E,
        0x75,0x9C,0x64,0xF1,
        0x42,0xBC,0x81,0x91,
        0xF2,0x2C,0xA9,0x32
    };
    
    return hmac_sha1(bfbb_sig, sizeof(bfbb_sig), data, len);
}

// NOTE(jelly): test code - change this to 0
#if 0

#define StringLiteralLen(s) (sizeof(s)-1)
#define StringLiteralAndLen(s) s, StringLiteralLen(s)

char *print_hash(sha1_hash160 hash) {
    static char buf[64];
    int i;
    buf[0] = 0;
    for (i = 0; i < 20; i++) {
        char bruh[64];
        sprintf(bruh, "%02x", hash.hash[i]);
        strcat(buf, bruh);
    }
    return buf;
}

void test_hash(char *str, char *expected) {
    sha1_hash160 hash = sha1(str, strlen(str));
    char *hash_str = print_hash(hash);
    int success = !memcmp(hash_str, expected, 40);
    printf("test('%s') = %s ", str, expected);
    if (success) {
        printf("PASSED!\n");
    } else {
        printf("FAILED with %s\n", hash_str);
    }
}

int main(void) {
    int n = 0xc9c8;
    char *buffer = malloc(n);
    FILE *in = fopen("GameData.xsv", "rb");
    if (in) {
        fread(buffer, 1, n, in);
        puts(print_hash(bfbb_hmac_sha1(buffer, n)));
    }
}

#endif