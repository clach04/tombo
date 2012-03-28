/*
 Data Storage
 Abstraction of file and directory.
 */
#import <Foundation/Foundation.h>

@interface Storage : NSObject

@property NSString *documentRoot;
@property NSString *currentDirectory;
@property NSFileManager *fileManager;

+ (id)init;

-(NSArray*)listItems;

@end
