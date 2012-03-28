#import "Storage.h"
#import "FileItem.h"

@implementation Storage

@synthesize currentDirectory;
@synthesize documentRoot;
@synthesize fileManager;

+(id)init {
    Storage *storage = [Storage alloc];
    storage.currentDirectory = @"/";

    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    storage.documentRoot = [paths objectAtIndex:0];

    storage.fileManager = [NSFileManager defaultManager];

    return storage;
}

/*
 * Enumerate current directory and return array of FileItem.
 */
-(NSArray*)listItems {
    NSError *error = nil;
    NSString *currentPath = [documentRoot stringByAppendingString: currentDirectory];
    NSArray *files = [fileManager contentsOfDirectoryAtPath: currentPath
                                                      error: &error];
    if (error) {
        return [NSArray alloc];
    }
    
    NSMutableArray *result = [NSMutableArray arrayWithCapacity: files.count];
    for (NSString *f in files) {
        FileItem *item = [FileItem alloc];
        item.path = [currentPath stringByAppendingString: f];
        item.name = [f stringByDeletingPathExtension];
        BOOL bDir = NO;
        [fileManager fileExistsAtPath:item.path isDirectory:&bDir];
        item.isDirectory = bDir;
        [result addObject:item];
    }
    return result;
}

@end
