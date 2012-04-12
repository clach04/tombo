#import "PasswordManager.h"
#import "SinglePasswordDialog.h"

@implementation PasswordManager {
    NSTimer *timer;
}

@synthesize password = _password;

- (id)init {
    id obj = [super init];
    self.password = nil;
    return obj;
}

- (BOOL)preparePassword {
    if (!self.password) {
        SinglePasswordDialog *dialog = [[SinglePasswordDialog alloc] initWithTitle:@"Password" 
                                                                           message:@"Please input password"];
        NSString *pass = [dialog showAndWait];
        if (pass == nil) return NO;
        
        self.password = pass;
    }

    [self resetTimer];
    return YES;
}

- (BOOL)preparePasswordConfirm {
    if (!self.password) {
        SinglePasswordDialog *dialog = [[SinglePasswordDialog alloc] initWithTitle:@"Password" 
                                                                           message:@"Please input password"];
        NSString *pass1 = [dialog showAndWait];
        if (pass1 == nil) return NO;
        
        dialog = [[SinglePasswordDialog alloc] initWithTitle:@"Confirm" message:@"Input password again"];
        NSString *pass2 = [dialog showAndWait];
        if (pass2 == nil) return NO;
        
        if (![pass1 isEqualToString:pass2]) {
            UIAlertView *mismatch = [[UIAlertView alloc] initWithTitle:@"Warn"
                                                               message:@"Password mismatch."
                                                              delegate:nil cancelButtonTitle:@"OK"
                                                     otherButtonTitles:nil];
            [mismatch show];
            return NO;
        }
    }

    [self resetTimer];

    return YES;
}

- (void)resetTimer {
    if (timer) [timer invalidate];
    timer = [NSTimer scheduledTimerWithTimeInterval:60
                                             target:self 
                                           selector:@selector(fireTimer:) 
                                           userInfo:nil 
                                            repeats:NO];
}

- (void)fireTimer:(NSTimer *)timer {
    self.password = nil;
}
@end
