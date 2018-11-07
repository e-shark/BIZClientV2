
#include <string.h>
#include <string>

#include "BIZ_Parser.h"
#include "BIZ_Exceptions.h"
#include "LogUtils.h"


//-----------------------------------------------------------------------------
//      ������� ��� ������� �� ������
//      ������ ������ ����������� ������������� �������� #0
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
//      ���� ������ ����� � ������
//      ������ ����������� ������ ������������� �������� \0
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
    if (strstr(URI, "/office/")) return 1;              // �����
    if (strstr(URI, "/shop/")) return 2;                // �������
    if (strstr(URI, "/factory/")) return 3;             // �����
    if (strstr(URI, "/warehouse/")) return 4;           // �����
    if (strstr(URI, "/mine/")) return 5;                // �����, ��������
    if (strstr(URI, "/sawmill/")) return 6;             // ��������� 
    if (strstr(URI, "/animalfarm/")) return 7;          // ���������������� �����
    if (strstr(URI, "/farm/")) return 8;                // ��������������� �����
    if (strstr(URI, "/greenhouse/")) return 9;          // ��������� ���������
    if (strstr(URI, "/sciencelab/")) return 10;         // ����������������� ��������
    if (strstr(URI, "/construction/")) return 11;       // ������������ �����������
    if (strstr(URI, "/mediaunit/?")) return 12;         // ���
    if (strstr(URI, "/catering/")) return 13;           // ������������ �������
    if (strstr(URI, "/cargounit/")) return 14;          // ������������ �����������

    return 0;
}


//-----------------------------------------------------------------------------
//      ��������� ������ � ����� ��� POST ������� ����������� ����
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
        res += "+%D1%80.";      // "�."
        break;

    case curUSD:
        res = "%24";            //$
        res += ps;
        break;

    case curUAH:
        res = "+%D0%B3%D1%80%D0%BD."; //"���."
        res += ps;
        break;

    case curJPY:
    case curCNY:
    case curEUR:
    case curGBP:
        // ��� ������ ���� ��������. !!!
        res = ps;
    }

    return res;
}

//-----------------------------------------------------------------------------
//      ��������� �� �������� ����� �� �������
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
        // ���������, � �������� �� ��� ������
        pST = strstr(Page, "<html>");
        if (!pST) throw EParserException(1);

        // ���� �����, ���������� �� ����� �����
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

        // ���� ������ ������ � �����
        pST = strstr(pST + 6, "����:");
        if (!pST) throw EParserException(5);
        pST = strstr(pST + 5, "<b>");
        if (!pST) throw EParserException(6);
        pST += 3;
        pEnd = strstr(pST, "</b>");
        if (!pEnd) throw EParserException(7);

        // ���� ���� ������
        pc1 = pST;
        if (!pc1) throw EParserException(8);
        pc2 = strchr(pc1 + 1, ',');
        if (!pc2) throw EParserException(9);
        memset(DayStr, 0, sizeof(DayStr));
        memcpy(DayStr, pc1, pc2 - pc1);
        if (strstr(DayStr, "�����������")) ST.SrvWeekDay = 1;    //����
        if (strstr(DayStr, "�������")) ST.SrvWeekDay = 2;        //����
        if (strstr(DayStr, "�����")) ST.SrvWeekDay = 3;
        if (strstr(DayStr, "����")) ST.SrvWeekDay = 4;
        if (strstr(DayStr, "�����")) ST.SrvWeekDay = 5;
        if (strstr(DayStr, "�������")) ST.SrvWeekDay = 6;
        if (strstr(DayStr, "�������")) ST.SrvWeekDay = 7;

        // ���� �����
        pc1 = pc2 + 2;
        pc2 = strchr(pc1 + 1, ' ');
        if (!pc2) throw EParserException(10);
        memset(ts, 0, sizeof(ts));
        memcpy(ts, pc1, pc2 - pc1);
        ST.SrvDay = atoi(SpaceRemove(ts));

        // ���� �����
        pc1 = pc2 + 1;
        pc2 = strchr(pc1 + 1, ' ');
        if (!pc2) throw EParserException(11);
        memset(MonthStr, 0, sizeof(MonthStr));
        memcpy(MonthStr, pc1, pc2 - pc1);
        if (strstr(MonthStr, "������"))   ST.SrvMonth = 1;
        if (strstr(MonthStr, "�������"))  ST.SrvMonth = 2;
        if (strstr(MonthStr, "�����"))    ST.SrvMonth = 3;    //����
        if (strstr(MonthStr, "������"))   ST.SrvMonth = 4;
        if (strstr(MonthStr, "���"))      ST.SrvMonth = 5;
        if (strstr(MonthStr, "����"))     ST.SrvMonth = 6;
        if (strstr(MonthStr, "����"))     ST.SrvMonth = 7;
        if (strstr(MonthStr, "�������"))  ST.SrvMonth = 8;
        if (strstr(MonthStr, "��������")) ST.SrvMonth = 9;
        if (strstr(MonthStr, "�������"))  ST.SrvMonth = 10;
        if (strstr(MonthStr, "������"))   ST.SrvMonth = 11;
        if (strstr(MonthStr, "�������"))  ST.SrvMonth = 12;

        // ���� ���
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
        s = "ERROR BIZ_ParseServerTime : ������ �������� ������: ";
        s += IntToStr(e.Cod);
        LogMessage(s.c_str(), ML_ERR3);
        res = false;
    }
    return res;

}


