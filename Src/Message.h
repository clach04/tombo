#ifndef MESSAGE_H
#define MESSAGE_H

//////////////////////////////////////////////////
// message definition
//////////////////////////////////////////////////

#include "MsgRes/MsgDef.h"

//////////////////////////////////////////////////
// Message manager
//////////////////////////////////////////////////

class TomboMessage {
	DWORD nNumMsgs;
	LPCTSTR pMsg[NUM_MESSAGES];

	LPTSTR pMsgBuf;

protected:
	BOOL LoadMsg(HANDLE hFile);
	LPTSTR GetNatvieData(HANDLE hFile);

public:
	TomboMessage();
	~TomboMessage();

	BOOL Init();


	LPCTSTR GetMsg(DWORD nMsgID);
};

//////////////////////////////////////////////////
// message manager instance declaration
//////////////////////////////////////////////////

extern TomboMessage g_mMsgRes;


#endif

