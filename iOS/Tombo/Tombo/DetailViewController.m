#import "DetailViewController.h"

@interface DetailViewController ()
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
    
    [self configureView];
}

- (void)viewDidUnload
{
    [self setDetailText:nil];
    [super viewDidUnload];
    // Release any retained subviews of the main view.
}

- (void)viewWillDisappear:(BOOL)animated {
    // Leaving detail view
    NSString *note = self.detailText.text;
    [storage save:note item: self.detailItem];
    
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

#pragma mark - Notification handler

- (void)keyboardDidShow:(NSNotification*)notification {
    NSDictionary *info = [notification userInfo];
    
    CGRect rect = [[info objectForKey:UIKeyboardFrameBeginUserInfoKey] CGRectValue];
    CGRect frame = self.detailText.frame;
    frame.size.height -= rect.size.height;
    self.detailText.frame = frame;
}

- (void)keyboardDidHide:(NSNotification*)notification {
    NSDictionary *info = [notification userInfo];
    
    CGRect rect = [[info objectForKey:UIKeyboardFrameEndUserInfoKey] CGRectValue];
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
