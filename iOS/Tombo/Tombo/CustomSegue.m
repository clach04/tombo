#import "CustomSegue.h"

@implementation CustomSegue

@synthesize isStop;

- (void)perform {
    if (self.isStop) return;
    
    UINavigationController *ctrl = [self.sourceViewController navigationController];
    [ctrl pushViewController:self.destinationViewController 
                    animated:YES];
}
@end