//-----------------------------------------------------------------------------
//      ��������� ������������ ��, ��� ���
//  frame - �������, ��� ��� �� ��� ��������, � ������ ����� � ��� (default: true)
//-----------------------------------------------------------------------------
bool BIZ_CheckLogined(tHTML_Response *RSP, bool frame)
{
    char *pC1, *pC2;

    if (!RSP->Body) return false;
    if (!RSP->Header) return false;

    if (RSP->BodyLen < 5) return false;

    pC1 = strstr(RSP->Header, "Location: /user/login");    // ��������������� �� �������� �����
    if (pC1) return false;

    pC1 = strstr(RSP->Body, "login required");
    if (pC1) return false;

    pC1 = strstr(RSP->Body, "<html>");
    if (!pC1) return false;
    pC1 += 6;

    pC2 = strstr(pC1, "����� � ����");
    if (pC2) return false;

    pC2 = strstr(pC1, "href=\"/user/login");
    if (pC2) return false;

    if (!frame) {
        pC2 = strstr(pC1, "������������:");
        if (!pC2) return false;
    }

    return true;
}


//-----------------------------------------------------------------------------
//      ��������� ������ � ����� �� ���� � ������
//      ���������� ture ���� ������� ��������� ������
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
        // ������� ���������� ���������
        fPrice = 0;
        pc1 = my_strdig(str);
        if (!pc1) throw EParserException(1);
        memset(ts, 0, sizeof(ts));
        strncpy(ts, pc1, 31);
        fPrice = atof(SpaceRemove(ts));

        // ����� ���������� ������
        fCurrency = curUKNWN;
        if (strstr(str, "&yen;"))  fCurrency = curJPY;
        if (strstr(str, "?"))      fCurrency = curCNY;
        if (strstr(str, "&euro;")) fCurrency = curEUR;
        if (strstr(str, "�"))      fCurrency = curEUR;
        if (strstr(str, "���"))    fCurrency = curUAH;
        if (strstr(str, "�."))     fCurrency = curRUB;
        if (strstr(str, "$"))      fCurrency = curUSD;
        if (strstr(str, "&pound;")) fCurrency = curGBP;

        Currency = fCurrency;
        Price = fPrice;
        res = true;

    }
    catch (EParserException e) {
        s = "ERROR BIZ_ParsePrice: ������ �������� ������: ";
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

    // ����� ������ �������� �������������
    CityID = 0;
    pcCity = NULL;
    pcUnit = NULL;
    pcEndCity = strstr(Page, "compcountry");
    pcEndTable = FindCloseTag(pcEndCity, "table");
    if (!pcEndTable) throw EParserException(44);                                              // ������: ��� ����� ��������
    //pcEndCity = FindCloseTag(pcEndCity, "tr");
    //pcEndCity = FindCloseTag(pcEndCity, "tr");

    try {

        do {
            pcCity = strstr(pcEndCity, "/world/country/?id=");
            if ((!pcCity) && (!CityID)) throw EParserException(45);                                     // ������, ���� ��� �� ������ ������
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

                    // ���� �������� �������������
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

                        //���� URI �������������
                        pc1 = strstr(pc2 + 2, "href=\"/units/");
                        if (!pc1) throw EParserException(50);
                        pc1 += 6;
                        pc2 = strchr(pc1, '"');
                        if (!pc2) throw EParserException(51);
                        memset(tURI, 0, sizeof(tURI));
                        memcpy(tURI, pc1, pc2 - pc1);
                        // ��������� ID �������������
                        pc1 = strstr(tURI, "id=");
                        if (!pc1) throw EParserException(52);
                        UnitID = atoi(pc1 + 3);

                        // ���� ������ �� �������� (��� ������� ����� ������������ ������ �������������)
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
                                    LogMessage("Error BIZ_ParseCompanyStructure: �� ������� ������� ������ tBIZUnit", ML_ERR3);
                            }
                        }

                    }
                    else pcUnit = NULL;
                } while (pcUnit);
            }

        } while (pcCity);
    }   //try
    catch (EParserException e) {
        s = "ERROR BIZ_ParseOwnCompanyPage: ������ �������� ������: ";
        s += IntToStr(e.Cod);
        LogMessage(s.c_str(), ML_ERR3);
        //res = 0;
    }

    return res;
}

