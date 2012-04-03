// File/directory placeholder

#import <Foundation/Foundation.h>

@interface FileItem : NSObject

@property NSString *name;
@property NSString *path;
@property BOOL isDirectory;
@property BOOL isUp;

-(NSString*)description;

+(id)allocWithName:(NSString *)name;

- (BOOL)isNewItem;

@end
