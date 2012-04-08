#import <Foundation/Foundation.h>

@interface CryptCore : NSObject

+ (void)md5:(NSString *)input buffer:(unsigned char*)digest;

+ (NSString *)bytes2hex:(unsigned char *)data length:(NSUInteger) len;

+ (BOOL)selftest;
@end
