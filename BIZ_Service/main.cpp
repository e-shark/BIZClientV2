
#include <Windows.h>
#include <stdio.h>
#include <time.h>

#include "main.h"
#include "LogUtils.h"
#include "BIZ_DB.h"
#include "BIZ_Client.h"

char ExecPath[256]="";

HANDLE hMainThread = NULL;       // Handle to thread implementing MainForm cycle
bool bMainThreadStopped = false; // set to true to stop Main thread



//-----------------------------------------------------------------------------
//  MAIN CYCLE
//-----------------------------------------------------------------------------
DWORD MainCycleThread(LPDWORD lpdwParam)
{
    int PID = 4;
    char s[1024];
    clock_t tstart = clock();
    clock_t tnow;
    double ElapsedTime;
    tBIZ_Client *BIZClient;
    bool LoginOk;
    int SchedType;
    int SchedID;
    int ParamI1, ParamI2;
    char *ParamBuf;
    int ParamBufLen = 16384;
    int ParamLen;

    LogMessage("BIZ service Started.", ML_WRK1);

    ParamBuf = (char*)malloc(ParamBufLen);
    BIZClient = new tBIZ_Client();
    BIZClient->SetCookiesPath(ExecPath);
	BIZClient->TOR_SetUp("127.0.0.1", 9170, 9171);
	BIZClient->TOR_On();
	BIZClient->ChangeEntryPoint();

    // ���� ������ ������, �������� �������� ������
    if (BIZClient && BIZClient->LoadPerson(PID)) {


		snprintf(s, sizeof(s), "������ %s ������� ������ %s ", BIZClient->ServerName.c_str(), BIZClient->UserLogin.c_str());
		LogMessage(s);
		/*
		// ���������, ���� ����
		LoginOk = BIZClient->CheckLogin();
		if (!LoginOk)
			LoginOk = BIZClient->Login();
		*/

		//BIZClient->GetCompanyInfo();
        //BIZClient->SetGoodsPrice(1854438, NULL);
        //BIZClient->SetGoodsPrice(1901385, NULL);
        //BIZClient->AutoPurchaseGoods(1901385, 10);
        //BIZClient->SetGoodsPrice(1901385, NULL);
        //BIZClient->AutoPurchaseGoods(1915773, 10);
        //BIZClient->SetGoodsPrice(1901352, NULL);
        //BIZClient->AutoPurchaseForMarkets(3);
        //BIZClient->AutoPurchaseGoods(1901352, 7);
        //BIZClient->Test2();

		sprintf(s, "%bizdb_home.sdb", ExecPath);
		BIZClient->Test3(s);

        // �������� ���� (�������� ���� �� ���������� ������)
        while (!bMainThreadStopped) {
            // �������� �������� ��������� ������ � ����������
            SchedID = DB_GetShedule(BIZClient->PersCID(), SchedType, ParamI1, ParamI2, ParamBuf, ParamBufLen, ParamLen);
            if (SchedID) {
                sprintf(s, "*** �������� ������� %d (type %d)", SchedID, SchedType);
                LogMessage(s, ML_WRK1);

                // ---------------------
                // !!!  �����������  !!!
                // �� ����� ����, ��� ���� ��������� ��������� ����� � ��������
                // ��������� ������, � �� ���������� �������, ���� ����� ��� �� �����������, ������ �
                // ---------------------

                try {
                    //tstart = clock();
                    DB_SetScheduleLastRun(SchedID);
                    //sprintf(s, "�� ������ DB_SetScheduleLastRun ��������� %f ���", (double)(clock() - tstart) / CLOCKS_PER_SEC);
                    //LogMessage(s, ML_DBG3);

                    switch (SchedType) {
                    case 1:             // - �������� ���������� ����� -
                        BIZClient->GetDailyBonus();
                        break;

                    case 2:             // - ���������� ����������� ��� ���������
                        // ���� ����� ParamI1, �� ��� ������ ID ��������, �� �������� ���� ������ �������
                        // ���� �� ����� (��� =0), �� ����������� ��� ���� ���������
                        // ParamI2 - �� ������� ���� ��������
                        if (ParamI1) {
                            BIZClient->AutoPurchaseForMarket(ParamI1, ParamI2);
                        }else
                            BIZClient->AutoPurchaseForAllMarkets(ParamI2);
                        break;

                    case 3:             // - ��������� ���� ��� ��������� -
                        if (ParamI1) {
                            BIZClient->SetGoodsPrice(ParamI1, NULL);
                        }
                        else
                            BIZClient->SetGoodsPriceForAllMarkets();
                        break;

                    case 5:             // - ��������� ���� �� ������������ ������� -
                        break;

                    case 6:             // - ����� ������� ��������� ������������ �� ������������ ����� -
                        break;

                    case 7:             // - ���������� ������ ������������ ������ -
                        break;

                    case 8:            // - ��������� ���������� �� �������� -
                        break;

                    case 9:            // - �������� ���������� � �������� -
                        BIZClient->GetCompanyInfo();
                        // �������� ����������� �������� ��������
                        // ���������� � �������� � ���� ��������� ��������
                        // ���������� ���������� ��������
                        // �������� ���������� � �������
                        // ������� �� ������� ������ ����������
                        break;

                    case 10:             // - ��������� ��������� -
                        break;

                    case 13:            // - ������� ����� �����
                        BIZClient->ChangeEntryPoint();
                        break;

                    case 14:             // - �������� ������� ��� �� �������� � ������ -
                        break;

                    case 15:             // - ��������� �������� ���� -
                        BIZClient->SaveExchangeState();
                        break;


                    default:
                        Sleep(1000);    // ���� �� ��������� ������� �������
                    }   //switch
                } //try
                catch (...) {
                    LogMessage("Uexpected Exception !", ML_ERR1);
                    Sleep(1000); 
                }
                sprintf(s, "*** ���������� ������� %d (type %d)", SchedID, SchedType);
                LogMessage(s, ML_WRK1);

            }else Sleep(1000);      // ���� �� ���� ������� �������
        }   //�������� while

    }
    else {
        LogMessage("Error! Can't create BIZ Client.", ML_ERR1);
    }

    free(ParamBuf);
    LogMessage("BCservice Stoped.", ML_WRK1);
    return 0;
}


