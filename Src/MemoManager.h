#ifndef MEMOMANAGER_H
#define MEMOMANAGER_H

class MemoDetailsView;
class MemoSelectView;
class MainFrame;
class PasswordManager;
class SearchEngineA;
class TreeViewItem;
class TString;
class TomboURI;

/////////////////////////////////////
// Control other view
/////////////////////////////////////
// control select - detail view relation and mainframe items(menu, toolbar, etc.)

class MemoManager {
protected:
	MemoDetailsView *pMemoDetailsView;
	MemoSelectView *pMemoSelectView;
	MainFrame *pMainFrame;

	BOOL AllocNewMemo(LPCTSTR pText, BOOL bCopy);

	SearchEngineA *pSearchEngineA;

	BOOL bMSSearchFlg;
	BOOL bMDSearchFlg;

public:
	/////////////////////////////////////
	// ctor & dtor

	MemoManager();
	~MemoManager();

	BOOL Init(MainFrame *p, MemoDetailsView *md, MemoSelectView *ms);

	/////////////////////////////////////

	// メモが更新されていたらセーブする
	//	pYNCにはIDOK,IDYES,IDNO,IDCANCELのいずれかが入る。
	//		セーブの必要がなかった場合にはIDOKが設定される。
	//		IDYESが設定されている場合には保存が実行された。
	//  pYNCにNULLを設定した場合、問い合わせのメッセージボックスは表示されない
	//		この場合、bDupModeがTRUEの場合にはメモは常に別ファイルとして保存される。
	//				  bDupModeがFALSEの場合には通常動作
	BOOL SaveIfModify(LPDWORD pYNC, BOOL bDupMode);

	/////////////////////////////////////
	// 検索関連

	void SetSearchEngine(SearchEngineA *p);
	SearchEngineA *GetSearchEngine() { return pSearchEngineA; }

	/////////////////////////////////////
	// data accessor

	MainFrame *GetMainFrame() { return pMainFrame; }
	MemoSelectView *GetSelectView() { return pMemoSelectView; }
	MemoDetailsView *GetDetailsView() { return pMemoDetailsView; }

	/////////////////////////////////////
	// searching

	// 「次を検索」で現在選択されている項目を検索対象に含めるか?
	// 一覧ビューの場合、ユーザが選択を変更したら含める
	BOOL MSSearchFlg() { return bMSSearchFlg; }
	void SetMSSearchFlg(BOOL b) { bMSSearchFlg = b; }

	BOOL MDSearchFlg() { return bMDSearchFlg; }
	void SetMDSearchFlg(BOOL b) { bMDSearchFlg = b; }

	/////////////////////////////////////
	// 
	void ChangeURINotify(const TomboURI *pNewURI);

	// 詳細ビューに表示されている場合に必要なら保存し、一覧ビューにフォーカスを移す
	// TODO: 保存依頼に置き換えられる気がする
	void InactiveDetailsView();

	BOOL GetCurrentSelectedPath(TString *pPath);

};

#endif