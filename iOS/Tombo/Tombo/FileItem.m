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

- (NSComparisonResult)compare:(FileItem*)other {
    if (self.isUp) return NSOrderedAscending;
    if (other.isUp) return NSOrderedDescending;
    
    if (self.isDirectory) {
        if (other.isDirectory) {
            return [self.name compare:other.name];
        } else {
            return NSOrderedAscending;
        }
    } else {
        if (other.isDirectory) {
            return NSOrderedDescending;
        } else {
            return [self.name compare:other.name];            
        }
    }
}

@end
