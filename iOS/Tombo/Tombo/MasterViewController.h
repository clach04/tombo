#import <UIKit/UIKit.h>
#import "FileItem.h"
#import "EditViewController.h"
#import "DetailViewController.h"

@interface MasterViewController : UITableViewController <EditViewControllerDelegate, DetailViewControllerDelegate>

@property (weak, nonatomic) DetailViewController *detailViewController;
@end
