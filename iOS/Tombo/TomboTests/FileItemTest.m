#import "FileItemTest.h"
#import "FileItem.h"

@implementation FileItemTest

- (void)testCompare1 {
    FileItem *f1 = [FileItem allocWithName:@"abc"];
    FileItem *f2 = [FileItem allocWithName:@"def"];
    
    NSComparisonResult r = [f1 compare:f2];
    STAssertTrue(r == NSOrderedAscending, @"A1");
    
    r = [f1 compare:f1];
    STAssertTrue(r == NSOrderedSame, @"A2");

    r = [f2 compare:f1];
    STAssertTrue(r == NSOrderedDescending, @"A3");    
}

- (void)testCompare2 {
    FileItem *f1 = [FileItem allocWithName:@"abc"];
    FileItem *f2 = [FileItem allocWithName:@"def"];
    f2.isDirectory = YES;
    
    NSComparisonResult r = [f1 compare:f2];
    STAssertTrue(r == NSOrderedDescending, @"A4");
    
    r = [f2 compare:f1];
    STAssertTrue(r == NSOrderedAscending, @"A5");
}

- (void)testCompare3 {
    FileItem *f1 = [FileItem allocWithName:@"abc"];
    FileItem *f2 = [FileItem allocWithName:@"def"];
    f2.isUp = YES;
    
    NSComparisonResult r = [f1 compare:f2];
    STAssertTrue(r == NSOrderedDescending, @"A6");

    r = [f2 compare:f1];
    STAssertTrue(r == NSOrderedAscending, @"A7");
    
}
@end
