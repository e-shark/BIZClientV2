#ifndef BIZPARSER_H
#define BIZPARSER_H

#include "BIZ_Types.h"
#include "HTTPTransport.h"

char* SpaceRemove(char *s);                                                     // Убирает все пробелы из строки
char* my_strdig(char *s);                                                       // Поиск первой цифры в строке
std::string GetMoneyFormatStr(double Price);										// Формирует строку в денежном формате NNN NNN NNN.NN 

bool BIZ_CheckLogined(tHTML_Response *RSP, bool frame = true);
bool BIZ_ParseServerTime(char* Page, sSrvTime *pSrvTime);
int  BIZ_ParseCompanyStructure(char* Page, tmUnits *Company);
bool BIZ_ParsePrice(char *str, float &Price, int &Currency);
int  BIZ_ParseShoopGoodsTable(char* Page, tmGoodsList *pGL, tmCityPriceMap *pCPM);
bool  BIZ_ParseProductInfoFromShop(char* Page, sBIZGoods *Info);
int   BIZ_ParseVendorsPagesCount(char *Page);                                   // Получить кол-во страниц, на которых размещен весь список поставщиков
int   BIZ_ParseVendors(char* Page, TVendorList *Vendors);
float BIZ_ParseAveragePrice(char *Page, float &Quality);


std::string GetPOSTPriceByCurrency(float Price, int Currency);
int BIZ_GetUnitTypeByURI(char URI);
std::string GetIPStr(DWORD IP);
#endif

