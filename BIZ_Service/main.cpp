
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
extern int PID;                        // PersonID - id юзер в базе данных, для которого запускается сервис



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

    // если клиент создан, пытаемся получить учетку
    if (BIZClient && BIZClient->LoadPerson(PID)) {
        snprintf(s, sizeof(s), "сервер %s учетная запись %s ", BIZClient->ServerName.c_str(), BIZClient->UserLogin.c_str());
        LogMessage(s);
        BIZClient->ChangeEntryPoint();

		/*
		// логинимся, если надо
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



        // основной цикл (крутится пока не остановить сервис)
        while (!bMainThreadStopped) {
            // пытаемся получить следующую задачу к выполнению
            SchedID = DB_GetSheduleForCID(BIZClient->PersCID(), SchedType, ParamI1, ParamI2, ParamStr, ParamBuf, ParamBufLen, ParamLen);
            if (SchedID) {
                sprintf(s, "*** Выполняю задание %d (type %d)", SchedID, SchedType);
                LogMessage(s, ML_WRK1);

                // ---------------------
                // !!!  Предложение  !!!
                // На самом деле, тут надо создавать отдельную нитку с заданием
                // запускать таймер, и по прошествии таймера, если нитка еще не завершилась, килять её
                // ---------------------

                // ---------------------
                // !!!  Предложение  !!!
                // Перед началом выполнения задания проверить наличие сети
                // и если нет выхода в инет, попробовать его поднять
                // Если поднять не удалось, то не пытаться выполнять задание (бо бесполезно кроме задачи слияния базы)
                // и следовательно не двигать метку последнего запуска задания
                // ---------------------

                try {
                    //tstart = clock();
                    DB_SetScheduleLastRun(SchedID);
                    //sprintf(s, "на запрос DB_SetScheduleLastRun затрачено %f сек", (double)(clock() - tstart) / CLOCKS_PER_SEC);
                    //LogMessage(s, ML_DBG3);

                    switch (SchedType) {
                    case 1:             // - получить ежедневный бонус -
                        BIZClient->GetDailyBonus();
                        break;

                    case 2:             // - произвести автозакупки для магазинов
                        // Если задан ParamI1, то это указан ID магазина, по которому надо делать закупку
                        // если не задан (или =0), то автозакупка для всех магащинов
                        // ParamI2 - на сколько дней закупать
                        if (ParamI1) {
                            BIZClient->AutoPurchaseForMarket(ParamI1, ParamI2);
                        }else
                            BIZClient->AutoPurchaseForAllMarkets(ParamI2);
                        break;

                    case 3:             // - выставить цены для магазинов -
                        if (ParamI1) {
                            BIZClient->SetGoodsPrice(ParamI1);
                        }
                        else
                            BIZClient->SetGoodsPriceForAllMarkets();
                        break;

                    case 5:             // - выставить цены на общественных складах -
                        break;

                    case 6:             // - слить избыток продуктов производства на общественный склад -
                        break;

                    case 7:             // - попытаться купить строительные проект -
                        break;

                    case 8:            // - сохранить статистику по магазину -
                        break;

                    case 9:            // - Обновить иныормацию о компании -
                        BIZClient->GetCompanyInfo();
                        // получить залогининую страницу компании
                        // распарсить и записать в базу структуру компании
                        // распарсить статистику компании
                        // записать статичтику в историю
                        // удалить на глубину старую статистику
                        break;

					case 10:            // - сменить точку входа
						BIZClient->ChangeEntryPoint();
						break;

                    case 13:             // - прочитать сообщения -
                        break;

                    case 14:             // - записать историю цен на продукты в городе -
                        break;

                    case 15:             // - прочитать обменный курс -
                        BIZClient->SaveExchangeState();
                        break;

					case 16:             // - импортирувать историю обменного курса
						//sprintf(s, "%sbizdb_home.sdb", ExecPath);
						BIZClient->ImportExchangeState(ParamStr);
						break;


                    default:
                        Sleep(1000);    // если не Выполняли никакие задания
                    }   //switch
                } //try
                catch (...) {
                    LogMessage("Uexpected Exception !", ML_ERR1);
                    Sleep(1000); 
                }
                sprintf(s, "*** завершение задания %d (type %d)", SchedID, SchedType);
                LogMessage(s, ML_WRK1);

            }else Sleep(1000);      // если не было никаких заданий
        }   //основной while

    }
    else {
        LogMessage("Error! Can't create BIZ Client.", ML_ERR1);
    }

    free(ParamBuf);
    LogMessage("BIZ service Stoped.", ML_WRK1);
    return 0;
}


//-----------------------------------------------------------------------------
//	Старт сервиса BIZ клиента 
//	Возвращаемое значение:
//		>0 - сервис запущен нормально
//		<=0 - ошибка (0 - код ошибки)
//-----------------------------------------------------------------------------
int BCService_Start(int argc, char **argv)
{
    char s[512];
    DWORD dwThreadId;
    int ErrCod = 0;
    char *pC;
    int i;
    char ts[32];

    // Определяем параметры запуска
    /*тут пока нет параметров*/

    if (0 == PID) PID = 4;  // для отладки, если не задан PID, задаем его

    // Определяем путь к запускаемому файлу 
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

    // Задаем куда складывать логи
    SetLogPath(ExecPath);                           // Указываем, куда писать логи

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
    SetLogPath(s);                                  // Указываем, куда писать логи
#endif
    LogMessage(" ");
    LogMessage("========================================================================================", ML_WRK1);
    sprintf(s, "BIZ Service (build %s %s)", __DATE__, __TIME__);
    LogMessage(s);
    LogMessage("starting service...", ML_WRK2);

    sprintf(s, "ExecPath: %s", ExecPath);
    LogMessage(s, ML_DBG1);

    // ЗУказываем, где лежит база данных
    sprintf(s, "%sbizdb.sdb", ExecPath);
    ErrCod = -1;
    if (SetDBFileName(s)) {

        DB_GetDBContext();

        // Сообщаем о запуске сервиса

        // cсоздаем основную нитку 
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
            // запускаем созданную нитку
            if (ResumeThread(hMainThread) == 1)
                ErrCod = 1;
            else
                ErrCod = -3;
        }
    }
    return ErrCod;
}

//-----------------------------------------------------------------------------
//	Останов сервиса BIZ клиента 
//-----------------------------------------------------------------------------
void BCService_Stop()
{
    LogMessage("stoping BIZ service...", ML_WRK2);
    bMainThreadStopped = true;
}



