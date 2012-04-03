#import <UIKit/UIKit.h>
#import "FileItem.h"
#import "EditViewController.h"
@class DetailViewController;

@interface MasterViewController : UITableViewController <EditViewControllerDelegate>

@property (strong, nonatomic) DetailViewController *detailViewController;
@end
