#import <UIKit/UIKit.h>
#import "FileItem.h"

@class MasterViewController;
@class Storage;
@class PasswordManager;

@protocol DetailViewControllerDelegate;

/*
 * Right pane of split view.
 * Used on iPad only.
 */

@interface DetailViewController : UIViewController

@property (weak, nonatomic) FileItem *item;
@property (weak, nonatomic) Storage *storage;
@property (weak, nonatomic) MasterViewController *master;
@property (weak, nonatomic) PasswordManager *passwordManager;
@property (weak, nonatomic) id <DetailViewControllerDelegate> delegate;

@property (weak, nonatomic) IBOutlet UITextView *text;

@end

/*
 * Delegate protocol to notify encrypt/decrypt
 */
@protocol DetailViewControllerDelegate <NSObject>
-(void)detailViewFileItemChanged:(FileItem*)oldItem to:(FileItem *)newItem;
@end