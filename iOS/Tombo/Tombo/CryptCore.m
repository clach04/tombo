#import <CommonCrypto/CommonDigest.h>
#import <CommonCrypto/CommonCryptor.h>
#import "CryptCore.h"

@implementation CryptCore

// digest size should be CC_MD5_DIGEST_LENGTH
+ (void)md5:(NSString *)input buffer:(unsigned char*)digest {
    const char *cStr = [input UTF8String];
    [CryptCore md5WithByte:(unsigned char *)cStr length:strlen(cStr) buffer:digest];
}

+ (void)md5WithByte:(const void *)input length:(NSUInteger)len buffer:(unsigned char *)digest {
    CC_MD5(input, len, digest);
}

+ (NSString *)bytes2hex:(unsigned char *)data length:(NSUInteger) len {
    NSMutableString *out = [NSMutableString stringWithCapacity:len];
    for (int i = 0; i < len; i++) {
        [out appendFormat:@"%02x", data[i]];
    }
    return out;
}

+ (BOOL)selftest {
    NSError *err = nil;
	unsigned char key1[] = {0x83, 0x45, 0x92, 0x17, 0x68, 0xA9, 0x35, 0xDF,
                           0x25, 0x58, 0x9F, 0x11, 0x72, 0x87, 0x99, 0xFE};
	unsigned char dat1[] = {0x85, 0x28, 0xF9, 0x43, 0xE8, 0xC2, 0x12, 0x50};
    
    unsigned char enc1[] = {0x87, 0x66, 0x8b, 0xc8, 0xeb, 0xfc, 0xe6, 0xde};

    unsigned char buf[16];
    if (![CryptCore encrypt:key1 keyLen:16 plain:dat1 dataLen:8 enc:buf error:&err]) {
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
    if (![CryptCore encrypt:key1 keyLen:16 plain:dat2 dataLen:16 enc:buf error:&err]) {
        NSLog(@"Test2: Encrypt fail.");
        return NO;
    }
    if (![CryptCore check:enc2 and:buf length:16]) {
        NSLog(@"Test2: Check fail.");
        return NO;
    }
    NSError *error = nil;
    if (![CryptCore decrypt:key1 keyLen:16 crypt:enc1 dataLen:8 plain:buf error:&error]) {
        NSLog(@"Test3: Decrypt fail.");
        return NO;
    }
    if (![CryptCore check:dat1 and:buf length:8]) {
        NSLog(@"Test3: Check fail.");
        return NO;
    }
    
    if (![CryptCore decrypt:key1 keyLen:16 crypt:enc2 dataLen:16 plain:buf error:&error]) {
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
          enc:(unsigned char*)enc
          error:(NSError**)error {
    unsigned char inBuffer[8];
    unsigned char buffer[8];
    unsigned char *iv = (unsigned char*)[@"BLOWFISH" UTF8String];
    unsigned char zeroIv[] = {0, 0, 0, 0, 0, 0, 0, 0};
    
    unsigned char *vector = iv;
    
    CCCryptorStatus status;
    CCCryptorRef ref;
    status = CCCryptorCreate(kCCEncrypt, kCCAlgorithmBlowfish, 0,
                    key, keyLen, NULL, &ref);
    if (status != kCCSuccess) {
        *error = [[NSError alloc] initWithDomain:@"TomboCrypt" code:status userInfo:nil];
        return NO;
    }
    
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
        status = CCCryptorReset(ref, NULL);
        if (status != kCCSuccess) {
            *error = [[NSError alloc] initWithDomain:@"TomboCrypt" code:status userInfo:nil];
            CCCryptorRelease(ref);
            return NO;
        }
        status = CCCryptorUpdate(ref, inBuffer, 8, buffer, 8, &numCrypt);
        if (status != kCCSuccess) {
            *error = [[NSError alloc] initWithDomain:@"TomboCrypt" code:status userInfo:nil];
            CCCryptorRelease(ref);
            return NO;
        }
        memcpy(enc + j * 8, buffer, 8);
        vector = buffer;
    }
    
    status = CCCryptorFinal(ref, buffer, 8, &numCrypt);
    if (status != kCCSuccess) {
        *error = [[NSError alloc] initWithDomain:@"TomboCrypt" code:status userInfo:nil];
        CCCryptorRelease(ref);
        return NO;
    }
    CCCryptorRelease(ref);
    
    return YES;
}

+ (BOOL)decrypt:(unsigned char*)key keyLen:(size_t)keyLen
          crypt:(unsigned char*)crypt dataLen:(size_t)dataLen
          plain:(unsigned char*)plain
          error:(NSError**)error {

    unsigned char buffer[8];
    unsigned char *iv = (unsigned char*)[@"BLOWFISH" UTF8String];    
    unsigned char *vector = iv;

    CCCryptorStatus status;
    CCCryptorRef ref;
    status = CCCryptorCreate(kCCDecrypt, kCCAlgorithmBlowfish, 0, key, keyLen, NULL, &ref);
    if (status != kCCSuccess) {
        *error = [[NSError alloc] initWithDomain:@"TomboCrypt" code:status userInfo:nil];
        return NO;
    }
   
    size_t numCrypt;
    int j;
    int n = dataLen / 8;
    
    if (n == 1) {
        status = CCCryptorUpdate(ref, crypt, 8, plain, 8, &numCrypt);
        if (status != kCCSuccess) {
            *error = [[NSError alloc] initWithDomain:@"TomboCrypt" code:status userInfo:nil];
            CCCryptorRelease(ref);
            return NO;
        }
        status = CCCryptorFinal(ref, buffer, 8, &numCrypt);
        if (status != kCCSuccess) {
            *error = [[NSError alloc] initWithDomain:@"TomboCrypt" code:status userInfo:nil];
            CCCryptorRelease(ref);
            return NO;
        }
        status = CCCryptorRelease(ref);
        if (status != kCCSuccess) {
            *error = [[NSError alloc] initWithDomain:@"TomboCrypt" code:status userInfo:nil];
            CCCryptorRelease(ref);
            return NO;
        }
        return YES;
    }

    for (j = 0; j < n; j++) {
        status = CCCryptorReset(ref, NULL);
        if (status != kCCSuccess) {
            *error = [[NSError alloc] initWithDomain:@"TomboCrypt" code:status userInfo:nil];
            CCCryptorRelease(ref);
            return NO;
        }
        status = CCCryptorUpdate(ref, crypt + j * 8, 8, buffer, 8, &numCrypt);
        if (status != kCCSuccess) {
            *error = [[NSError alloc] initWithDomain:@"TomboCrypt" code:status userInfo:nil];
            CCCryptorRelease(ref);
            return NO;
        }
        for (int i = 0; i < 8; i++) {
            buffer[i] = buffer[i] ^ vector[i];
        }        
        memcpy(plain + j * 8, buffer, 8);
        vector = crypt + j * 8;
    }
    
    status = CCCryptorFinal(ref, buffer, 8, &numCrypt);
    if (status != kCCSuccess) {
        *error = [[NSError alloc] initWithDomain:@"TomboCrypt" code:status userInfo:nil];
        CCCryptorRelease(ref);
        return NO;
    }
    CCCryptorRelease(ref);
    
    return YES;
}

// The format of the container is:
// 0-3  : BF01(4 bytes)
// 4-7  : data length (exclude random area + md5sum)(4 bytes)
// 8-15 :* random data(8 bytes)
//16-31 :* md5sum of plain text(16 bytes)
//32-   :* data
// '*' is encrypted.

+ (NSData *)encrypt:(NSString *)key data:(NSData *)plain error:(NSError **)error {
    const char *plainByte = (const char *)[plain bytes];
    NSUInteger n = strlen(plainByte);
    NSUInteger nBlocks = (n + 7) / 8;
    
    NSUInteger dataLen = 32 + nBlocks * 8;
    
    NSMutableData *pre = [[NSMutableData alloc] initWithLength:dataLen];
    unsigned char *preBuf = (unsigned char *)[pre mutableBytes];
    // fill random data
    for (int i = 0; i < 8; i++) {
        preBuf[8 + i] = rand() % 256;
    }
    // fill md5sum
    [CryptCore md5WithByte:(unsigned char *)plainByte length:n buffer:preBuf + 16];
    
    // copy data
    memcpy(preBuf + 32, plainByte, n);
    
    // key
    unsigned char keymd5[16];
    [CryptCore md5:key buffer:keymd5];
    
    // output buffer
    NSMutableData *result = [[NSMutableData alloc] initWithLength:dataLen];
    unsigned char *resultBuf = (unsigned char *)[result mutableBytes];
    
    // encrypt
    NSError *err = nil;
    if (![CryptCore encrypt:keymd5 keyLen:16 
                 plain:(unsigned char*)preBuf + 8 dataLen:nBlocks * 8 + 24 
                   enc:resultBuf + 8
                      error:&err]) {
        *error = err;
        return nil;
    }
    
    // header
    memcpy(resultBuf, "BF01", 4);
    *(int *)(resultBuf + 4) = n;
    
    return result;
}

+ (NSData *)decrypt:(NSString *)key data:(NSData *)crypt error:(NSError **)error{
    
    // Key
    unsigned char keymd5[16];
    [CryptCore md5:key buffer:keymd5];
    
    NSUInteger dataLen = [crypt length] - 8;
    unsigned char *cryptBuf = (unsigned char *)[crypt bytes];
    
    NSMutableData *outBuf = [[NSMutableData alloc] initWithLength:dataLen];

    NSError *err = nil;
    if (![CryptCore decrypt:keymd5 keyLen:16
                 crypt:(unsigned char *)(cryptBuf + 8) dataLen:dataLen 
                      plain:(unsigned char *)[outBuf mutableBytes] error:&err]) {
        *error = err;
        return nil;
    }
    
    NSUInteger bodySize = *(int*)(cryptBuf + 4);

    NSMutableData *plainBytes = [[NSMutableData alloc] initWithLength:bodySize];
    memcpy([plainBytes mutableBytes], [outBuf bytes] + 24, bodySize);
    
    //check md5
    unsigned char decryptMd5[16];
    [CryptCore md5WithByte:(const void*)[plainBytes mutableBytes] 
                    length:bodySize 
                    buffer:(void *)decryptMd5];
    if (memcmp([outBuf bytes] + 8, decryptMd5, 16) != 0) {
        NSError *err = [[NSError alloc] initWithDomain:@"TomboCryptFormat" 
                                                  code:tcfMD5Mismatch
                                              userInfo:nil];
        *error = err;
        return nil;
    }
    
    return plainBytes;
}
@end
