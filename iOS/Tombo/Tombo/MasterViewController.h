#import <UIKit/UIKit.h>
#import "FileItem.h"

@class DetailViewController;

@interface MasterViewController : UITableViewController

@property (strong, nonatomic) DetailViewController *detailViewController;

// Call back handler for detail view.
- (void)itemChanged:(FileItem *)from to:(FileItem *)to;
- (void)itemAdded: (FileItem *)item;
@end
