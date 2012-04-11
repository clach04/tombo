#import "EditCancelAlert.h"

@implementation EditCancelAlert {
    BOOL finished;
    BOOL isOK;
}

- (id)initWithDefault {
    return [self initWithTitle:@"Confirm" 
                       message:@"Note is modified. Are you sure to discard changes?" delegate:self 
             cancelButtonTitle:@"Cancel"
             otherButtonTitles:@"OK", nil];
}

- (BOOL)showAndWait {
    [self show];
    finished = NO;
    
    // Wait in runloop till a button is clicked.
    while (!finished) {
        [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode 
                                 beforeDate:[NSDate dateWithTimeIntervalSinceNow:300]];
    }
    return isOK;
}

- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex {
    finished = YES;
    
    if (buttonIndex == 0) {
        // Cancel
        isOK = NO;
    } else if (buttonIndex == 1) {
        isOK = YES;
    }
}


@end
