#import "NewFolderAlert.h"

@implementation NewFolderAlert {
    BOOL finished;
}

@synthesize folderName=_folderName;

- (id)initWithDefault {
    return [self initWithTitle:@"New folder" 
                       message:@"Input folder name"
                      delegate:self
             cancelButtonTitle:@"Cancel"
             otherButtonTitles:@"Done", nil];
}

- (NSString *)showAndWait {
    self.alertViewStyle = UIAlertViewStylePlainTextInput;
    [self show];
    finished = NO;
    
    // Wait in runloop till a button is clicked.
    while (!finished) {
        [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode 
                                 beforeDate:[NSDate dateWithTimeIntervalSinceNow:300]];
    }
    return self.folderName;
}

- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex {
    finished = YES;
    
    UITextField *field = [self textFieldAtIndex:0];
    if (buttonIndex == 0) {
        // Cancel
        self.folderName = nil;
    } else if (buttonIndex == 1) {
        // OK
        NSString *fname = [field.text stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
        if (fname.length == 0) {
            self.folderName = nil;
        } else {
            self.folderName = fname;
        }
    }
    [field resignFirstResponder];
}

@end
