#import "DetailViewController.h"
#import "EditViewController.h"
#import "MasterViewController.h"
#import "PasswordManager.h"
#import "Storage.h"

@interface DetailViewController ()

@end

@implementation DetailViewController
@synthesize text = _text;
@synthesize item = _item;
@synthesize master = _master;
@synthesize storage = _storage;
@synthesize passwordManager = _passwordManager;
@synthesize delegate = _delegate;

#pragma mark - framework
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
    
    [self.navigationController setToolbarHidden:NO];
    if (self.item != nil) {
        [self setupToolbar];
    }
    
    [self loadNote];
}

- (void)setupToolbar {
    NSString *btnTitle;
    if (self.item.isCrypt) {
        btnTitle = @"Decrypt";
    } else {
        btnTitle = @"Encrypt";
    }
    UIBarButtonItem *cryptBtn = [[UIBarButtonItem alloc] initWithTitle:btnTitle
                                                                 style:UIBarButtonItemStyleBordered
                                                                target:self
                                                                action:@selector(crypt:)];
    UIBarButtonItem *moveBtn = [[UIBarButtonItem alloc] initWithTitle:@"Move"
                                                                style:UIBarButtonItemStyleBordered
                                                               target:self 
                                                               action:@selector(move:)];
    [self setToolbarItems:[NSArray arrayWithObjects:cryptBtn, moveBtn, nil] animated:NO];
    
}

- (void)viewDidUnload
{
    [self setText:nil];
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
        self.item.body = self.text.text;
        
        edit.detailItem = self.item;
        edit.delegate = self.master;
    } else if ([[segue identifier] isEqualToString:@"moveNote"]) {
        MoveViewController *move = (MoveViewController*)[[[segue destinationViewController] viewControllers] objectAtIndex:0];
        move.delegate = self;
        move.folders = [self.storage listFolders];
    }
}

#pragma mark - Logic

- (void)crypt:(id)sender {
    if (self.item.isCrypt) {
        // Decrypting
        /*
        void (^callback)(NSString *) = ^(NSString *password) {
            FileItem *newItem = [self.storage decrypt:password item:self.item];
            if (newItem) {
                [self.delegate detailViewFileItemChanged:self.item to:newItem];
                self.item = newItem;
            }
        };
        // Callback request to passwordManager.
        [self.passwordManager requestPassword:callback];
         */
        if ([self.passwordManager preparePassword]) {
            FileItem *newItem = [self.storage decrypt:self.passwordManager.password item:self.item];
            if (newItem) {
                [self.delegate detailViewFileItemChanged:self.item to:newItem];
                self.item = newItem;
            } else {
                UIAlertView *decFail = [[UIAlertView alloc]initWithTitle:@"Info" message:@"Decrypt failed." delegate:nil cancelButtonTitle:@"OK" otherButtonTitles:nil];
                [decFail show];
            }
        }
    } else {
        // Encrypting
        if ([self.passwordManager preparePasswordConfirm]) {
            FileItem *newItem = [self.storage encrypt:self.passwordManager.password item:self.item];
            if (newItem) {
                [self.delegate detailViewFileItemChanged:self.item to:newItem];
                self.item = newItem;
            }
        }
    }
}

- (void)move:(id)sender {
    [self performSegueWithIdentifier:@"moveNote" sender:self];
}

- (void)loadNote {
    if (!self.item) return;
    
    if (self.item.isCrypt) {
        NSString *note = [Storage loadCryptFile:self.item.path password:self.passwordManager.password];
        if (note) {
            self.text.text = note;
        }
    } else {
        NSString *note = [Storage load:self.item.path];
        if (note) {
            self.text.text = note;
        } else {
            self.text.text = @"";
        }
    }
}

- (void)setItem:(FileItem *)item {
    _item = item;
    if (item.isNewItem) {
        self.text.text = @"";
    } else {
        // On iPhone and call by segue, self.text is nil because view is not loaded yet.
        if (self.text) {
            [self loadNote];
            [self setupToolbar];
        }
    }
}

#pragma mark - MoveViewControllerDelegate

- (void)moveViewControllerCancel:(MoveViewController *)view {
    [self dismissModalViewControllerAnimated:YES];
}

- (void)moveViewControllerSelect:(MoveViewController *)view path:(NSString *)path {
    NSString *toPath = [self.storage moveFrom:self.item toPath:path];
    [self.delegate detailViewFileItemRemoved:self.item];
    self.item.path = toPath;
    [self dismissModalViewControllerAnimated:YES];
}

@end
