#import <UIKit/UIKit.h>

@interface EditCancelAlert : UIAlertView <UIAlertViewDelegate>

- (id)initWithDefault;

- (BOOL)showAndWait;

@end
