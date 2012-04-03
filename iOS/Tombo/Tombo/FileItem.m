#import "FileItem.h"

@implementation FileItem

@synthesize name;
@synthesize path;
@synthesize isDirectory;
@synthesize isUp;

-(NSString*)description {
    return name;
}

+(id)allocWithName:(NSString *)name {
    FileItem *fileItem = [FileItem alloc];
    fileItem.name = name;
    fileItem.isDirectory = NO;
    fileItem.isUp = NO;
    return fileItem;
}

- (BOOL)isNewItem {
    return self.name == nil;
}
@end
