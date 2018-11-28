#ifndef BIZTYPES_H
#define BIZTYPES_H

#include <time.h>
#include <string>
#include <map>


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#define vlfFirst5       0x0001      // Входит в пятерку лучших по коэффициенту качество/цена
#define vlfHiBest       0x0002      // Пара лучших выше среднего качества
#define vlfLoBest       0x0004      // Пара лучших ниже среднего качества
#define vlfMidQuality   0x0008      // Среднее качество по городу
#define vlfInStock      0x0100      // Товары на складе, для которого формируется перечень поставщиков
#define vlfOnTheWay     0x0200      // Товаты в пути на склад
#define vlfSkipRanking  0x0400      // Не участвует в ранжировании для закупок

class TVendorList;
//-----------------------------------------------------------------------------
//      Описание записи перчня поставщиков товара
//-----------------------------------------------------------------------------
class sVendorRec 
{
public:
    // Заполняемые поля
    int   InStock;                                // Количество товара на складе
    float Quality;                                // Качество товара
    int   Delivery;                               // Срок поставки
    float Price;                                  // Цена (отпускная)
    float Cost;                                   // Стоимость (с учетом доставки)
    int   Currency;                               // Валюта, в которой указана цена
    int   CityID;                                 // Код города
    int   OrderNum;                               // Количество для заказа

                                                  // Расчетные поля
    int   Flags;                                  // Флаги  (0xFF00-флаги признаки 0x00FF-флаги сортировок)
    float Kqp;                                    // Коэффициент Качество/Цена
    float Kdqp;                                   // Коэффициент ОтносительноеКачество/Цена
    int   KeyIndex;                               // Ключ для сорировки
    int   VendorID;                               // ID предприятия поставщика
    int   CompanyID;                              // ID компании поставщика

public:
    std::string VendorName;                       // Название склада
    std::string VendorURI;                        // Ссылка на склад
    std::string CompanyName;                      // Название компании
    std::string CompanyURI;                       // Ссылка на компанию

    TVendorList* Owner;                           // Указатель на список, в который входить эта запись

public:
    sVendorRec(TVendorList *aOwner, char *aVName, char *aVURI, int aCityID, char *aCName, char *aCURI, int aInStock, float aQuality, int aDelivery, float aPrice, float aCost, int Currency);
    sVendorRec(TVendorList *aOwner);
    ~sVendorRec();

    float GetKdqp(void);
};


//-----------------------------------------------------------------------------
//      Запись списков поставки
//-----------------------------------------------------------------------------
struct sDeliveryRec {
    int Delivered;        // Доставлено в эту неделю
    int Num;              // Общее кол-во на конец недели
    float Quality;        // Среднее качество
    float Price;          // Средняя цена
    int   Currency;       // Валюта, в которой указана цена
    int Day;              // День последней поставки на этой неделе
};

// Понедельный план поставок
// Содержит только 4 записи (план на 4 недели) поскольку большая глубина просто не мсеет смысла
struct sDeliveryPaln {
    sDeliveryRec WeekDeliv[4];     // понедельный план поставок
    int Weeks;                    // на сколько недель растягивается поставка
    int MaxDelivery;              // максимальный срок поставки
    int AllPurchased;             // всего закуплено товара
};

//-----------------------------------------------------------------------------
//      Типы сортировки для списка поставщиков
//-----------------------------------------------------------------------------
enum eSortBy {
    srtbyQualUp, srtbyQualDowm, srtbyKUp, srtbyKDown, srtbyPriceUp, srtbyPriceDown, srtbyCostUp, srtbyCostDown,
    srtbyKdqpUp, srtbyKdqpDown
};


//-----------------------------------------------------------------------------
//      Список поставщиков
//-----------------------------------------------------------------------------
class TVendorList
{
private:
    char         DbgLogId[32];                          // Имя, которым будут подписываться отладочные сообщения
    unsigned int fMasLen;                               // Длина массива указателей
    unsigned int fCount;                                // Количество записей в массиве
    sVendorRec   **VendorsMas;                          // Массив указателей на объекты
    float        OrderQErLimint;                        // Допустимое отклонение по качеству при подборе заказа

public:
    float        fAverageQuality;                       // Среднее качество по городу (читается на первой странице)
    float        fAveragePrice;                         // Средняя цена по городу (читается на первой странице)

    float        SellingPrice;                         // Отпускная цена
    int          Currency;                             // Валюта, в которой указана цена
    int          ProductId;                            // ID продукта, для которого составлен список поставщиков
    int          CityDestID;                           // ID города назначения, для которого производятся расчеты доставки и средняяй цена
    int          Reques;                               // Ожидаемый спрос на товар

public:
    TVendorList(void);
    ~TVendorList();

    void        Clear(void);
    sVendorRec* Add(char *aVName, char *aVURI, int aCityID, char *aCName, char *aCURI, int aInStock, float aQuality, int aDelivery, float aPrice, float aCost, int aCurrency);      // Добавить запись и заполнить поля
    sVendorRec* Add(sVendorRec* Vendor);                     // Добавить заполненную запись
    sVendorRec* Add(void);                                   // Добавить запись

    void  KeySort(eSortBy SortBy);                           // Сортировать ключ
    void  ArrangeFlagsV2(unsigned int deep);                 // Расставить флаги версия 2

