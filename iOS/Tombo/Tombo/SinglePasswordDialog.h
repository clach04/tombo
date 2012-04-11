#import <UIKit/UIKit.h>

@interface SinglePasswordDialog : UIAlertView <UIAlertViewDelegate>

@property (strong,nonatomic) NSString *password;

- (id)initWithTitle:(NSString *)title message:(NSString *)message;

- (NSString *)showAndWait;
@end
