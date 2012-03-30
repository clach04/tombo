#import <UIKit/UIKit.h>
#import "FileItem.h"
#import "Storage.h"

@interface DetailViewController : UIViewController <UISplitViewControllerDelegate,UITextViewDelegate>

@property (strong, nonatomic) FileItem *detailItem;
@property (strong, nonatomic) Storage *storage;

@property (weak, nonatomic) IBOutlet UITextView *detailText;

@end
