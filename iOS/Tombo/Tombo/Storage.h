/*
 Data Storage
 Abstraction of file and directory.
 */
#import <Foundation/Foundation.h>

@interface Storage : NSObject

@property NSString *currentDirectory;

+ (id)init;

-(NSArray*)listItems;

@end