                                                             // Выбрать для заказа подходящий товар
    int __fastcall   SelectForOrder(float Quality,                      // Требуемое качество товара
        int Number,                         // Требуемое количество товара
        bool PurchaseAnyway = false,        // Покуапать в лбом случае, даже некачественный товар (требуется, если рынок совсем мертвый)
        float MaxPurchasePriceExcess = 20,  // На сколько цена закупаемого товара может превосходить среднегородскую (товар с завышеной ценой иногда лучше, чем никакого товара)
        int maxDeliv = 35,                  // Максимальный срок поставок (с помощью этих параметров можно подгодать поставку в определенную неделю)
        int minDeliv = 0);                  // Минимальный срок поставок

                                            
    void __fastcall  ClearOrder(void);      // Сбросить заказ

    // Расчитать заказ
    float __fastcall CalcOrder(int &Num, float &midPrice, float &midQuality, int &maxDeliv);

    // Расчитать заказ понедельно
    float __fastcall CalcOrderForFewWeeks(int Today, int Consumption, sDeliveryPaln *Plan);

    // вместо __property sVendorRec*  Vendors[unsigned int i] = { read = GetVendor };
    sVendorRec*  GetVendor(unsigned int index) { if (index < fCount) return VendorsMas[index]; else return NULL; };
    // вместо __property unsigned int Count = { read = fCount };
    unsigned int Count(void) { return fCount; };

    // вместо __property float        AveragePrice = { read = fAveragePrice, write = SetAveragePrice };
    void SetAveragePrice(float Value);
    // вместо __property float        AverageQuality = { read = fAverageQuality, write = SetAverageQuality };
    void SetAverageQuality(float Value);

};

//-----------------------------------------------------------------------------
//      Время сервера
//-----------------------------------------------------------------------------
struct sSrvTime {
    time_t TimeStamp;           // Когда была получена информацтя о времени сервера
    int LeftToMdnt;               // Сколько секунд осталось до до смены суток на сервере
    int SrvYear;
    int SrvMonth;
    int SrvDay;
    int SrvWeekDay;               // День недели 1..7 1-понедельник
};


class tBIZUnit {
public:
    tBIZUnit(int unitid) :ID(unitid) {};
    int         ID;                     // ID предприятия (берется из URI)                         (определяется по URI)
    std::string URI;                    // ССылка на страницу предприятия
    std::string Name;                   // Название предприятия/компании
    int         Type;                   // код типа юнита (магазин, завод, склад, компания и т.д.) (определяется по URI)
    int         CityID;                 // Город (кроме компании)
    int         Level;                  // Уровень
    int         LevelMargin;            // Свободные звезды
    std::string Image;                  // Имя файла с картинкой
};

//-----------------------------------------------------------------------------
// Список подразделений (ключ - ID юнита)
typedef std::map <int, tBIZUnit*> tmUnits;

//-----------------------------------------------------------------------------
//      Описание товара в магазине
//-----------------------------------------------------------------------------
struct sBIZGoods {
    int   ID;                     // ID продукта
    int   Available;              // На складе
    float Quality;                // Качество
    float Cost;                   // Цена закупочная
    float Price;                  // Отпускная цена
    int   Currency;               // Валюта, в которой указана цена
    int   Reques;                 // Спрос
    int   Sales;                  // Объем продаж
    int   Proceeds;               // Выручка
    int   Profit;                 // Прибыль
    char  VendorURI[50];          // URI поставщика
                                  //int   ID;                     // ID предприятия (берется из URI)
    int   Purchase;                // Объем авто-закупок
};

//-----------------------------------------------------------------------------
//      Типы валют
//-----------------------------------------------------------------------------
enum eCurrencies { curUKNWN = 0, curJPY = 1, curCNY = 2, curEUR = 3, curUAH = 4, curRUB = 5, curUSD = 6, curGBP = 7 };

// Курсы валют
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
//      Структуры, связанные со списками цен на товар      
//-----------------------------------------------------------------------------
struct sQPrice {
    float Price;
    int Currency;         // Валюта, в которой указана цена
    float Quality;
};
typedef const sQPrice* pQPrice;

typedef std::map<int, sQPrice>  tmCityPriceMap;         // список цен для разных типов товаров. ключ - ID товара

typedef std::map <int, sBIZGoods> tmGoodsList;          // Список товаров (ключ - ID товара)
typedef std::map <int, float> tmPriceFactor;            // Коэффициенты для цен  (ключ - ID товара)

struct sSalesHistoryRec {
    void Clear() {
        Sales = 0; Price = 0; Quality = 0; Proceeds = 0; Profit = 0;
        DateTime.TimeStamp = 0; DateTime.LeftToMdnt = 0;
        DateTime.SrvYear = 0; DateTime.SrvMonth = 0; DateTime.SrvDay = 0; DateTime.SrvWeekDay = 0;
    };
    sSrvTime DateTime;    // Дата
    int   Sales;          // Объем продаж
    float Price;          // Цена
    float Quality;        // Качество
    int   Proceeds;       // Выручка
    int   Profit;         // Прибыль
};



//-----------------------------------------------------------------------------
//      Структуры, связанные со списками цен на товар      
//-----------------------------------------------------------------------------

std::string IntToStr(long Value);
std::string FloatToStr(float Value, int precision =2);
std::string GetCurrencyName(int currency);
float GetPriceInCurrency(float Price, int Currency, int currencyTo = curRUB);


#endif

