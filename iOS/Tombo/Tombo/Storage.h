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

/*
 * Enumerate top directory recursively and return array of NSString.
 */
- (NSArray *)listFolders;


-(FileItem*)newItem;

/*
 * Save note to file.
 * 
 * Returns new FileItem. This may be path/name is changed if note's title is changed.
 * If path is not changed, returns item itself.
 */
-(FileItem *)savePlain:(NSString *)note item:(FileItem *)item;
-(FileItem *)saveCrypt:(NSString *)note item:(FileItem *)item password:(NSString *)password;

- (void)deleteItem:(FileItem*)item;

- (FileItem *)newFolder:(NSString *)folder;

/*
 * Load note.
 */
+ (NSString *)load:(NSString *)path;
+ (NSString *)loadCryptFile:(NSString *)path password:(NSString *)password;

- (FileItem *)encrypt:(NSString *)key item:(FileItem*)item;
- (FileItem *)decrypt:(NSString *)key item:(FileItem*)item;

- (void)moveFrom:(FileItem *)from to:(FileItem *)to;
- (NSString *)moveFrom:(FileItem *)from toPath:(NSString *)to;

@end
