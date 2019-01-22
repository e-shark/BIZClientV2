
#include "BIZ_DB.h"
#include "BIZ_Exceptions.h"
#include "BIZ_Types.h"
#include "LogUtils.h"

char DBString[256] = "bizdb.sdb";


//-----------------------------------------------------------------------------
//  Перевод строки из cp2151 в UTF8
//-----------------------------------------------------------------------------
static void cp1251_to_utf8(char *out, const char *in) {
    static const int table[128] = {
        0x82D0,0x83D0,0x9A80E2,0x93D1,0x9E80E2,0xA680E2,0xA080E2,0xA180E2,
        0xAC82E2,0xB080E2,0x89D0,0xB980E2,0x8AD0,0x8CD0,0x8BD0,0x8FD0,
        0x92D1,0x9880E2,0x9980E2,0x9C80E2,0x9D80E2,0xA280E2,0x9380E2,0x9480E2,
        0,0xA284E2,0x99D1,0xBA80E2,0x9AD1,0x9CD1,0x9BD1,0x9FD1,
        0xA0C2,0x8ED0,0x9ED1,0x88D0,0xA4C2,0x90D2,0xA6C2,0xA7C2,
        0x81D0,0xA9C2,0x84D0,0xABC2,0xACC2,0xADC2,0xAEC2,0x87D0,
        0xB0C2,0xB1C2,0x86D0,0x96D1,0x91D2,0xB5C2,0xB6C2,0xB7C2,
        0x91D1,0x9684E2,0x94D1,0xBBC2,0x98D1,0x85D0,0x95D1,0x97D1,
        0x90D0,0x91D0,0x92D0,0x93D0,0x94D0,0x95D0,0x96D0,0x97D0,
        0x98D0,0x99D0,0x9AD0,0x9BD0,0x9CD0,0x9DD0,0x9ED0,0x9FD0,
        0xA0D0,0xA1D0,0xA2D0,0xA3D0,0xA4D0,0xA5D0,0xA6D0,0xA7D0,
        0xA8D0,0xA9D0,0xAAD0,0xABD0,0xACD0,0xADD0,0xAED0,0xAFD0,
        0xB0D0,0xB1D0,0xB2D0,0xB3D0,0xB4D0,0xB5D0,0xB6D0,0xB7D0,
        0xB8D0,0xB9D0,0xBAD0,0xBBD0,0xBCD0,0xBDD0,0xBED0,0xBFD0,
        0x80D1,0x81D1,0x82D1,0x83D1,0x84D1,0x85D1,0x86D1,0x87D1,
        0x88D1,0x89D1,0x8AD1,0x8BD1,0x8CD1,0x8DD1,0x8ED1,0x8FD1
    };
    while (*in)
        if (*in & 0x80) {
            int v = table[(int)(0x7f & *in++)];
            if (!v)
                continue;
            *out++ = (char)v;
            *out++ = (char)(v >> 8);
            if (v >>= 16)
                *out++ = (char)v;
        }
        else
            *out++ = *in++;
    *out = 0;
}

//-----------------------------------------------------------------------------
//  Устанавливае имя файла БД и проверяет его наличие
//-----------------------------------------------------------------------------
bool SetDBFileName(const char * FileName)
{
    char s[1024];
    bool res = false;
    FILE *in;

    // копируем имя файла
    if (FileName) 
        snprintf(DBString, sizeof(DBString), FileName);
    snprintf(s, sizeof(s), "SetDBFileName : %s ", FileName);
    LogMessage(s, ML_DBG1);

    // проверяем наличие файла БД
    if ( (in = fopen(DBString, "rt")) != NULL) {
        res = true;
        fclose(in);
    } else {
        snprintf(s, sizeof(s), "File DB no exists (%s) ", FileName);
        LogMessage(s, ML_ERR2);
        res = false;
    }

    return res;
}

