#import "SinglePasswordDialog.h"

@implementation SinglePasswordDialog {
    BOOL finished;
}

@synthesize password=_password;

- (id)initWithDefault {
    return [self initWithTitle:@"Password" message:@"Please input password" delegate:self
             cancelButtonTitle:@"Cancel" otherButtonTitles:@"OK", nil];
}

- (NSString *)showAndWait {
    self.alertViewStyle = UIAlertViewStyleSecureTextInput;
    [self show];
    finished = NO;
    
    // Wait in runloop till a button is clicked.
    while (!finished) {
        [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode 
                                 beforeDate:[NSDate dateWithTimeIntervalSinceNow:300]];
    }
    return self.password;
}

#pragma mark - UIAlertViewDelegate

- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex {
    finished = YES;
    if (buttonIndex == 0) {
        // Cancel
        self.password = nil;
    } else if (buttonIndex == 1) {
        // OK
        UITextField *field = [self textFieldAtIndex:0];
        NSString *pw = [field.text stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
        if (pw.length == 0) {
            self.password = nil;
        } else {
            self.password = pw;
        }
    }
}

@end
