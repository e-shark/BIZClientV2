#ifndef BIZCLIENT_H
#define BIZCLIENT_H

#include "HTTPTransport.h"
#include "BIZ_Types.h"

class tBIZ_Client : public tBIZ_Transport
{
private:
protected:
    sSrvTime SrvTime;
    int     PersonCompanyID;
    tmUnits *Company;

public:
    tBIZ_Client(void);
    ~tBIZ_Client();

    int PersCID(void) { return PersonCompanyID; };
    bool LoadPerson(int PID);

    void WirteReport(const char *Text, int type = 1);

    bool CheckLogin(void);
    bool GetDailyBonus(void);

    // �������� ���� ����� ������� �����
    bool GetExchangeState(long &PurchasePrice,                  // ���������� ����, (�� ������� ����� ������) (�� ������� ����� �������)
                          long &SellingPrice);                  // ��������� ����, (�� ������� ����� �������) (�� ������� ����� ��������)
    // �������� � ������� � �������� � ���� ���� ����� ������� �����
    bool SaveExchangeState(void);

    // ��������� �� �������� ������� ������� ����� �������
    bool TryGetServerTime(char* Page);

    int GetcompanyStructure(int cid, tmUnits *Company);
    bool GetCompanyInfo(void);

    // �������� ���������� �� ������� � ��������      
    int GetShopGoodsList(int unitId, tmGoodsList *pGL, tmCityPriceMap *pCPM);

    // ��������� ���� �� ������ � �������� ������ �� ������� ���� �� ������
    bool SetGoodsPrice(int unitId, tmPriceFactor *PF);

    //      ���������� ����������� ������ � �������
    float AutoPurchaseGoods(int unitId, int Days);

    //  ����������� ��� �������� � ����������� ���������� ���
	float AutoPurchaseForMarket(int UId, int days =7);
    //  ����������� ��� ���� ��������� �������� � ����������� ���������� ���
    bool AutoPurchaseForAllMarkets(int days =7);

    bool SetGoodsPriceForAllMarkets(void);

    bool GetShopProductInfo(int WHID, int ProductID, sBIZGoods *Info);
    int MakeShopVendorsList(int unitId, int Product, TVendorList *Vendors);
    int GetVendorsForUnit(int unitId, int Product, TVendorList *Vendors);
    bool PurchaseProduct(int UnitId, int VendorId, int ProductId, int Number);
    int ExecuteOreder(int UnitID, TVendorList *VendorList, float* Cost =NULL);

    


    bool Test1(void);
    bool Test2(void);
    //int GetCitiesList(void);
};


#endif
