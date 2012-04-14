#import <UIKit/UIKit.h>

@protocol MoveViewControllerDelegate;

@interface MoveViewController : UITableViewController

@property (weak,nonatomic) id <MoveViewControllerDelegate> delegate;
@property (strong, nonatomic) NSArray *folders;

- (IBAction)cancel:(id)sender;

@end

@protocol MoveViewControllerDelegate <NSObject>

- (void)moveViewControllerCancel:(MoveViewController *)view;
- (void)moveViewControllerSelect:(MoveViewController *)view path:(NSString *)path;
@end
