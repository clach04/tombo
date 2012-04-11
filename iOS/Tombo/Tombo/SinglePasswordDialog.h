#import <UIKit/UIKit.h>

@interface SinglePasswordDialog : UIAlertView <UIAlertViewDelegate>

@property (strong,nonatomic) NSString *password;

- (id)initWithDefault;

- (NSString *)showAndWait;
@end
