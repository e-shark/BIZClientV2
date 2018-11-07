#ifndef DBINTERFACE_H
#define DBINTERFACE_H

//-----------------------------------------------------------------------------
#include <exception>
#include <string>

#include "sqlite3.h"

#include "BIZ_Types.h"
//-----------------------------------------------------------------------------
typedef sqlite3 DB_DBC;

//-----------------------------------------------------------------------------

bool SetDBFileName(const char * FileName);

DB_DBC *DB_GetDBContext(void);
DB_DBC *DB_GetDBContextEx(void);

void DB_CloseDBC(DB_DBC* DBC);

int DB_GetPerson(int id, std::string &srv, std::string &login, std::string &psw, int &ProxyList, int &company);
int DB_GetPersonEx(int id, std::string &srv, std::string &login, std::string &psw, int &ProxyList, int &company);

int DB_GetShedule(int cid, int &type, int &ParamI1, int &ParamI2, void *ParamBin, int ParamBinBufLen, int &ParamBinLen);
int DB_GetSheduleEx(int cid, int &type, int &ParamI1, int &ParamI2, void *ParamBin, int ParamBinBufLen, int &ParamBinLen);

int DB_SetShedule(int cid, int type, time_t start, int period, void *Param, int ParamBufLen, char* Desription);
int DB_SetSheduleEx(int cid, int type, time_t start, int period, void *Param, int ParamBufLen, char* Desription);

bool DB_SetScheduleLastRun(int SchedID);
bool DB_SetScheduleLastRunEx(int SchedID);

bool DB_SaveExchangeState(double PurchasePrice, double SellingPrice, const sSrvTime *STime);
bool DB_SaveExchangeStateEx(double PurchasePrice, double SellingPrice, const sSrvTime *STime);

bool DB_SaveUnitsList(int CID, tmUnits *UnitsList);
bool DB_SaveUnitsListEx(int CID, tmUnits *UnitsList);

int DB_ExternExchangeSate(std::string BDName, int fromID);
int DB_ExternExchangeSateEx(std::string BDName, int fromID);

#endif