//-----------------------------------------------------------------------------
//	����� ������� BIZ ������� 
//	������������ ��������:
//		>0 - ������ ������� ���������
//		<=0 - ������ (0 - ��� ������)
//-----------------------------------------------------------------------------
int BCService_Start(int argc, char **argv)
{
    char s[512];
    DWORD dwThreadId;
    int ErrCod = 0;
    char *pC;

    // ���������� ���� � ������������ ����� 
#ifdef _LINUX_
    strncpy(ExecPath, argv[0], sizeof(ExecPath) - 1);
    pC = strrchr(ExecPath, '/');
#else
    if (GetModuleFileName(NULL, ExecPath, sizeof(ExecPath)) == 0) 
        memset(ExecPath, 0, sizeof(ExecPath));
    pC = strrchr(ExecPath, '\\');
#endif
    if (pC) {
        pC++;
        *(pC) = 0;
    }
    else
        memset(ExecPath, 0, sizeof(ExecPath));

    // ������ ���� ���������� ����
    SetLogPath(ExecPath);                           // ���������, ���� ������ ����

#ifdef LogInSingleDir
#ifdef _LINUX_
    sprintf(s, "Create Dir: %sLig/", ExecPath);
    LogMessage(s, ML_DBG2);
    sprintf(s, "%sLog/", ExecPath);
    mkdir(s);
#else
    sprintf(s, "Create Dir: %sLig\\", ExecPath);
    //LogMessage(s, ML_DBG2);
    sprintf(s, "%sLog\\", ExecPath);
    CreateDirectory(s, NULL);
#endif
    SetLogPath(s);                                  // ���������, ���� ������ ����
#endif

    // ����������, ��� ����� ���� ������
    sprintf(s, "%sbizdb.sdb", ExecPath);
    ErrCod = -1;
    if (SetDBFileName(s)) {

        DB_GetDBContext();

        // �������� � ������� �������
        LogMessage("========================================================================================", ML_WRK1);
        LogMessage("starting BIZ service...", ML_WRK2);
        sprintf(s, "ExecPath: %s", ExecPath);
        LogMessage(s, ML_DBG1);

        // c������� �������� ����� 
        bMainThreadStopped = false;
        hMainThread = CreateThread(
            NULL,                                     /* no security attributes */
            0,                                        /* use default stack size */
            (LPTHREAD_START_ROUTINE)MainCycleThread, /* thread function */
            NULL,                                     /* argument to thread function */
            CREATE_SUSPENDED,                         /* creation flags */
            &dwThreadId );
        if (hMainThread) {
            ErrCod = -2;
            // ��������� ��������� �����
            if (ResumeThread(hMainThread) == 1)
                ErrCod = 1;
            else
                ErrCod = -3;
        }
    }
    return ErrCod;
}

//-----------------------------------------------------------------------------
//	������� ������� BIZ ������� 
//-----------------------------------------------------------------------------
void BCService_Stop()
{
    LogMessage("stoping BIZ service...", ML_WRK2);
    bMainThreadStopped = true;
}