//-----------------------------------------------------------------------------
//      ������ ������� ������� � ��������
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

            // �������� ������
            pST = strstr(pST + 4, "<td");
            if (!pST) throw EParserException(4);

            // �������� ������
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

            // ���������� ������
            pST = strstr(pST + 4, "<td");
            if (!pST) throw EParserException(8);
            pST += 9;

            // ������� � �������
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
            //   �������
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

            // ���� ������� � �������
            pST = strstr(pST + 4, "<td");
            if (!pST) throw EParserException(14);
            pST += 3;

            // ���-�� �� ������
            pST = strstr(pST + 4, "<td align=\"center\">");
            if (!pST) throw EParserException(15);
            pc1 = strchr(pST + 10, '>');
            if (!pc1) throw EParserException(16);
            pc1 += 1;
            pc3 = strstr(pc1, "</td>");
            if (!pc3) throw EParserException(16);
            pc2 = strstr(pc1, "<b>");                                                 //������ ����� ������������ ���������� ������� (�������� ���� �� ������ ��� ������)
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

            // �������� ������ �� ������
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

            // ��������� ������ �� ������
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

            // ���� ������ �� �������
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

            // ������� �� ������ ��������
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

            // ������� �� ������ ����
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

            // ����� ������
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

            // ������� "���� ��������"
            pST = strstr(pST + 4, "<td");
            if (!pST) throw EParserException(36);
            pST += 3;

            // �������
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
        s = "ERROR BIZ_ParseShoopGoodsTable : ������ �������� ������: ";
        s += IntToStr(e.Cod);
        LogMessage(s.c_str(), ML_ERR3);
        //res = 0;
    }
    return res;
}


