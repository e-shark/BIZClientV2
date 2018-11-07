
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <stdio.h>

#include "main.h"

char DB[256];
char SRVFullName[256];

char* ServiceName = "BIZ_Service";                                  // ��� �������
bool bConsole = false;                                              // ���� ������� �������

SERVICE_TABLE_ENTRY ServiceTable[1];
SERVICE_STATUS ServiceStatus;	                                      // ��������� ������� �������
SERVICE_STATUS_HANDLE hStatus;                                      // handle �-��� ���������� ��������

//-----------------------------------------------------------------------------
//	����� �������
//-----------------------------------------------------------------------------
int StartSrv(int argc, char **argv)
{
	return BCService_Start(argc, argv);
}

//---------------------------------------------------------------------------
//	������� �������
//---------------------------------------------------------------------------
void StopSrv()
{
	BCService_Stop();
}


//---------------------------------------------------------------------------
//	������� ��������� �������� SCM
//---------------------------------------------------------------------------
VOID WINAPI Handler(DWORD fdwControl)
{
    switch (fdwControl) {
    case SERVICE_CONTROL_STOP:
        ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
        SetServiceStatus(hStatus, &ServiceStatus);
        StopSrv();
        ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        SetServiceStatus(hStatus, &ServiceStatus);
        break;
    case SERVICE_CONTROL_PAUSE:
        break;
    case SERVICE_CONTROL_CONTINUE:
        break;
    case SERVICE_CONTROL_INTERROGATE:
        SetServiceStatus(hStatus, &ServiceStatus);
        break;
    case SERVICE_CONTROL_SHUTDOWN:
        ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
        SetServiceStatus(hStatus, &ServiceStatus);
        StopSrv();
        ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        SetServiceStatus(hStatus, &ServiceStatus);
        break;
    }
}


//-----------------------------------------------------------------------------
//	������������� �������
//-----------------------------------------------------------------------------
VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv)
{
    int error;

    ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;                //� ������ ���������� 1 ������
    ServiceStatus.dwCurrentState = SERVICE_STOPPED;                         //������� ��������� �������
    ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;  //����� ���������� �������� ����������
    ServiceStatus.dwWin32ExitCode = NO_ERROR;                                      //������� ������ �������� �������
    ServiceStatus.dwServiceSpecificExitCode = 0;                            //��� ������ �������� �������
    ServiceStatus.dwCheckPoint = 0;                                         //������� ���������� ������������ �������� �������
    ServiceStatus.dwWaitHint = 0;                                           //min. �������� ������������ �������� ������� � ��

    hStatus = RegisterServiceCtrlHandler(ServiceName, (LPHANDLER_FUNCTION)Handler);
    if (hStatus == (SERVICE_STATUS_HANDLE)0) {
        return;
    }
    ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    SetServiceStatus(hStatus, &ServiceStatus);
    error = StartSrv(argc, argv);
    if (error>0) {
        ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    }
    else {
        ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        ServiceStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
        ServiceStatus.dwServiceSpecificExitCode = error*(-1);
    }
    SetServiceStatus(hStatus, &ServiceStatus);
    return;
}

//-----------------------------------------------------------------------------
//  ������� ����������� �������
//-----------------------------------------------------------------------------
int ServiceInstall() {
    SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if (!hSCManager) {
        printf("Error: Can't open Service Control Manager\n");
        return -1;
    }
    printf("Create Service for %s\n", SRVFullName);
    SC_HANDLE hService = CreateService(
        hSCManager,
        ServiceName,
        ServiceName,
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_DEMAND_START,
        SERVICE_ERROR_NORMAL,
        SRVFullName,
        NULL, NULL, NULL, NULL, NULL
        );

    if (!hService) {
        int err = GetLastError();
        switch (err) {
        case ERROR_ACCESS_DENIED:
            printf("Error: ERROR_ACCESS_DENIED\n");
            break;
        case ERROR_CIRCULAR_DEPENDENCY:
            printf("Error: ERROR_CIRCULAR_DEPENDENCY\n");
            break;
        case ERROR_DUPLICATE_SERVICE_NAME:
            printf("Error: ERROR_DUPLICATE_SERVICE_NAME\n");
            break;
        case ERROR_INVALID_HANDLE:
            printf("Error: ERROR_INVALID_HANDLE\n");
            break;
        case ERROR_INVALID_NAME:
            printf("Error: ERROR_INVALID_NAME\n");
            break;
        case ERROR_INVALID_PARAMETER:
            printf("Error: ERROR_INVALID_PARAMETER\n");
            break;
        case ERROR_INVALID_SERVICE_ACCOUNT:
            printf("Error: ERROR_INVALID_SERVICE_ACCOUNT\n");
            break;
        case ERROR_SERVICE_EXISTS:
            printf("Error: ERROR_SERVICE_EXISTS\n");
            break;
        case ERROR_SERVICE_MARKED_FOR_DELETE:
            printf("Error: ERROR_SERVICE_MARKED_FOR_DELETE\n");
            break;
        default:
            printf("Error: Undefined (%d)\n", err);
        }
        CloseServiceHandle(hSCManager);
        return -1;
    }
    CloseServiceHandle(hService);

    CloseServiceHandle(hSCManager);
    printf("Success install service!\n");
    return 0;
}

