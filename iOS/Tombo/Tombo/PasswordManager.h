#import <Foundation/Foundation.h>

@interface PasswordManager : NSObject

@property (strong,nonatomic) NSString *password;

/*
 * If current password is not set (including timeout), popup dialog and ask to user. Or not, do nothing.
 * Note user may be cancel and finally prepare may be failed.
 * return YES when prepare is success, otherwise return NO.
 */
- (BOOL)preparePassword;

- (BOOL)preparePasswordConfirm;
@end
