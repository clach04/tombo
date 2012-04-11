#import "EditViewController.h"
#import "MasterViewController.h"

@interface EditViewController () {
}
@end

@implementation EditViewController

@synthesize detailItem = _detailItem;
@synthesize detailText = _detalText;
@synthesize delegate = _delegate;
@synthesize isModify = _isModify;

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
    }
    return self;
}

- (void)setDetailItem:(id)newDetailItem
{
    if (_detailItem != newDetailItem) {
        // set item
        _detailItem = newDetailItem;
        
        // Update the view.
        [self configureView];
    }
}

- (void)configureView {
    NSString *noteData;
    if (self.detailItem && self.detailItem.path) {
        noteData = self.detailItem.body;
    } else {
        noteData = @"";
    }
    self.detailText.text = noteData;
}


- (void)viewDidLoad {
    [super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.
    NSNotificationCenter *notify = [NSNotificationCenter defaultCenter];
    [notify addObserver:self 
               selector:@selector(keyboardDidShow:) 
                   name:UIKeyboardDidShowNotification 
                 object:nil];
    
    [notify addObserver:self
               selector:@selector(keyboardDidHide:)
                   name:UIKeyboardDidHideNotification
                 object:nil];
    
    self.detailText.delegate = self;
    [self configureView];
    self.isModify = NO;
}

- (void)viewDidUnload {
    [self setDetailText:nil];
    self.detailItem = nil;
    [super viewDidUnload];    
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPhone) {
        return (interfaceOrientation != UIInterfaceOrientationPortraitUpsideDown);
    } else {
        return YES;
    }
}
- (void)textViewDidChange:(UITextView *)textView {
    self.isModify = YES;
}

#pragma mark - Notification handler

- (void)keyboardDidShow:(NSNotification*)notification {
    NSDictionary *info = [notification userInfo];
    
    CGRect beginRect = [self.detailText convertRect:[[info objectForKey:UIKeyboardFrameBeginUserInfoKey] CGRectValue] fromView:nil];
    CGRect rect = [self.detailText convertRect:[[info objectForKey:UIKeyboardFrameEndUserInfoKey] CGRectValue] fromView:nil];
    CGRect frame = self.detailText.frame;
    
    if (beginRect.size.height == rect.size.height) {
        // slide in
        frame.size.height -= rect.size.height;
    } else {
        // keyboard change
        frame.size.height -= (rect.size.height - beginRect.size.height);
    }
    self.detailText.frame = frame;
    
}

- (void)keyboardDidHide:(NSNotification*)notification {
    
    NSDictionary *info = [notification userInfo];
    
    CGRect rect = [self.detailText convertRect:[[info objectForKey:UIKeyboardFrameEndUserInfoKey] CGRectValue] fromView:nil];
    CGRect frame = self.detailText.frame;
    
    frame.size.height += rect.size.height;
    self.detailText.frame = frame;
}

#pragma mark - Action handler

- (IBAction)cancel:(id)sender {
    [[self delegate] editViewControllerDidCancel:self];
}
- (IBAction)done:(id)sender {
    [[self delegate] editViewControllerDidFinish:self];
}

@end
