
#include "windows.h"
#include <stdio.h>
#include <time.h>
#include <sys/timeb.h>
//#include <stdlib.h>

#include "LogUtils.h"

char LogPath[256] = "";

//-----------------------------------------------------------------------------
// Устанавливает путь, куда будут писаться лог файлы
//-----------------------------------------------------------------------------
bool SetLogPath(const char * Path)
{
    if (!Path) return false;
    snprintf(LogPath, sizeof(LogPath)-1, "%s", Path);
    return true;
}

//-----------------------------------------------------------------------------
//  Записать лог сообщение в файл
//-----------------------------------------------------------------------------
int LogToFile(const char* Message, int Level)
{
    char fname[256];
    time_t t;
    tm*tm;
    timeb tb;

    time(&t);
    tm = localtime(&t);
    sprintf(fname, "%sBIZ%02d%02d%02d.log", LogPath, tm->tm_mday, tm->tm_mon + 1, tm->tm_year - 100);

    FILE *hfile = fopen(fname, "a");
    if (hfile == NULL) return -1;

    ftime(&tb);
    tm = localtime(&tb.time);

    fprintf(hfile, "\n%02d:%02d:%02d.%03d %s", tm->tm_hour, tm->tm_min, tm->tm_sec, tb.millitm, Message);

    fclose(hfile);
    return 1;
}

//-----------------------------------------------------------------------------
//  Записать лог сообщение
//-----------------------------------------------------------------------------
int LogMessage(const char* Message, int Level)
{
    LogToFile(Message, Level);
    return 1;
}

//-----------------------------------------------------------------------------
//  Записть лог сообщение от конкретного источника
//-----------------------------------------------------------------------------
int LogMessageProv(const char* Prov, const char* Message, int Level)
{
    char sm[1024];
    snprintf(sm, sizeof(sm), "%s:%s", Prov, Message);
    LogMessage(sm, Level);
    return 1;
}

