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

-(void)chdir:(NSString *)subdir {
    NSString *newCurrent = [currentDirectory stringByAppendingPathComponent:subdir];
    self.currentDirectory = [newCurrent stringByAppendingString:@"/"];
}

-(void)updir {
    NSString *parent = [currentDirectory stringByDeletingLastPathComponent];
    if ([parent isEqualToString:@"/"]) {
        self.currentDirectory = parent;
    } else {
        self.currentDirectory = [parent stringByAppendingString:@"/"];        
    }
}

-(BOOL)isTopDir {
    return [currentDirectory isEqualToString:@"/"];
}

// save note
-(FileItem *)save:(NSString *)note item:(FileItem *)item {
    if (!item) return nil;

    // Decide new title.
    NSRange r;
    r.location = 0;
    r.length = 0;
    NSRange titleRange = [note lineRangeForRange:r];
    NSString *title = [note substringWithRange:titleRange];
    if ([title characterAtIndex:(title.length - 1)] == '\n') {
        title = [title substringToIndex:(title.length - 1)];
    }
    if (title.length == 0) {
        title = @"New document";
    }

    // If title is changed, rename one.
    FileItem *result;    
    if (!item.name || ![title isEqualToString: item.name]) {        
        result = [self decideFileName:title path:item.path];
        
        if (item.name) {
            // If it is not new item, needs rename.
            NSError *error = nil;
            [fileManager moveItemAtPath:item.path toPath:result.path error:&error];
        }
    } else {
        result = item;
    }

    // Save note.
    NSError *error = nil;
    [note writeToFile:result.path atomically:YES encoding:NSUTF8StringEncoding error:&error];
    
    return result;
}
// Remove characters which can't use file name from given string.
- (NSString *)removeInvalidFilenameChars:(NSString *)src {
    NSString *result = src;
    // chars are same as Tombo for Windows.
    NSArray *chars = [[NSArray alloc] initWithObjects:@"\\", @"/", @":", @",", @";", @"*", @"?", @"<", @">", @"\"", @"\t", nil];
    
    for (NSString *t in chars) {
        result = [result stringByReplacingOccurrencesOfString:t withString:@""];
    }
    return result;
}

- (FileItem *)decideFileName:(NSString *)titleCand path:(NSString *)origPath {
    FileItem *result = [FileItem alloc];

    NSString *ext = [origPath pathExtension];
    
    NSMutableString *path = [NSMutableString stringWithCapacity:256];
    [path appendString:[origPath stringByDeletingLastPathComponent]];
    [path appendString:@"/"];
    [path appendString:[self removeInvalidFilenameChars:titleCand]];
    NSUInteger n = [path length];
    [path appendString:@"."];
    [path appendString:ext];
    NSUInteger u = [path length];
    
    if ([fileManager fileExistsAtPath:path]) {
        NSUInteger cnt = 1;
        while(YES) {
            NSRange r;
            r.location = n;
            r.length = u - n;
            [path deleteCharactersInRange:r];
            [path appendFormat:@"(%d)", cnt];
            [path appendString:@"."];
            [path appendString:ext];
            u = [path length];
            
            if (![fileManager fileExistsAtPath:path]) break;
            cnt++;
        }
        
        result.name = [[path lastPathComponent] stringByDeletingPathExtension];
    } else {
        result.name = titleCand;
    }
    result.path = path;            

    return result;
}

- (FileItem *)newItem {
    FileItem *p = [FileItem allocWithName: nil];
    p.path = [documentRoot stringByAppendingString:@"/_dummy.txt"];
    return p;
}

- (void)deleteItem:(FileItem*)item {
    [fileManager removeItemAtPath:item.path error:nil];
}

@end
