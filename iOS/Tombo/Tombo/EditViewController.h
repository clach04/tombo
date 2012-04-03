#import <UIKit/UIKit.h>
#import "FileItem.h"
#import "Storage.h"

@protocol EditViewControllerDelegate;

@interface EditViewController : UIViewController <UITextViewDelegate>

@property (weak, nonatomic) id <EditViewControllerDelegate> delegate;
@property (strong, nonatomic) FileItem *detailItem;
@property (nonatomic) BOOL isModify;

@property (weak, nonatomic) IBOutlet UITextView *detailText;

- (IBAction)done:(id)sender;
- (IBAction)cancel:(id)sender;

@end

@protocol EditViewControllerDelegate <NSObject>
- (void)editViewControllerDidCancel:(EditViewController*)controller;
- (void)editViewControllerDidFinish:(EditViewController *)controller;
@end