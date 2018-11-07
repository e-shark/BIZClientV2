#ifndef LOGUTILS_H
#define LOGUTILS_H

#define LogInSingleDir

#define ML_WRK1     0x0001      // Основные этапы рабочего алгоритма
#define ML_WRK2     0x0002      // Шаги основных этапов
#define ML_WRK3     0x0004      // Подробности алгоритма

#define ML_ERR1     0x0010      // Критические ошибки
#define ML_ERR2     0x0020      // Неритические ошибки
#define ML_ERR3     0x0040      // Мелкие "спотыкания"

#define ML_DBG1     0x0100      // Основные этапы отладки
#define ML_DBG2     0x0200      // Доплнительная информация
#define ML_DBG3     0x0400      // Специальная отладочная информация

bool SetLogPath(const char * Path);

int LogMessage(const char* Message, int Level = ML_DBG1);
int LogMessageProv(const char* Prov, const char* Message, int Level);


#endif

