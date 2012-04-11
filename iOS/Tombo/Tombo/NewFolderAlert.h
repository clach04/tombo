#import <UIKit/UIKit.h>

@interface NewFolderAlert : UIAlertView <UIAlertViewDelegate>

@property (strong,nonatomic) NSString *folderName;

- (id)initWithDefault;

- (NSString *)showAndWait;
@end
