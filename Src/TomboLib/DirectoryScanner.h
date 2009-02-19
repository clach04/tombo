#ifndef DIRECTORYSCANNER_H
#define DIRECTORYSCANNER_H

//////////////////////////////////////////////////
// フラグ定義
//////////////////////////////////////////////////

// ディレクトリの再帰を行わない
#define DIRECTORY_SCAN_NOREC	1
// ファイルについては呼び出さない
#define DIRECTORY_SCAN_NOFILE	2

//////////////////////////////////////////////////
// ディレクトリ階層を走査する
//////////////////////////////////////////////////
//
// スキャン処理(DirectoryScanner::Scan())はDirectoryScanner::StopScan()でスキャンを打ち切ることができる。
// ただし、AfterScan(), PostDirectory()についてはStopScan()によりスキャンを打ち切られた場合でも
// 実行される。(メモリ開放処理等が行われる可能性があるため)
// 必要であればDirectoryScanner::IsStopScan()で判定し、処理をスキップさせること

#define SCANPATH_LEN (MAX_PATH * 2)

class DirectoryScanner {
	TCHAR aScanPath[SCANPATH_LEN]; // サイズに余裕を見るため*2している

	BOOL bContinue;
	DWORD nScanFlag;

	void ScanDirectory();

protected:
	////////////////////////////////
	// Sub class 向けメンバ

	// 初期化
	BOOL Init(LPCTSTR pTopPath, DWORD nFlg);

	// 現在スキャン中のパス
	LPCTSTR CurrentPath() { return aScanPath; }

	// スキャン中断指示
	void StopScan() { bContinue = FALSE; }
	BOOL IsStopScan() { return !bContinue; }

	////////////////////////////////
	// Template Method

	virtual void InitialScan() = 0;	// スキャン開始前
	virtual void AfterScan() = 0; // スキャン処理後
	virtual void PreDirectory(LPCTSTR p) = 0; // ディレクトリ走査前
	virtual void PostDirectory(LPCTSTR p) = 0; // ディレクトリ走査後
	virtual void File(LPCTSTR p) = 0; // ファイル

public:
	// スキャン開始
	BOOL Scan();
};

#endif
