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

    // Получить курс биржи игровых очков
    bool GetExchangeState(long &PurchasePrice,                  // закупочная цена, (по которой можно купить) (по которой биржа продает)
                          long &SellingPrice);                  // продажная цена, (по которой можно продать) (по которой биржа покупает)
    // Получить с сервера и записать в базу курс биржи игровых очков
    bool SaveExchangeState(void);

    // Прочитать на странице текущее игровое время сервера
    bool TryGetServerTime(char* Page);

    int GetcompanyStructure(int cid, tmUnits *Company);
    bool GetCompanyInfo(void);

    // Вычитать информацию по товарам в магазине      
    int GetShopGoodsList(int unitId, tmGoodsList *pGL, tmCityPriceMap *pCPM);

    // Выставить цену на товары в магазине исходя из средней цены по городу
    bool SetGoodsPrice(int unitId, tmPriceFactor *PF);

    //      Произвести автозакупку товара в магазин
    float AutoPurchaseGoods(int unitId, int Days);

    //  Автозакупка для магазина с последующей коррекцией цен
	float AutoPurchaseForMarket(int UId, int days =7);
    //  Автозакупка для всех магазинов компании с последующей коррекцией цен
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
