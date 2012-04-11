#import "PasswordManager.h"
#import "SinglePasswordDialog.h"

@implementation PasswordManager

@synthesize password = _password;

- (BOOL)preparePassword {
    SinglePasswordDialog *dialog = [[SinglePasswordDialog alloc] initWithDefault];
    NSString *pass = [dialog showAndWait];
    if (pass == nil) return NO;
    
    self.password = pass;
    return YES;
}

- (BOOL)preparePasswordConfirm {
    //TOOD: implement
    return [self preparePassword];
}
@end