//-----------------------------------------------------------------------------
//  Открытие контекста БД
//  По окончании работы с Бд при любом результате
//  обязательно выполнить DB_CloseDBC
//-----------------------------------------------------------------------------
DB_DBC *DB_GetDBContextEx(void) {
    DB_DBC *DBC = NULL;
    if (sqlite3_open(DBString, &DBC) == SQLITE_OK) {
        sqlite3_busy_timeout(DBC, 5000);
    }
    else {
        char msg[1024];
        int ercod;
        ercod = sqlite3_errcode(DBC);
        snprintf(msg, sizeof(msg) - 1, "Не удалось открыть соединение с БД.( %d: %s.)", ercod, sqlite3_errmsg(DBC));
        sqlite3_close(DBC);
        throw EDBException(msg);
    }

    return DBC;
}

DB_DBC *DB_GetDBContext(void) {
    DB_DBC *DBC = NULL;
    try {
        DBC = DB_GetDBContextEx();
    }
    catch (EDBException &e) {
        LogMessage(e.Message.c_str(), ML_ERR2);
    }
    return DBC;
}

//-----------------------------------------------------------------------------
//  Закрытие контекста БД
//-----------------------------------------------------------------------------
void DB_CloseDBC(DB_DBC* DBC) {
    sqlite3_close(DBC);
}

//-----------------------------------------------------------------------------
//  Получить данные персонажа по ID
//-----------------------------------------------------------------------------
int DB_GetPersonEx(int id, std::string &srv, std::string &login, std::string &psw, int &ProxyList, int &company, std::string &tor_IP, int &tor_port, int &tor_cmdport)
{
    int res = 0;
    DB_DBC *DBC = DB_GetDBContext();
    sqlite3_stmt *stmt;

    if (DBC) {
        if (sqlite3_prepare_v2(DBC, "select Server, User, Psw, ProxyList, Company, TOR_IP, TOR_PORT, TOR_cmdPORT from Person where id = ?1 limit 1; ", -1, &stmt, 0) != SQLITE_OK) goto DB_GetPrsnErr;
        if (sqlite3_bind_int(stmt, 1, id) != SQLITE_OK)  goto DB_GetPrsnErr;
        if (SQLITE_ROW == sqlite3_step(stmt)) {
            srv = (char*) sqlite3_column_text(stmt, 0);
            login = (char*)sqlite3_column_text(stmt, 1);
            psw = (char*)sqlite3_column_text(stmt, 2);
            ProxyList = sqlite3_column_int(stmt, 3);
            company = sqlite3_column_int(stmt, 4);
            tor_IP = (char*)sqlite3_column_text(stmt, 5);
            tor_port = sqlite3_column_int(stmt, 6);
            tor_cmdport = sqlite3_column_int(stmt, 7);
            res = company;
        } else res = 0;
        sqlite3_finalize(stmt);
    }
    DB_CloseDBC(DBC);
    return res;

DB_GetPrsnErr:
    char msg[1024];
    int ercod;
    ercod = sqlite3_errcode(DBC);
    snprintf(msg, sizeof(msg) - 1, "DB_GetPerson Error.(%d: %s)", ercod, sqlite3_errmsg(DBC));
    sqlite3_finalize(stmt);
    sqlite3_close(DBC);
    throw EDBException(msg);
}

int DB_GetPerson(int id, std::string &srv, std::string &login, std::string &psw, int &ProxyList, int &company, std::string &tor_IP, int &tor_port, int &tor_cmdport)
{
    int res = 0;
    try {
        res = DB_GetPersonEx(id, srv, login, psw, ProxyList, company, tor_IP, tor_port, tor_cmdport);
    } catch (EDBException &e) {
        LogMessage(e.Message.c_str(), ML_ERR2);
    }
    return res;
}

