#ifndef BIZTYPES_H
#define BIZTYPES_H

#include <time.h>
#include <string>
#include <map>


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#define vlfFirst5       0x0001      // ������ � ������� ������ �� ������������ ��������/����
#define vlfHiBest       0x0002      // ���� ������ ���� �������� ��������
#define vlfLoBest       0x0004      // ���� ������ ���� �������� ��������
#define vlfMidQuality   0x0008      // ������� �������� �� ������
#define vlfInStock      0x0100      // ������ �� ������, ��� �������� ����������� �������� �����������
#define vlfOnTheWay     0x0200      // ������ � ���� �� �����
#define vlfSkipRanking  0x0400      // �� ��������� � ������������ ��� �������

class TVendorList;
//-----------------------------------------------------------------------------
//      �������� ������ ������ ����������� ������
//-----------------------------------------------------------------------------
class sVendorRec 
{
public:
    // ����������� ����
    int   InStock;                                // ���������� ������ �� ������
    float Quality;                                // �������� ������
    int   Delivery;                               // ���� ��������
    float Price;                                  // ���� (���������)
    float Cost;                                   // ��������� (� ������ ��������)
    int   Currency;                               // ������, � ������� ������� ����
    int   CityID;                                 // ��� ������
    int   OrderNum;                               // ���������� ��� ������

                                                  // ��������� ����
    int   Flags;                                  // �����  (0xFF00-����� �������� 0x00FF-����� ����������)
    float Kqp;                                    // ����������� ��������/����
    float Kdqp;                                   // ����������� ���������������������/����
    int   KeyIndex;                               // ���� ��� ���������
    int   VendorID;                               // ID ����������� ����������
    int   CompanyID;                              // ID �������� ����������

public:
    std::string VendorName;                       // �������� ������
    std::string VendorURI;                        // ������ �� �����
    std::string CompanyName;                      // �������� ��������
    std::string CompanyURI;                       // ������ �� ��������

    TVendorList* Owner;                           // ��������� �� ������, � ������� ������� ��� ������

public:
    sVendorRec(TVendorList *aOwner, char *aVName, char *aVURI, int aCityID, char *aCName, char *aCURI, int aInStock, float aQuality, int aDelivery, float aPrice, float aCost, int Currency);
    sVendorRec(TVendorList *aOwner);
    ~sVendorRec();

    float GetKdqp(void);
};


//-----------------------------------------------------------------------------
//      ������ ������� ��������
//-----------------------------------------------------------------------------
struct sDeliveryRec {
    int Delivered;        // ���������� � ��� ������
    int Num;              // ����� ���-�� �� ����� ������
    float Quality;        // ������� ��������
    float Price;          // ������� ����
    int   Currency;       // ������, � ������� ������� ����
    int Day;              // ���� ��������� �������� �� ���� ������
};

// ����������� ���� ��������
// �������� ������ 4 ������ (���� �� 4 ������) ��������� ������� ������� ������ �� ����� ������
struct sDeliveryPaln {
    sDeliveryRec WeekDeliv[4];     // ����������� ���� ��������
    int Weeks;                    // �� ������� ������ ������������� ��������
    int MaxDelivery;              // ������������ ���� ��������
    int AllPurchased;             // ����� ��������� ������
};

//-----------------------------------------------------------------------------
//      ���� ���������� ��� ������ �����������
//-----------------------------------------------------------------------------
enum eSortBy {
    srtbyQualUp, srtbyQualDowm, srtbyKUp, srtbyKDown, srtbyPriceUp, srtbyPriceDown, srtbyCostUp, srtbyCostDown,
    srtbyKdqpUp, srtbyKdqpDown
};


//-----------------------------------------------------------------------------
//      ������ �����������
//-----------------------------------------------------------------------------
class TVendorList
{
private:
    char         DbgLogId[32];                          // ���, ������� ����� ������������� ���������� ���������
    unsigned int fMasLen;                               // ����� ������� ����������
    unsigned int fCount;                                // ���������� ������� � �������
    sVendorRec   **VendorsMas;                          // ������ ���������� �� �������
    float        OrderQErLimint;                        // ���������� ���������� �� �������� ��� ������� ������

public:
    float        fAverageQuality;                       // ������� �������� �� ������ (�������� �� ������ ��������)
    float        fAveragePrice;                         // ������� ���� �� ������ (�������� �� ������ ��������)

    float        SellingPrice;                         // ��������� ����
    int          Currency;                             // ������, � ������� ������� ����
    int          ProductId;                            // ID ��������, ��� �������� ��������� ������ �����������
    int          CityDestID;                           // ID ������ ����������, ��� �������� ������������ ������� �������� � �������� ����
    int          Reques;                               // ��������� ����� �� �����

public:
    TVendorList(void);
    ~TVendorList();

    void        Clear(void);
    sVendorRec* Add(char *aVName, char *aVURI, int aCityID, char *aCName, char *aCURI, int aInStock, float aQuality, int aDelivery, float aPrice, float aCost, int aCurrency);      // �������� ������ � ��������� ����
    sVendorRec* Add(sVendorRec* Vendor);                     // �������� ����������� ������
    sVendorRec* Add(void);                                   // �������� ������

    void  KeySort(eSortBy SortBy);                           // ����������� ����
    void  ArrangeFlagsV2(unsigned int deep);                 // ���������� ����� ������ 2

