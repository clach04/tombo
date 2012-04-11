#import "FileItem.h"

@implementation FileItem

@synthesize name;
@synthesize path;
@synthesize isDirectory;
@synthesize isUp;
@synthesize isCrypt;
@synthesize body;

-(NSString*)description {
    return name;
}

+(id)allocWithName:(NSString *)name {
    FileItem *fileItem = [FileItem alloc];
    fileItem.name = name;
    fileItem.isDirectory = NO;
    fileItem.isUp = NO;
    fileItem.isCrypt = NO;
    fileItem.body = nil;
    return fileItem;
}

- (BOOL)isNewItem {
    return self.name == nil;
}
@end
