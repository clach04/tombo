#import "PasswordManager.h"
#import "SinglePasswordDialog.h"

@implementation PasswordManager

@synthesize password = _password;

- (BOOL)preparePassword {
    SinglePasswordDialog *dialog = [[SinglePasswordDialog alloc] initWithTitle:@"Password" 
                                                                       message:@"Please input password"];
    NSString *pass = [dialog showAndWait];
    if (pass == nil) return NO;
    
    self.password = pass;
    return YES;
}

- (BOOL)preparePasswordConfirm {
    SinglePasswordDialog *dialog = [[SinglePasswordDialog alloc] initWithTitle:@"Password" 
                                                                       message:@"Please input password"];
    NSString *pass1 = [dialog showAndWait];
    if (pass1 == nil) return NO;

    dialog = [[SinglePasswordDialog alloc] initWithTitle:@"Confirm" message:@"Input password again"];
    NSString *pass2 = [dialog showAndWait];
    if (pass2 == nil) return NO;

    if (![pass1 isEqualToString:pass2]) {
        UIAlertView *mismatch = [[UIAlertView alloc] initWithTitle:@"Warn" message:@"Password mismatch." delegate:nil cancelButtonTitle:@"OK" otherButtonTitles:nil];
        [mismatch show];
        return NO;
    }
    return YES;
}
@end
