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

/*
 * Enumerate current directory and return array of FileItem.
 */
-(NSArray*)listItems;

/*
 * Change directory.
 */
-(void)chdir:(NSString *)subdir;

-(void)updir;

/*
 * Is current directory Top?
 */
-(BOOL)isTopDir;
@end