//-----------------------------------------------------------------------------
//  Получить очередное задание для компании CID
//  Возвращает ID задания. А так-же:
//  type - тип задания
//  в буфер Param параметры задания (двоичная структура данных)
//  ParamLen - размер данных в параметре (не более ParamBufLen)
//-----------------------------------------------------------------------------
int DB_GetSheduleEx(int cid, int &type, int &ParamI1, int &ParamI2, std::string &ParamStr, void *ParamBin, int ParamBinBufLen, int &ParamBinLen)
{
    int res = 0;
    DB_DBC *DBC = DB_GetDBContext();
    sqlite3_stmt *stmt;
    int id;
    int n;
	char *pC;
    char select[2024] =
        "select ID , Type, ParamI1, ParamI2, ParamStr, ParamBin "
        "from Schedule "
        "where ( CID = ?1 ) "
        "  and ( Disabled=0 or Disabled is null ) "
        "  and ( Start <= datetime('now', 'localtime') ) "
        "  and ( (datetime(LastRun, '+'||Period||' minutes') <= datetime('now', 'localtime')) or LastRun is null ) "
        "order by LastRun "
        "limit 1; ";

    if (DBC) {
        if (sqlite3_prepare_v2(DBC, select, -1, &stmt, 0) != SQLITE_OK) goto DB_GetSchedErr;
        if (sqlite3_bind_int(stmt, 1, cid) != SQLITE_OK)  goto DB_GetSchedErr;
        if (SQLITE_ROW == sqlite3_step(stmt)) {
            res = sqlite3_column_int(stmt, 0);
            type = sqlite3_column_int(stmt, 1);
            ParamI1 = sqlite3_column_int(stmt, 2);
            ParamI2 = sqlite3_column_int(stmt, 3);
			pC = (char*)sqlite3_column_text(stmt, 4);
			if (pC) ParamStr = pC;
			else ParamStr = "";
            n = sqlite3_column_bytes(stmt, 5);
            if ((ParamBin) && (ParamBinBufLen)) {
                if (n > ParamBinBufLen) n = ParamBinBufLen;
                memcpy(ParamBin, sqlite3_column_blob(stmt, 5), n);
                ParamBinLen = n;
            }
            else ParamBinLen = 0;
        }
        else res = 0;
        sqlite3_finalize(stmt);
    }
    DB_CloseDBC(DBC);
    return res;

DB_GetSchedErr:
    char msg[1024];
    int ercod;
    ercod = sqlite3_errcode(DBC);
    snprintf(msg, sizeof(msg) - 1, "DB_GetShedule Error.(%d: %s)", ercod, sqlite3_errmsg(DBC));
    sqlite3_finalize(stmt);
    sqlite3_close(DBC);
    throw EDBException(msg);

}

int DB_GetShedule(int cid, int &type, int &ParamI1, int &ParamI2, std::string &ParamStr, void *ParamBin, int ParamBinBufLen, int &ParamBinLen)
{
    int res = 0;
    try {
        res = DB_GetSheduleEx(cid, type, ParamI1, ParamI2, ParamStr, ParamBin, ParamBinBufLen, ParamBinLen);
    }
    catch (EDBException &e) {
        LogMessage(e.Message.c_str(), ML_ERR2);
    }
    return res;
}

//-----------------------------------------------------------------------------
//  Записать новое задание
//-----------------------------------------------------------------------------
int DB_SetSheduleEx(int cid, int type, time_t start, int period, void *Param, int ParamBufLen, char* Desription)
{
    int res = 0;
    DB_DBC *DBC = DB_GetDBContext();
    sqlite3_stmt *stmt;
    int n;
    char select[2024];
    if (start)
        sprintf(select,"insert into Schedule (CID, Type, Desription, ParamI1, ParamI2, ParamBin, Period, Start ) values (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8); ");
    else
        sprintf(select, "insert into Schedule (CID, Type, Desription, ParamI1, ParamI2, ParamBin, Period, Start ) values (?1, ?2, ?3, ?4, ?5, ?6, ?7, datetime('now', 'localtime')); ");

    if (DBC) {
        if (sqlite3_prepare_v2(DBC, select, -1, &stmt, 0) != SQLITE_OK) goto DB_SetSchedErr;
        if (sqlite3_bind_int(stmt, 1, cid) != SQLITE_OK) goto DB_SetSchedErr;
        if (sqlite3_bind_int(stmt, 2, type) != SQLITE_OK) goto DB_SetSchedErr;
        if (sqlite3_bind_text(stmt, 3, Desription, -1, NULL) != SQLITE_OK) goto DB_SetSchedErr;
        if (sqlite3_bind_int(stmt, 4, type) != SQLITE_OK) goto DB_SetSchedErr;
        if (sqlite3_bind_int(stmt, 5, type) != SQLITE_OK) goto DB_SetSchedErr;
        if (sqlite3_bind_blob(stmt, 6, Param, ParamBufLen, NULL) != SQLITE_OK) goto DB_SetSchedErr;
        if (sqlite3_bind_int(stmt, 7, period) != SQLITE_OK) goto DB_SetSchedErr;
        if (start)
            if (sqlite3_bind_int(stmt, 8, start) != SQLITE_OK) goto DB_SetSchedErr;
        if (SQLITE_DONE != sqlite3_step(stmt))  goto DB_SetSchedErr;
        res = sqlite3_last_insert_rowid(DBC);       // Получаем ID созданной записи
        sqlite3_finalize(stmt);
    }
    DB_CloseDBC(DBC);
    return res;

DB_SetSchedErr:
    char msg[1024];
    int ercod;
    ercod = sqlite3_errcode(DBC);
    snprintf(msg, sizeof(msg) - 1, "DB_SetShedule Error.(%d: %s)", ercod, sqlite3_errmsg(DBC));
    sqlite3_finalize(stmt);
    sqlite3_close(DBC);
    throw EDBException(msg);
}