//-----------------------------------------------------------------------------
//  �������� �������
//-----------------------------------------------------------------------------
int ServiceRemove() {
    int err;
    SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!hSCManager) {
        printf("Error: Can't open Service Control Manager\n");
        return -1;
    }
    SC_HANDLE hService = OpenService(hSCManager, ServiceName, SERVICE_STOP | DELETE);
    if (!hService) {
        printf("Error: Can't remove service\n");
        CloseServiceHandle(hSCManager);
        return -1;
    }

    if (DeleteService(hService))
        printf("Success remove service!\n");
    else 
        printf("DeleteService error: %d\n", GetLastError());

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);
    return 0;
}

//-----------------------------------------------------------------------------
//  ������ �������
//-----------------------------------------------------------------------------
int ServiceStart() {
    SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    SC_HANDLE hService = OpenService(hSCManager, ServiceName, SERVICE_START);
    if (!StartService(hService, 0, NULL)) {
        CloseServiceHandle(hSCManager);
        printf("Error: Can't start service\n");
        return -1;
    }

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);
    return 0;
}

//-----------------------------------------------------------------------------
//  �������� ���������
//-----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	int i;
	char *pC;

      if (GetModuleFileName(NULL, SRVFullName, sizeof(SRVFullName)) == 0)
        memset(SRVFullName, 0, sizeof(SRVFullName));

	// ���������� ��������� �������
	DB[0] = 0;
	for (i = 0; i<argc; i++) if (strstr(argv[i], "DB=") != NULL) {
		strncpy(DB, argv[i] + 3, sizeof(DB));
		break;
	}
      if (!DB[0])
          sprintf(DB, "bizdb.sdb");


      for (i = 0; i < argc; i++) if (strstr(strupr(argv[i]), "/C") != NULL) {
          bConsole = true;
          break;
      }

      for (i = 0; i<argc; i++) if (strstr(strupr(argv[i]), "INSTALL") != NULL) {
          ServiceInstall();
          return 0;
      }

      for (i = 0; i<argc; i++) if (strstr(strupr(argv[i]), "REMOVE") != NULL) {
          ServiceRemove();
          return 0;
      }
      
      for (i = 0; i<argc; i++) if (strstr(strupr(argv[i]), "START") != NULL) {
          ServiceStart();
          return 0;
      }

	// �������� ������ (��� �������)
	if (bConsole) {                    //---------------------  Console mode
		i = StartSrv(argc, argv);
		if (i > 0) {
			printf("BIZClientServer is started in console mode. \n");
                  printf("Press any key to shut down...");
                  getchar();
			printf("Attempting to stop BIZClientServer. Please wait...\n");
			StopSrv();
			printf("BIZClientServer stopped OK!   Press any key...\n");
			//getchar();
		}
		else {
			printf("Error starting BIZClientServer: %d.  Press any key...\n", i);
			getchar();
		}
	}
    else {                             //---------------------  Service mode
        ServiceTable[0].lpServiceName = ServiceName;
        //ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;
        ServiceTable[0].lpServiceProc = ServiceMain;

        i = StartServiceCtrlDispatcher(ServiceTable);
        return i;
    }

	return 0;
}


