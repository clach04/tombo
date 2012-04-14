#import "Storage.h"
#import "FileItem.h"
#import "CryptCore.h"

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
        
        if (!bDir) {
            if ([@"chi" isEqualToString:[f pathExtension]]) {
                item.isCrypt = YES;
            }
        }
        
        [result addObject:item];
    }
    return result;
}

- (NSArray *)listFolders {
    NSMutableArray *result = [NSMutableArray arrayWithCapacity:10];
    
    [result addObject:@"/"];
    [self listFoldersRec:result path:@"/"];
    NSComparator compr = ^(id a, id b) {
        NSString *strA = (NSString *)a;
        NSString *strB = (NSString *)b;
        return [strA compare:strB];
    };
    return [result sortedArrayUsingComparator:compr];
}

- (void)listFoldersRec:(NSMutableArray *)result path:(NSString *)path {
    NSError *error = nil;
    NSString *partPath = [documentRoot stringByAppendingString:path];
    NSArray *files = [fileManager contentsOfDirectoryAtPath:partPath
                                                      error:&error];
    
    for (NSString *f in files) {
        BOOL bDir = NO;
        NSString *p = [partPath stringByAppendingString:f];
        [fileManager fileExistsAtPath:p isDirectory:&bDir];
        if (!bDir) continue;
        NSString *pFolder = [path stringByAppendingString:f];
        [result addObject:pFolder];
        [self listFoldersRec:result path:[pFolder stringByAppendingString:@"/"]];
    }
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

- (void)saveDataWithBOM:(NSString *)note file:(NSString *)path {
    const char *noteBytes = [note cStringUsingEncoding:NSUTF8StringEncoding];
    int n = strlen(noteBytes);
    NSMutableData *data = [[NSMutableData alloc] initWithLength:n + 3];
    char *buf = (char *)[data mutableBytes];
    memcpy(buf + 3, noteBytes, n);
    *(char*)(buf + 0) = 0xEF;
    *(char*)(buf + 1) = 0xBB;
    *(char*)(buf + 2) = 0xBF;
    
    [data writeToFile:path atomically:YES];
}

- (NSString *)pickupTitle:(NSString *)note {
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
    return title;
}

- (FileItem *)savePlain:(NSString *)note item:(FileItem *)item {
    if (!item) return nil;

    NSString *title = [self pickupTitle:note];

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
    [self saveDataWithBOM:note file:result.path];
    
    return result;
}

- (FileItem *)saveCrypt:(NSString *)note item:(FileItem *)item password:(NSString *)password {
    NSString *title = [self pickupTitle:note];
    
    FileItem *result;
    if (!item.name || ![title isEqualToString: item.name]) {
        result = [self decideFileName:title path:item.path];
        
        if (item.name) {
            NSError *error = nil;
            [fileManager moveItemAtPath:item.path toPath:result.path error:&error];            
        }
    } else {
        result = item;
    }
    
    // Save note.
    const char *noteBytes = [note cStringUsingEncoding:NSUTF8StringEncoding];
    int n = strlen(noteBytes);
    NSMutableData *data = [[NSMutableData alloc] initWithLength:n + 3];
    char *buf = (char *)[data mutableBytes];
    memcpy(buf + 3, noteBytes, n);
    *(char*)(buf + 0) = 0xEF;
    *(char*)(buf + 1) = 0xBB;
    *(char*)(buf + 2) = 0xBF;

    NSError *error = nil;
    NSData *encData = [CryptCore encrypt:password data:data error:&error];
    if (error) return nil;
    
    if (![encData writeToFile:result.path atomically:YES]) return nil;
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

- (FileItem *)newFolder:(NSString *)folder {
    NSMutableString *path = [[NSMutableString alloc]initWithCapacity:256];
    [path appendString:documentRoot];
    [path appendString:currentDirectory];
    [path appendString:folder];
    NSError *error = nil;
    [fileManager createDirectoryAtPath:path withIntermediateDirectories:YES attributes:nil error:&error];
    
    FileItem *item = [FileItem allocWithName:folder];
    item.path = path;
    item.isDirectory = YES;
    return item;
}

+(NSString *)load:(NSString *)path {
    NSData *data = [NSData dataWithContentsOfFile:path];
    return [Storage trimBOM:data];
}

+(NSString *)trimBOM:(NSData *)data {
    const char *header = [data bytes];
    if ([data length] > 3 && 
        *header == 0xEF && *(header + 1) == 0xBB && *(header + 2) == 0xBF) {
        // BOM exists. UTF-8.
        NSString *note = [[NSString alloc] initWithBytes:[data bytes] + 3
                                                  length:[data length] - 3
                                                encoding:NSUTF8StringEncoding];
        return note;
    }

    NSString *note;
    
    if ([[[NSLocale currentLocale] localeIdentifier] isEqualToString:@"ja_JP"]) {
        note = [[NSString alloc] initWithBytes:[data bytes]
                                        length:[data length]
                                      encoding:NSShiftJISStringEncoding];
        if (note) return note;
    }

    // UTF-8
    note = [[NSString alloc] initWithBytes:[data bytes] 
                                    length:[data length] 
                                  encoding:NSUTF8StringEncoding];
    if (note) return note;

    // encode UTF-8 fail.
    return @"";
}

+ (NSString *)loadCryptFile:(NSString *)path password:(NSString *)password {
    NSData *encData = [NSData dataWithContentsOfFile:path];
    NSError *error = nil;
    NSData *plainData = [CryptCore decrypt:password data:encData error:&error];
    if (error) return nil;
    
    return [Storage trimBOM:plainData];
}

- (FileItem *)encrypt:(NSString *)key item:(FileItem*)item {
    NSData *plainData = [NSData dataWithContentsOfFile:item.path];
    NSError *error = nil;
    NSData *encData = [CryptCore encrypt:key data:plainData error:&error];
    if (error) return nil;

    NSMutableString *newPath = [[NSMutableString alloc] initWithCapacity:256];
    [newPath appendString:[item.path stringByDeletingPathExtension]];
    [newPath appendString:@".chi"];
    
    FileItem *newItem = [self decideFileName:item.name path:newPath];
    newItem.isCrypt = YES;
    
    if (![encData writeToFile:newItem.path atomically:YES]) {
        return nil;
    }
    [fileManager removeItemAtPath:item.path error:&error];
    return newItem;
}

- (FileItem *)decrypt:(NSString *)key item:(FileItem*)item {
    NSData *encData = [NSData dataWithContentsOfFile:item.path];
    NSError *error = nil;
    NSData *plainData = [CryptCore decrypt:key data:encData error:&error];
    if (error) return nil;
    
    NSMutableString *newPath = [[NSMutableString alloc] initWithCapacity:256];
    [newPath appendString:[item.path stringByDeletingPathExtension]];
    [newPath appendString:@".txt"];
    FileItem *newItem = [self decideFileName:item.name path:newPath];

    if (![plainData writeToFile:newItem.path atomically:YES]) {
        return nil;
    }
    [fileManager removeItemAtPath:item.path error:&error];
    return newItem;    
}

- (void)moveFrom:(FileItem *)from to:(FileItem *)to {
    NSString *name = [from.path lastPathComponent];
    NSMutableString *toPath = [[NSMutableString alloc]initWithCapacity:200];
    [toPath appendString:to.path];
    [toPath appendString:@"/"];
    [toPath appendString:name];
    
    NSError *error = nil;
    [fileManager moveItemAtPath:from.path toPath:toPath error:&error];
}

- (NSString *)moveFrom:(FileItem *)from toPath:(NSString *)to {
    NSString *name = [from.path lastPathComponent];
    NSMutableString *toPath = [[NSMutableString alloc]initWithCapacity:200];
    [toPath appendString:documentRoot];
    [toPath appendString:to];
    [toPath appendString:@"/"];
    [toPath appendString:name];
    
    NSError *error = nil;
    [fileManager moveItemAtPath:from.path toPath:toPath error:&error];
    return toPath;
}

@end
