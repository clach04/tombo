// File/directory placeholder

#import <Foundation/Foundation.h>

@interface FileItem : NSObject

@property NSString *name;
@property NSString *path;

@property NSString *body;
@property BOOL isDirectory;
@property BOOL isUp;
@property BOOL isCrypt;

-(NSString*)description;

+(id)allocWithName:(NSString *)name;

- (BOOL)isNewItem;

@end
