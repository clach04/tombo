#ifndef PASSWORDMANAGER_H
#define PASSWORDMANAGER_H

// タイムアウトのチェック
//
// 電源ON フォアグラウンドの状態で放置
// 電源OFFにして放置
// 電源ON 他のアプリを起動して放置


//////////////////////////////////////////////
// パスワードマネージャ
//////////////////////////////////////////////
// パスワードを管理する。
// アプリ起動後パスワード未入力の場合、ダイアログを表示してパスワードの入力を促す。

class PasswordManager {
	char *pPassword;

	HWND hParent;
	HINSTANCE hInstance;
	FILETIME ftLastAccess;

public:
	PasswordManager() : pPassword(NULL), hParent(NULL), hInstance(NULL) {}
	~PasswordManager();

	BOOL Init(HWND hParent, HINSTANCE hInstance);

	// 戻り値がNULLでbCancelがTRUEの場合、ユーザがキャンセルした
	// 暗号化の場合には再入力が必要となる。bEncrypt = TRUEとすること。
	const char *Password(BOOL *bCancel, BOOL bEncrypt);

	// パスワードを記憶しているか
	BOOL IsRememberPassword() { return pPassword != NULL; }

	// パスワードの消去
	void ForgetPassword();

	// タイマによるパスワード消去用関数
	void UpdateAccess();
	void ForgetPasswordIfNotAccessed();
};

//////////////////////////////////////////////
// パスワードFingerPrintの取得・確認
//////////////////////////////////////////////
// pFpは32バイトのバッファ

// FingerPrintは1バイトの識別子,16バイトのseedと16バイトの結果、計33バイトからなる。
// pFP[ 0]          : 識別子(バージョン情報)
// pFp[ 1]～pFp[16] : seed
// pFp[17]～pFp[32] : 結果

BOOL GetFingerPrint(LPBYTE pFp, const char *pPassword);
BOOL CheckFingerPrint(LPBYTE pFp, const char *pPassword);

//////////////////////////////////////////////
// global definitions
//////////////////////////////////////////////

extern PasswordManager *g_pPassManager;

#endif
