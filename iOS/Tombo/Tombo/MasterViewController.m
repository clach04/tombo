#import "MasterViewController.h"
#import "EditViewController.h"
#import "DetailViewController.h"
#import "EditCancelAlert.h"
#import "NewFolderAlert.h"

#import "Storage.h"
#import "FileItem.h"

@interface BackgroundView : UIView {
}
@end

@implementation BackgroundView
- (id)initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        self.backgroundColor = [UIColor clearColor];
    }
    return self;
}

- (void)drawRect:(CGRect)rect {
    UIBezierPath *path = [UIBezierPath bezierPathWithRoundedRect:self.bounds cornerRadius:5.0];
    [path moveToPoint:CGPointMake(0, self.bounds.size.height/2.0)];
    [path addLineToPoint:CGPointMake(self.bounds.size.width, self.bounds.size.height/2.0)];
    [[UIColor whiteColor] set];
    [path fill];
}
@end

@interface MasterViewController () <UIAlertViewDelegate, UITableViewDelegate, UISplitViewControllerDelegate> {
    NSMutableArray *_objects;
    Storage *storage;
    
    UIImage *imgFolder;
    UIImage *imgDocument;
    UIImage *imgUp;
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
    NewFolderAlert *alert = [[NewFolderAlert alloc] initWithTitle:@"Folder name:" 
                                                    message:@"\n"
                                                   delegate:self
                                          cancelButtonTitle:@"Cancel"
                                          otherButtonTitles:@"Done", nil];
    BackgroundView *back = [[BackgroundView alloc] initWithFrame:CGRectMake(20.0, 43.0, 245.0, 25.0)];
    [alert addSubview:back];
    UITextField *textField = [[UITextField alloc] initWithFrame:CGRectMake(20.0, 45.0, 245.0, 25.0)];
    [alert addSubview:textField];
    
    [alert show];
//    [textField becomeFirstResponder];
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
    [_objects insertObject:item atIndex:0];
    NSIndexPath *indexPath = [NSIndexPath indexPathForRow:0 inSection:0];
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

// Select Row(iPhone/iPad)
// set item for iPad
- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
//    if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPad) {
//        [self transitDetailView:indexPath controller:self.detailViewController];
//    }
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
        if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPad) {
            self.detailViewController.item = item;
        } else {
            [self performSegueWithIdentifier:@"editNote" sender:self];
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
        FileItem *item = [storage save:note item:controller.detailItem];
        controller.isModify = NO;
        
        self.detailViewController.item = item;

        [self insertItem:item];
    }
    [self dismissModalViewControllerAnimated:YES];
}

- (void)editViewControllerDidCancel:(EditViewController *)controller {
    if (controller.isModify) {
        UIAlertView *alert = [[EditCancelAlert alloc] initWithTitle:@"Confirm" 
                                                        message:@"Note is modified. Are you sure to discard changes?" delegate:self 
                                              cancelButtonTitle:@"OK"
                                              otherButtonTitles:@"Cancel", nil];
        [alert show];
    } else {
        [self dismissModalViewControllerAnimated:YES];                
    }
}

#pragma mark - AlertViewDelegate

-(void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex {
    if ([alertView isKindOfClass:[EditCancelAlert class]]) {
        if (buttonIndex == 0) {
            [self dismissModalViewControllerAnimated:YES];        
        }
    } else if ([alertView isKindOfClass:[NewFolderAlert class]]) {
        if (buttonIndex == 1) {
            //
            UITextField *field = [alertView.subviews lastObject];
            NSString *folderName = [field.text stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
            if (folderName.length > 0) {
                FileItem *item = [storage newFolder:folderName];
                [self insertItem:item];
            }
        }
    }
}
@end