int DB_SetShedule(int cid, int type, time_t start, int period, void *Param, int ParamBufLen, char* Desription)
{
    int res = 0;
    try {
        res = DB_SetSheduleEx(cid, type, start, period, Param, ParamBufLen, Desription);
    }
    catch (EDBException &e) {
        LogMessage(e.Message.c_str(), ML_ERR2);
    }
    return res;
}

//-----------------------------------------------------------------------------
//  Записать время последнего запуска задания
//-----------------------------------------------------------------------------
bool DB_SetScheduleLastRunEx(int SchedID)
{
    bool res = false;
    DB_DBC *DBC = DB_GetDBContext();
    sqlite3_stmt *stmt;
    char select[2024];

    if (DBC) {
        if (sqlite3_prepare_v2(DBC, "UPDATE  Schedule Set LastRun = datetime('now', 'localtime') where ID = ?1; ", -1, &stmt, 0) != SQLITE_OK) goto DB_SetSchedLastRunErr;
        if (sqlite3_bind_int(stmt, 1, SchedID) != SQLITE_OK) goto DB_SetSchedLastRunErr;
        if (SQLITE_DONE != sqlite3_step(stmt))  goto DB_SetSchedLastRunErr;
        sqlite3_finalize(stmt);
        res = true;
    }
    DB_CloseDBC(DBC);
    return res;

DB_SetSchedLastRunErr:
    char msg[1024];
    int ercod;
    ercod = sqlite3_errcode(DBC);
    snprintf(msg, sizeof(msg) - 1, "DB_SetScheduleLastRun Error.(%d: %s)", ercod, sqlite3_errmsg(DBC));
    sqlite3_finalize(stmt);
    sqlite3_close(DBC);
    throw EDBException(msg);
}

bool DB_SetScheduleLastRun(int SchedID)
{
    int res = 0;
    try {
        res = DB_SetScheduleLastRunEx(SchedID);
    }
    catch (EDBException &e) {
        LogMessage(e.Message.c_str(), ML_ERR2);
    }
    return res;
}


//-----------------------------------------------------------------------------
//  Записать текущий курс на бирже ИО
//-----------------------------------------------------------------------------
bool DB_SaveExchangeStateEx(double PurchasePrice, double SellingPrice, const sSrvTime *STime)
{
    int res = false;
    DB_DBC *DBC = DB_GetDBContext();
    sqlite3_stmt *stmt;
    char s[32];

    if (DBC) {
        if (sqlite3_prepare_v2(DBC, "insert into ExchangeState(PurchasePrice, SellingPrice, TimeStamp, ServerTime, ServerTimeStr) values( ?1, ?2, datetime('now', 'localtime'), ?3, ?4 ); ", -1, &stmt, 0) != SQLITE_OK) goto DB_SaveExchStateErr;
        if (SQLITE_OK != sqlite3_bind_double(stmt, 1, PurchasePrice)) goto DB_SaveExchStateErr;
        if (SQLITE_OK != sqlite3_bind_double(stmt, 2, SellingPrice)) goto DB_SaveExchStateErr;
        int n = sizeof(sSrvTime);
        if (sqlite3_bind_blob(stmt, 3, STime, n, NULL) != SQLITE_OK) goto DB_SaveExchStateErr;
        sprintf(s, "%d.%02d.00%02d", STime->SrvDay, STime->SrvMonth, STime->SrvYear);
        if (sqlite3_bind_text(stmt, 4, s, -1, NULL) != SQLITE_OK) goto DB_SaveExchStateErr;
        if (SQLITE_DONE != sqlite3_step(stmt))  goto DB_SaveExchStateErr;
        sqlite3_finalize(stmt);
        res = true;
    }
    DB_CloseDBC(DBC);
    return res;

DB_SaveExchStateErr:
    char msg[1024];
    int ercod;
    ercod = sqlite3_errcode(DBC);
    snprintf(msg, sizeof(msg) - 1, "DB_SaveExchangeState Error.(%d: %s)", ercod, sqlite3_errmsg(DBC));
    sqlite3_finalize(stmt);
    sqlite3_close(DBC);
    throw EDBException(msg);
}

