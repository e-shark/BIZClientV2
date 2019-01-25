
#include <Windows.h>
#include <stdio.h>
#include <time.h>

#include "main.h"
#include "LogUtils.h"
#include "BIZ_DB.h"
#include "BIZ_Client.h"
#include "BIZ_Parser.h"

char ExecPath[256]="";

HANDLE hMainThread = NULL;          // Handle to thread implementing MainForm cycle
bool bMainThreadStopped = false;    // set to true to stop Main thread
extern int PID;                        // PersonID - id ���� � ���� ������, ��� �������� ����������� ������



//-----------------------------------------------------------------------------
//  MAIN CYCLE
//-----------------------------------------------------------------------------
DWORD MainCycleThread(LPDWORD lpdwParam)
{
    char s[1024];
    clock_t tstart = clock();
    clock_t tnow;
    double ElapsedTime;
    tBIZ_Client *BIZClient;
    bool LoginOk;
    int SchedType;
    int SchedID;
    int ParamI1, ParamI2;
	std::string ParamStr;
    char *ParamBuf;
    int ParamBufLen = 16384;
    int ParamLen;

    LogMessage("BIZ service started.");

    ParamBuf = (char*)malloc(ParamBufLen);
    BIZClient = new tBIZ_Client();
    BIZClient->SetCookiesPath(ExecPath);
    //BIZClient->TOR_SetUp("127.0.0.1", 9170, 9171);
    //BIZClient->TOR_On();
    //BIZClient->ChangeEntryPoint();

    // ���� ������ ������, �������� �������� ������
    if (BIZClient && BIZClient->LoadPerson(PID)) {
        snprintf(s, sizeof(s), "������ %s ������� ������ %s ", BIZClient->ServerName.c_str(), BIZClient->UserLogin.c_str());
        LogMessage(s);
        BIZClient->ChangeEntryPoint();

		/*
		// ���������, ���� ����
		LoginOk = BIZClient->CheckLogin();
		if (!LoginOk)
			LoginOk = BIZClient->Login();
		*/

        //BIZClient->GetCompanyInfo();
        //BIZClient->SetGoodsPrice(1712842);
        //BIZClient->SetGoodsPrice(1901385);
        //BIZClient->AutoPurchaseGoods(1901385, 10);
        //BIZClient->SetGoodsPrice(1901385);
        //BIZClient->AutoPurchaseGoods(1915773, 10);
        //BIZClient->SetGoodsPrice(1901352);
        //BIZClient->AutoPurchaseForMarkets(3);
        //BIZClient->AutoPurchaseGoods(1901352, 7);
        //BIZClient->Test2();

	  //sprintf(s, "%sbizdb_home.sdb", ExecPath);
	  //BIZClient->Test3(s);



        // �������� ���� (�������� ���� �� ���������� ������)
        while (!bMainThreadStopped) {
            // �������� �������� ��������� ������ � ����������
            SchedID = DB_GetSheduleForCID(BIZClient->PersCID(), SchedType, ParamI1, ParamI2, ParamStr, ParamBuf, ParamBufLen, ParamLen);
            if (SchedID) {
                sprintf(s, "*** �������� ������� %d (type %d)", SchedID, SchedType);
                LogMessage(s, ML_WRK1);

                // ---------------------
                // !!!  �����������  !!!
                // �� ����� ����, ��� ���� ��������� ��������� ����� � ��������
                // ��������� ������, � �� ���������� �������, ���� ����� ��� �� �����������, ������ �
                // ---------------------

                // ---------------------
                // !!!  �����������  !!!
                // ����� ������� ���������� ������� ��������� ������� ����
                // � ���� ��� ������ � ����, ����������� ��� �������
                // ���� ������� �� �������, �� �� �������� ��������� ������� (�� ���������� ����� ������ ������� ����)
                // � ������������� �� ������� ����� ���������� ������� �������
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
                            BIZClient->SetGoodsPrice(ParamI1);
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

					case 10:            // - ������� ����� �����
						BIZClient->ChangeEntryPoint();
						break;

                    case 13:             // - ��������� ��������� -
                        break;

                    case 14:             // - �������� ������� ��� �� �������� � ������ -
                        break;

                    case 15:             // - ��������� �������� ���� -
                        BIZClient->SaveExchangeState();
                        break;

					case 16:             // - ������������� ������� ��������� �����
						//sprintf(s, "%sbizdb_home.sdb", ExecPath);
						BIZClient->ImportExchangeState(ParamStr);
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
    LogMessage("BIZ service Stoped.", ML_WRK1);
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
    int i;
    char ts[32];

    // ���������� ��������� �������
    /*��� ���� ��� ����������*/

    if (0 == PID) PID = 4;  // ��� �������, ���� �� ����� PID, ������ ���

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
    LogMessage(" ");
    LogMessage("========================================================================================", ML_WRK1);
    sprintf(s, "BIZ Service (build %s %s)", __DATE__, __TIME__);
    LogMessage(s);
    LogMessage("starting service...", ML_WRK2);

    sprintf(s, "ExecPath: %s", ExecPath);
    LogMessage(s, ML_DBG1);

    // ����������, ��� ����� ���� ������
    sprintf(s, "%sbizdb.sdb", ExecPath);
    ErrCod = -1;
    if (SetDBFileName(s)) {

        DB_GetDBContext();

        // �������� � ������� �������

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



