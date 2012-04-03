/*
 Data Storage
 Abstraction of file and directory.
 */
#import <Foundation/Foundation.h>
#import "FileItem.h"

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


-(FileItem*)newItem;

/*
 * Save note to file.
 * 
 * Returns new FileItem. This may be path/name is changed if note's title is changed.
 * If path is not changed, returns item itself.
 */
-(FileItem *)save:(NSString *)note item:(FileItem *)item;

- (void)deleteItem:(FileItem*)item;

- (FileItem *)newFolder:(NSString *)folder;

@end