bool DB_SaveExchangeState(double PurchasePrice, double SellingPrice, const sSrvTime *STime)
{
    int res = false;
    try {
        res = DB_SaveExchangeStateEx(PurchasePrice, SellingPrice, STime);
    }
    catch (EDBException &e) {
        LogMessage(e.Message.c_str(), ML_ERR2);
    }
    return res;
}

//-----------------------------------------------------------------------------
//  
//-----------------------------------------------------------------------------
bool DB_SaveUnitsListEx(int CID, tmUnits *UnitsList)
{
    int res = false;
    DB_DBC *DBC = DB_GetDBContext();
    sqlite3_stmt *stmt;
    char s[32];
    char bufName[2048];


    if (DBC) {
// !!! ... тут надо будет  обернуть все в BEGIN ... COMMIT?RESTORE
        
        // Сначала удаляем старый список подразделений
        if (sqlite3_prepare_v2(DBC, "delete from Units where Company = ?1; ", -1, &stmt, 0) != SQLITE_OK) goto DB_SaveUnitListErr;
        if (sqlite3_bind_int(stmt, 1, CID) != SQLITE_OK) goto DB_SaveUnitListErr;
        if (SQLITE_DONE != sqlite3_step(stmt))  goto DB_SaveUnitListErr;
        sqlite3_finalize(stmt);

        for (tmUnits::iterator iUL = UnitsList->begin(); iUL != UnitsList->end(); iUL++) {
            //                                               1    2     3     4     5      6         NULL       7
            if (sqlite3_prepare_v2(DBC, "insert into Units (UID, PID, Type, City, Name, Company, Corporation, Image) values (?1, ?2, ?3, ?4, ?5, ?6, NULL, ?7); ", -1, &stmt, 0) != SQLITE_OK) goto DB_SaveUnitListErr;
            if (sqlite3_bind_int(stmt, 1, (*iUL).second->ID) != SQLITE_OK) goto DB_SaveUnitListErr;
            if (sqlite3_bind_int(stmt, 2, CID) != SQLITE_OK) goto DB_SaveUnitListErr;
            if (sqlite3_bind_int(stmt, 3, (*iUL).second->Type) != SQLITE_OK) goto DB_SaveUnitListErr;
            if (sqlite3_bind_int(stmt, 4, (*iUL).second->CityID) != SQLITE_OK) goto DB_SaveUnitListErr;
            cp1251_to_utf8(bufName, (*iUL).second->Name.c_str());
            if (sqlite3_bind_text(stmt, 5, bufName, -1, NULL) != SQLITE_OK) goto DB_SaveUnitListErr;
            if (sqlite3_bind_int(stmt, 6, CID) != SQLITE_OK) goto DB_SaveUnitListErr;
            if (sqlite3_bind_text(stmt, 7, (*iUL).second->Image.c_str(), -1, NULL) != SQLITE_OK) goto DB_SaveUnitListErr;
            if (SQLITE_DONE != sqlite3_step(stmt))  goto DB_SaveUnitListErr;
            sqlite3_finalize(stmt);
        }

    }
    DB_CloseDBC(DBC);
    return res;

DB_SaveUnitListErr:
    char msg[1024];
    int ercod;
    ercod = sqlite3_errcode(DBC);
    snprintf(msg, sizeof(msg) - 1, "DB_SaveExchangeState Error.(%d: %s)", ercod, sqlite3_errmsg(DBC));
    sqlite3_finalize(stmt);
    sqlite3_close(DBC);
    throw EDBException(msg);
}

