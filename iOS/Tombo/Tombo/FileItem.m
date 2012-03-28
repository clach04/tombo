#import "FileItem.h"

@implementation FileItem

@synthesize name;
@synthesize path;
@synthesize isDirectory;

-(NSString*)description {
    return name;
}

+(id)allocWithName:(NSString *)name {
    FileItem *fileItem = [FileItem alloc];
    fileItem.name = name;
    fileItem.isDirectory = NO;
    return fileItem;
}

@end