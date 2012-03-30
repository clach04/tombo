#import "MasterViewController.h"
#import "DetailViewController.h"

#import "Storage.h"
#import "CustomSegue.h"
#import "FileItem.h"

@interface MasterViewController () {
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
    self.navigationItem.leftBarButtonItem = self.editButtonItem;

    UIBarButtonItem *addButton = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemAdd 
                                                                               target:self
                                                                               action:@selector(openNewNote:)];
    self.navigationItem.rightBarButtonItem = addButton;
    self.detailViewController = (DetailViewController *)[[self.splitViewController.viewControllers lastObject] topViewController];

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
        [self.detailViewController setDetailItem:nil];
        
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
    return YES;
}

- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath
{
    if (editingStyle == UITableViewCellEditingStyleDelete) {
        [_objects removeObjectAtIndex:indexPath.row];
        [tableView deleteRowsAtIndexPaths:[NSArray arrayWithObject:indexPath] withRowAnimation:UITableViewRowAnimationFade];
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

- (void)transitDetailView:(NSIndexPath *)indexPath controller:(DetailViewController*)controller {
    FileItem *item = [_objects objectAtIndex:indexPath.row];
    if (item.isUp) {
        // switch view items
        [self removeAllItems];
        [storage updir];
        [self insertItems];
    } else if (item.isDirectory) {
        // switch view items
        [self removeAllItems];
        [storage chdir: item.name];
        [self insertItems];
        
    } else {
        [controller setDetailItem:item];
        [controller setStorage: storage];
    }
    
}

// Select Row(iPhone/iPad)
// set item for iPad
- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPad) {
        [self transitDetailView:indexPath controller:self.detailViewController];
    }
}

// set item (for iPhone)
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
    if ([[segue identifier] isEqualToString:@"showDetail"]) {
        NSIndexPath *indexPath = [self.tableView indexPathForSelectedRow];         
        FileItem *item = [_objects objectAtIndex:indexPath.row];
        if (item.isUp || item.isDirectory) {
            CustomSegue *customSegue = (CustomSegue*)segue;
            customSegue.isStop = YES;            
        }
        [self transitDetailView:indexPath controller:[segue destinationViewController]];
    } else if ([[segue identifier] isEqualToString:@"newNote"]) {
        // When open new document, detailItem is newItem.
        DetailViewController *detail = [segue destinationViewController];        
        detail.detailItem = [storage newItem];
        detail.storage = storage;
    }
}

- (void)itemChanged:(FileItem *)from to:(FileItem *)to {
    // Remove old item and insert new item
    [self deleteItem: from];
    [self insertItem: to];
}

- (void)itemAdded:(FileItem *)item {
    // Simply insert a item.
    [self insertItem: item];
}
@end
