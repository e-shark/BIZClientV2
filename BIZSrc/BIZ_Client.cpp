

#include "BIZ_Client.h"
#include "LogUtils.h"
#include "BIZ_DB.h"
#include "BIZ_Parser.h"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
tBIZ_Client::tBIZ_Client(void) : tBIZ_Transport()
{
    PersonCompanyID = 0;
    Company = NULL;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
tBIZ_Client::~tBIZ_Client()
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool tBIZ_Client::LoadPerson(int PID)
{
    bool res = false;
    std::string login, psw, srv, torip;
    int pxl, cid, torport, torcmdport;

    if (DB_GetPerson(PID, srv, login, psw, pxl, cid, torip, torport, torcmdport)) {
        ServerName = srv;
        UserLogin = login;
        UserPassword = psw;
        PersonCompanyID = cid;
        if (torport) {
            TOR_SetUp((char*)torip.c_str(), torport, torcmdport);
            TOR_On();
        }
        res = true;
    }
    return res;
}

//-----------------------------------------------------------------------------
//      ��������, ���������� �� �� �����, ��� ���.
//      ����� ���������, ����������� ���������� ��������� �� ��������������
//      ��� �������������� ����� ������� ����� ��������������� �� �������� �����
//-----------------------------------------------------------------------------
bool tBIZ_Client::CheckLogin(void)
{
    bool res = false;
    bool getres = false;
    tHTML_Response *RSP;
    char *pc1;

    RSP = new tHTML_Response();
    getres = POST("/user/messages/updatecontacts/?59", "", RSP);
    if (getres) {
        res = true;
        pc1 = strstr(RSP->Header, "Location: /user/login");
        if (pc1) res = false;
    }
    delete RSP;
    return res;
}

//-----------------------------------------------------------------------------
//      ������� ������ � ����� ������
//  ����������� ������� ������ ���������� ���������� � �������
//  �� ���������� ������ ���������. �������� ������� ������� ��� ����������� ����
//  ����� �������� ������ �������� ���������������
//-----------------------------------------------------------------------------
void tBIZ_Client::WirteReport(const char *Text, int type)
{
    //DB_AddActionLog(char *Text, int type);
    LogMessage(Text, type);
}

//-----------------------------------------------------------------------------
//      �������� ����� �� ����������� ����
//-----------------------------------------------------------------------------
bool tBIZ_Client::GetDailyBonus(void)
{
    bool res = false;
    bool getres = false;
    tHTML_Response *RSP;
    char *pc1;

    LogMessage("==== �������� ���������� ����� ====", ML_WRK2);

    RSP = new tHTML_Response();

    getres = POST("/user/dailybonus/", "$post=on", RSP);
    if (getres) {

        // �������� �� ����������� �� ��� �� ������
        pc1 = strstr(RSP->Header, "Location: /user/login");
        if (pc1) {
            // ��� ����������� ����������
            if (Login()) {
                // ������� ��� ���
                getres = POST("/user/dailybonus/", "$post=on", RSP);
                if (getres) {
                    /*
                    // ��� ��� ������ �� ���� ����� ��� ���������� ���������� ������� �� ������ �������
                    // ������ ����� ���������������
                    // ��������� �����
                    // � ����, ��������, � ������� ������ ����� ����
                    pc1=strstr(RSP->Header,"window.parent.reloadProducts");
                    if (pc1) res = false;
                    */

                    // ����� ����������� ��� ��������� �������� ... ����� � ������
                    res = true;
                }
            }
        }
        else res = true;

    }
    delete RSP;

    return res;
}


//-----------------------------------------------------------------------------
//      �������� ��������� ����� ������� �����
//   bLog - ������ ��������� � ����
// ����������:
//   PurchasePrice - ���������� ����, (�� ������� ����� ������) (�� ������� ����� �������)
//   SellingPrice  - ��������� ����, (�� ������� ����� �������) (�� ������� ����� ��������)
//-----------------------------------------------------------------------------
bool tBIZ_Client::GetExchangeState( long &PurchasePrice,
                                    long &SellingPrice )
{
    bool res1 = false;
    bool res2 = false;
    std::string s;
    bool getres;
    char *pC1, *pC2;
    char ts[50];
    int i;
    long long thous;
    FILE *f;
    tHTML_Response *RSP = new tHTML_Response();

    LogMessage("==== �������� ���� ����� �� ====", ML_WRK2);

    //--------------------------------------------------------------------
    // ������� ������ �������� ������� �����
    s = "/exchange/points/?currency=RUB&tab=buy";
    getres = GetAuthorizedPage((char*)s.c_str(), RSP);
    if (getres) {

        BIZ_ParseServerTime(RSP->Body, &SrvTime);

        pC1 = strstr(RSP->Body, "<html>");
        if (pC1) {
            // ������ ����������� ������
            // �������� ����� ������� ����, �� ������� ����� ������
            // (��� � ����� ������ �������)

            // ���� ������ �������
            pC1 = strstr(RSP->Body, "������ ����������� �� �������");
            if (pC1) {
                pC1 += 29;
                pC1 = strstr(pC1, "/person/?id=");
                if (pC1) {
                    pC1 += 12;
                    // ���� ����
                    pC1 = strstr(pC1, "nowrap>");
                    if (pC1) {
                        pC1 += 7;
                        pC2 = strchr(pC1, '&');
                        if (!pC2)
                            pC2 = strstr(pC1, "</td>");      // �� ������ ������ (����� ��������� ���������� ��� � ������ ������� ���������)
                        if (pC2) {
                            if (pC2 - pC1<50) {
                                // ����� ������ ����
                                // ��� ����� ����� ���������� ������� ������� ��� atoi, ������� ���� �� ������
                                thous = 0;
                                if (pC2 - pC1>3) {
                                    memset(ts, 0, sizeof(ts));
                                    memcpy(ts, pC1, pC2 - pC1 - 3);
                                    thous = atol(SpaceRemove(ts));
                                    pC1 = pC2 - 3;
                                }
                                memset(ts, 0, sizeof(ts));
                                memcpy(ts, pC1, pC2 - pC1);
                                i = atol(SpaceRemove(ts));
                                PurchasePrice = 1000.0 * thous;
                                PurchasePrice += i;

                                res1 = true;
                            }
                        }
                    }
                }
            }
        }
        else {
            LogMessage("GetExchangeState: �� ������ ��� <html> �� �������� ������� !", ML_ERR1);
            res1 = false;
        }
    }


    //--------------------------------------------------------------------
    // ������ ������ �������� ������ �����

    RSP->Clear();
    s = "/exchange/points/?currency=RUB&tab=sell";
    getres = GetAuthorizedPage((char*)s.c_str(), RSP);

    if (getres) {

        pC1 = strstr(RSP->Body, "<html>");
        if (pC1) {
            // ������ ����������� �������
            // �������� ����� ������� ����, �� ������� ����� �������
            // (��� � ����� ������ �������)

            // ���� ������ �������
            pC1 = strstr(RSP->Body, "������ ����������� �� �������");
            if (pC1) {
                pC1 += 29;
                pC1 = strstr(pC1, "/person/?id=");
                if (pC1) {
                    pC1 += 12;
                    // ���� ����
                    pC1 = strstr(pC1, "nowrap>");
                    if (pC1) {
                        pC1 += 7;
                        pC2 = strchr(pC1, '&');
                        if (!pC2)
                            pC2 = strstr(pC1, "</td>");      // �� ������ ������ (����� ��������� ���������� ��� � ������ ������� ���������)
                        if (pC2) {
                            if (pC2 - pC1<50) {
                                // ����� ������ ����
                                // ��� ����� ����� ���������� ������� ������� ��� atoi, ������� ���� �� ������
                                thous = 0;
                                if (pC2 - pC1>3) {
                                    memset(ts, 0, sizeof(ts));
                                    memcpy(ts, pC1, pC2 - pC1 - 3);
                                    thous = atol(SpaceRemove(ts));
                                    pC1 = pC2 - 3;
                                }
                                memset(ts, 0, sizeof(ts));
                                memcpy(ts, pC1, pC2 - pC1);
                                i = atol(SpaceRemove(ts));
                                SellingPrice = 1000 * thous;
                                SellingPrice += i;

                                res2 = true;
                            }
                        }
                    }
                }
            }
        }
        else {
            s = "ERROR GetExchangeState: �� ������ ��� <html> �� �������� ������ !";
            LogMessage(s.c_str(),ML_DBG1);
            res2 = false;
        }
    }

    delete RSP;

    return res1 && res2;
}

//-----------------------------------------------------------------------------
//  �������� � ��������� � ���� �������� ���� ��
//-----------------------------------------------------------------------------
bool tBIZ_Client::SaveExchangeState(void)
{
    bool res = false;
    long PurchasePrice;
    long SellingPrice;

    LogMessage("==== �������� � ������� ���� ����� �� ====", ML_WRK3);

    if (GetExchangeState(PurchasePrice, SellingPrice)) {
        DB_SaveExchangeState(PurchasePrice, SellingPrice, &SrvTime);
        res = true;
    }
    return res;
}

//-----------------------------------------------------------------------------
//  ������������� ������� ��������� ����� �� ������ ��
//-----------------------------------------------------------------------------
bool tBIZ_Client::ImportExchangeState(std::string BDName)
{
	std::string str;
	str = "==== ������� ������� ���� ����� �� �� ";
	str += BDName;
	str += " ====";
	LogMessage(str.c_str(), ML_WRK2);
	return DB_ExternExchangeSate(BDName);
}

//-----------------------------------------------------------------------------
// �������� �� �������� ������� ������� ����� �������
//-----------------------------------------------------------------------------
bool tBIZ_Client::TryGetServerTime(char* Page)
{
    bool res = true;
    res = BIZ_ParseServerTime(Page, &SrvTime);
    return res;
}

//-----------------------------------------------------------------------------
//  �������� ���������� � ��������
//-----------------------------------------------------------------------------
int tBIZ_Client::GetcompanyStructure(int cid, tmUnits *Company)
{

    int res = 0;
    char URI[256];
    bool getres;
    tHTML_Response *RSP;
    bool bDailyBonusPresent = false;

    if (!cid) return 0;

    RSP = new tHTML_Response();

    sprintf(URI,"/company/id/%d", cid);
    getres = GetPublicPage(URI, RSP);

    if (getres) {

        if (BIZ_CheckLogined(RSP)) {
            TryGetServerTime(RSP->Body);

            // ����� ���� �������� ����� ��������
            res = BIZ_ParseCompanyStructure(RSP->Body, Company);
        }
        // ����� ����� �������� ��������� ����� �������� BIZ_ParseStrangeCompanyPage
    }
    delete RSP;
    return res;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool tBIZ_Client::GetCompanyInfo(void)
{
    bool result = false;

    char URI[256];
    bool getres;
    tHTML_Response *RSP;
    //tmUnits *Company;

    RSP = new tHTML_Response();
    if (Company) delete Company;
    Company = new tmUnits();

    // ... ��� ��� ���� ������ ��������� ������� �������� � ���������� ID �������� ��� �������

    sprintf(URI, "/company/id/%d", PersonCompanyID);
    getres = GetAuthorizedPage(URI, RSP);

    if (getres) {

            TryGetServerTime(RSP->Body);
            if (GetcompanyStructure(PersonCompanyID, Company) > 0)
                if (DB_SaveUnitsList(PersonCompanyID, Company))
                    result = true;


   //         tBIZUnitStructure *Company = new tBIZUnitStructure();
            // ����� ���� �������� ����� ��������
   //         res = BIZ_ParseCompanyStructure(RSP->Body, Company);
            //DB_SaveCompanyStructure
   //         delete Company;

            //BIZ_ParseOwnCompanyInfo(RSP->Body, Company, &bDailyBonusPresent);
            //DB_AddCompanyHistory


       // ����� ����� �������� ��������� ����� �������� BIZ_ParseStrangeCompanyPage
    }
    //delete Company;
    delete RSP;
    return result;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool tBIZ_Client::Test1(void)
{
    std::string s;
    bool getres;
    tHTML_Response *RSP = new tHTML_Response();

    LogMessage("==== Test 1 ====", ML_WRK2);

    TOR_SetUp("127.0.0.1");
    //TOR_On();

    DWORD addr;
    bool tp;
    addr = TOR_Check(&tp);
    s = "tor addr: ";
    s += IntToStr((addr >> 24) & 0xFF); s += ".";
    s += IntToStr((addr >> 16) & 0xFF); s += ".";
    s += IntToStr((addr >> 8) & 0xFF); s += ".";
    s += IntToStr((addr >> 0) & 0xFF);
    LogMessage((char*)s.c_str(), ML_WRK2);

    addr = CheckIP();
    s = "tor addr: ";
    s += IntToStr((addr >> 24) & 0xFF); s += ".";
    s += IntToStr((addr >> 16) & 0xFF); s += ".";
    s += IntToStr((addr >> 8) & 0xFF); s += ".";
    s += IntToStr((addr >> 0) & 0xFF);
    LogMessage((char*)s.c_str(), ML_WRK2);

    LogMessage("Turn On TOR", ML_WRK2);
    TOR_On();

    addr = CheckIP();
    s = "tor addr: ";
    s += IntToStr((addr >> 24) & 0xFF); s += ".";
    s += IntToStr((addr >> 16) & 0xFF); s += ".";
    s += IntToStr((addr >> 8) & 0xFF); s += ".";
    s += IntToStr((addr >> 0) & 0xFF);
    LogMessage((char*)s.c_str(), ML_WRK2);

    LogMessage("Try Change IP", ML_WRK2);
    //TOR_ChangeIP();
    bool opres = false;
    opres = ChangeIP();
    if (opres) LogMessage("Change Ok", ML_WRK2);
    else LogMessage("Change fail", ML_WRK2);

    addr = CheckIP();
    s = "tor addr: ";
    s += IntToStr((addr >> 24) & 0xFF); s += ".";
    s += IntToStr((addr >> 16) & 0xFF); s += ".";
    s += IntToStr((addr >> 8) & 0xFF); s += ".";
    s += IntToStr((addr >> 0) & 0xFF);
    LogMessage((char*)s.c_str(), ML_WRK2);


    //--------------------------------------------------------------------
    // ������� ������ �������� ������� �����
    //s = "/exchange/points/?currency=RUB&tab=buy";
    //getres = GetPublicPage((char*)s.c_str(), RSP);
    s = "https://check.torproject.org";
    getres = GetInternetPage((char*)s.c_str(), RSP);
    if (getres) {
        LogMessage("==== ���� �������� ====", ML_DBG3);
    }
    return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool tBIZ_Client::Test2(void)
{

    bool res = false;
    bool getres = false;
    tHTML_Response *RSP;
    char *pc1;

    LogMessage("==== �������� ����� ������  ====", ML_WRK2);

    RSP = new tHTML_Response();

    getres = POST("/city/map/?id=27", "$post=on&cmd=init&r=4309", RSP);
    if (getres) {

        // �������� �� ����������� �� ��� �� ������
        pc1 = strstr(RSP->Header, "Location: /user/login");
        if (pc1) {
            // ��� ����������� ����������
            if (Login()) {
                // ������� ��� ���
                getres = POST("/user/dailybonus/", "$post=on", RSP);
                if (getres) {
                    /*
                    // ��� ��� ������ �� ���� ����� ��� ���������� ���������� ������� �� ������ �������
                    // ������ ����� ���������������
                    // ��������� �����
                    // � ����, ��������, � ������� ������ ����� ����
                    pc1=strstr(RSP->Header,"window.parent.reloadProducts");
                    if (pc1) res = false;
                    */

                    // ����� ����������� ��� ��������� �������� ... ����� � ������
                    res = true;
                }
            }
        }
        else res = true;

    }
    delete RSP;

    return res;

}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool tBIZ_Client::Test3(std::string BDName)
{
	LogMessage("==== ���� ������ � ��  ====", ML_WRK2);
	DB_ExternExchangeSateEx(BDName);
	return true;
}

//-----------------------------------------------------------------------------
//      �������� ���������� �� ������� � ��������      
//-----------------------------------------------------------------------------
//http://bizmania.ru/units/shop/?id=NNNNNNN&tab=goods
//-----------------------------------------------------------------------------
int tBIZ_Client::GetShopGoodsList(int unitId, tmGoodsList *pGL, tmCityPriceMap *pCPM)
{
    int res = 0;
    std::string s;
    tHTML_Response *RSP = new tHTML_Response();
    bool getres;
    char *pct1;

    s = "  �������� �������� ������� �������� (";
    s += IntToStr(unitId); 
	s += ")";
    LogMessage(s.c_str(), ML_WRK3);

    s = "/units/shop/?id=";
    s += IntToStr(unitId);
    s += "&tab=goods";
    getres = GetAuthorizedPage((char*)s.c_str(), RSP);

    if (getres) {

        pct1 = strstr(RSP->Body, "<html>");
        if (pct1) {
            // ������ ��������
			try {
				res = BIZ_ParseShoopGoodsTable(RSP->Body, pGL, pCPM);
			}
			catch (...) {
				LogMessage("ERROR GetShopGoodsList: exception � ��������� BIZ_ParseShoopGoodsTable !", ML_ERR2);
			}
        }
        else {
            LogMessage("ERROR GetShopGoodsList: �� ������ ��� <html> �� �������� !", ML_ERR2);
            res = 0;
        }

    }
    delete RSP;
    return res;

}


//-----------------------------------------------------------------------------
//      ������� ���� ������� � ��������
//-----------------------------------------------------------------------------
bool tBIZ_Client::SetGoodsPrice(int unitId)
{
    bool res = false;
    bool getres = false;
    tHTML_Response *RSP;
    char *pc1;
    std::string sm;
    std::string sh;
    std::string sb;
    tmGoodsList GL;
    tmCityPriceMap CPM;
    tmGoodsList::iterator iGL;
    tmCityPriceMap::iterator iCPM;
//    tmPriceFactor::iterator iPF;
    float PFactor;
    float Price;
    int i;
    char sp[5];
    int p1, p2, p3, p4;

    // !!! .... ��� ����� ���� ������ ����� �� ����
    float fDefaultDumping = -5;                // �� ������� ��������� ������ ����� ��������� ���� ��������� ��� �������������� ���� (-99% .. +99%)
    float fPermittedExcess = 10;               // �� ������� ��������� ���� �� ������� ����� ��������� ������� ���� �� ������ (-99% .. +99%)

    int resLevel;
    int resFactor;

    RSP = new tHTML_Response();

    sh = "==== Set Goods Price in Shop. ID= ";
    sh += IntToStr(unitId); sh += " ====";
    LogMessage(sh.c_str(), ML_WRK2);

    GL.clear();
    CPM.clear();
    GetShopGoodsList(unitId, &GL, &CPM);

    sh = "/units/shop/?id=";
    sh += IntToStr(unitId);
    sh += "&tab=goods";
    sb = "";

    i = 0;
    for (iGL = GL.begin(); iGL != GL.end(); iGL++) {

        // ���� ������ ��� �� ������ ��������, �� ��������� ������� ����
        if (0 == (*iGL).second.Available) continue;
        if (0 == (*iGL).second.Cost) continue;
        //if (0 == (*iGL).second.Price) continue;

        // ���� ����������� ��� ���� � ������, ���������� � �������� ��������� ���������
        if (DB_GetFactor(1, (*iGL).second.ID, unitId, resLevel, resFactor)) {
            Price = resFactor;
            sm = " ��� ������ ";
            sm += IntToStr((*iGL).second.ID);
            sm += " ���������� ���� ";
            sm += IntToStr(resFactor);
            sm += " ";
            sm += DB_GetFactorLocationTypeStr(resLevel);
            WirteReport(sm.c_str(), ML_WRK3);
        }
        else {
            if (DB_GetFactor(2, (*iGL).second.ID, unitId, resLevel, resFactor)) 
                fDefaultDumping = resFactor;
            if (DB_GetFactor(3, (*iGL).second.ID, unitId, resLevel, resFactor)) 
                fPermittedExcess = resFactor;

            PFactor = 1 + fDefaultDumping / 100.0;

            // ��������� ����, ������� ������� �� ������ (���� ����) �� �����������  
            iCPM = CPM.find((*iGL).first);                                                              // �������� ������� �� ������ ����

            if ((CPM.end() != iCPM) && (iCPM->second.Quality > 0.0001)) {
                Price = iCPM->second.Price * PFactor *((*iGL).second.Quality / iCPM->second.Quality);
                if (Price > iCPM->second.Price * (1.0 + fPermittedExcess / 100))                            // �� ��������� ��������� ���� ���� �������
                    Price = iCPM->second.Price* (1.0 + fPermittedExcess / 100);
            }
            else
                Price = (*iGL).second.Price;                                                            // ���� ������-�� ��� ������� ����, ��������� �������� ����

            if (Price < (*iGL).second.Cost * 1.1)
                Price = (*iGL).second.Cost * 1.1;                                                       // �� ��������� �������� ���� ���� ������������� + 10%

                                                                                                        // ���� �� �����-�� �������� ���� ���������� �������, �� ��������� ������� ����
            if (0 == Price) continue;

            // ��������� ���� (��� �������)
            if (Price > 10)
                Price = int(Price + 0.5);
            if (Price > 100)
                Price = (int(Price / 10 + 0.5) * 10);
            if (Price > 1000)
                Price = (int(Price / 100 + 0.5) * 100);
            //if (Price>10000)
            //  Price = (int(Price/1000 + 0.5)*1000);
        }

        // ��������� ��������
        if (i > 0) sb += "&";
        sb += "product[";
        sb += IntToStr(i);
        sb += "]=";
        sb += IntToStr((*iGL).first);
        sb += "&price[";
        sb += IntToStr(i);
        sb += "]=";


        sb += GetPOSTPriceByCurrency(Price, (*iGL).second.Currency);            // ��������� ����, �������� ������ �� ������ (��������� 26.VIII)
        sb += "&price_currency=";        // ��� ��������� 26.VIII
        sb += GetCurrencyName((*iGL).second.Currency);
        sb += "&purchaseQuantity[";
        sb += IntToStr(i);
        sb += "]=";
        sb += IntToStr((*iGL).second.Purchase);

        // ����������� ����� ���������
        i++;
    }


    //sb += "&autoSupply=on";
    //sb += "&autoSupplyReserve=1.2";
    sb += "&$post=on&sort=null";
    sb += "&p=1";

    getres = POST(sh.c_str(), sb.c_str(), RSP);
    if (getres) {
        res = true;
        pc1 = strstr(RSP->Header, "/units/shop/?id=");
        if (!pc1) res = false;
    }
    delete RSP;

    if (res) {
        sh = " ����������� �������� � �������� id=";
        sh += IntToStr(unitId);
        WirteReport(sh.c_str(), ML_WRK2);
    }
    else {
        sh = " �� ������� ���������� �������� � �������� id=";
        sh += IntToStr(unitId);
        WirteReport(sh.c_str(), ML_ERR2);
    }

    return res;
}


//-----------------------------------------------------------------------------
//      �������� ���������� �� ������ � ��������
//-----------------------------------------------------------------------------
//http://bizmania.ru/units/shop/?id=NNNNN&tab=goods&product=110
//-----------------------------------------------------------------------------
bool tBIZ_Client::GetShopProductInfo(int WHID, int ProductID, sBIZGoods *Info)
{
    bool res = false;
    std::string s;
    tHTML_Response *RSP = new tHTML_Response();
    bool getres;
    char *pct1;

    s = "  Get Info for Product from Shop ID= ";
    s += IntToStr(WHID);        s += " product ";
    s += IntToStr(ProductID);
    LogMessage(s.c_str(), ML_WRK3);

    memset(Info, 0, sizeof(sBIZGoods));

    s = "/units/shop/?id=";
    s += IntToStr(WHID);
    s += "&tab=goods&product=";
    s += IntToStr(ProductID);
    getres = GetAuthorizedPage(s.c_str(), RSP);

    if (getres) {

        pct1 = strstr(RSP->Body, "<html>");
        if (pct1) {
            TryGetServerTime(RSP->Body);

            // ������ ��������
            res = BIZ_ParseProductInfoFromShop(RSP->Body, Info);
            Info->ID = ProductID;
            res = true;
        }
        else {
            s = "ERROR GetShopProductInfo: �� ������ ��� <html> �� �������� !";
            LogMessage(s.c_str(), ML_ERR2);
            res = false;
        }

    }

    delete RSP;
    return res;
}


//-----------------------------------------------------------------------------
//      �������� �������� ����������� ������ Product ��� �������� ��� ������ unitId
//      ���������� ���-�� ����������� � ������
//-----------------------------------------------------------------------------
int tBIZ_Client::GetVendorsForUnit(int unitId, int Product, TVendorList *Vendors)
{
    int res = 0;
    int page = 0;
    int PN = 0;
    std::string s;
    bool getres;
    char *tpc;
    float favprice = 0;
    float favqual = 0;
    tHTML_Response *RSP = new tHTML_Response();

    s = "  Get Vendors For for Shop ID= ";
    s += IntToStr(unitId);      
	s += " product ";
    s += IntToStr(Product);
    LogMessage(s.c_str(), ML_WRK3);

    //if (fOnProgress) fOnProgress(1);

    //Vendors->Clear();
    //Vendors->ProductId = Product;
    if (Vendors->ProductId != Product) {
        s = "   ��������� ProductId (";
        s += IntToStr(Product);
        s += ") �� ��������� � ������� � ������ (";
        s += IntToStr(Vendors->ProductId);
        s += ")";
        LogMessage(s.c_str(), ML_ERR2);
        return res;
    }

    PN = 1;
    getres = true;
    while (getres && (page<PN)) {
        page++;

        // ��������� ������
        s = "/units/vendor/select/?id=";
        s += IntToStr(unitId);
        s += "&product=";
        s += IntToStr(Product);
        s += "&sort=quality";
        s += "&p=";
        s += IntToStr(page);
        getres = GetAuthorizedPage(s.c_str(), RSP);

        if (getres) {

            tpc = strstr(RSP->Body, "<html>");
            if (!tpc) {
                s = "  �� ������ ��� <html> �� �������� ";
                s += IntToStr(page);
                LogMessage(s.c_str(), ML_ERR2);
                return res;
            }


            if (BIZ_CheckLogined(RSP))
            {
                // �� ������ �������� ���������� ������� ����� ������� � �������
                if (page == 1) {

                    PN = BIZ_ParseVendorsPagesCount(RSP->Body); // ���� �������� ������ ����, � ��� paginator, �� ������� ������ 0
                    if (0 == PN) PN = 1;    // ��� ����������

                    s = "   ���-�� ������� : ";
                    s += IntToStr(PN);
                    LogMessage(s.c_str(), ML_DBG3);

                    favprice = BIZ_ParseAveragePrice(RSP->Body, favqual);
                    if (favprice) {
                        Vendors->fAveragePrice = favprice;
                        Vendors->fAverageQuality = favqual;
                    }
                }

                //if (fOnProgress) if (PN) fOnProgress(100 / PN*page); else fOnProgress(100);

                // ��������� ���� ����������� �� ��������
                BIZ_ParseVendors(RSP->Body, Vendors);
            }
        }
    }  //while

    if (getres) {
        s = "   ��������� ";
        s += IntToStr(page);
        s += " ������� �� ";
        s += IntToStr(PN);
        LogMessage(s.c_str(), ML_DBG3);
    }
    else {
        s = "   ��������� ������ ";
        s += IntToStr(page);
        s += " ������� �� ";
        s += IntToStr(PN);
        LogMessage(s.c_str(), ML_DBG3);
    }

    //if (fOnProgress) fOnProgress(0);

    res = Vendors->Count();
    delete RSP;
    return res;
}

//-----------------------------------------------------------------------------
//      �������� �������� ����������� ������ Product ��� �������� unitId
//      ��������� ���� ���� � ������ � �������
//      ���������� ���-�� ����������� � ������
//-----------------------------------------------------------------------------
int tBIZ_Client::MakeShopVendorsList(int unitId, int Product, TVendorList *Vendors)
{
    int res = 0;
    sBIZGoods Info;
    sVendorRec *VR = NULL;

    Vendors->Clear();
    Vendors->ProductId = Product;

    if (GetShopProductInfo(unitId, Product, &Info)) {
        Vendors->Reques = Info.Reques;
        Vendors->SellingPrice = Info.Price;
        VR = Vendors->Add("�� ������", "---", 0, "---", "---", Info.Available, Info.Quality, 0, Info.Price, Info.Cost, Info.Currency);
        if (VR) {
            VR->Flags |= vlfInStock;
            VR->OrderNum = VR->InStock;
        }
        res++;
    }
    res += GetVendorsForUnit(unitId, Product, Vendors);
    return res;
}


//-----------------------------------------------------------------------------
//!!!!          ��������� ���� �� ����� �������������
//      ������ ��� ������ �����
//-----------------------------------------------------------------------------
bool tBIZ_Client::PurchaseProduct(int UnitId, int VendorId, int ProductId, int Number)
{
    bool res = false;
    std::string sUrl;
    std::string sMsg;
    bool getres = false;
    tHTML_Response *RSP;
    char *pc1;

    RSP = new tHTML_Response();

    sUrl = "/units/vendor/select/?id=";
    sUrl += IntToStr( UnitId );

    sMsg = "vendor=";
    sMsg += IntToStr( VendorId );
    sMsg += "&replace=";
    sMsg += "&product[0]=";
    sMsg += IntToStr( ProductId );
    sMsg += "&selected[";
    sMsg += IntToStr( ProductId) ;
    sMsg += "]=on";
    sMsg += "&purchase[0]=";
    sMsg += IntToStr( Number );
    sMsg += "&$post=on";
    sMsg += "&sort=null";
    sMsg += "&$scene=products";

    getres = POST(sUrl.c_str(), sMsg.c_str(), RSP);
    if (getres) {
        // ��� ��� ������ �� ���� ����� ��� ���������� ���������� ������� �� ������ �������
        // ������ ����� ���������������
        res = true;                 //��� ������
                                    // �������� �� ����������� �� ��� �� ������
        pc1 = strstr(RSP->Header, "Location: /user/login");
        if (pc1) res = false;

        // ����� ��������� �����
        // � ����, ��������, � ������� ������ ����� ����
        pc1 = strstr(RSP->Header, "window.parent.reloadProducts");
        if (pc1) res = false;

        // � ������, ����� �� ������� ����� 
        pc1 = strstr(RSP->Body, "������������ �������� �������");
        //pc1 = strstr(RSP->Body, "class = error");
        if (pc1) res = false;

        // ����� ����������� ��� ��������� �������� ... ����� � ������
    }
    delete RSP;

    return res;
}

//-----------------------------------------------------------------------------
//      ���������� ������� ������� �������� �������.
//      ���������� ���-�� �������.
//      ���� ������� ������ �� Cost, ��������� � Cost ����� ��������� �������.
//-----------------------------------------------------------------------------
int tBIZ_Client::ExecuteOreder(int UnitID, TVendorList *VendorList, float* Cost)
{
    int res = 0;
    sVendorRec* pV;
    bool ResPrcsh;
    std::string sMsg;
    float fcost = 0.0;

    for (unsigned int i = 0; i < VendorList->Count(); i++) {
        pV = VendorList->GetVendor(i);
        // ���� ���� � ������, � ��� �� ����� �� ������, ��� ����� � ����
        if (pV && (pV->OrderNum))
            if (0 == (pV->Flags & (vlfInStock | vlfOnTheWay))) {
                // ����� ������� ������� �� ������ (�� ����� 500 ����� �������)          
                if (GetPriceInCurrency(pV->Cost, pV->Currency, curRUB) * pV->OrderNum < 500000000)
                {
                    ResPrcsh = PurchaseProduct(UnitID, pV->VendorID, VendorList->ProductId, pV->OrderNum);

                    if (ResPrcsh) {
                        sMsg = " ����������� ��� (";
                        fcost += pV->OrderNum * pV->Cost;
                    }
                    else {
                        sMsg = " �� ������� ���������� ��� (";
                    }
                    sMsg += IntToStr(UnitID);
                    sMsg += ") � ";
                    sMsg += pV->VendorName;
                    sMsg += "(";
                    sMsg += pV->CompanyName;
                    sMsg += ") (";
                    sMsg += IntToStr(pV->VendorID);
                    sMsg += ") ";
                    //sMsg += GetProductRecByID(VendorList->ProductId)->ProductName;
                    sMsg += "  ";
                    sMsg += " (";
                    sMsg += IntToStr( VendorList->ProductId );
                    sMsg += ")";
                    if (ResPrcsh) {
                        sMsg += " � ���������� ";
                        sMsg += IntToStr( pV->OrderNum );
                        sMsg += " ��. �� ���� ";
                        sMsg += FloatToStr(pV->Cost);
                        sMsg += " (�� ����� ";
                        sMsg += FloatToStr(pV->OrderNum * pV->Cost);
                        sMsg += ")";
                    }

                    WirteReport(sMsg.c_str(), ML_WRK2);

                    res++;
                }
            }
    }
    if (Cost) *Cost += fcost;
    return res;
}

//-----------------------------------------------------------------------------
//      ���������� ����������� ������ � �������
//        unitId - ID ��������
//        Days   - �� ������� ���� ��������
//      ���������� ����������� �����
//-----------------------------------------------------------------------------
float tBIZ_Client::AutoPurchaseGoods(int unitId, int Days)
{
    bool bRes;
    std::string s;
    tmGoodsList GL;
    tmCityPriceMap CPM;
    tmGoodsList::iterator iGL;
    tmCityPriceMap::iterator iCPM;
    tmPriceFactor::iterator iPF;
    TVendorList *VL;
    int Num;
    bool bPrcshAnwy = false;
    float fCost = 0.0;
    int QMinLimit = -10, QMaxLimit = 10;               // �� ������� ��������� �������� ����������� ������ ����� ���� ���� ��� ���� ����������

    bool  fPurchaseAnyway = false;                 // ��������� � ���� ������, ���� �������������� ����� (���������, ���� ����� ������ �������)

    int GCount = 0;               // ��� ��� ������ ��� �������
    int GCount2 = 0;              //
    int GCount3 = 0;

    // !!! .... ��� ����� ���� ������ ����� �� ����
    float fQualityOverpricing = 1;            // �� ������� ��������� ������ ����� ��������� �������� ��������� ��� ����������� (-99% .. +99%)
    float fMaxPurchasePriceExcess = -10;        // �� ������� ���� ����������� ������ ����� ������������ ��������������� (-99% .. +99%) (����� � ��������� ����� ������ �����, ��� �������� ������)

    VL = new TVendorList();

    s = "==== Auto Purchase Goods. ID= ";
    s += IntToStr(unitId); 
	s += " for ";
	s += IntToStr(Days);
	if (1 == Days) s += " day"; 
	else s += " days";
	s += " ====";
    LogMessage(s.c_str(), ML_WRK2);

    GL.clear();
    CPM.clear();
    GetShopGoodsList(unitId, &GL, &CPM);

    for (iGL = GL.begin(); iGL != GL.end(); iGL++) {       // ��� ���� ������
        GCount++;

goto x3;
        // ��� ������ �������������� �����������, � ����� �� ������� ������, ��� ����� ���� �� ��������
        if (int((*iGL).second.Sales / 8.0) > 0) {
            if ((*iGL).second.Available >= (*iGL).second.Sales / 8.0 * Days) {
                GCount3++;
                continue;
            }
        }
x3:
        //����� ����������� ���-�� ������� ������ �� ���������
        VL->Clear();
        MakeShopVendorsList(unitId, (*iGL).second.ID, VL);                          // ��������� ������ ����������� ������ ��� ��������
        if (VL->Reques == 0)                                                       // ������ � �����, ���� ����� ������ ������
            VL->Reques = 105;
        Num = VL->Reques / 7 * Days - (*iGL).second.Available;                      // �������� �� �������� ���-�� ����� (��������, ��� �� ������ ��� ���-�� �����)

        GCount2++;
        if (Num < 0) continue;

        // ���� �� ������ ��� ��� ������� ������, � � ���������� ��� ���������
        // �� �������� ����� �����, ������� ���� �� �����
        if (((*iGL).second.Available < 5) && fPurchaseAnyway)
            bPrcshAnwy = true;
        else
            bPrcshAnwy = false;

        VL->SelectForOrder(VL->fAverageQuality * (1.0 + fQualityOverpricing / 100.0), Num, bPrcshAnwy, fMaxPurchasePriceExcess, QMinLimit, QMaxLimit);
        if (ExecuteOreder(unitId, VL, &fCost)) {
            s = " ����������� � �������� id=";
            s += IntToStr(unitId);
        }
        else {
            s = " �� ������� ����������� � �������� id=";
            s += IntToStr(unitId);
        }

        /*
        s += IntToStr(unitId);
        s += " ������ ";
        s += GetProductRecByID((*iGL).second.ID)->ProductName;
        s += " (id:";
        s += IntToStr((*iGL).second.ID);
        s += ")";
        */
        // WirteReport(s.c_str(), ML_WRK2);
    }
    if (fCost > 0.1) {
        s = "Total cost autopurchase for ID ";
        s += IntToStr(unitId);
        s += ":  ";
		s += GetMoneyFormatStr(fCost);	//FloatToStr(fCost);
        WirteReport(s.c_str(), ML_WRK2);
    }

    return fCost;

}

//-----------------------------------------------------------------------------
//  ����������� ��� �������� �
//  � ����������� ���������� ���
//-----------------------------------------------------------------------------
float tBIZ_Client::AutoPurchaseForMarket(int UId, int days)
{
    float res = 0.0;
    res = AutoPurchaseGoods(UId, days);
    SetGoodsPrice(UId);
    return res;
}

//-----------------------------------------------------------------------------
//  ����������� �� ���� ��������� ��������
//  � ����������� ���������� ���
//-----------------------------------------------------------------------------
bool tBIZ_Client::AutoPurchaseForAllMarkets(int days)
{
    float fCost = 0.0;
    std::string s;

    if (NULL == Company)            // ���� ��� ���������� �� ��������� ��������
        GetCompanyInfo();           // ��������� �
    for (tmUnits::iterator iUL = Company->begin(); iUL != Company->end(); iUL++) {
        if (2 == (*iUL).second->Type) {
            fCost += AutoPurchaseForMarket((*iUL).second->ID, days);
        }
    }
    s = "Total cost of autopurchase :  ";
    s += GetMoneyFormatStr(fCost);
    WirteReport(s.c_str(), ML_WRK1);
    return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool tBIZ_Client::SetGoodsPriceForAllMarkets(void)
{
    if (NULL == Company)            // ���� ��� ���������� �� ��������� ��������
        GetCompanyInfo();           // ��������� �
    for (tmUnits::iterator iUL = Company->begin(); iUL != Company->end(); iUL++) {
        if (2 == (*iUL).second->Type) {
            SetGoodsPrice((*iUL).second->ID);
        }
    }
    return true;
}

