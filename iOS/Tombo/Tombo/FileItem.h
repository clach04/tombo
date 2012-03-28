// File/directory placeholder

#import <Foundation/Foundation.h>

@interface FileItem : NSObject

@property NSString *name;
@property NSString *path;
@property BOOL isDirectory;

-(NSString*)description;

+(id)allocWithName:(NSString *)name;

@end
