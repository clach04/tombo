#import "Storage.h"

@implementation Storage

@synthesize currentDirectory;

+(id)init {
    Storage *storage = [Storage alloc];
    storage.currentDirectory = @"";
    return storage;
}

-(NSArray*)listItems {
    return [NSArray arrayWithObjects:@"abc", @"def", @"ghi", @"jkl", @"mno", @"pqr", nil];
}

@end
