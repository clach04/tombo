#import <Foundation/Foundation.h>

enum TomboCryptFormatErrorCode {
    tcfMD5Mismatch = 1,
    tcfEncodingError = 2,
};

@interface CryptCore : NSObject

+ (void)md5:(NSString *)input buffer:(unsigned char*)digest;

+ (NSString *)bytes2hex:(unsigned char *)data length:(NSUInteger) len;

+ (BOOL)selftest;

+ (NSData *)encrypt:(NSString *)key data:(NSData *)plain error:(NSError**)error;
+ (NSData *)decrypt:(NSString *)key data:(NSData *)crypt error:(NSError**)error;

@end
