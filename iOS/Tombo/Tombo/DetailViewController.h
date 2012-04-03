#import <UIKit/UIKit.h>
#import "FileItem.h"

@class MasterViewController;
/*
 * Right pane of split view.
 * Used on iPad only.
 */

@interface DetailViewController : UIViewController

@property (strong, nonatomic) FileItem *item;
@property (weak, nonatomic) MasterViewController *master;

@property (weak, nonatomic) IBOutlet UITextView *text;

@end