bool DB_SaveUnitsList(int CID, tmUnits *UnitsList)
{
    int res = false;
    try {
        res = DB_SaveUnitsListEx(CID, UnitsList);
    }
    catch (EDBException &e) {
        LogMessage(e.Message.c_str(), ML_ERR2);
    }
    return res;
}




//-----------------------------------------------------------------------------
//	Экспорт обменного курса ИО из другой базы.
//	Берет из указанной базы значение обменного курса с метко й времени, которых нет в текущей базе.
//	Таким образом выполняется странение "дырок" в таблице обменного курса.
//-----------------------------------------------------------------------------
bool DB_ExternExchangeSateEx(std::string BDName)
{
	int res = false;
	DB_DBC *DBC = DB_GetDBContext();
	std::string sql;
	int sqlres;
	char *errStr;

	if (DBC) {
		sql = "ATTACH '";																			// Присоединяем другую базу
		sql += BDName;
		sql += "'AS zB2 ; ";
		sql += "DROP TABLE IF EXISTS tmp1 ; ";														// На всякий случай дропаем временную таблицу, если она почему-то уже есть
		sql += "CREATE TABLE IF NOT EXISTS tmp1 AS SELECT * FROM main.ExchangeState LIMIT 1; ";		// Создаем временную таблицу по образу и подобию существующей таблицы обменного курса (пока для этого дергаем одну запись, но нужно будет сделать создание только структуры без наполнения)
		sql += "DELETE FROM tmp1; ";																// Очищаем временную таблицу
		sql += "INSERT INTO tmp1 (PurchasePrice, SellingPrice, TimeStamp, ServerTime, ServerTimeStr) SELECT PurchasePrice, SellingPrice, TimeStamp, ServerTime, ServerTimeStr FROM zB2.ExchangeState WHERE TimeStamp NOT IN(SELECT TimeStamp FROM main.ExchangeState); ";	// Инсертим во временную таблицу записи из внешней базы, которых нет у нас в нашей базе
		sql += "INSERT INTO tmp1 (PurchasePrice, SellingPrice, TimeStamp, ServerTime, ServerTimeStr) SELECT PurchasePrice, SellingPrice, TimeStamp, ServerTime, ServerTimeStr FROM main.ExchangeState; ";																	// Добавляем во временную таблицу записи, которые есть у нас в базе
		sql += "DELETE FROM main.ExchangeState; ";													// Очищаем нашу таблицу курса
		sql += "INSERT INTO main.ExchangeState (PurchasePrice, SellingPrice, TimeStamp, ServerTime, ServerTimeStr) SELECT PurchasePrice, SellingPrice, TimeStamp, ServerTime, ServerTimeStr FROM tmp1 ORDER BY TimeStamp; "; // Вставляем в нашу таблицу курса данные из временной таблицы
		sql += "DROP TABLE IF EXISTS tmp1; ";														// Убираем за собой
		sql += "DETACH zB2; ";																		// Отсоединчем другую базу

		sqlres = sqlite3_exec(DBC,sql.c_str(),NULL,NULL,&errStr);
		//if (0 != sqlres) {
		if (NULL != errStr) {
			char msg[1024];
			snprintf(msg, sizeof(msg) - 1, "ExternExchangeSate Error.(%d: %s)", sqlres, errStr);
			sqlite3_close(DBC);
			throw EDBException(msg);
		}
		res = true;
	}
	sqlite3_close(DBC);
	return res;

}