//-----------------------------------------------------------------------------
//      ������ ���������� �� ������� ������ � ��������
//-----------------------------------------------------------------------------
bool  BIZ_ParseProductInfoFromShop(char* Page, sBIZGoods *Info)
{
    bool res = false;
    std::string s;
    char *pc1, *pc2, *pc3;
    char *pcB, *pcE;
    char *pcRB, *pcRE;
    char ts[50];
    int   fInStock = 0;                               // ���������� ������ �� ������
    float fQuality = 0;                               // �������� ������
    float fCost = 0;                                  // ������������
    float fPrice = 0;                                 // ���� (���������)
    int   fCurrency = 0;                              // � ����� ������ ����    
    int   fDDay = 0;                                  // ���� ����
    int   fDMonth = 0;                                // ���� �����
    int   fDYear = 0;                                 // ���� ���
    int   fSales = 0;                                 // ����� ������
    int   fProceeds = 0;                              // �������
    int   fProfit = 0;                                // �������
    int i;

    sSalesHistoryRec SalesHistory[7];
    for (i = 0; i<7; i++)
        SalesHistory[i].Clear();

    if (!Page) return 0;

    try {
        // ���� ����� �� ��������, ��� ����������� ������ ��� ��������=�
        pcB = strstr(Page, "<tr align=\"center\"><td>�� ����</td><td>�� �������</td>");
        if (!pcB) throw EParserException(1);
        pcB = strstr(pcB, "<table");
        if (!pcB) throw EParserException(1);
        pcE = FindCloseTag(pcB+5, "table");
        if (!pcE) throw EParserException(1);
        pc1 = strstr(pcB, "<tbody>");
        if (pc1 && (pcB < pcE)) {                                                    // � ����� �������� ������ ����� � �� ����, ����� ������� ����� ������
            pcB = pc1;
            pcE = strstr(pcB, "</tbody>");
            if (!pcE) throw EParserException(1);

            // ������ ������� ������
            pcRB = strstr(pcB, "<tr class=\"tbl");
            i = 0;
            while (pcRB && (pcRB<pcE)) {
                pcRB += 10;
                pcRE = strstr(pcRB, "</tr>");
                if (!pcRE) throw EParserException(1);
                if (pcRE>pcE) throw EParserException(1);

                // ������ ���� ������
                pc1 = strstr(pcRB, "<td");
                if (!pc1) throw EParserException(1);
                pc1 = strchr(pc1, '>');
                if (!pc1) throw EParserException(1);
                pc1 += 1;
                pc3 = strstr(pc1, "</td>");
                if (!pc3) throw EParserException(1);
                if (pc3 >= pcRE) throw EParserException(1);
                // ... ����
                pc2 = strchr(pc1, '.');
                if (!pc2) throw EParserException(1);
                if (pc2 >= pc3) throw EParserException(1);
                memset(ts, 0, sizeof(ts));
                memcpy(ts, pc1, pc2 - pc1);
                fDDay = atoi(ts);
                // ... �����
                pc1 = pc2 + 1;
                pc2 = strchr(pc1, '.');
                if (!pc2) throw EParserException(1);
                if (pc2 >= pc3) throw EParserException(1);
                memset(ts, 0, sizeof(ts));
                memcpy(ts, pc1, pc2 - pc1);
                fDMonth = atoi(ts);
                // ... ���
                pc1 = pc2 + 1;
                pc2 = pc3;
                memset(ts, 0, sizeof(ts));
                memcpy(ts, pc1, pc2 - pc1);
                fDYear = atoi(ts);

                // ������ ����� ������
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

                // ������ ����
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

                // ������ �������
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

                // ������ �������
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

                // ���� ������ ��������� ������
                pcRB = strstr(pcRB, "<tr class=\"tbl");
                i++;
            }
        }
        pcB = pcE;

        // ���� ������ ��������� ������
        pcB = strstr(pcB, "<table cellpadding=\"4\" cellspacing=\"1\" class=\"datatable\" width=\"100%\">");
        if (!pcB) throw EParserException(1);
        pcE = strstr(pcB, "</table>");
        if (!pcE) throw EParserException(1);

        // ��� ����� ��������
        pcRB = strstr(pcB, "<tr class=\"tbl");
        if (!pcRB) throw EParserException(1);
        pcRB += 10;
        pcRE = strstr(pcRB, "</tr>");
        if (!pcRE) throw EParserException(1);
        if (pcRE>pcE) throw EParserException(1);

        // ����� ������ ��������� ������: ���-��, ���� � ��...
        pcRB = strstr(pcRE, "<tr align=\"center\">");
        if (!pcRB) throw EParserException(1);
        pcRB += 10;
        pcRE = strstr(pcRB, "</tr>");
        if (!pcRE) throw EParserException(1);
        if (pcRE>pcE) throw EParserException(1);

        // ��� �������
        pc1 = strstr(pcRB, "<td");
        if (!pc1) throw EParserException(1);
        pc1 += 1;
        pc3 = strstr(pc1, "</td>");
        if (!pc3) throw EParserException(1);
        if (pc3 >= pcRE) throw EParserException(1);

        // ������ ���-�� ������ �� ������
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

        // ������ �������� ������ �� ������
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

        // ��� ������ ���� �����-�� "�����"
        pc1 = strstr(pc3, "<td");
        if (!pc1) throw EParserException(1);
        pc1 += 1;
        pc3 = strstr(pc1, "</td>");
        if (!pc3) throw EParserException(1);
        if (pc3 >= pcRE) throw EParserException(1);

        // ������ ������������� ������ �� ������
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

        // ������ ��������� ���� ������
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


        // ����� ���� �������������� ������
        pcRB = strstr(pcRE, "<tr align=\"center\" class=\"tblgr\">");
        if (!pcRB) throw EParserException(1);
        pcRB += 10;
        pcRE = strstr(pcRB, "</tr>");
        if (!pcRE) throw EParserException(1);
        if (pcRE>pcE) throw EParserException(1);

        // ������ ������� �������� �� ������
        // ���� ���. �� ������.
        // ������ ���.
        // �� ���� �����������, ����� ��� ��������.
        // �� ��� ������

        // ������ ������� ���� �� ������
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

        Info->Reques *= 7;  // ����������� ��������� �����������, ��� ����� ������� �������� �������, ���������� �� 7 ����

        res = true;
    }
    catch (EParserException e) {
        s = "ERROR BIZ_ParseProductInfoFromWarehouse : ������ �������� ������: ";
        s += e.Cod;
        LogMessage(s.c_str(), ML_ERR3);
        //res = 0;
    }
    return res;
}


