#import <UIKit/UIKit.h>
#import "FileItem.h"
#import "EditViewController.h"
@class DetailViewController;

@interface MasterViewController : UITableViewController <EditViewControllerDelegate>

@property (weak, nonatomic) DetailViewController *detailViewController;
@end