bool DB_ExternExchangeSate(std::string BDName)
{
	bool res = false;
	try {
		res = DB_ExternExchangeSateEx(BDName);
	}
	catch (EDBException &e) {
		LogMessage(e.Message.c_str(), ML_ERR2);
	}
	return res;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
std::string DB_GetFactorLocationTypeStr(int LocationType)
{
    switch (LocationType) {
    case 1: return "для юнита"; break;
    case 2: return "для города"; break;
    case 3: return "для региона"; break;
    case 4: return "для страны"; break;
    case 5: return "для для компании"; break;
    case 0:
    default:
        return "локация неизвестна";
    }
}

//-----------------------------------------------------------------------------
//  Найти значение коэффициента
// Типы коэффициентов
//      1 - Price                               // Цена продукта (товара)
//      2 - Price Dumping = -5;                 // На сколько процентов больше нужно выставить цену продуктов при корректироваке цены (-99% .. +99%)
//      3 - Price Permitted Excess = 1.0;       // На сколько процентов цена на продукт может превышать среднюю цену по городу (-99% .. +99%)
//      4 - Quality                             // Тредуемое качество для продукта
//      5 - Quality Dumping = 2;                // На сколько процентов лучше по качеству продукты нужно пытаться закупать (-99% .. +99%)
//      6 - Quality Minimum                     // Минимальное допустимое качество для магазина
//      7 - Quality Maximum                     // Максимальное допустимое качество для магазина
// Виды LocationType
//      1 - для магазина
//      2 - для города
//      3 - для региона
//      4 - для страны
//      5 - для всей компании
//      0 - коэффициент не найден
//-----------------------------------------------------------------------------
int DB_GetFactorEx(int FactorType, int ProductID, int ShopID, int &resLevel , int &resFactor)
{
    int res = 0;
    DB_DBC *DBC = DB_GetDBContext();
    sqlite3_stmt *stmt;
    int LocationType;
    int Location;
    int Value;
    int Region = 0;     // Не стал задействовать пока регион, поэтому он не в параметрах, а здесь
    int Country = 0;    // В дальнейшем нужна таблица географии, все это можно будет вытащить по городу
    char select[2024] =
        "SELECT LocationType, Location, Value FROM Factors "
        "WHERE Type = ?1 AND Object = ?2 "
        "AND( "
          "    (LocationType = 1 AND Location = ?3) "   // магазин
          " OR (LocationType = 2 AND Location = (SELECT City FROM Units WHERE UID=1712842 LIMIT ?3)) "   // город
          " OR (LocationType = 3 AND Location = ?4) "   // регион
          " OR (LocationType = 4 AND Location = ?5) "   // страна
          " OR (LocationType = 5 AND Location = (SELECT City FROM Units WHERE UID=1712842 LIMIT ?3)) "   // компания
        ") ORDER BY LocationType; ";

    if (DBC) {
        if (sqlite3_prepare_v2(DBC, select, -1, &stmt, 0) != SQLITE_OK) goto DB_GetFactorErr;
        if (sqlite3_bind_int(stmt, 1, FactorType) != SQLITE_OK)  goto DB_GetFactorErr;
        if (sqlite3_bind_int(stmt, 2, ProductID) != SQLITE_OK)  goto DB_GetFactorErr;
        if (sqlite3_bind_int(stmt, 3, ShopID) != SQLITE_OK)  goto DB_GetFactorErr;
        if (sqlite3_bind_int(stmt, 4, Region) != SQLITE_OK)  goto DB_GetFactorErr;
        if (sqlite3_bind_int(stmt, 5, Country) != SQLITE_OK)  goto DB_GetFactorErr;
        if (SQLITE_ROW == sqlite3_step(stmt)) {
            LocationType = sqlite3_column_int(stmt, 0);
            Location = sqlite3_column_int(stmt, 1);
            Value = sqlite3_column_int(stmt, 2);

            resLevel = LocationType;
            resFactor = Value;
            res = resLevel;
        }
        else res = 0;
        sqlite3_finalize(stmt);
    }
    DB_CloseDBC(DBC);
    return res;

DB_GetFactorErr:
    char msg[1024];
    int ercod;
    ercod = sqlite3_errcode(DBC);
    snprintf(msg, sizeof(msg) - 1, "DB_GetFactorErr Error.(%d: %s)", ercod, sqlite3_errmsg(DBC));
    sqlite3_finalize(stmt);
    sqlite3_close(DBC);
    throw EDBException(msg);
}

int DB_GetFactor(int FactorType, int ProductID, int ShopID, int &resLevel, int &resFactor)
{
    int res = 0;
    try {
        res = DB_GetFactorEx(FactorType, ProductID, ShopID, resLevel, resFactor);
    }
    catch (EDBException &e) {
        LogMessage(e.Message.c_str(), ML_ERR2);
    }
    return res;
}

//-----------------------------------------------------------------------------
//  
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//  
//-----------------------------------------------------------------------------


