
#include "BIZ_Types.h"
#include "LogUtils.h"

//-----------------------------------------------------------------------------
//      ��������� ����� � ������ ������
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
//      �������� ������� ���������  sVendorRec
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
//      ��������� Kdqp - ����������� ��������/����
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
//      �������� ������� ������  TVendorList
//============================================================================================================================

//-----------------------------------------------------------------------------
//      ����������� ������
//-----------------------------------------------------------------------------
TVendorList::TVendorList(void)
{
    memset(DbgLogId, 0, sizeof(DbgLogId));
    sprintf(DbgLogId, "VendorList");

    fMasLen = 1000;         // ����� ������� ����������
    fCount = 0;            // ���������� ������� � �������
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
//      ���������� ������
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
//      �������� ������
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

    fMasLen = 1000;         // ����� ������� ����������
    fCount = 0;            // ���������� ������� � �������
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
//      �������� ������ � ������ �����������
//-----------------------------------------------------------------------------
sVendorRec* TVendorList::Add(sVendorRec* Vendor)
{
    if (fCount + 1 >= fMasLen) {                                      // ���� ������ ��� ��������
        fMasLen += 1000;                                            // ����������� ����������� �������
        VendorsMas = (sVendorRec**)realloc(VendorsMas, fMasLen);           // � �������� ��� ���� �������������� ������
    }
    VendorsMas[fCount] = Vendor;
    fCount++;
    return Vendor;
}

//-----------------------------------------------------------------------------
//      �������� ������ ������ � ������ �����������
//-----------------------------------------------------------------------------
sVendorRec* TVendorList::Add(void)
{
    sVendorRec* V = new sVendorRec(this);
    return Add(V);
}

//-----------------------------------------------------------------------------
//      �������� ����������� ������  ������ �����������
//              aVName -    �������� ����������
//              aVURI -     URI ����������� ����������
//              aCityID -   ID ������ ����������� ����������
//              aCName -    �������� �������� ����������
//              aCURI -     URI �������� ����������
//              aInStock -  ���������� ������ �� ������ ����������
//              aQuality -  �������� ������
//              aDelivery - ���� ��������
//              aPrice -    ���� ������
//              aCost -     ��������� ������ � ������ ��������
//              aCurrency - ������, � ������� ������� ����
//-----------------------------------------------------------------------------
sVendorRec* TVendorList::Add(char *aVName, char *aVURI, int aCityID, char *aCName, char *aCURI, int aInStock, float aQuality, int aDelivery, float aPrice, float aCost, int aCurrency)
{
    sVendorRec* V = new sVendorRec(this, aVName, aVURI, aCityID, aCName, aCURI, aInStock, aQuality, aDelivery, aPrice, aCost, aCurrency);
    return Add(V);
}

//-----------------------------------------------------------------------------
//      ���������� �������� ������� ����
//-----------------------------------------------------------------------------
void TVendorList::SetAveragePrice(float Value)
{
    sVendorRec * pV;

    fAveragePrice = Value;
    // ������������� ������������, ��������� �� AveragePrice
    for (unsigned int N = 0; N<fCount; N++) {
        pV = VendorsMas[N];
        if (pV)
            pV->Kdqp = pV->GetKdqp();
    }
}

//-----------------------------------------------------------------------------
//      ���������� �������� �������� ��������
//-----------------------------------------------------------------------------
void TVendorList::SetAverageQuality(float Value)
{
    sVendorRec * pV;

    fAverageQuality = Value;
    // ������������� ������������, ��������� �� AverageQuality
    for (unsigned int N = 0; N<fCount; N++) {
        pV = VendorsMas[N];
        if (pV)
            pV->Kdqp = pV->GetKdqp();
    }
}

//-----------------------------------------------------------------------------
//      ����������� ����� �������
//-----------------------------------------------------------------------------
void TVendorList::KeySort(eSortBy SortBy)
{
    int n;

    // ������ ������� �����
    for (unsigned int i = 0; i<fCount; i++)
        VendorsMas[i]->KeyIndex = -1;

    // ����������� ������� ���� �� 1 �� fCount
    for (unsigned int i = 1; i - 1<fCount; i++) {
        n = -1;
        for (unsigned int k = 0; k<fCount; k++) {
            if (VendorsMas[k]->Flags & (vlfInStock | vlfOnTheWay)) // ��� �������, ��� ��������� �� ������, ��� ��� ������������ �� ����, ����������� �� �����
                continue;
            if (VendorsMas[k]->KeyIndex == -1) {        // ������������� ������ ��� �� ��������� ��������
                if (n == -1) n = k;
                switch (SortBy) {
                case srtbyQualUp:                                       // ����������� �� �������� ����� �����. ������ ������� � ����� ������ ���������
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
//      ���������������� ������� ����������� � ���������� ����� �� Kdqp
//      ������ 2
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

    // ��������� ���� �� �������� �������� ������������
    this->KeySort(srtbyKdqpDown);

    // ���������� ��� ����� ���������� (��������� ��������)
    for (i = 0; i < this->Count(); i++)
        this->GetVendor(i)->Flags &= 0xFF00;

    // ���� �������� ���� ������� Quality
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

    // ���� �������� ���� ������� Quality
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

    // ����������� �����
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
//      ��������� ��� ������� ������ �����
//      ���������� ����� ����� ������, ������� ��������� ��������
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
    float Qs;           // �������� ������ �� ������
    float Qn;           // �������� ������, ������� ����� ��������, � ������ ���� ������, ������� ��� ���� �� ������
    int Ns;             // ���������� ������ �� ������
    float dQhi, dQlow;
    float Qhi, Qlow;
    int Nhi = 0, Nlow = 0;
    unsigned int i;
    sVendorRec* pV;
    float Price;

    if (Quality == 0) return 0;
    if (Number == 0) return 0;

    // ���� �����, ������� ��� ���� �� ������
    // ������ ��������� ����� ��������, ������� �� �������� ��� �� ������ ��������
    for (i = 0; i < this->Count(); i++) {
        pV = this->GetVendor(i);
        if (pV) {
            if (pV->Flags & vlfInStock)
                iSt = i;
            if ((pV->Delivery < minDeliv) || (pV->Delivery > maxDeliv))
                pV->Flags |= vlfSkipRanking;
        }
    }

    // ����������� ������� � ������ �������� ����� ���������� ������
    // ������ �� ���������� � �������� ��������� ������, � ������� ������ �� ������
    if (iSt >= 0) {
        Qs = GetVendor(iSt)->Quality;           // �������� ������ �� ������
        Ns = GetVendor(iSt)->InStock;           // ���-�� ������ �� ������
    }
    else {        // �� ������ ��� ������
        Qs = 0;
        Ns = 0;
    }
    Qn = (Quality * (Number + Ns) - Qs * Ns) / Number;         // � ����� ������� ���������

    // ���� �������� ����� (� ����� ������� ������������ ��������/����)
    // ���� � ���������, ���� ����������, ������ ����
    // ����� �� ��� ����� ���� ������� ����� � ������ ������� ���������
    //mAverageQuality = fAverageQuality;
    //fAverageQuality = Qn;
    ArrangeFlagsV2(1);
    //fAverageQuality = mAverageQuality;

    // ������� ������ � �������, ������� ����� ��������
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

    //if ((NAhi == 0) && (Ns > 0) && (Qn > Qs)) return 0;                                      // ���� ��������, � �����
    //if ((NAlow == 0) && (Ns > 0)  && (Qn < Qs)) return 0;                                      // ���� ��������, � �����
    if (NAhi + NAlow == 0) return 0;                                              // ���������� �� �� ���� ����������

    if ((NAhi > 0) && (NAlow > 0)) {
        // ��������� �������, ����� ���� ����� � ��������, � ������� ��������

        // ����������� ������� ����� ������ ������ � ������� ���������
        // � ������� � ������
        if (dQhi < dQlow) {
            if (dQlow > 0.0001)                                                          // ���������� �� ������� �� 0
                Nhi = Number / (1 + dQhi / dQlow);
            else
                Nhi = Number;
            if (Nhi < 0) Nhi = 0;                                                     // �� ������ ������
                                                                                      //Nlow = Number - Nhi;                                                      // ����� � ���
            if (NAlow && (Qlow != Quality))
                Nlow = (Ns*(Qs / Quality - 1) + Nhi*(Qhi / Quality - 1)) / (1 - Qlow / Quality);  // �� ��� ������ quality ���������
            else
                Nlow = 0;                 // ������ ���� �� ������
        }
        else {
            if (dQhi > 0.0001)
                Nlow = Number / (1 + dQlow / dQhi);
            else
                Nlow = Number;
            if (Nlow < 0) Nlow = 0;                                                   // �� ������ ������
                                                                                      //Nhi = Number - Nlow;
            if (NAhi && (Qhi != Quality))
                Nhi = (Ns*(Qs / Quality - 1) + Nlow*(Qlow / Quality - 1)) / (1 - Qhi / Quality);
            else
                Nhi = 0;                 // ������ ���� �� ������
        }
    }
    else {
        // �������, ����� ���� ������ ���� ����� (��� �������, ��� �������� ��������)
        // ����� ����� ������ ���� �� ����� ������� �������� ������ �� ������ ��������
        if (Ns>0) {         // ���� �� ������ ���� �����
            if ((NAhi>0) && (Qs < Quality)) {
                if (dQhi > 0.0001)                                                     // ���������� �� ������� �� 0
                    Nhi = Ns * (Qs - Quality) / (Quality - Qhi);
                else
                    Nhi = Number;
            }

            if ((NAlow>0) && (Qs > Quality)) {
                if (dQlow > 0.0001)                                                     // ���������� �� ������� �� 0
                    Nlow = Ns * (Qs - Quality) / (Quality - Qlow);
                else
                    Nlow = Number;
            }

        }
        // ��� ���� ����� �������� �� ������� ��������, �� ����� ���� �� ��� (��� �����, ��� ������)
        if ((0 == Nhi) && (0 == Nlow)) {                            // ���� ���� ��� ������ �� ������� � ������
            // �������� ���� ���-��, ����� �������� � ����� �������
            if  (NAhi > 0) {
                int NWhi = Number;                                          // ������� ����� ���������� ��������
                if (NWhi > NAhi) NWhi = NAhi;                               // ������� ��������� ��������, ��������, ������� ���� �� ������ ����������
                float QWHi = (Qhi* NWhi + Qs*Ns) / (NWhi + Ns);             // ������ �������� ��������� ����� �� ������ �������� ����� ������
                if( (QWHi > Quality*(1.0 + QMinLimit/100.0)) && (QWHi < Quality*(1.0 + QMaxLimit / 100.0)) )    // ���� �� ����� �������� � ����� �������
                    Nhi = NWhi;                                                                  // �� ���� �����
            }else
            if (NAlow > 0) {
                int NWlow = Number;                                         // ������� ����� ���������� ��������
                if (NWlow > NAlow) NWlow = NAhi;                            // ������� ��������� ��������, ��������, ������� ���� �� ������ ����������
                float QWlow = (Qlow* NWlow + Qs * Ns) / (NWlow + Ns);       // ������ �������� ��������� ����� �� ������ �������� ����� ������
                if ((QWlow > Quality*(1.0 + QMinLimit / 100.0)) && (QWlow < Quality*(1.0 + QMaxLimit / 100.0)))    // ���� �� ����� �������� � ����� �������
                    Nhi = NWlow;                                                                  // �� ���� �����
            }
        }
    }

    if (Nlow < 0) Nlow = 0;
    if (Nhi < 0) Nhi = 0;
    if (Nlow + Nhi > 0) {

        // ���������, ���������� �� ������� ������ � ����������
        // ���� �� ����������, �� ������������� ���-�� ����������� ������
        // ���, ����� ���������� �������������� ��������
        if (NAhi < Nhi) {
            Nhi = NAhi;
            if (NAlow && (Qlow != Quality))
                Nlow = (Ns*(Qs / Quality - 1) + Nhi*(Qhi / Quality - 1)) / (1 - Qlow / Quality);
            else
                Nlow = 0;                 // ������ ���� �� ������
        }
        if (Nhi < 0) Nhi = 0;

        if (NAlow < Nlow) {
            Nlow = NAlow;
            if (NAhi && (Qhi != Quality))
                Nhi = (Ns*(Qs / Quality - 1) + Nlow*(Qlow / Quality - 1)) / (1 - Qhi / Quality);
            else
                Nhi = 0;                 // ������ ���� �� ������
        }
        if (Nlow < 0) Nlow = 0;

        // ��������� ������� ���-�������� � ����������� ����� � �� ���������
        // ����������� ����� �������������� �������� ������ �� �������
        Qn = (Qs * Ns + Qhi * Nhi + Qlow* Nlow) / (Ns + Nhi + Nlow);
        //if ( abs( Qn ) > OrderQErLimint ) return 0;

        // ��������� �� ������������ ����
        Price = 0;
        // ���������, ����� ������� ���� ��������� � ����������� ������
        if (iHi >= 0)
            Price += GetVendor(iHi)->Cost * Nhi;
        if (iLow >= 0)
            Price += GetVendor(iLow)->Cost * Nlow;
        if (Nlow + Nhi == 0) return 0;
        Price /= (Nhi + Nlow);
        // ���������� �� ������� ����� ��� ������
        if (Price > fAveragePrice * (1 - 0.1)) return 0;                               // ����� ������ �� ������� ��������

    }
    else {
        // ���� �� ������� ��������� ���������� �����
        // �� ������ ���������� ��������� �����
        // �� ��������� ������� �� ����� ����������� ������
        if (PurchaseAnyway) {
            // ������ ����������� � ������ ������� ��������
            if (iHi >= 0) {
                if (GetVendor(iHi)->Quality != 0)                   // ������ �� ������� �� 0
                    Price = GetVendor(iHi)->Price * (fAverageQuality / GetVendor(iHi)->Quality);
                else
                    Price = fAveragePrice * 10;
                //  ���������, �� ������� �� ������� �������
                if (Price < fAveragePrice * (1 + MaxPurchasePriceExcess / 100))
                    Nhi = Number;
            }
            // ���� ������ �������� �������� �� ������� ���� ���
            // �� � ������ ���� ����� ������ ��������, ������� ���
            if ((Nhi == 0) && (iLow >= 0)) {
                if (GetVendor(iLow)->Quality != 0)                   // ������ �� ������� �� 0
                    Price = GetVendor(iLow)->Price * (fAverageQuality / GetVendor(iLow)->Quality);
                else
                    Price = fAveragePrice * 10;
                //  ���������, �� ������� �� ������� �������
                if (Price < fAveragePrice * (1 + MaxPurchasePriceExcess / 100))
                    Nlow = Number;
            }
        }
    }

    // �������� ������ ���-�� ������ � ����� �������
    if (iHi >= 0)
        GetVendor(iHi)->OrderNum = Nhi;
    if (iLow >= 0)
        GetVendor(iLow)->OrderNum = Nlow;

    // ���������� ���������� ����������
    return Nhi + Nlow;
}

//-----------------------------------------------------------------------------
//      ��������� �����
//      ���������� ����� ����� ������, � �����:
//        Num         - ������� � ���������� �� ������ ����� ������
//        midPrice    - ����� ��������� ������� ���� ��� ������
//        midQuality  - ����� ��������� ������� �������� ��� ������
//        maxDeliv    - ������������ ���� ��������
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
//      ��������� ����� � ��������� ����������� �������� � ������ ������������ �����������
//      ��� ������������ ������� �������� ���������� ������� ���� ������ � ��������� �����������
//        Today       - ����� ������� ���� ������ (��� ���������� ��� ����������� ����� ������������� � ���������� ������)
//        Consumption - ��������� ����������� ������
//      ���������� ����� ����� ������, � �����:
//        purchased   - ����� ���-�� ����������� ������
//        pDelivRec   - ��������� �� ����� ����������� ��������
//        Weeks       - ���-�� ������� (������) � ������ ����������� ��������
//      ������ ������ ������ �������� ���:
//        Num         - ������� � ���������� �� ������ ����� ������ � ����� ������
//        midPrice    - ����� ��������� ������� ���� ��� ������
//        midQuality  - ����� ��������� ������� �������� ��� ������
//        maxDeliv    - ���� ��������� �������� �� ������
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
    // ����������� ���������� ��������
    // ���������� ������� � ������ ������ ����� � ������� ����������� ������ ������
    for (unsigned int i = 0; i < this->Count(); i++) {
        pV = this->GetVendor(i);
        if (pV && (pV->OrderNum)) {                         // ���� �� ����� ����� ������
            if (Plan->MaxDelivery < pV->Delivery)
                Plan->MaxDelivery = pV->Delivery;

            /* �������� � ������ ��������� ��������� ������� ���-�� ���
            if (pV->Delivery>1) pV->Delivery++;
            �� ���� ��� �� ��������
            */

            // ���������� � ����� ���� ����� ������ �������� �����
            if ((Today > 0) && (Today <7)) {
                // ���� ���������� �������� ����� ������� ����
                DeliveryWeek = (pV->Delivery + Today - 1) / 7;
                DeliveryDay = (pV->Delivery + Today - 1) % 7 + 1;
            }
            else {
                // ���, ���� �� �� ����� ����� ������� ����
                // ��� ���������� � ���� ������
                DeliveryWeek = 0;
                DeliveryDay = 1;
            }

            // ��������� �������� � ��������������� ������ ����� ��������
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

    // ��������� ������� ��������������
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

        // ����� ���������� � ������ ���������� �����������
        if (i>0) {
            rNum = Plan->WeekDeliv[i - 1].Num - Consumption; //������� ��������� ������ �� ���������� ������ � ������ ���������� �����������
            if (rNum > 0) {
                // ���� � ���������� ������ ���-������ ���������, �� ���� ����� ���������
                // � ������, ���������� �� ���� ������
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


