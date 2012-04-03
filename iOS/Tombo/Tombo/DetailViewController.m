#import "DetailViewController.h"
#import "EditViewController.h"
#import "MasterViewController.h"

@interface DetailViewController ()

@end

@implementation DetailViewController
@synthesize text = _text;
@synthesize item = _item;
@synthesize master = _master;

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
    }
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view.
}

- (void)viewDidUnload
{
    [self setText:nil];
    [super viewDidUnload];
    // Release any retained subviews of the main view.
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
	return YES;
}

- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    if ([[segue identifier] isEqualToString:@"editNote"]) {
        EditViewController *edit = (EditViewController*)[[[segue destinationViewController] viewControllers] objectAtIndex:0];
        edit.detailItem = self.item;
        edit.delegate = self.master;

    }
}

- (void)setItem:(FileItem *)item {
    if (self.item == item) return;
    if (item.isNewItem) {
        self.text.text = @"";
    } else {
        NSError *error;
        NSString *note = [NSString stringWithContentsOfFile:item.path
                                                   encoding:NSUTF8StringEncoding
                                                      error:&error];
        if (error) {
            self.text.text = @"";
        } else {
            self.text.text = note;
        }
    }
    _item = item;
}
@end
