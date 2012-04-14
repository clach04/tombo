#import "MoveViewController.h"

@interface MoveViewController ()

@end

@implementation MoveViewController {
    UIImage *imgFolder;
}

@synthesize delegate = _delegate;
@synthesize folders = _folders;

- (void)viewDidLoad
{
    [super viewDidLoad];

    imgFolder = [UIImage imageNamed:@"Folder-32"];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    return YES;
}

- (IBAction)cancel:(id)sender {
    if (self.delegate) [self.delegate moveViewControllerCancel:self];
}

#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return [self.folders count];
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    static NSString *CellIdentifier = @"Cell";
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];

    NSString *folder = [self.folders objectAtIndex:indexPath.row];
    cell.textLabel.text = folder;
    cell.imageView.image = imgFolder;
    return cell;
}

#pragma mark - Table view delegate

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    NSString *path = [self.folders objectAtIndex:indexPath.row];
    if (self.delegate) [self.delegate moveViewControllerSelect:self path:path];
}

@end
