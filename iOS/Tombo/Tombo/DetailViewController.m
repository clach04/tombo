#import "DetailViewController.h"
#import "MasterViewController.h"

@interface DetailViewController () {
    BOOL isModify;
}
@property (strong, nonatomic) UIPopoverController *masterPopoverController;
- (void)configureView;
@end

@implementation DetailViewController

@synthesize detailItem = _detailItem;
@synthesize storage;
@synthesize detailText = _detailText;
@synthesize masterPopoverController = _masterPopoverController;

#pragma mark - Managing the detail item

- (void)setDetailItem:(id)newDetailItem
{
    if (_detailItem != newDetailItem) {
        // save current note
        [self save];
        
        // set item
        _detailItem = newDetailItem;
        
        // Update the view.
        [self configureView];
    }

    if (self.masterPopoverController != nil) {
        [self.masterPopoverController dismissPopoverAnimated:YES];
    }        
}

- (void)configureView
{
    NSString *noteData;
    if (self.detailItem && self.detailItem.path) {
        NSError *error;
        noteData = [NSString stringWithContentsOfFile:self.detailItem.path 
                                             encoding:NSUTF8StringEncoding
                                                error:&error];
        if (error) return;
    } else {
        noteData = @"";
    }
    self.detailText.text = noteData;
}

- (void)viewDidLoad
{
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
    isModify = NO;
}

- (void)viewDidUnload
{
    [self setDetailText:nil];
    self.detailItem = nil;
    self.storage = nil;
    [super viewDidUnload];
}

- (void)viewWillDisappear:(BOOL)animated {
    // Leaving detail view
    [self save];
    [super viewWillDisappear: animated];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPhone) {
        return (interfaceOrientation != UIInterfaceOrientationPortraitUpsideDown);
    } else {
        return YES;
    }
}

- (void)textViewDidChange:(UITextView *)textView {
    isModify = YES;
}

- (void)save {
    if (!isModify) return;
    
    NSString *note = self.detailText.text;    
    FileItem *newPath = [storage save:note item: self.detailItem];
    isModify = NO;
    
    MasterViewController *master;
    
    if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPad) {
        UINavigationController *v = [self.splitViewController.viewControllers objectAtIndex:0];
        NSArray *a = v.viewControllers;
        master = [a objectAtIndex:0];
    } else {
        // To notify master view, retract reference from navigation controller.
        master = [self.navigationController.viewControllers objectAtIndex:0];
    }
    
    if (self.detailItem.name) {
        // item exists
        if (self.detailItem != newPath) {
            [master itemChanged: self.detailItem to:newPath];
        }
    } else {
        // new item
        [master itemAdded: newPath];
    }
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

#pragma mark - Split view

- (void)splitViewController:(UISplitViewController *)splitController willHideViewController:(UIViewController *)viewController withBarButtonItem:(UIBarButtonItem *)barButtonItem forPopoverController:(UIPopoverController *)popoverController
{
    barButtonItem.title = NSLocalizedString(@"Master", @"Master");
    [self.navigationItem setLeftBarButtonItem:barButtonItem animated:YES];
    self.masterPopoverController = popoverController;
}

- (void)splitViewController:(UISplitViewController *)splitController willShowViewController:(UIViewController *)viewController invalidatingBarButtonItem:(UIBarButtonItem *)barButtonItem
{
    // Called when the view is shown again in the split view, invalidating the button and popover controller.
    [self.navigationItem setLeftBarButtonItem:nil animated:YES];
    self.masterPopoverController = nil;
}

@end
