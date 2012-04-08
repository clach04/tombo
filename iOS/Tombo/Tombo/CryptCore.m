#import <CommonCrypto/CommonDigest.h>
#import <CommonCrypto/CommonCryptor.h>
#import "CryptCore.h"

@implementation CryptCore

// digest size should be CC_MD5_DIGEST_LENGTH
+ (void)md5:(NSString *)input buffer:(unsigned char*)digest {
    const char *cStr = [input UTF8String];
    CC_MD5(cStr, strlen(cStr), digest);    
}

+ (NSString *)bytes2hex:(unsigned char *)data length:(NSUInteger) len {
    NSMutableString *out = [NSMutableString stringWithCapacity:len];
    for (int i = 0; i < len; i++) {
        [out appendFormat:@"%02x", data[i]];
    }
    return out;
}

+ (BOOL)selftest {
	unsigned char key1[] = {0x83, 0x45, 0x92, 0x17, 0x68, 0xA9, 0x35, 0xDF,
                           0x25, 0x58, 0x9F, 0x11, 0x72, 0x87, 0x99, 0xFE};
	unsigned char dat1[] = {0x85, 0x28, 0xF9, 0x43, 0xE8, 0xC2, 0x12, 0x50};
    
    unsigned char enc1[] = {0x87, 0x66, 0x8b, 0xc8, 0xeb, 0xfc, 0xe6, 0xde};

    unsigned char buf[16];
    if (![CryptCore encrypt:key1 keyLen:16 plain:dat1 dataLen:8 enc:buf]) {
        NSLog(@"Test1: Encrypt fail.");
        return NO;
    }
    
    if (![CryptCore check:enc1 and:buf length:8]) {
        NSLog(@"Test1: Check fail.");
        return NO;
    }
    
    unsigned char dat2[] = {0x85, 0x28, 0xF9, 0x43, 0xE8, 0xC2, 0x12, 0x50,
                            0x48, 0x02, 0xE1, 0x45, 0x69, 0x3C, 0x8A, 0x9F};
    unsigned char enc2[] = {0xbd, 0xfd, 0x34, 0xa6, 0xb2, 0xa6, 0xed, 0xb9,
                            0x3f, 0x23, 0x3f, 0x73, 0x5a, 0x52, 0x09, 0xfb};
    if (![CryptCore encrypt:key1 keyLen:16 plain:dat2 dataLen:16 enc:buf]) {
        NSLog(@"Test2: Encrypt fail.");
        return NO;
    }
    if (![CryptCore check:enc2 and:buf length:16]) {
        NSLog(@"Test2: Check fail.");
        return NO;
    }
    if (![CryptCore decrypt:key1 keyLen:16 crypt:enc1 dataLen:8 plain:buf]) {
        NSLog(@"Test3: Decrypt fail.");
        return NO;
    }
    if (![CryptCore check:dat1 and:buf length:8]) {
        NSLog(@"Test3: Check fail.");
        return NO;
    }
    
    if (![CryptCore decrypt:key1 keyLen:16 crypt:enc2 dataLen:16 plain:buf]) {
        NSLog(@"Test4: Decrypt fail.");
        return NO;
    }
    if (![CryptCore check:dat2 and:buf length:16]) {
        NSLog(@"Test4: Check fail.");
        return NO;
    }
    return YES;
}

+ (BOOL) check:(unsigned char *)c1 and:(unsigned char *)c2 length:(size_t)len {
    for (size_t i = 0; i < len; i++) {
        if (c1[i] != c2[i]) return NO;
    }
    return YES;
}

+ (BOOL)encrypt:(unsigned char*)key keyLen:(size_t)keyLen 
        plain:(unsigned char*)dat dataLen:(size_t)dataLen 
          enc:(unsigned char*)enc {
    unsigned char inBuffer[8];
    unsigned char buffer[8];
    unsigned char *iv = (unsigned char*)[@"BLOWFISH" UTF8String];
    unsigned char zeroIv[] = {0, 0, 0, 0, 0, 0, 0, 0};
    
    unsigned char *vector = iv;
    
    CCCryptorRef ref;
    CCCryptorCreate(kCCEncrypt, kCCAlgorithmBlowfish, 0,
                    key, keyLen, NULL, &ref);
    
    size_t numCrypt;

    int j;
    int n = dataLen / 8;
    
    if (n == 1) {
        vector = zeroIv;
    }
    
    for (j = 0; j < n; j++) {
        for (int i = 0; i < 8; i++) {
            inBuffer[i] = dat[i+ j * 8] ^ vector[i];
        }
        CCCryptorReset(ref, NULL);
        CCCryptorUpdate(ref, inBuffer, 8, buffer, 8, &numCrypt);
        memcpy(enc + j * 8, buffer, 8);
        vector = buffer;
    }
    
    CCCryptorFinal(ref, buffer, 8, &numCrypt);
    CCCryptorRelease(ref);
    
    return YES;
}

+ (BOOL)decrypt:(unsigned char*)key keyLen:(size_t)keyLen
          crypt:(unsigned char*)crypt dataLen:(size_t)dataLen
          plain:(unsigned char*)plain {

    unsigned char buffer[8];
    unsigned char *iv = (unsigned char*)[@"BLOWFISH" UTF8String];    
    unsigned char *vector = iv;

    CCCryptorRef ref;
    CCCryptorCreate(kCCDecrypt, kCCAlgorithmBlowfish, 0, key, keyLen, NULL, &ref);
    
    size_t numCrypt;
    int j;
    int n = dataLen / 8;
    
    if (n == 1) {
        CCCryptorUpdate(ref, crypt, 8, plain, 8, &numCrypt);
        CCCryptorFinal(ref, buffer, 8, &numCrypt);
        CCCryptorRelease(ref);
        return YES;
    }

    for (j = 0; j < n; j++) {
        CCCryptorReset(ref, NULL);
        CCCryptorUpdate(ref, crypt + j * 8, 8, buffer, 8, &numCrypt);
        for (int i = 0; i < 8; i++) {
            buffer[i] = buffer[i] ^ vector[i];
        }        
        memcpy(plain + j * 8, buffer, 8);
        vector = crypt + j * 8;
    }
    
    CCCryptorFinal(ref, buffer, 8, &numCrypt);
    CCCryptorRelease(ref);
    
    return YES;
}
@end
