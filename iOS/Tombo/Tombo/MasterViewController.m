#import "MasterViewController.h"
#import "EditViewController.h"
#import "DetailViewController.h"
#import "EditCancelAlert.h"
#import "NewFolderAlert.h"

#import "Storage.h"
#import "FileItem.h"
#import "PasswordManager.h"

@interface MasterViewController () <UITableViewDelegate, UISplitViewControllerDelegate> {
    NSMutableArray *_objects;
    Storage *storage;
    PasswordManager *passwordManager;
    
    UIImage *imgFolder;
    UIImage *imgDocument;
    UIImage *imgUp;
    UIImage *imgKey;
}
@end

@implementation MasterViewController

@synthesize detailViewController = _detailViewController;

- (void)awakeFromNib
{
    if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPad) {
        self.clearsSelectionOnViewWillAppear = NO;
        self.contentSizeForViewInPopover = CGSizeMake(320.0, 600.0);
    }
    [super awakeFromNib];
}

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.
    self.detailViewController = (DetailViewController *)[[self.splitViewController.viewControllers lastObject] topViewController];
    self.detailViewController.master = self;
    self.splitViewController.delegate = self;
    
    passwordManager = [[PasswordManager alloc] init];
    
    UIBarButtonItem *newFolderBtn = [[UIBarButtonItem alloc] initWithTitle:@"New Folder"
                                                                     style:UIBarButtonItemStyleBordered
                                                                    target:self
                                                                    action:@selector(createNewFolder:)];
    [self setToolbarItems:[NSArray arrayWithObjects:newFolderBtn, nil] animated:YES];
    [self.navigationController setToolbarHidden:NO];
    
    imgFolder = nil;
    imgDocument = nil;
    if (!storage) {
        storage = [Storage init];
    }
    
    // Init detailview for iPad.
    self.detailViewController.storage = storage;
    self.detailViewController.passwordManager = passwordManager;
    self.detailViewController.delegate = self;
    
    // Load initial items.
    [self insertItems];
}

- (void)viewDidUnload
{
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    storage = nil;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPhone) {
        return (interfaceOrientation != UIInterfaceOrientationPortraitUpsideDown);
    } else {
        
        return YES;
    }
}

- (void)openNewNote:(id)sender
{
    if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPhone) {
        // iPhone : Transit to detail view.
        [self performSegueWithIdentifier:@"newNote" sender:self];
    } else {
        // iPad : Clear detail view.
    }
}

- (void)createNewFolder:(id)sender {
    NewFolderAlert *alert = [[NewFolderAlert alloc] initWithDefault];
    NSString *folderName = [alert showAndWait];
    if (folderName) {
        FileItem *item = [storage newFolder:folderName];
        [self insertItem:item];
    }
}
#pragma mark - Item operations

- (void)insertItems {
    for (FileItem *file in [storage listItems]) {
        [self insertItem: file];
    }    
    if (!storage.isTopDir) {
        FileItem *up = [FileItem allocWithName:@"UP"];
        up.isUp = YES;
        [self insertItem: up];
    }
}

- (void)removeAllItems {
    NSUInteger n = [_objects count];
    
    NSMutableArray *rmItems = [[NSMutableArray alloc] initWithCapacity:n];
    for (int i = 0; i < n; i++) {
        NSIndexPath *indexPath = [NSIndexPath indexPathForRow:i inSection:0];
        [rmItems addObject: indexPath];
    }
    [_objects removeAllObjects];
    [self.tableView deleteRowsAtIndexPaths: rmItems withRowAnimation:UITableViewRowAnimationAutomatic];
}

- (void)insertItem:(FileItem *)item {
    if (!_objects) {
        _objects = [[NSMutableArray alloc] init];
    }
    NSUInteger n = [_objects count];
    NSUInteger i;
    for (i = 0; i < n; i++) {
        if (item.isUp) break;
        
        FileItem *cur = [_objects objectAtIndex:i];
        NSComparisonResult r = [item compare:cur];
        if (r == NSOrderedAscending || r == NSOrderedSame) {
            break;
        }
    }
    
    [_objects insertObject:item atIndex:i];
    NSIndexPath *indexPath = [NSIndexPath indexPathForRow:i inSection:0];
    [self.tableView insertRowsAtIndexPaths:[NSArray arrayWithObject:indexPath]
                          withRowAnimation:UITableViewRowAnimationAutomatic];
}

- (void) deleteItem:(FileItem *)item {
    NSUInteger i = 0;
    for (FileItem *f in _objects) {
        if ([f.name isEqualToString: item.name]) {
            [_objects removeObjectAtIndex:i];
            NSIndexPath *indexPath = [NSIndexPath indexPathForRow:i inSection:0];
            [self.tableView deleteRowsAtIndexPaths:[NSArray arrayWithObject:indexPath] 
                                  withRowAnimation:UITableViewRowAnimationAutomatic];
            break;
        }
        i++;
    }
}

- (BOOL)splitViewController:(UISplitViewController *)svc shouldHideViewController:(UIViewController *)vc inOrientation:(UIInterfaceOrientation)orientation {
    return NO;
}

#pragma mark - Table View

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return _objects.count;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"Cell"];

    FileItem *fItem = [_objects objectAtIndex:indexPath.row];
    cell.textLabel.text = [fItem name];
    if (fItem.isUp) {
        if (!imgUp) {
            imgUp = [UIImage imageNamed:@"sub_blue_up-32"];
        }
        cell.imageView.image = imgUp;
    } else if (fItem.isCrypt) {
        if (!imgKey) {
            imgKey = [UIImage imageNamed:@"key-32"];
        }
        cell.imageView.image = imgKey;
    } else if (fItem.isDirectory) {
        if (!imgFolder) {
            imgFolder = [UIImage imageNamed:@"Folder-32"];

        }
        cell.imageView.image = imgFolder;
    } else {
        if (!imgDocument) {
            imgDocument = [UIImage imageNamed:@"TextDocument-32"];
        }
        cell.imageView.image = imgDocument;
    }
    return cell;
}

- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath
{
    // Return NO if you do not want the specified item to be editable.
    FileItem *item = [_objects objectAtIndex:indexPath.row];
    if (item.isUp) return NO;
    return YES;
}

- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath
{
    if (editingStyle == UITableViewCellEditingStyleDelete) {
        FileItem *item = [_objects objectAtIndex:indexPath.row];
        [storage deleteItem:item];
        
        [_objects removeObjectAtIndex:indexPath.row];
        [tableView deleteRowsAtIndexPaths:[NSArray arrayWithObject:indexPath] 
                         withRowAnimation:UITableViewRowAnimationFade];
    } else if (editingStyle == UITableViewCellEditingStyleInsert) {
        // Create a new instance of the appropriate class, insert it into the array, and add a new row to the table view.
    }
}

/*
// Override to support rearranging the table view.
- (void)tableView:(UITableView *)tableView moveRowAtIndexPath:(NSIndexPath *)fromIndexPath toIndexPath:(NSIndexPath *)toIndexPath
{
}
*/

/*
// Override to support conditional rearranging of the table view.
- (BOOL)tableView:(UITableView *)tableView canMoveRowAtIndexPath:(NSIndexPath *)indexPath
{
    // Return NO if you do not want the item to be re-orderable.
    return YES;
}
*/

- (void)transitToDetailView:(FileItem *)item {
    if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPad) {
        self.detailViewController.item = item;
    } else {
        [self performSegueWithIdentifier:@"showNote" sender:self];
    }
}

// Select Row(iPhone/iPad)
// set item for iPad
- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    FileItem *item = [_objects objectAtIndex:indexPath.row];
    if (item.isUp) {
        // switch view items
        [self removeAllItems];
        [storage updir];
        [self insertItems];        
    } else if (item.isDirectory) {
        [self removeAllItems];
        [storage chdir: item.name];
        [self insertItems];        
    } else {
        if (item.isCrypt) {
            if ([passwordManager preparePassword]) {
                // Check to success decrypt and transit.
                NSString *note = [Storage loadCryptFile:item.path password:passwordManager.password];
                if (note) {
                    [self transitToDetailView:item];
                } else {
                    UIAlertView *decryptFail = [[UIAlertView alloc] initWithTitle:@"Error"
                                                                          message:@"Decryption failed." 
                                                                         delegate:nil 
                                                                cancelButtonTitle:@"OK" 
                                                                otherButtonTitles:nil];
                    [decryptFail show];
                    // Invalid password so clear it.
                    passwordManager.password = nil;
                }
            }
        } else {
            [self transitToDetailView:item];
        }
    }
}

// set item (for iPhone)
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
    if ([[segue identifier] isEqualToString:@"newNote"]) {
        EditViewController *edit = (EditViewController*)[[[segue destinationViewController] viewControllers] objectAtIndex:0];
        edit.detailItem = [storage newItem];
        edit.delegate = self;
    } else if ([[segue identifier] isEqualToString:@"editNote"]) {
        // This path is used only iPhone
        EditViewController *edit = (EditViewController*)[[[segue destinationViewController] viewControllers] objectAtIndex:0];
        NSIndexPath *indexPath = [self.tableView indexPathForSelectedRow];
        FileItem *item = [_objects objectAtIndex:indexPath.row];
        edit.detailItem = item;
        edit.delegate = self;
    } else if ([[segue identifier] isEqualToString:@"showNote"]) {
        DetailViewController *detail = [segue destinationViewController];
        self.detailViewController = detail;
        NSIndexPath *indexPath = [self.tableView indexPathForSelectedRow];
        FileItem *item = [_objects objectAtIndex:indexPath.row];
        detail.item = item;
        detail.storage = storage;
        detail.passwordManager = passwordManager;
        detail.master = self;
        detail.delegate = self;
    }
}

#pragma mark - EditViewControllerDelegate

- (void)editViewControllerDidFinish:(EditViewController *)controller {
    if (controller.isModify) {
        // save note
        if (![controller.detailItem isNewItem]) {
            [self deleteItem:controller.detailItem];
        }
        NSString *note = controller.detailText.text;
        FileItem *item;
        if (controller.detailItem.isCrypt) {
            if ([passwordManager preparePasswordConfirm]) {
                item = [storage saveCrypt:note item:controller.detailItem password:passwordManager.password];
            } else {
                // Don't dismiss view.
                return;
            }
        } else {
            item = [storage savePlain:note item:controller.detailItem];
        }
        controller.isModify = NO;
        
        self.detailViewController.item = item;

        [self insertItem:item];
    }
    [self dismissModalViewControllerAnimated:YES];
}

- (void)editViewControllerDidCancel:(EditViewController *)controller {
    if (controller.isModify) {
        EditCancelAlert *cancel = [[EditCancelAlert alloc] initWithDefault];
        if ([cancel showAndWait]) {
            [self dismissModalViewControllerAnimated:YES];                    
        }
    } else {
        [self dismissModalViewControllerAnimated:YES];                
    }
}

#pragma mark - DetailViewDelegate

-(void)detailViewFileItemChanged:(FileItem*)oldItem to:(FileItem *)newItem {
    [self deleteItem:oldItem];
    [self insertItem:newItem];
}

-(void)detailViewFileItemRemoved:(FileItem *)item {
    [self deleteItem:item];
}

@end