                                                             // ������� ��� ������ ���������� �����
    int __fastcall   SelectForOrder(float Quality,                      // ��������� �������� ������
        int Number,                         // ��������� ���������� ������
        bool PurchaseAnyway = false,        // ��������� � ���� ������, ���� �������������� ����� (���������, ���� ����� ������ �������)
        float MaxPurchasePriceExcess = 20,  // �� ������� ���� ����������� ������ ����� ������������ ��������������� (����� � ��������� ����� ������ �����, ��� �������� ������)
        int maxDeliv = 35,                  // ������������ ���� �������� (� ������� ���� ���������� ����� ��������� �������� � ������������ ������)
        int minDeliv = 0);                  // ����������� ���� ��������

                                            
    void __fastcall  ClearOrder(void);      // �������� �����

    // ��������� �����
    float __fastcall CalcOrder(int &Num, float &midPrice, float &midQuality, int &maxDeliv);

    // ��������� ����� ����������
    float __fastcall CalcOrderForFewWeeks(int Today, int Consumption, sDeliveryPaln *Plan);

    // ������ __property sVendorRec*  Vendors[unsigned int i] = { read = GetVendor };
    sVendorRec*  GetVendor(unsigned int index) { if (index < fCount) return VendorsMas[index]; else return NULL; };
    // ������ __property unsigned int Count = { read = fCount };
    unsigned int Count(void) { return fCount; };

    // ������ __property float        AveragePrice = { read = fAveragePrice, write = SetAveragePrice };
    void SetAveragePrice(float Value);
    // ������ __property float        AverageQuality = { read = fAverageQuality, write = SetAverageQuality };
    void SetAverageQuality(float Value);

};

//-----------------------------------------------------------------------------
//      ����� �������
//-----------------------------------------------------------------------------
struct sSrvTime {
    time_t TimeStamp;           // ����� ���� �������� ���������� � ������� �������
    int LeftToMdnt;               // ������� ������ �������� �� �� ����� ����� �� �������
    int SrvYear;
    int SrvMonth;
    int SrvDay;
    int SrvWeekDay;               // ���� ������ 1..7 1-�����������
};


class tBIZUnit {
public:
    tBIZUnit(int unitid) :ID(unitid) {};
    int         ID;                     // ID ����������� (������� �� URI)                         (������������ �� URI)
    std::string URI;                    // ������ �� �������� �����������
    std::string Name;                   // �������� �����������/��������
    int         Type;                   // ��� ���� ����� (�������, �����, �����, �������� � �.�.) (������������ �� URI)
    int         CityID;                 // ����� (����� ��������)
    int         Level;                  // �������
    int         LevelMargin;            // ��������� ������
    std::string Image;                  // ��� ����� � ���������
};

//-----------------------------------------------------------------------------
// ������ ������������� (���� - ID �����)
typedef std::map <int, tBIZUnit*> tmUnits;

//-----------------------------------------------------------------------------
//      �������� ������ � ��������
//-----------------------------------------------------------------------------
struct sBIZGoods {
    int   ID;                     // ID ��������
    int   Available;              // �� ������
    float Quality;                // ��������
    float Cost;                   // ���� ����������
    float Price;                  // ��������� ����
    int   Currency;               // ������, � ������� ������� ����
    int   Reques;                 // �����
    int   Sales;                  // ����� ������
    int   Proceeds;               // �������
    int   Profit;                 // �������
    char  VendorURI[50];          // URI ����������
                                  //int   ID;                     // ID ����������� (������� �� URI)
    int   Purchase;                // ����� ����-�������
};

//-----------------------------------------------------------------------------
//      ���� �����
//-----------------------------------------------------------------------------
enum eCurrencies { curUKNWN = 0, curJPY = 1, curCNY = 2, curEUR = 3, curUAH = 4, curRUB = 5, curUSD = 6, curGBP = 7 };

// ����� �����
const double cBIZExchangeRates[8] = {
    1.0,  // NONE
    0.3,  // JPY   3566.663100
    5.0,  // CNY    214
    40.0,  // EUR     26.750000
    4.0,  // UAH    267.5
    1.0,  // RUB   1070
    30.0,  // USD     35.666310
    50.0,  // GBP     21.400000
};
//-----------------------------------------------------------------------------
//      ���������, ��������� �� �������� ��� �� �����      
//-----------------------------------------------------------------------------
struct sQPrice {
    float Price;
    int Currency;         // ������, � ������� ������� ����
    float Quality;
};
typedef const sQPrice* pQPrice;

typedef std::map<int, sQPrice>  tmCityPriceMap;         // ������ ��� ��� ������ ����� �������. ���� - ID ������

typedef std::map <int, sBIZGoods> tmGoodsList;          // ������ ������� (���� - ID ������)
typedef std::map <int, float> tmPriceFactor;            // ������������ ��� ���  (���� - ID ������)

struct sSalesHistoryRec {
    void Clear() {
        Sales = 0; Price = 0; Quality = 0; Proceeds = 0; Profit = 0;
        DateTime.TimeStamp = 0; DateTime.LeftToMdnt = 0;
        DateTime.SrvYear = 0; DateTime.SrvMonth = 0; DateTime.SrvDay = 0; DateTime.SrvWeekDay = 0;
    };
    sSrvTime DateTime;    // ����
    int   Sales;          // ����� ������
    float Price;          // ����
    float Quality;        // ��������
    int   Proceeds;       // �������
    int   Profit;         // �������
};



//-----------------------------------------------------------------------------
//      ���������, ��������� �� �������� ��� �� �����      
//-----------------------------------------------------------------------------

std::string IntToStr(long Value);
std::string FloatToStr(float Value, int precision =2);
std::string GetCurrencyName(int currency);
float GetPriceInCurrency(float Price, int Currency, int currencyTo = curRUB);


#endif