//-----------------------------------------------------------------------------
//      �������� ���-�� �������, �� ������� �������� ���� ������ �����������
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
//      �������� ����������� ������ �� �������� �� ������ ���������, � ������
//      (������ �������� ������ ���������� ��� ������ ��� ��������)
//      ���������� ���-�� ����������� �����������
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

    pBegin = strstr(Page, "<table id=\"vendorstbl");          // ������ �������
    pEnd = FindCloseTag(pBegin+10, "table");
    //pEnd = strstr(Page, "</tbody>");         // ����� �������
    if ((pBegin) && (pEnd)) {
        pRow = pBegin;
        while ((pRow = strstr(pRow + 7, "<tr id=\"vendorsrow[")) != NULL) {            // ������� ��������� ������ � �����������
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

            // ����  URI ������ ����������
            pct1 = strstr(pRow, "href=\"/units/");
            if (!pct1) continue;
            pct1 += 6;
            pct2 = strchr(pct1, '"');
            if (!pct2) continue;
            memcpy(fVURI, pct1, pct2 - pct1);

            // ���� �������� ������ ����������
            pct1 = strstr(pct2, "formsSelRadio(purchaseForm.vendor");
            if (!pct1) continue;
            pct1 = strchr(pct1 + 16, '>');
            if (!pct1) continue;
            pct1 += 1;
            pct2 = strstr(pct1, "</a>");
            if (!pct2) continue;
            memcpy(fVName, pct1, pct2 - pct1);

            // ���� �������� � ��� ������
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

            // ���� �������� URI �������� ����������
            pct1 = strstr(pct2, "href=\"/company/?id=");
            if (!pct1) continue;
            pct1 += 6;
            pct2 = strchr(pct1, '"');
            if (!pct2) continue;
            memcpy(fCURI, pct1, pct2 - pct1);

            // ���� �������� �������� ����������
            pct1 = strstr(pct1, "\">");
            if (!pct1) continue;
            pct1 += 2;
            pct2 = strstr(pct1, "</a>");
            if (!pct2) continue;
            memcpy(fCName, pct1, pct2 - pct1);

            // ���� ���-�� �� ������
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

            // ���� ��������
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

            // ���� ���� ��������
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

            // ���� ���������
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
        LogMessage("������ ������� ParseVendors! �� ������� ������� �������", ML_ERR3);

    return res;
}


//-----------------------------------------------------------------------------
//      ������ �� �������� ������� ���� (� ��������) �� ������ �� �������� ������ �����������
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

        // ���� ��������
        pc1 = strstr(pc, "id=\"quality_");
        if (pc1) {
            pc1 += 12;
            pc2 = strstr(pc1, "\">");
            if (pc2) {
                memset(ts, 0, sizeof(ts));
                memcpy(ts, pc1, pc2 - pc1);
                fQuality = atof(ts);

                //sprintf(ls, "������� ���-�� : %s", ts);
                //WriteLogFile(ls);
            }
        }

        // ���� ���������
        pc1 = strstr(pc, "id=\"cost_");
        if (pc1) {
            pc1 += 12;
            pc2 = strstr(pc1, "\">");
            if (pc2) {
                memset(ts, 0, sizeof(ts));
                memcpy(ts, pc1, pc2 - pc1);
                res = atof(ts);
                Quality = fQuality;

                //sprintf(ls, "������� ���� : %s", ts);
                //WriteLogFile(ls);
            }
        }

    }
    else
        LogMessage("������ ������� ParsetAveragePrice! �� ������� tfoot �������", ML_ERR3);


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