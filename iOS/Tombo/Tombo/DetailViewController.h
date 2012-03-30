#import <UIKit/UIKit.h>
#import "FileItem.h"

@interface DetailViewController : UIViewController <UISplitViewControllerDelegate>

@property (strong, nonatomic) FileItem *detailItem;

@property (weak, nonatomic) IBOutlet UITextView *detailText;

@end
