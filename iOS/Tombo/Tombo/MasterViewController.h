#import <UIKit/UIKit.h>

@class DetailViewController;

@interface MasterViewController : UITableViewController <UINavigationControllerDelegate>

@property (strong, nonatomic) DetailViewController *detailViewController;

@end
