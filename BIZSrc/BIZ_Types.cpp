
#include "BIZ_Types.h"
#include "LogUtils.h"

//-----------------------------------------------------------------------------
//      Перевести сумму в другую валюту
//-----------------------------------------------------------------------------
float GetPriceInCurrency(float Price, int Currency, int currencyTo)
{
    float result;
    if ((Currency > 7) || (currencyTo > 7))
        result = Price;
    else {
        result = Price * cBIZExchangeRates[Currency] / cBIZExchangeRates[currencyTo];
    }
    return result;
}

//============================================================================================================================
//      Описание методов структуры  sVendorRec
//============================================================================================================================

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
__fastcall sVendorRec::sVendorRec(TVendorList *aOwner)
{
    this->Owner = aOwner;
    this->VendorName = "";
    this->VendorURI = "";
    this->CompanyName = "";
    this->CompanyURI = "";
    this->InStock = 0;
    this->Quality = 0.0;
    this->Delivery = 0;
    this->Price = 0.0;
    this->Cost = 0.0;
    this->Currency = curUKNWN;
    this->Kqp = 0.0;
    this->Kdqp = 0.0;
    this->Flags = 0;
    this->CompanyID = 0;
    this->VendorID = 0;
    this->CityID = 0;
    this->KeyIndex = 0;
    this->OrderNum = 0;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
__fastcall sVendorRec::sVendorRec(TVendorList *aOwner, char *aVName, char *aVURI, int aCityID, char *aCName, char *aCURI, int aInStock, float aQuality, int aDelivery, float aPrice, float aCost, int Currency)
{
    int l;
    char ts[20];
    char *pc1, *pc2;

    this->Owner = aOwner;

    VendorName = aVName;
    VendorURI = aVURI;
    CompanyName = aCName;
    CompanyURI = aCURI;

    this->InStock = aInStock;
    this->Quality = aQuality;
    this->Delivery = aDelivery;
    this->Price = aPrice;
    this->Cost = aCost;
    if (this->Cost) {
        this->Kqp = this->Quality / this->Cost;
        this->Kdqp = GetKdqp();
    }
    else {
        this->Kqp = 0;
        this->Kdqp = 0;
    }
    this->Currency = Currency;

    this->KeyIndex = 0;
    this->CityID = aCityID;

    this->CompanyID = 0;
    pc1 = strstr(aCURI, "id=");
    if (pc1) {
        pc1 += 3;
        memset(ts, 0, sizeof(ts));
        strncpy(ts, pc1, sizeof(ts));
        this->CompanyID = atoi(ts);
    }

    this->VendorID = 0;
    pc1 = strstr(aVURI, "id=");
    if (pc1) {
        pc1 += 3;
        memset(ts, 0, sizeof(ts));
        strncpy(ts, pc1, sizeof(ts));
        this->VendorID = atoi(ts);
    }
    this->Flags = 0;
    this->OrderNum = 0;
}

//-----------------------------------------------------------------------------
//      Расчитать Kdqp - коэффициент качество/цена
//-----------------------------------------------------------------------------
float sVendorRec::GetKdqp(void)
{
    float res;
    float Ka, Kb;
    float Qm;
    float Pm;
    float dQ;
    float DQ1 = 300;

    if (Owner) {
        Qm = Owner->fAverageQuality;
        Pm = Owner->fAveragePrice;
        if (Qm != 0) {
            dQ = Qm - Quality;
            if (dQ != 0) {
                Ka = Qm + DQ1 + DQ1*Quality / dQ;
                Kb = (Pm*(Qm + DQ1)) / Qm + DQ1*Cost / dQ;
                if (Kb != 0)
                    res = Ka / Kb;
                else
                    res = 0;
            }
            else
                res = 0;
        }
        else
            res = 0;
    }
    else
        res = 0;
    return res;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
sVendorRec::~sVendorRec()
{
}


//============================================================================================================================
//      Описание методов класса  TVendorList
//============================================================================================================================

//-----------------------------------------------------------------------------
//      Конструктор списка
//-----------------------------------------------------------------------------
TVendorList::TVendorList(void)
{
    memset(DbgLogId, 0, sizeof(DbgLogId));
    sprintf(DbgLogId, "VendorList");

    fMasLen = 1000;         // длина массива указателей
    fCount = 0;            // количество записей в массиве
    VendorsMas = NULL;
    VendorsMas = (sVendorRec**)calloc(fMasLen, sizeof(sVendorRec));
    fAveragePrice = 0;
    fAverageQuality = 0;
    Currency = curUKNWN;
    ProductId = 0;
    CityDestID = 0;
    SellingPrice = 0;
    Reques = 0;
    OrderQErLimint = 0.1;
}

//-----------------------------------------------------------------------------
//      Деструктор списка
//-----------------------------------------------------------------------------
TVendorList::~TVendorList()
{
    sVendorRec* pV;
    for (unsigned int i = 0; i<Count(); i++) {
        pV = VendorsMas[i];
        delete pV;
        VendorsMas[i] = NULL;
    }
    free(VendorsMas);
}

//-----------------------------------------------------------------------------
//      Очистить список
//-----------------------------------------------------------------------------
void TVendorList::Clear(void)
{
    sVendorRec* pV;
    for (unsigned int i = 0; i<Count(); i++) {
        pV = VendorsMas[i];
        delete pV;
        VendorsMas[i] = NULL;
    }
    free(VendorsMas);

    fMasLen = 1000;         // длина массива указателей
    fCount = 0;            // количество записей в массиве
    VendorsMas = NULL;
    VendorsMas = (sVendorRec**)calloc(fMasLen, sizeof(sVendorRec));
    fAveragePrice = 0;
    fAverageQuality = 0;
    ProductId = 0;
    CityDestID = 0;
    SellingPrice = 0;
    Reques = 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void TVendorList::ClearOrder(void)
{
    sVendorRec* pV;
    for (unsigned int i = 0; i<Count(); i++) {
        pV = VendorsMas[i];
        if (pV) {
            if ((pV->Flags & (vlfInStock | vlfOnTheWay)) == 0)
                pV->OrderNum = 0;
        }
    }
}

//-----------------------------------------------------------------------------
//      Добавить запись в список поставщиков
//-----------------------------------------------------------------------------
sVendorRec* TVendorList::Add(sVendorRec* Vendor)
{
    if (fCount + 1 >= fMasLen) {                                      // Если массив уже заполнен
        fMasLen += 1000;                                            // увеличиваем размерность массива
        VendorsMas = (sVendorRec**)realloc(VendorsMas, fMasLen);           // и выделяем под него дополнительную память
    }
    VendorsMas[fCount] = Vendor;
    fCount++;
    return Vendor;
}

//-----------------------------------------------------------------------------
//      Добавить пустую запись в список поставщиков
//-----------------------------------------------------------------------------
sVendorRec* TVendorList::Add(void)
{
    sVendorRec* V = new sVendorRec(this);
    return Add(V);
}

//-----------------------------------------------------------------------------
//      Добавить заполненную запись  список поставщиков
//              aVName -    Название воставщика
//              aVURI -     URI предприятия поставщика
//              aCityID -   ID города предприятия поставщика
//              aCName -    название компании поставщика
//              aCURI -     URI компании поставщика
//              aInStock -  количество товара на складе поставщика
//              aQuality -  качество товара
//              aDelivery - срок поставки
//              aPrice -    цена товара
//              aCost -     стоимость товара с учетом поставки
//              aCurrency - валюта, в которой указаны цены
//-----------------------------------------------------------------------------
sVendorRec* TVendorList::Add(char *aVName, char *aVURI, int aCityID, char *aCName, char *aCURI, int aInStock, float aQuality, int aDelivery, float aPrice, float aCost, int aCurrency)
{
    sVendorRec* V = new sVendorRec(this, aVName, aVURI, aCityID, aCName, aCURI, aInStock, aQuality, aDelivery, aPrice, aCost, aCurrency);
    return Add(V);
}

//-----------------------------------------------------------------------------
//      Установить значение средней цены
//-----------------------------------------------------------------------------
void TVendorList::SetAveragePrice(float Value)
{
    sVendorRec * pV;

    fAveragePrice = Value;
    // Пересчитываем коэффициенты, зависящие от AveragePrice
    for (unsigned int N = 0; N<fCount; N++) {
        pV = VendorsMas[N];
        if (pV)
            pV->Kdqp = pV->GetKdqp();
    }
}

//-----------------------------------------------------------------------------
//      Установить значение среднего качество
//-----------------------------------------------------------------------------
void TVendorList::SetAverageQuality(float Value)
{
    sVendorRec * pV;

    fAverageQuality = Value;
    // Пересчитываем коэффициенты, зависящие от AverageQuality
    for (unsigned int N = 0; N<fCount; N++) {
        pV = VendorsMas[N];
        if (pV)
            pV->Kdqp = pV->GetKdqp();
    }
}

//-----------------------------------------------------------------------------
//      Сортировать ключи списока
//-----------------------------------------------------------------------------
void TVendorList::KeySort(eSortBy SortBy)
{
    int n;

    // Сперва очищаем ключи
    for (unsigned int i = 0; i<fCount; i++)
        VendorsMas[i]->KeyIndex = -1;

    // Расставляем индексы веса от 1 до fCount
    for (unsigned int i = 1; i - 1<fCount; i++) {
        n = -1;
        for (unsigned int k = 0; k<fCount; k++) {
            if (VendorsMas[k]->Flags & (vlfInStock | vlfOnTheWay)) // для товаров, уже имеющихся на складе, или уже доставляемых на него, коэффициент не нужен
                continue;
            if (VendorsMas[k]->KeyIndex == -1) {        // Просматриваем только еще не оцененные элементы
                if (n == -1) n = k;
                switch (SortBy) {
                case srtbyQualUp:                                       // Сортировака по качеству снизу вверх. Первый элемент с самым низким качеством
                    if ((*VendorsMas[k]).Quality < (*VendorsMas[n]).Quality) n = k;
                    break;
                case srtbyQualDowm:
                    if ((*VendorsMas[k]).Quality >(*VendorsMas[n]).Quality) n = k;
                    break;
                case srtbyKUp:
                    if ((*VendorsMas[k]).Kqp < (*VendorsMas[n]).Kqp) n = k;
                    break;
                case srtbyKDown:
                    if ((*VendorsMas[k]).Kqp >(*VendorsMas[n]).Kqp) n = k;
                    break;
                case srtbyKdqpUp:
                    if ((*VendorsMas[k]).Kdqp < (*VendorsMas[n]).Kdqp) n = k;
                    break;
                case srtbyKdqpDown:
                    if ((*VendorsMas[k]).Kdqp >(*VendorsMas[n]).Kdqp) n = k;
                    break;
                case srtbyPriceUp:
                    break;
                case srtbyPriceDown:
                    break;
                case srtbyCostUp:
                    break;
                case srtbyCostDown:
                    break;
                }
            }
        }
        if (n>-1) VendorsMas[n]->KeyIndex = i;
    }
}

//-----------------------------------------------------------------------------
//      Проанализировать таблицу поставщиков и расставить флаги по Kdqp
//      Версия 2
//-----------------------------------------------------------------------------
void TVendorList::ArrangeFlagsV2(unsigned int deep)
{
    sVendorRec* pV;
    int *minH = new int[deep];
    int *minL = new int[deep];
    unsigned int i, k, n;
    int temp;

    for (i = 0; i<deep; i++) {
        minH[i] = -1;
        minL[i] = -1;
    }

    // Сортируем ключ по убыванию значение коэффициента
    this->KeySort(srtbyKdqpDown);

    // Сбрасываем все флаги сортировок (оставляем признаки)
    for (i = 0; i < this->Count(); i++)
        this->GetVendor(i)->Flags &= 0xFF00;

    // Ищем максимум выше средней Quality
    for (i = 0; i<this->Count(); i++) {
        pV = this->GetVendor(i);
        if (pV) {
            temp = pV->Flags & (vlfInStock | vlfOnTheWay);
            if (0 == temp) {
                if (pV->Quality > this->fAverageQuality) {
                    if ((-1 == minH[deep - 1]) || (pV->Kdqp >( this->GetVendor(minH[deep - 1]) )->Kdqp)) {
                        for (k = 0; k<deep; k++)
                            if ((-1 == minH[k]) || (pV->Kdqp >( this->GetVendor(minH[k]) )->Kdqp))
                                break;
                        for (n = deep - 1; n>k; n--)
                            minH[n] = minH[n - 1];
                        minH[k] = i;
                    }
                }
            }
        }
    }

    // Ищем максимум ниже средней Quality
    for (i = 0; i<this->Count(); i++) {
        pV = this->GetVendor(i);
        if (pV) {
            temp = pV->Flags & (vlfInStock | vlfOnTheWay);
            if (0 == temp) {
                if (pV->Quality < this->fAverageQuality) {
                    if ((-1 == minL[deep - 1]) || (pV->Kdqp >(this->GetVendor(minL[deep - 1]))->Kdqp)) {
                        for (k = 0; k<deep; k++)
                            if ((-1 == minL[k]) || (pV->Kdqp >(this->GetVendor(minL[k]))->Kdqp))
                                break;
                        for (n = deep - 1; n>k; n--)
                            minL[n] = minL[n - 1];
                        minL[k] = i;
                    }
                }
            }
        }
    }

    // Расставляем флаги
    for (i = 0; i < this->Count(); i++) {
        pV = this->GetVendor(i);
        if (pV) {
            pV->Flags &= 0xFF00;

            if (((i == this->Count() - 1) && (pV->Quality >= this->fAverageQuality))
                || ((pV->Quality >= this->fAverageQuality) && (i < this->Count() - 1) &&
                    (this->GetVendor(i + 1)->Quality < this->fAverageQuality))) {
                pV->Flags |= vlfMidQuality;
            }

            if ((pV->KeyIndex >= 0) && (pV->KeyIndex <= 5)) pV->Flags |= vlfFirst5;

            for (k = 0; k<deep; k++) {
                if (minH[k] == (int)i) pV->Flags |= vlfHiBest;
                if (minL[k] == (int)i) pV->Flags |= vlfLoBest;
            }
        }
    }
    delete[] minH;
    delete[] minL;
}

//-----------------------------------------------------------------------------
//      Подобрать для закупки нужный товар
//      Возвращает общее число товара, который прийдется закупить
//-----------------------------------------------------------------------------
int __fastcall TVendorList::SelectForOrder(float Quality, int Number,
    bool PurchaseAnyway, float MaxPurchasePriceExcess,
    int QMinLimit, int QMaxLimit,
    int maxDeliv, int minDeliv)
{
    int iSt = -1;
    int iHi = -1;
    int iLow = -1;
    float mAverageQuality;
    int NAhi, NAlow;
    float Qs;           // Качество товара на складе
    float Qn;           // Качество товара, которое нужно докупить, с учетом того товара, который уже есть на складе
    int Ns;             // Количество товара на складе
    float dQhi, dQlow;
    float Qhi, Qlow;
    int Nhi = 0, Nlow = 0;
    unsigned int i;
    sVendorRec* pV;
    float Price;

    if (Quality == 0) return 0;
    if (Number == 0) return 0;

    // Ищем товар, который уже есть на складе
    // заодно блокируем поиск поставок, которые не подходят нам по срокам доставки
    for (i = 0; i < this->Count(); i++) {
        pV = this->GetVendor(i);
        if (pV) {
            if (pV->Flags & vlfInStock)
                iSt = i;
            if ((pV->Delivery < minDeliv) || (pV->Delivery > maxDeliv))
                pV->Flags |= vlfSkipRanking;
        }
    }

    // Расчитываем сколько и какого качества нужно приобрести товара
    // исходя из требований к качеству итогового товара, и наличия товара на складе
    if (iSt >= 0) {
        Qs = GetVendor(iSt)->Quality;           // Качество товара на складе
        Ns = GetVendor(iSt)->InStock;           // Кол-во товара на складе
    }
    else {        // на складе нет товара
        Qs = 0;
        Ns = 0;
    }
    Qn = (Quality * (Number + Ns) - Qs * Ns) / Number;         // с таким средним качеством

    // Ищем выгодный товар (с самым высоким соотношением качество/цена)
    // один с качеством, выше требуемого, другой ниже
    // чтобы из них можно было набрать товар с нужным средним качеством
    //mAverageQuality = fAverageQuality;
    //fAverageQuality = Qn;
    ArrangeFlagsV2(1);
    //fAverageQuality = mAverageQuality;

    // Находим записи с товаром, который будем закупать
    for (i = 0; i < this->Count(); i++) {
        pV = this->GetVendor(i);
        if (pV) {
            if (pV->Flags & vlfHiBest) iHi = i;
            if (pV->Flags & vlfLoBest) iLow = i;
        }
    }

    if (iHi >= 0) {
        Qhi = GetVendor(iHi)->Quality;
        dQhi = Qhi - Qn;
        NAhi = GetVendor(iHi)->InStock;
    }
    else {
        Qhi = 0;
        NAhi = 0;
        dQhi = 100000;
    }

    if (iLow >= 0) {
        Qlow = GetVendor(iLow)->Quality;
        dQlow = Qn - Qlow;
        NAlow = GetVendor(iLow)->InStock;
    }
    else {
        Qlow = 0;
        NAlow = 0;
        dQlow = 100000;
    }

    //if ((NAhi == 0) && (Ns > 0) && (Qn > Qs)) return 0;                                      // Надо повышать, а нечем
    //if ((NAlow == 0) && (Ns > 0)  && (Qn < Qs)) return 0;                                      // Надо понижать, а нечем
    if (NAhi + NAlow == 0) return 0;                                              // проскольку не из чего заказывать

    if ((NAhi > 0) && (NAlow > 0)) {
        // Идеальный вариант, когда есть товар и высокого, и низкого качества

        // Расчитываем сколько нужно купить товара с высоким качеством
        // и скольно с низким
        if (dQhi < dQlow) {
            if (dQlow > 0.0001)                                                          // защищаемся от деления на 0
                Nhi = Number / (1 + dQhi / dQlow);
            else
                Nhi = Number;
            if (Nhi < 0) Nhi = 0;                                                     // на всякий случай
                                                                                      //Nlow = Number - Nhi;                                                      // можно и так
            if (NAlow && (Qlow != Quality))
                Nlow = (Ns*(Qs / Quality - 1) + Nhi*(Qhi / Quality - 1)) / (1 - Qlow / Quality);  // но так точнее quality получится
            else
                Nlow = 0;                 // такого быть не должно
        }
        else {
            if (dQhi > 0.0001)
                Nlow = Number / (1 + dQlow / dQhi);
            else
                Nlow = Number;
            if (Nlow < 0) Nlow = 0;                                                   // на всякий случай
                                                                                      //Nhi = Number - Nlow;
            if (NAhi && (Qhi != Quality))
                Nhi = (Ns*(Qs / Quality - 1) + Nlow*(Qlow / Quality - 1)) / (1 - Qhi / Quality);
            else
                Nhi = 0;                 // такого быть не должно
        }
    }
    else {
        // Вариант, когда есть только один товар (или плохого, или хорошего качества)
        // имеет смысл только если им можно поднять качество товара нв складе магазина
        if (Ns>0) {         // если на складе есть товар
            if ((NAhi>0) && (Qs < Quality)) {
                if (dQhi > 0.0001)                                                     // защищаемся от деления на 0
                    Nhi = Ns * (Qs - Quality) / (Quality - Qhi);
                else
                    Nhi = Number;
            }

            if ((NAlow>0) && (Qs > Quality)) {
                if (dQlow > 0.0001)                                                     // защищаемся от деления на 0
                    Nlow = Ns * (Qs - Quality) / (Quality - Qlow);
                else
                    Nlow = Number;
            }

        }
        // или если товар подходит по лимитам качества, то взять хотя бы это (все лучше, чем ничего)
        if ((0 == Nhi) && (0 == Nlow)) {                            // если пока еще ничего не выбрано к заказу
            // выбираем хоть что-то, лижбы попадало в рамки лимитов
            if  (NAhi > 0) {
                int NWhi = Number;                                          // Сколько нужно дозаказать продукта
                if (NWhi > NAhi) NWhi = NAhi;                               // Сколько получится заказать, учитывая, сколько есть на складе поставщика
                float QWHi = (Qhi* NWhi + Qs*Ns) / (NWhi + Ns);             // Какого качества получится товар на складе итоговый после заказа
                if( (QWHi > Quality*(1.0 + QMinLimit/100.0)) && (QWHi < Quality*(1.0 + QMaxLimit / 100.0)) )    // Если ав итоге попадаем в рамки лимитов
                    Nhi = NWhi;                                                                  // то нада брать
            }else
            if (NAlow > 0) {
                int NWlow = Number;                                         // Сколько нужно дозаказать продукта
                if (NWlow > NAlow) NWlow = NAhi;                            // Сколько получится заказать, учитывая, сколько есть на складе поставщика
                float QWlow = (Qlow* NWlow + Qs * Ns) / (NWlow + Ns);       // Какого качества получится товар на складе итоговый после заказа
                if ((QWlow > Quality*(1.0 + QMinLimit / 100.0)) && (QWlow < Quality*(1.0 + QMaxLimit / 100.0)))    // Если ав итоге попадаем в рамки лимитов
                    Nhi = NWlow;                                                                  // то нада брать
            }
        }
    }

    if (Nlow < 0) Nlow = 0;
    if (Nhi < 0) Nhi = 0;
    if (Nlow + Nhi > 0) {

        // Проверяем, достаточно ли нужного товара у поставщика
        // Если не достаточно, то пересчитываем кол-во закупаемого товара
        // так, чтобы соххранить результирующее качество
        if (NAhi < Nhi) {
            Nhi = NAhi;
            if (NAlow && (Qlow != Quality))
                Nlow = (Ns*(Qs / Quality - 1) + Nhi*(Qhi / Quality - 1)) / (1 - Qlow / Quality);
            else
                Nlow = 0;                 // такого быть не должно
        }
        if (Nhi < 0) Nhi = 0;

        if (NAlow < Nlow) {
            Nlow = NAlow;
            if (NAhi && (Qhi != Quality))
                Nhi = (Ns*(Qs / Quality - 1) + Nlow*(Qlow / Quality - 1)) / (1 - Qhi / Quality);
            else
                Nhi = 0;                 // такого быть не должно
        }
        if (Nlow < 0) Nlow = 0;

        // Поскольку нужного кол-ватовара у поставщиков могло и не оказаться
        // подсчитоаем какое результирующее качество товара мы получим
        Qn = (Qs * Ns + Qhi * Nhi + Qlow* Nlow) / (Ns + Nhi + Nlow);
        //if ( abs( Qn ) > OrderQErLimint ) return 0;

        // Проверяем на допустимость цены
        Price = 0;
        // вычисляем, какая средняя цена получится у закупаемого товара
        if (iHi >= 0)
            Price += GetVendor(iHi)->Cost * Nhi;
        if (iLow >= 0)
            Price += GetVendor(iLow)->Cost * Nlow;
        if (Nlow + Nhi == 0) return 0;
        Price /= (Nhi + Nlow);
        // сравниваем со средней ценой для города
        if (Price > fAveragePrice * (1 - 0.1)) return 0;                               // Иначе просто не выгодно закупать

    }
    else {
        // Если не удалось подобрать подходящий товар
        // но купить чегонибудь ввсеравно нужно
        // то попробуем выбрать из менее подходящего товара
        if (PurchaseAnyway) {
            // Сперва примеряемся к товару лучшего качества
            if (iHi >= 0) {
                if (GetVendor(iHi)->Quality != 0)                   // защита от деления на 0
                    Price = GetVendor(iHi)->Price * (fAverageQuality / GetVendor(iHi)->Quality);
                else
                    Price = fAveragePrice * 10;
                //  Проверяем, не слишком ли дорогой продукт
                if (Price < fAveragePrice * (1 + MaxPurchasePriceExcess / 100))
                    Nhi = Number;
            }
            // Если товара высокого качества на закупку таки нет
            // но в списке есть товар низкго качества, пробуем его
            if ((Nhi == 0) && (iLow >= 0)) {
                if (GetVendor(iLow)->Quality != 0)                   // защита от деления на 0
                    Price = GetVendor(iLow)->Price * (fAverageQuality / GetVendor(iLow)->Quality);
                else
                    Price = fAveragePrice * 10;
                //  Проверяем, не слишком ли дорогой продукт
                if (Price < fAveragePrice * (1 + MaxPurchasePriceExcess / 100))
                    Nlow = Number;
            }
        }
    }

    // Отмечаем нужное кол-во товара в плане закупки
    if (iHi >= 0)
        GetVendor(iHi)->OrderNum = Nhi;
    if (iLow >= 0)
        GetVendor(iLow)->OrderNum = Nlow;

    // Возвращаем закупаемое количество
    return Nhi + Nlow;
}

//-----------------------------------------------------------------------------
//      Расчитать заказ
//      Возвращает сумму всего заказа, а также:
//        Num         - сколько в результате на складе будет товара
//        midPrice    - какая получится средняя цена для товара
//        midQuality  - какое получится среднее качество для товара
//        maxDeliv    - максимальный срок поставки
//-----------------------------------------------------------------------------
float __fastcall TVendorList::CalcOrder(int &Num, float &midPrice, float &midQuality, int &maxDeliv)
{
    sVendorRec* pV;
    float res = 0;
    int rNum = 0;
    float rPrice = 0.0;
    float rQuality = 0.0;
    int rDeliv = 0;

    for (unsigned int i = 0; i < this->Count(); i++) {
        pV = this->GetVendor(i);
        if (pV && (pV->OrderNum)) {
            rNum += pV->OrderNum;
            rPrice += pV->OrderNum * pV->Cost;
            res += rPrice;
            rQuality += pV->OrderNum * pV->Quality;
            if (rDeliv < pV->Delivery)
                rDeliv = pV->Delivery;
        }
    }

    Num = rNum;
    if (rNum) {
        midPrice = rPrice / rNum;
        midQuality = rQuality / rNum;
    }
    else {
        midPrice = 0;
        midQuality = 0;
    }

    maxDeliv = rDeliv;
    return res;
}


//-----------------------------------------------------------------------------
//      Расчитать заказ и расчитать понедельную поставку с учетом понедельного потребления
//      Для понедельного расчета поставок необходимо указать день заказа и недельное потребление
//        Today       - какой сегодня день недели (это необходимо для правильного учета потребляемого в перерасчет товара)
//        Consumption - недельное потребление товара
//      Возвращает сумму всего заказа, а также:
//        purchased   - общее кол-во заказанного товара
//        pDelivRec   - указатель на масив понедельной доставки
//        Weeks       - кол-во записей (недель) в списке понедельной поставки
//      Каждая запись списка поставок это:
//        Num         - сколько в результате на складе будет товара к концу недели
//        midPrice    - какая получится средняя цена для товара
//        midQuality  - какое получится среднее качество для товара
//        maxDeliv    - день последней поставки на неделе
//-----------------------------------------------------------------------------
float __fastcall TVendorList::CalcOrderForFewWeeks(int Today, int Consumption, sDeliveryPaln *Plan)
{
    sVendorRec* pV;
    float res = 0;
    int rNum = 0;
    int DeliveryWeek;
    int DeliveryDay;

    if (!Plan) return res;
    memset(Plan, 0, sizeof(sDeliveryPaln));
    //Plan->WeekDeliv[0].Day = 1;
    //Plan->WeekDeliv[1].Day = 1;
    //Plan->WeekDeliv[2].Day = 1;
    //Plan->WeekDeliv[3].Day = 1;
    // расчитываем понедельно поставки
    // определяем сколько и какого товара будет к моменту перерасчета каждой недели
    for (unsigned int i = 0; i < this->Count(); i++) {
        pV = this->GetVendor(i);
        if (pV && (pV->OrderNum)) {                         // есть ли заказ этого товара
            if (Plan->MaxDelivery < pV->Delivery)
                Plan->MaxDelivery = pV->Delivery;

            /* возможно с учетом разгрузки надобудет сделать как-то так
            if (pV->Delivery>1) pV->Delivery++;
            но пока это не доказано
            */

            // определяем в какой день какой недели прибудет товар
            if ((Today > 0) && (Today <7)) {
                // Если доподленно известно какой сегодня день
                DeliveryWeek = (pV->Delivery + Today - 1) / 7;
                DeliveryDay = (pV->Delivery + Today - 1) % 7 + 1;
            }
            else {
                // или, если мы не знаеи какой сегодня день
                // все записываем в одну неделю
                DeliveryWeek = 0;
                DeliveryDay = 1;
            }

            // добавляем поставку в соответствующюю неделю плана поставок
            if (Plan->WeekDeliv[DeliveryWeek].Day < DeliveryDay)
                Plan->WeekDeliv[DeliveryWeek].Day = DeliveryDay;
            Plan->WeekDeliv[DeliveryWeek].Num += pV->OrderNum;
            Plan->WeekDeliv[DeliveryWeek].Quality += pV->OrderNum * pV->Quality;
            Plan->WeekDeliv[DeliveryWeek].Price += pV->OrderNum * pV->Cost;

            if ((pV->Flags & (vlfInStock | vlfOnTheWay)) == 0) {
                Plan->AllPurchased += pV->OrderNum;
                res += pV->OrderNum * pV->Cost;
            }
        }
    }

    // вычисляем среднее арифметическое
    for (unsigned int i = 0; i<4; i++) {
        Plan->WeekDeliv[i].Delivered = Plan->WeekDeliv[i].Num;
        if (Plan->WeekDeliv[i].Num) {
            Plan->WeekDeliv[i].Quality /= Plan->WeekDeliv[i].Num;
            Plan->WeekDeliv[i].Price /= Plan->WeekDeliv[i].Num;
        }
        else {
            Plan->WeekDeliv[i].Quality = 0;
            Plan->WeekDeliv[i].Price = 0;
        }

        // далее перерасчет с учетом недельного потребления
        if (i>0) {
            rNum = Plan->WeekDeliv[i - 1].Num - Consumption; //сколько останется товара от предидущей недели с учетом недельного потребления
            if (rNum > 0) {
                // если с предидущей недели что-нибудь останется, то этот товар добавится
                // к товару, пришедшему на этой неделе
                Plan->WeekDeliv[i].Quality *= Plan->WeekDeliv[i].Num;
                Plan->WeekDeliv[i].Quality += Plan->WeekDeliv[i - 1].Quality * rNum;
                Plan->WeekDeliv[i].Quality /= Plan->WeekDeliv[i].Num + rNum;

                Plan->WeekDeliv[i].Price *= Plan->WeekDeliv[i].Num;
                Plan->WeekDeliv[i].Price += Plan->WeekDeliv[i - 1].Price * rNum;
                Plan->WeekDeliv[i].Price /= Plan->WeekDeliv[i].Num + rNum;

                Plan->WeekDeliv[i].Num += rNum;
            }
        }
    }


    return res;
}

//===========================================================================================================================================
//
//===========================================================================================================================================

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
std::string GetCurrencyName(int currency)
{
    std::string Name;
    switch (currency) {
    case curJPY: Name = "JPY"; break;
    case curCNY: Name = "CNY"; break;
    case curEUR: Name = "EUR"; break;
    case curUAH: Name = "UAH"; break;
    case curRUB: Name = "RUB"; break;
    case curUSD: Name = "USD"; break;
    case curGBP: Name = "GBP"; break;
    default:
    case curUKNWN: Name = "---";
    }
    return Name;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
std::string IntToStr(long Value)
{
    std::string res;
    char s[50];
    sprintf(s, "%d", Value);
    res = s;
    return res;
};

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
std::string FloatToStr(float Value, int precision)
{
    std::string res;
    char s[50];
    char fs[10];
    sprintf(fs,"%c.%df", '%',precision);
    sprintf(s, fs, Value);
    res = s;
    return res;
};


