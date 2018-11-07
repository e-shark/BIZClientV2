
#include <string.h>
#include <string>

#include "BIZ_Parser.h"
#include "BIZ_Exceptions.h"
#include "LogUtils.h"


//-----------------------------------------------------------------------------
//      Убирает все пробелы из строки
//      Строка должна обязательно заканчиваться символом #0
//-----------------------------------------------------------------------------
char* SpaceRemove(char * s)
{
    char *p = s;
    while (*p) {
        if (*p == ' ')
            strcpy(p, p + 1);
        p++;
    }
    return s;
}

//-----------------------------------------------------------------------------
//      Ищет первую цифру в строке
//      Строка обязательно должна заканчиваться символом \0
//-----------------------------------------------------------------------------
char * my_strdig(char *s)
{
    char *pc = s;
    if (!s) return NULL;
    while (*pc != 0) {
        if ((*pc >= '0') && (*pc <= '9')) return pc;
        pc++;
    }
    return NULL;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int BIZ_GetUnitTypeByURI(char *URI)
{
    if (strstr(URI, "/office/")) return 1;              // оффис
    if (strstr(URI, "/shop/")) return 2;                // магазин
    if (strstr(URI, "/factory/")) return 3;             // завод
    if (strstr(URI, "/warehouse/")) return 4;           // склад
    if (strstr(URI, "/mine/")) return 5;                // шахта, скважина
    if (strstr(URI, "/sawmill/")) return 6;             // Лесопилка 
    if (strstr(URI, "/animalfarm/")) return 7;          // Животноводческая ферма
    if (strstr(URI, "/farm/")) return 8;                // Земледельческая ферма
    if (strstr(URI, "/greenhouse/")) return 9;          // Тепличное хозяйство
    if (strstr(URI, "/sciencelab/")) return 10;         // Исследовательский институт
    if (strstr(URI, "/construction/")) return 11;       // Строительное предприятие
    if (strstr(URI, "/mediaunit/?")) return 12;         // СМИ
    if (strstr(URI, "/catering/")) return 13;           // Общественное питание
    if (strstr(URI, "/cargounit/")) return 14;          // Транспортное предприятие

    return 0;
}


//-----------------------------------------------------------------------------
//      Формирует строку с ценой для POST запроса выставления цены
//-----------------------------------------------------------------------------
std::string GetPOSTPriceByCurrency(float Price, int Currency)
{
    int p1, p2, p3, p4;
    std::string res;
    std::string ps = "";
    char ts[10];

    p1 = (int)Price % 1000;
    p2 = ((int)(Price / 1000)) % 1000;
    p3 = ((int)(Price / 1000000)) % 1000;
    p4 = ((int)(Price / 1000000000)) % 1000;

    if (p4) {
        ps += IntToStr(p4);
        ps += "+";
    }

    if (p4)
        sprintf(ts, " %.3d+", p3);
    else
        if (p3)
            sprintf(ts, " %d+", p3);
        else
            sprintf(ts, "");
    ps += ts;

    if (p4 || p3)
        sprintf(ts, " %.3d+", p2);
    else
        if (p2)
            sprintf(ts, " %d+", p2);
        else
            sprintf(ts, "");
    ps += ts;

    if (p4 || p3 || p2)
        sprintf(ts, " %.3d", p1);
    else
        if (p1)
            sprintf(ts, " %d", p1);
        else
            sprintf(ts, "0");
    ps += ts;

    switch (Currency) {
    case curRUB:
        res = ps;
        res += "+%D1%80.";      // "р."
        break;

    case curUSD:
        res = "%24";            //$
        res += ps;
        break;

    case curUAH:
        res = "+%D0%B3%D1%80%D0%BD."; //"грн."
        res += ps;
        break;

    case curJPY:
    case curCNY:
    case curEUR:
    case curGBP:
        // ЭТИ ВАЛЮТЫ НАДО ДОПИСАТЬ. !!!
        res = ps;
    }

    return res;
}

//-----------------------------------------------------------------------------
//      Извлекает со страницы время на сервере
//-----------------------------------------------------------------------------
bool BIZ_ParseServerTime(char* Page, sSrvTime *pSrvTime)
{
    std::string s;
    int res = false;
    char ts[50];
    char *pc1, *pc2;
    char *pEnd, *pST;
    int id;

    char DayStr[20];
    char MonthStr[20];

    sSrvTime ST;
    ST.TimeStamp = time(NULL);
  
    if (!Page) return 0;

    try {
        // Проверяем, а страница ли это вообще
        pST = strstr(Page, "<html>");
        if (!pST) throw EParserException(1);

        // Ищем время, оставшееся до смены суток
        pST = strstr(pST + 6, "var gsecs = ");
        if (!pST) throw EParserException(2);
        pc1 = strchr(pST + 8, '=');
        if (!pc1) throw EParserException(3);
        pc1 += 1;
        pc2 = strchr(pc1, ';');
        if (!pc2) throw EParserException(4);
        memset(ts, 0, sizeof(ts));
        memcpy(ts, pc1, pc2 - pc1);
        ST.LeftToMdnt = atoi(SpaceRemove(ts));

        // Ищем начало текста с датой
        pST = strstr(pST + 6, "Дата:");
        if (!pST) throw EParserException(5);
        pST = strstr(pST + 5, "<b>");
        if (!pST) throw EParserException(6);
        pST += 3;
        pEnd = strstr(pST, "</b>");
        if (!pEnd) throw EParserException(7);

        // Ищем день недели
        pc1 = pST;
        if (!pc1) throw EParserException(8);
        pc2 = strchr(pc1 + 1, ',');
        if (!pc2) throw EParserException(9);
        memset(DayStr, 0, sizeof(DayStr));
        memcpy(DayStr, pc1, pc2 - pc1);
        if (strstr(DayStr, "Понедельник")) ST.SrvWeekDay = 1;    //пров
        if (strstr(DayStr, "Вторник")) ST.SrvWeekDay = 2;        //пров
        if (strstr(DayStr, "Среда")) ST.SrvWeekDay = 3;
        if (strstr(DayStr, "Четв")) ST.SrvWeekDay = 4;
        if (strstr(DayStr, "Пятни")) ST.SrvWeekDay = 5;
        if (strstr(DayStr, "Суббота")) ST.SrvWeekDay = 6;
        if (strstr(DayStr, "Воскрес")) ST.SrvWeekDay = 7;

        // Ищем число
        pc1 = pc2 + 2;
        pc2 = strchr(pc1 + 1, ' ');
        if (!pc2) throw EParserException(10);
        memset(ts, 0, sizeof(ts));
        memcpy(ts, pc1, pc2 - pc1);
        ST.SrvDay = atoi(SpaceRemove(ts));

        // Ищем месяц
        pc1 = pc2 + 1;
        pc2 = strchr(pc1 + 1, ' ');
        if (!pc2) throw EParserException(11);
        memset(MonthStr, 0, sizeof(MonthStr));
        memcpy(MonthStr, pc1, pc2 - pc1);
        if (strstr(MonthStr, "января"))   ST.SrvMonth = 1;
        if (strstr(MonthStr, "февраля"))  ST.SrvMonth = 2;
        if (strstr(MonthStr, "марта"))    ST.SrvMonth = 3;    //пров
        if (strstr(MonthStr, "апреля"))   ST.SrvMonth = 4;
        if (strstr(MonthStr, "мая"))      ST.SrvMonth = 5;
        if (strstr(MonthStr, "июня"))     ST.SrvMonth = 6;
        if (strstr(MonthStr, "июля"))     ST.SrvMonth = 7;
        if (strstr(MonthStr, "августа"))  ST.SrvMonth = 8;
        if (strstr(MonthStr, "сентября")) ST.SrvMonth = 9;
        if (strstr(MonthStr, "октября"))  ST.SrvMonth = 10;
        if (strstr(MonthStr, "ноября"))   ST.SrvMonth = 11;
        if (strstr(MonthStr, "декабря"))  ST.SrvMonth = 12;

        // Ищем год
        pc1 = pc2 + 1;
        pc2 = pEnd;
        if (!pc2) throw EParserException(12);
        memset(ts, 0, sizeof(ts));
        memcpy(ts, pc1, pc2 - pc1);
        ST.SrvYear = atoi(SpaceRemove(ts));


        *pSrvTime = ST;
        res = true;
    }
    catch (EParserException &e) {
        s = "ERROR BIZ_ParseServerTime : Ошибка парсинга данных: ";
        s += IntToStr(e.Cod);
        LogMessage(s.c_str(), ML_ERR3);
        res = false;
    }
    return res;

}


//-----------------------------------------------------------------------------
//      Проверяем авторизованы мы, или нет
//  frame - признак, что это не вся страница, а только фрейм с нее (default: true)
//-----------------------------------------------------------------------------
bool BIZ_CheckLogined(tHTML_Response *RSP, bool frame)
{
    char *pC1, *pC2;

    if (!RSP->Body) return false;
    if (!RSP->Header) return false;

    if (RSP->BodyLen < 5) return false;

    pC1 = strstr(RSP->Header, "Location: /user/login");    // Перенаправление на страницу входа
    if (pC1) return false;

    pC1 = strstr(RSP->Body, "login required");
    if (pC1) return false;

    pC1 = strstr(RSP->Body, "<html>");
    if (!pC1) return false;
    pC1 += 6;

    pC2 = strstr(pC1, "Войти в игру");
    if (pC2) return false;

    pC2 = strstr(pC1, "href=\"/user/login");
    if (pC2) return false;

    if (!frame) {
        pC2 = strstr(pC1, "Пользователь:");
        if (!pC2) return false;
    }

    return true;
}


//-----------------------------------------------------------------------------
//      Разобрать строку с ценой на цену и валюту
//      Возвращает ture если удалось разобрать строку
//-----------------------------------------------------------------------------
bool BIZ_ParsePrice(char *str, float &Price, int &Currency)
{
    bool res = false;
    float fPrice = 0;
    int   fCurrency = curUKNWN;
    char *pc1, *pc2;
    char ts[32];
    std::string s;

    try {
        // Сначала определяем стоимость
        fPrice = 0;
        pc1 = my_strdig(str);
        if (!pc1) throw EParserException(1);
        memset(ts, 0, sizeof(ts));
        strncpy(ts, pc1, 31);
        fPrice = atof(SpaceRemove(ts));

        // Затем определяем валюту
        fCurrency = curUKNWN;
        if (strstr(str, "&yen;"))  fCurrency = curJPY;
        if (strstr(str, "?"))      fCurrency = curCNY;
        if (strstr(str, "&euro;")) fCurrency = curEUR;
        if (strstr(str, "€"))      fCurrency = curEUR;
        if (strstr(str, "грн"))    fCurrency = curUAH;
        if (strstr(str, "р."))     fCurrency = curRUB;
        if (strstr(str, "$"))      fCurrency = curUSD;
        if (strstr(str, "&pound;")) fCurrency = curGBP;

        Currency = fCurrency;
        Price = fPrice;
        res = true;

    }
    catch (EParserException e) {
        s = "ERROR BIZ_ParsePrice: Ошибка парсинга данных: ";
        s += IntToStr(e.Cod);
        LogMessage(s.c_str(), ML_ERR3);
        res = false;
    }

    return res;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
char* FindCloseTag(char* str, char* tag)
{
    char *res = NULL;
    char *pc, *pcto, *pctc;
    char sto[100], stc[100];
    int opencount = 1;
    if (!str) return res;
    if (!tag) return res;
    sto[sizeof(sto) - 1] = 0;
    stc[sizeof(stc) - 1] = 0;
    snprintf(sto, sizeof(sto) - 1, "<%s", tag);
    snprintf(stc, sizeof(stc) - 1, "</%s>", tag);
    pc = str;
    do {
        pcto = strstr(pc, sto);
        pctc = strstr(pc, stc);
        if (pctc) {
            if ((NULL != pcto) && (pcto < pctc)) {
                opencount++;
                pc = pcto + 2;
            }
            else {
                opencount--;
                pc = pctc + 4;
                if (0 == opencount) res = pctc;
            }
        }
    }while ((0 != opencount) && (pctc));

    return res;
}

//-----------------------------------------------------------------------------
//      
//-----------------------------------------------------------------------------
int BIZ_ParseCompanyStructure(char* Page, tmUnits *Company)
{
    int res = 0;
    std::string s;
    char ts[50];
    char tURI[50];
    char tImgName[50];
    char *pc1, *pc2, *pcb;
    char *pcCity, *pcUnit;
    char *pcEndTable, *pcEndCity;
    int CityID;
    int UnitID;
    tBIZUnit *unit;

    if (!Page) return 0;

    // Далее парсим перечень подразделений
    CityID = 0;
    pcCity = NULL;
    pcUnit = NULL;
    pcEndCity = strstr(Page, "compcountry");
    pcEndTable = FindCloseTag(pcEndCity, "table");
    if (!pcEndTable) throw EParserException(44);                                              // ошибка: нет конца страницы
    //pcEndCity = FindCloseTag(pcEndCity, "tr");
    //pcEndCity = FindCloseTag(pcEndCity, "tr");

    try {

        do {
            pcCity = strstr(pcEndCity, "/world/country/?id=");
            if ((!pcCity) && (!CityID)) throw EParserException(45);                                     // ошибка, если нет ни одного города
            if (pcCity && (pcCity < pcEndTable)) {
                pcCity += 19;
                pc1 = strstr(pcCity, "/city/?id=");
                if (pc1) {
                    pc1 += 10;
                    pc2 = strchr(pc1, '"');
                    if (pc2) {
                        memset(ts, 0, sizeof(ts));
                        memcpy(ts, pc1, pc2 - pc1);
                        CityID = atoi(SpaceRemove(ts));
                    }
                }
            }else pcCity = NULL;
            if (pcCity) {
                pcEndCity = FindCloseTag(pcCity, "tr");
                if (!pcEndCity) throw EParserException(46);
                pcEndCity = strstr(pcEndCity, "<td");
                if (!pcEndCity) throw EParserException(46);
                pcEndCity = FindCloseTag(pcEndCity+4, "tr");
                if (pcEndCity > pcEndTable) throw EParserException(48);
                pcUnit = pcCity;
                do {

                    // Ищем название подразделения
                    pcUnit = strstr(pcUnit, "<span class=\"tooltip up\" hint=\"");

                    if (pcUnit && (pcUnit < pcEndCity)) {

                        pcUnit += 31;
                        pc2 = strstr(pcUnit, "\">");
                        if (!pc2) throw EParserException(49);
                        pc1 = strstr(pcUnit, "&nbsp");
                        if (pc1)
                            pc2 = pc1;
                        memset(ts, 0, sizeof(ts));
                        memcpy(ts, pcUnit, pc2 - pcUnit);

                        //Ищем URI подразделения
                        pc1 = strstr(pc2 + 2, "href=\"/units/");
                        if (!pc1) throw EParserException(50);
                        pc1 += 6;
                        pc2 = strchr(pc1, '"');
                        if (!pc2) throw EParserException(51);
                        memset(tURI, 0, sizeof(tURI));
                        memcpy(tURI, pc1, pc2 - pc1);
                        // извлекаем ID подразделения
                        pc1 = strstr(tURI, "id=");
                        if (!pc1) throw EParserException(52);
                        UnitID = atoi(pc1 + 3);

                        // Ищем ссылку на картинку (она поможет потом расшифровать подтип подразделения)
                        pc1 = strstr(pc2, "<img src=\"");
                        if (!pc1) throw EParserException(53);
                        pc1 += 10;
                        pc2 = strchr(pc1, '"');
                        if (!pc2) throw EParserException(54);
                        memset(tImgName, 0, sizeof(tURI));
                        memcpy(tImgName, pc1, pc2 - pc1);

                        pcUnit = pc2 + 1;

                        res++;
                        if (Company) {
                            if (UnitID) {
                                unit = new tBIZUnit(UnitID);
                                if (unit) {
                                    unit->CityID = CityID;
                                    unit->ID = UnitID;
                                    unit->URI = tURI;
                                    unit->Image = tImgName;
                                    unit->Type = BIZ_GetUnitTypeByURI(tURI);
                                    unit->Name = ts;
                                    Company->insert(std::pair <int, tBIZUnit*>(unit->ID, unit));
                                }
                                else
                                    LogMessage("Error BIZ_ParseCompanyStructure: Не удалось создать объект tBIZUnit", ML_ERR3);
                            }
                        }

                    }
                    else pcUnit = NULL;
                } while (pcUnit);
            }

        } while (pcCity);
    }   //try
    catch (EParserException e) {
        s = "ERROR BIZ_ParseOwnCompanyPage: Ошибка парсинга данных: ";
        s += IntToStr(e.Cod);
        LogMessage(s.c_str(), ML_ERR3);
        //res = 0;
    }

    return res;
}

//-----------------------------------------------------------------------------
//      Парсер перечня товаров в магазине
//-----------------------------------------------------------------------------
int  BIZ_ParseShoopGoodsTable(char* Page, tmGoodsList *pGL, tmCityPriceMap *pCPM)
{
    std::string s;
    int res = 0;
    char ts[50];
    char *pc1, *pc2, *pc3;
    char *pEnd, *pST;
    int id;

    sQPrice QP;
    sBIZGoods Goods;


    if (!Page) return 0;

    try {
        pc1 = strstr(Page, "name=\"goodsform\"");
        if (!pc1) throw EParserException(1);
        pc1 = strstr(pc1 + 16, "<tbody>");
        if (!pc1) throw EParserException(2);
        pEnd = strstr(pc1 + 7, "</tbody>");
        if (!pEnd) throw EParserException(3);

        pST = strstr(pc1 + 7, "<tr");

        while ((pST) && (pEnd > pST)) {

            memset(&QP, 0, sizeof(QP));
            memset(&Goods, 0, sizeof(Goods));

            // Картинка товара
            pST = strstr(pST + 4, "<td");
            if (!pST) throw EParserException(4);

            // Название товара
            pST = strstr(pST + 4, "<td");
            if (!pST) throw EParserException(5);
            pc1 = strstr(pST + 3, "&product=");
            if (!pc1) throw EParserException(6);
            pc1 += 9;
            pc2 = strchr(pc1 + 2, '"');
            if (!pc2) throw EParserException(7);
            memset(ts, 0, sizeof(ts));
            memcpy(ts, pc1, pc2 - pc1);
            id = atoi(SpaceRemove(ts));
            Goods.ID = id;

            // Индикаторы склада
            pST = strstr(pST + 4, "<td");
            if (!pST) throw EParserException(8);
            pST += 9;

            // Выручка и прибыль
            pST = strstr(pST + 4, "<td align=\"right\" nowrap>");
            if (!pST) throw EParserException(9);
            pc1 = strchr(pST + 20, '>');
            if (!pc1) throw EParserException(10);
            pc1 += 1;
            pc2 = strstr(pc1, "<br/>");
            if (!pc2) throw EParserException(11);
            pc1 = my_strdig(pc1);
            if (pc1 && (pc1<pc2)) {
                memset(ts, 0, sizeof(ts));
                memcpy(ts, pc1, pc2 - pc1);
                Goods.Proceeds = atoi(SpaceRemove(ts));
            }
            else Goods.Proceeds = 0;
            //   прибыль
            pc1 += 5;
            pc2 = strstr(pc1, "</td>");
            if (!pc2) throw EParserException(13);
            pc1 = my_strdig(pc1);
            if (pc1 && (pc1<pc2)) {
                memset(ts, 0, sizeof(ts));
                memcpy(ts, pc1, pc2 - pc1);
                Goods.Profit = atoi(SpaceRemove(ts));
            }
            else Goods.Profit = 0;

            // Бары выручка и прибыль
            pST = strstr(pST + 4, "<td");
            if (!pST) throw EParserException(14);
            pST += 3;

            // Кол-во на складе
            pST = strstr(pST + 4, "<td align=\"center\">");
            if (!pST) throw EParserException(15);
            pc1 = strchr(pST + 10, '>');
            if (!pc1) throw EParserException(16);
            pc1 += 1;
            pc3 = strstr(pc1, "</td>");
            if (!pc3) throw EParserException(16);
            pc2 = strstr(pc1, "<b>");                                                 //Иногда цифра отображается выделенным шрифтом (например если на складе нет товара)
            if (pc2 && (pc2<pc3)) {
                pc1 = pc2 + 3;
                pc2 = strstr(pc1, "</b>");
            }
            else
                pc2 = pc3;
            if (!pc2) throw EParserException(17);
            memset(ts, 0, sizeof(ts));
            memcpy(ts, pc1, pc2 - pc1);
            Goods.Available = atoi(SpaceRemove(ts));

            // Качество товара на складе
            pST = strstr(pST + 4, "<td align=\"center\"");
            if (!pST) throw EParserException(18);
            pc1 = strchr(pST + 10, '>');
            if (!pc1) throw EParserException(19);
            pc1 += 1;
            pc2 = strstr(pc1, "</td>");
            if (!pc2) throw EParserException(20);
            memset(ts, 0, sizeof(ts));
            memcpy(ts, pc1, pc2 - pc1);
            Goods.Quality = atof(SpaceRemove(ts));

            // Стоимость товара на складе
            pST = strstr(pST + 4, "<td align=\"right\"");
            if (!pST) throw EParserException(21);
            pc1 = strchr(pST + 16, '>');
            if (!pc1) throw EParserException(22);
            pc1 += 1;
            pc2 = strstr(pc1, "</td>");
            if (!pc2) throw EParserException(23);
            pc1 = my_strdig(pc1);
            if (pc1 && (pc1<pc2)) {
                memset(ts, 0, sizeof(ts));
                memcpy(ts, pc1, pc2 - pc1);
                Goods.Cost = atof(SpaceRemove(ts));
            }
            else Goods.Cost = 0;

            // Цена товара на продажу
            pST = strstr(pST + 4, "<input name=\"price[");
            if (!pST) throw EParserException(24);
            pc1 = strstr(pST + 5, "value=\"");
            if (!pST) throw EParserException(25);
            pc1 += 7;
            pc2 = strchr(pc1 + 7, '"');
            if (!pc2) throw EParserException(26);
            if (pc1<pc2) {
                memset(ts, 0, sizeof(ts));
                memcpy(ts, pc1, pc2 - pc1);
                if (!BIZ_ParsePrice(ts, Goods.Price, Goods.Currency)) throw EParserException(32);
            }
            else Goods.Price = 0;

            // среднее по городу качество
            pST = strstr(pST + 4, "<td align=\"center\"");
            if (!pST) throw EParserException(27);
            pc1 = strchr(pST + 10, '>');
            if (!pc1) throw EParserException(28);
            pc1 += 1;
            pc2 = strstr(pc1, "</td>");
            if (!pc2) throw EParserException(29);
            memset(ts, 0, sizeof(ts));
            memcpy(ts, pc1, pc2 - pc1);
            QP.Quality = atof(SpaceRemove(ts));

            // средняя по городу цена
            pST = strstr(pST + 4, "<td align=\"right\" nowrap");
            if (!pST) throw EParserException(30);
            pc1 = strchr(pST + 20, '>');
            if (!pc1) throw EParserException(31);
            pc1 += 1;
            pc2 = strstr(pc1, "</td>");
            if (!pc2) throw EParserException(32);
            if (pc1<pc2) {
                memset(ts, 0, sizeof(ts));
                memcpy(ts, pc1, pc2 - pc1);
                if (!BIZ_ParsePrice(ts, QP.Price, QP.Currency)) throw EParserException(32);
            }
            else QP.Price = 0;

            // объем продаж
            pST = strstr(pST + 4, "<td align=\"center\" nowrap");
            if (!pST) throw EParserException(33);
            pc1 = strchr(pST + 10, '>');
            if (!pc1) throw EParserException(34);
            pc1 += 1;
            pc2 = strstr(pc1, "</td>");
            if (!pc2) throw EParserException(35);
            memset(ts, 0, sizeof(ts));
            memcpy(ts, pc1, pc2 - pc1);
            Goods.Sales = atoi(SpaceRemove(ts));

            // галочка "своя компания"
            pST = strstr(pST + 4, "<td");
            if (!pST) throw EParserException(36);
            pST += 3;

            // закупки
            pST = strstr(pST + 4, "<input name=\"purchaseQuantity[");
            if (!pST) throw EParserException(37);
            pc1 = strstr(pST + 20, "value=\"");
            if (!pc1) throw EParserException(38);
            pc1 += 7;
            pc2 = strchr(pc1, '"');
            if (!pc2) throw EParserException(39);
            memset(ts, 0, sizeof(ts));
            memcpy(ts, pc1, pc2 - pc1);
            Goods.Purchase = atoi(SpaceRemove(ts));

            pGL->insert(pair<int, sBIZGoods>(id, Goods));
            pCPM->insert(pair<int, sQPrice>(id, QP));

            pST = strstr(pc1 + 4, "<tr");
            res++;
        } // while
    }
    catch (EParserException &e) {
        s = "ERROR BIZ_ParseShoopGoodsTable : Ошибка парсинга данных: ";
        s += IntToStr(e.Cod);
        LogMessage(s.c_str(), ML_ERR3);
        //res = 0;
    }
    return res;
}


//-----------------------------------------------------------------------------
//      Парсим информацию по наличию товара в магазине
//-----------------------------------------------------------------------------
bool  BIZ_ParseProductInfoFromShop(char* Page, sBIZGoods *Info)
{
    bool res = false;
    std::string s;
    char *pc1, *pc2, *pc3;
    char *pcB, *pcE;
    char *pcRB, *pcRE;
    char ts[50];
    int   fInStock = 0;                               // Количество товара на складе
    float fQuality = 0;                               // Качество товара
    float fCost = 0;                                  // Себетоимость
    float fPrice = 0;                                 // Цена (отпускная)
    int   fCurrency = 0;                              // В какой валюте цена    
    int   fDDay = 0;                                  // Дата день
    int   fDMonth = 0;                                // Дата месяц
    int   fDYear = 0;                                 // Дата год
    int   fSales = 0;                                 // Объем продаж
    int   fProceeds = 0;                              // Выручка
    int   fProfit = 0;                                // прибыль
    int i;

    sSalesHistoryRec SalesHistory[7];
    for (i = 0; i<7; i++)
        SalesHistory[i].Clear();

    if (!Page) return 0;

    try {
        // Ищем место на стрвнице, где расположена нужная нам страниця=а
        pcB = strstr(Page, "<tr align=\"center\"><td>по дням</td><td>по неделям</td>");
        if (!pcB) throw EParserException(1);
        pcB = strstr(pcB, "<table");
        if (!pcB) throw EParserException(1);
        pcE = FindCloseTag(pcB+5, "table");
        if (!pcE) throw EParserException(1);
        pc1 = strstr(pcB, "<tbody>");
        if (pc1 && (pcB < pcE)) {                                                    // В новом магазине продаж может и не быть, тогда таблица будет пустая
            pcB = pc1;
            pcE = strstr(pcB, "</tbody>");
            if (!pcE) throw EParserException(1);

            // Парсим таблицу продаж
            pcRB = strstr(pcB, "<tr class=\"tbl");
            i = 0;
            while (pcRB && (pcRB<pcE)) {
                pcRB += 10;
                pcRE = strstr(pcRB, "</tr>");
                if (!pcRE) throw EParserException(1);
                if (pcRE>pcE) throw EParserException(1);

                // Парсим дату продаж
                pc1 = strstr(pcRB, "<td");
                if (!pc1) throw EParserException(1);
                pc1 = strchr(pc1, '>');
                if (!pc1) throw EParserException(1);
                pc1 += 1;
                pc3 = strstr(pc1, "</td>");
                if (!pc3) throw EParserException(1);
                if (pc3 >= pcRE) throw EParserException(1);
                // ... день
                pc2 = strchr(pc1, '.');
                if (!pc2) throw EParserException(1);
                if (pc2 >= pc3) throw EParserException(1);
                memset(ts, 0, sizeof(ts));
                memcpy(ts, pc1, pc2 - pc1);
                fDDay = atoi(ts);
                // ... месяц
                pc1 = pc2 + 1;
                pc2 = strchr(pc1, '.');
                if (!pc2) throw EParserException(1);
                if (pc2 >= pc3) throw EParserException(1);
                memset(ts, 0, sizeof(ts));
                memcpy(ts, pc1, pc2 - pc1);
                fDMonth = atoi(ts);
                // ... год
                pc1 = pc2 + 1;
                pc2 = pc3;
                memset(ts, 0, sizeof(ts));
                memcpy(ts, pc1, pc2 - pc1);
                fDYear = atoi(ts);

                // Парсим объем продаж
                pc1 = strstr(pc3, "<td");
                if (!pc1) throw EParserException(1);
                pc1 = strchr(pc1, '>');
                if (!pc1) throw EParserException(1);
                pc1 += 1;
                pc3 = strstr(pc1, "</td>");
                if (!pc3) throw EParserException(1);
                if (pc3 >= pcRE) throw EParserException(1);
                memset(ts, 0, sizeof(ts));
                memcpy(ts, pc1, pc3 - pc1);
                fSales = atoi(SpaceRemove(ts));

                // Парсим цену
                pc1 = strstr(pc3, "<td");
                if (!pc1) throw EParserException(1);
                pc1 = strchr(pc1, '>');
                if (!pc1) throw EParserException(1);
                pc1 += 1;
                pc3 = strstr(pc1, "</td>");
                if (!pc3) throw EParserException(1);
                if (pc3 >= pcRE) throw EParserException(1);
                pc1 = my_strdig(pc1);
                if (!pc1) throw EParserException(1);
                if (pc1 >= pc3) throw EParserException(1);
                memset(ts, 0, sizeof(ts));
                memcpy(ts, pc1, pc3 - pc1);
                fPrice = atoi(SpaceRemove(ts));

                // Парсим выручку
                pc1 = strstr(pc3, "<td");
                if (!pc1) throw EParserException(1);
                pc1 = strchr(pc1, '>');
                if (!pc1) throw EParserException(1);
                pc1 += 1;
                pc3 = strstr(pc1, "</td>");
                if (!pc3) throw EParserException(1);
                if (pc3 >= pcRE) throw EParserException(1);
                pc1 = my_strdig(pc1);
                if (!pc1) throw EParserException(1);
                if (pc1 >= pc3) throw EParserException(1);
                memset(ts, 0, sizeof(ts));
                memcpy(ts, pc1, pc3 - pc1);
                fProceeds = atoi(SpaceRemove(ts));

                // Парсим прибыль
                pc1 = strstr(pc3, "<td");
                if (!pc1) throw EParserException(1);
                pc1 = strchr(pc1, '>');
                if (!pc1) throw EParserException(1);
                pc1 += 1;
                pc3 = strstr(pc1, "</td>");
                if (!pc3) throw EParserException(1);
                if (pc3 >= pcRE) throw EParserException(1);
                pc1 = my_strdig(pc1);
                if (!pc1) throw EParserException(1);
                if (pc1 >= pc3) throw EParserException(1);
                memset(ts, 0, sizeof(ts));
                memcpy(ts, pc1, pc3 - pc1);
                fProfit = atoi(SpaceRemove(ts));

                SalesHistory[i].DateTime.SrvDay = fDDay;
                SalesHistory[i].DateTime.SrvMonth = fDMonth;
                SalesHistory[i].DateTime.SrvYear = fDYear;
                SalesHistory[i].Sales = fSales;
                SalesHistory[i].Price = fPrice;
                SalesHistory[i].Quality = fQuality;
                SalesHistory[i].Proceeds = fProceeds;
                SalesHistory[i].Profit = fProfit;

                // ищем начало следующей строки
                pcRB = strstr(pcRB, "<tr class=\"tbl");
                i++;
            }
        }
        pcB = pcE;

        // Ищем начало остальных данных
        pcB = strstr(pcB, "<table cellpadding=\"4\" cellspacing=\"1\" class=\"datatable\" width=\"100%\">");
        if (!pcB) throw EParserException(1);
        pcE = strstr(pcB, "</table>");
        if (!pcE) throw EParserException(1);

        // Тут шапка таблички
        pcRB = strstr(pcB, "<tr class=\"tbl");
        if (!pcRB) throw EParserException(1);
        pcRB += 10;
        pcRE = strstr(pcRB, "</tr>");
        if (!pcRE) throw EParserException(1);
        if (pcRE>pcE) throw EParserException(1);

        // Далее парсим остальные данные: кол-во, цену и пр...
        pcRB = strstr(pcRE, "<tr align=\"center\">");
        if (!pcRB) throw EParserException(1);
        pcRB += 10;
        pcRE = strstr(pcRB, "</tr>");
        if (!pcRE) throw EParserException(1);
        if (pcRE>pcE) throw EParserException(1);

        // тут словеса
        pc1 = strstr(pcRB, "<td");
        if (!pc1) throw EParserException(1);
        pc1 += 1;
        pc3 = strstr(pc1, "</td>");
        if (!pc3) throw EParserException(1);
        if (pc3 >= pcRE) throw EParserException(1);

        // Парсим кол-во товара на складе
        pc1 = strstr(pc3, "<td");
        if (!pc1) throw EParserException(1);
        pc1 = strchr(pc1, '>');
        if (!pc1) throw EParserException(1);
        pc1 += 1;
        pc3 = strstr(pc1, "</td>");
        if (!pc3) throw EParserException(1);
        if (pc3 >= pcRE) throw EParserException(1);
        memset(ts, 0, sizeof(ts));
        memcpy(ts, pc1, pc3 - pc1);
        fInStock = atoi(SpaceRemove(ts));

        // Парсим качество товара на складе
        pc1 = strstr(pc3, "<td");
        if (!pc1) throw EParserException(1);
        pc1 = strchr(pc1, '>');
        if (!pc1) throw EParserException(1);
        pc1 += 1;
        pc3 = strstr(pc1, "</td>");
        if (!pc3) throw EParserException(1);
        if (pc3 >= pcRE) throw EParserException(1);
        memset(ts, 0, sizeof(ts));
        memcpy(ts, pc1, pc3 - pc1);
        fQuality = atof(SpaceRemove(ts));

        // тут должен быть какой-то "бренд"
        pc1 = strstr(pc3, "<td");
        if (!pc1) throw EParserException(1);
        pc1 += 1;
        pc3 = strstr(pc1, "</td>");
        if (!pc3) throw EParserException(1);
        if (pc3 >= pcRE) throw EParserException(1);

        // Парсим себестоимость товара на складе
        pc1 = strstr(pc3, "<td");
        if (!pc1) throw EParserException(1);
        pc1 = strchr(pc1, '>');
        if (!pc1) throw EParserException(1);
        pc1 += 1;
        pc3 = strstr(pc1, "</td>");
        if (!pc3) throw EParserException(1);
        if (pc3 >= pcRE) throw EParserException(1);
        pc1 = my_strdig(pc1);
        if (!pc1) throw EParserException(1);
        if (pc1 >= pc3) throw EParserException(1);
        memset(ts, 0, sizeof(ts));
        memcpy(ts, pc1, pc3 - pc1);
        fCost = atof(SpaceRemove(ts));

        // Парсим отпускнею цену товара
        pc1 = strstr(pc3, "<input name=\"price\" value=\"");
        if (!pc1) throw EParserException(1);
        pc1 = strstr(pc1, "value=\"");
        if (!pc1) throw EParserException(1);
        pc1 += 7;
        pc3 = strchr(pc1, '"');
        if (!pc3) throw EParserException(1);
        if (pc3 >= pcRE) throw EParserException(1);
        if (pc1 >= pc3) throw EParserException(1);
        memset(ts, 0, sizeof(ts));
        memcpy(ts, pc1, pc3 - pc1);
        if (!BIZ_ParsePrice(ts, fPrice, fCurrency)) throw EParserException(1);


        // Далее есть дополнительные данные
        pcRB = strstr(pcRE, "<tr align=\"center\" class=\"tblgr\">");
        if (!pcRB) throw EParserException(1);
        pcRB += 10;
        pcRE = strstr(pcRB, "</tr>");
        if (!pcRE) throw EParserException(1);
        if (pcRE>pcE) throw EParserException(1);

        // Парсим среднее качество по городу
        // хотя нет. не парсим.
        // лениво мне.
        // но если понадобится, можно тут дописать.
        // но это врядле

        // Парсим среднюю цену по городу
        // -//-

        Info->Available = fInStock;
        Info->Quality = fQuality;
        Info->Cost = fCost;
        Info->Price = fPrice;
        Info->Currency = fCurrency;
        Info->Sales = 0;
        Info->Reques = 0;
        Info->Proceeds = 0;
        Info->Profit = 0;

        for (i = 0; i<7; i++) {
            if (SalesHistory[i].Sales >  Info->Reques)
                Info->Reques = SalesHistory[i].Sales;
            Info->Sales += SalesHistory[i].Sales;
            Info->Proceeds += SalesHistory[i].Proceeds;
            Info->Profit += SalesHistory[i].Profit;
        }

        Info->Reques *= 7;  // Расчитываем ожидаемое потребление, как самые большие суточные продажи, умноженные на 7 дней

        res = true;
    }
    catch (EParserException e) {
        s = "ERROR BIZ_ParseProductInfoFromWarehouse : Ошибка парсинга данных: ";
        s += e.Cod;
        LogMessage(s.c_str(), ML_ERR3);
        //res = 0;
    }
    return res;
}


//-----------------------------------------------------------------------------
//      Получить кол-во страниц, на которых размещен весь список поставщиков
//-----------------------------------------------------------------------------
int BIZ_ParseVendorsPagesCount(char *Page)
{
    int res;
    char *pc1, *pc2;
    if (!Page) return 0;
    char ts[50];

    pc1 = strstr(Page, "createPGNavigator(\"vendors\",\"purchaseForm\",\"p\"");
    if (!pc1) return 0;
    pc1 += 44;

    pc1 = strchr(pc1, ',');
    if (!pc1) return 0;
    pc1++;

    pc1 = strchr(pc1, ',');
    if (!pc1) return 0;
    pc1++;

    pc2 = strchr(pc1, ',');
    if (!pc2) return 0;
    if (pc2 - pc1>50) return 0;
    memset(ts, 0, sizeof(ts));
    memcpy(ts, pc1, pc2 - pc1);
    res = atoi(SpaceRemove(ts));

    return res;
}


//-----------------------------------------------------------------------------
//      Добавить поставщиков товара со страницы из буфера приемника, к списку
//      (Парсит страницу выбора поставщика для склада или магазина)
//      Возвращает кол-во добавленных поставщиков
//-----------------------------------------------------------------------------
int BIZ_ParseVendors(char* Page, TVendorList *Vendors)
{
    std::string strname;
    int res = 0;
    char *pBegin, *pEnd, *pRow;
    char *pct1, *pct2;
    char ts[32];
    int l;
    char ls[1000];

    char fVName[100];
    char fVURI[250];
    char fCName[100];
    char fCURI[250];
    int   fInStock;
    float fQuality;
    float fDelivery;
    int   fiDelivery;
    float fCost;
    int   fCurrency;
    int   fCityID;

    sVendorRec *Rec;

    if (!Page) return res;

    //LogMessage("Add Vendors.", ML_DBG3);

    pBegin = strstr(Page, "<table id=\"vendorstbl");          // начало таблицы
    pEnd = FindCloseTag(pBegin+10, "table");
    //pEnd = strstr(Page, "</tbody>");         // конец таблицы
    if ((pBegin) && (pEnd)) {
        pRow = pBegin;
        while ((pRow = strstr(pRow + 7, "<tr id=\"vendorsrow[")) != NULL) {            // Находим следующую строку с поставщиком
            if (pRow>pEnd) break;

            memset(fVURI, 0, sizeof(fVURI));
            memset(fVName, 0, sizeof(fVName));
            memset(fCURI, 0, sizeof(fCURI));
            memset(fCName, 0, sizeof(fCName));
            fInStock = 0;
            fQuality = 0;
            fDelivery = 0;
            fCost = 0;
            fCityID = 0;

            // Ищем  URI склада поставщика
            pct1 = strstr(pRow, "href=\"/units/");
            if (!pct1) continue;
            pct1 += 6;
            pct2 = strchr(pct1, '"');
            if (!pct2) continue;
            memcpy(fVURI, pct1, pct2 - pct1);

            // Ищем Название склада поставщика
            pct1 = strstr(pct2, "formsSelRadio(purchaseForm.vendor");
            if (!pct1) continue;
            pct1 = strchr(pct1 + 16, '>');
            if (!pct1) continue;
            pct1 += 1;
            pct2 = strstr(pct1, "</a>");
            if (!pct2) continue;
            memcpy(fVName, pct1, pct2 - pct1);

            // Ищем название и код города
            fCityID = 0;
            pct1 = strstr(pct2, "href=\"/city/?id=");
            if (pct1) {
                pct1 += 16;
                pct2 = strchr(pct1, '"');
                if (pct2) {
                    memset(ts, 0, sizeof(ts));
                    memcpy(ts, pct1, pct2 - pct1);
                    fCityID = atoi(SpaceRemove(ts));
                }
            }

            // Ищем Название URI компании поставщика
            pct1 = strstr(pct2, "href=\"/company/?id=");
            if (!pct1) continue;
            pct1 += 6;
            pct2 = strchr(pct1, '"');
            if (!pct2) continue;
            memcpy(fCURI, pct1, pct2 - pct1);

            // Ищем Название компании поставщика
            pct1 = strstr(pct1, "\">");
            if (!pct1) continue;
            pct1 += 2;
            pct2 = strstr(pct1, "</a>");
            if (!pct2) continue;
            memcpy(fCName, pct1, pct2 - pct1);

            // Ищем кол-во на складе
            pct1 = strstr(pct2, "<td");
            if (!pct1) continue;
            pct1 += 3;
            pct1 = strchr(pct1 + 20, '>');
            if (!pct1) continue;
            pct1 += 1;
            pct2 = strstr(pct1, "</td>");
            if (!pct2) continue;
            memset(ts, 0, sizeof(ts));
            memcpy(ts, pct1, pct2 - pct1);
            fInStock = atoi(SpaceRemove(ts));

            // Ищем качество
            pct1 = strstr(pct2, "<td id=\"quality");
            if (!pct1) continue;
            pct1 += 16;
            pct1 = strchr(pct1 + 20, '>');
            if (!pct1) continue;
            pct1 += 1;
            pct2 = strstr(pct1, "</td>");
            if (!pct2) continue;
            memset(ts, 0, sizeof(ts));
            memcpy(ts, pct1, pct2 - pct1);
            fQuality = atof(ts);

            // Ищем срок поставок
            fiDelivery = 0;
            pct1 = strstr(pct2, "<td id=\"days_");
            if (pct1) {
                pct1 += 13;
                pct2 = strchr(pct1, '\"');
                if (pct2) {
                    memset(ts, 0, sizeof(ts));
                    memcpy(ts, pct1, pct2 - pct1);
                    fDelivery = atof(ts);
                    fiDelivery = (fDelivery + 0.5);
                }
            }

            // Ищем стоимость
            pct1 = strstr(pct2, "<td id=\"cost_");
            if (!pct1) continue;
            pct1 = strchr(pct1 + 14, '>');
            if (!pct1) continue;
            pct1 += 1;
            pct2 = strstr(pct1, "</td>");
            if (!pct2) continue;
            memset(ts, 0, sizeof(ts));
            memcpy(ts, pct1, pct2 - pct1);
            if (!BIZ_ParsePrice(ts, fCost, fCurrency)) continue;

            Vendors->Add(fVName, fVURI, fCityID, fCName, fCURI, fInStock, fQuality, fiDelivery, 0, fCost, fCurrency);
            res++;
        }

    }
    else
        LogMessage("Ошибка разбора ParseVendors! Не найдены границы таблицы", ML_ERR3);

    return res;
}


//-----------------------------------------------------------------------------
//      Извлеч на странице среднюю цену (и качество) по городу на странице выбора поставщиков
//-----------------------------------------------------------------------------
float BIZ_ParseAveragePrice(char *Page, float &Quality)
{
    float res = 0;
    char *pc, *pc1, *pc2;
    char ts[32];
    float fQuality;

    if (!Page) return res;

    //LogMessage("ParsetAveragePrice ", ML_DBG3);

    pc = strstr(Page, "tfoot");
    if (pc) {
        pc += 5;

        // Ищем качество
        pc1 = strstr(pc, "id=\"quality_");
        if (pc1) {
            pc1 += 12;
            pc2 = strstr(pc1, "\">");
            if (pc2) {
                memset(ts, 0, sizeof(ts));
                memcpy(ts, pc1, pc2 - pc1);
                fQuality = atof(ts);

                //sprintf(ls, "Среднее кач-во : %s", ts);
                //WriteLogFile(ls);
            }
        }

        // Ищем стоимость
        pc1 = strstr(pc, "id=\"cost_");
        if (pc1) {
            pc1 += 12;
            pc2 = strstr(pc1, "\">");
            if (pc2) {
                memset(ts, 0, sizeof(ts));
                memcpy(ts, pc1, pc2 - pc1);
                res = atof(ts);
                Quality = fQuality;

                //sprintf(ls, "Средняя цена : %s", ts);
                //WriteLogFile(ls);
            }
        }

    }
    else
        LogMessage("Ошибка разбора ParsetAveragePrice! Не найдена tfoot таблицы", ML_ERR3);


    return res;

}

std::string GetIPStr(DWORD IP)
{
    std::string res;
    res = IntToStr((IP & 0xFF000000) >> 24);
    res += ".";
    res += IntToStr((IP & 0x00FF0000) >> 16);
    res += ".";
    res += IntToStr((IP & 0x0000FF00) >> 8);
    res += ".";
    res += IntToStr(IP & 0x000000FF);
    return res;
}