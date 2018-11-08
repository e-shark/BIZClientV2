
//#include <stdlib.h>
//#include <string.h>
//#include <winnls.h>
#include "io.h"

#include "HTTPTransport.h"
#include "LogUtils.h"
#include "include/curl.h"
#include "BIZ_Parser.h"
#include "TCPSocket.h"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool FileExists(std::string fname)
{
    return::GetFileAttributes(fname.c_str()) != DWORD(-1);
}

//======================================================================================================================

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
tHTML_Response::tHTML_Response(void)
{
  Body = NULL;
  Header = NULL;
  BodyLen = 0;
  HeaderLen = 0;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
tHTML_Response::~tHTML_Response()
{
  if (Header)
    free(Header);
  if (Body)
    free(Body);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void tHTML_Response::ClearHeader(void)
{
  if (Header) {
    free(Header);
    Header = NULL;
  }
  HeaderLen = 0;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void tHTML_Response::ClearBody(void)
{
  if (Body) {
    free(Body);
    Body = NULL;
  }
  BodyLen = 0;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void tHTML_Response::Clear(void)
{
  ClearHeader();
  ClearBody();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void tHTML_Response::AddHeader(char *buf, int len)
{
  Header = (char*) realloc(Header, HeaderLen+len+1);
  if (Header) {
    memcpy(Header+HeaderLen, buf, len);
    HeaderLen = HeaderLen + len;
    Header[HeaderLen] = 0;                // на всякий пожарный
  }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void tHTML_Response::AddBody(char *buf, int len)
{
  Body = (char*) realloc(Body, BodyLen+len+1);
  if (Body) {
    memcpy(Body+BodyLen, buf, len);
    BodyLen = BodyLen + len;
    Body[BodyLen] = 0;                    // на всякий пожарный
  }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void tHTML_Response::SetHeader(char *buf, int len)
{
  ClearHeader();
  if (!buf) return;
  Header = (char*) malloc(len+1);                                              // для страховки делаем длину на 1 больше, чтобы в конец дописать 0
  if (Header) {
    memcpy(Header, buf, len);
    HeaderLen = len;
    Header[HeaderLen] = 0;                                                     // на всякий пожарный
  }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void tHTML_Response::SetHeader(char *buf)
{
  ClearHeader();
  if (!buf) return;
  int len = strlen(buf);
  Header = (char*) malloc(len+1);                                              // для страховки делаем длину на 1 больше, чтобы в конец дописать 0
  if (Header) {
    memcpy(Header, buf, len);
    HeaderLen = len;
    Header[HeaderLen] = 0;                                                     // на всякий пожарный
  }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void tHTML_Response::SetBody(char *buf, int len)
{
  ClearBody();
  if (!buf) return;
  Body = (char*) malloc(len+1);                                                // для страховки делаем длину на 1 больше, чтобы в конец дописать 0
  if (Body) {
    memcpy(Body, buf, len);
    BodyLen = len;
    Body[BodyLen] = 0;                                                         // на всякий пожарный
  }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void tHTML_Response::SetBody(char *buf)
{
  ClearBody();
  if (!buf) return;
  int len = strlen(buf);
  Body = (char*) malloc(len+1);                                                // для страховки делаем длину на 1 больше, чтобы в конец дописать 0
  if (Body) {
    memcpy(Body, buf, len);
    BodyLen = len;
    Body[BodyLen] = 0;                                                         // на всякий пожарный
  }
}

//-----------------------------------------------------------------------------
//      Перекодировка тела ответа
//-----------------------------------------------------------------------------
void tHTML_Response::BodyXCode(UINT srcCodePage, UINT dstCodePage)
{
    if (!Body) return;
    if (!BodyLen) return;

    int wsize = MultiByteToWideChar(srcCodePage, 0, Body, -1, 0, 0);           // Определяем какой нам нужен буфер
    LPWSTR wbuf = (LPWSTR)new char[wsize * sizeof(WCHAR)];                      // Выделяем буфер
    MultiByteToWideChar(srcCodePage, 0, Body, -1, wbuf, wsize);                // Декодируем в wchar

    int size = WideCharToMultiByte(dstCodePage, 0, wbuf, -1, 0, 0, 0, 0);       // Определяем какой нам нужен буфер
    ClearBody();                                                                // Удаляем уже не нужные данные
    Body = (char*) malloc(size+1);                                             // Выделяем память под новые данные
    WideCharToMultiByte(dstCodePage, 0, wbuf, -1, Body, size, 0, 0);           // Кодируем в нужную кодировку
    delete [] wbuf;                                                             // Удаляем временный буфер
    BodyLen = size;                                                             // Указываем новый размер данных
    Body[BodyLen] = 0;                                                         // на всякий пожарный
}

//======================================================================================================================

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
size_t write_callback(char *ptr, size_t size, size_t nmemb, tHTML_Response *RSP)
{
    size_t result = 0;
    if(RSP)
        for (int i = 0; i < nmemb; i++) {
            RSP->AddBody(ptr + i * size, size);
            result += size;
        }
    return result;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
size_t header_callback(char *buffer, size_t size, size_t nitems, tHTML_Response *RSP)
{
    size_t result = 0;
    if (RSP)
        for (int i = 0; i < nitems; i++) {
            RSP->AddHeader(buffer + i * size, size);
            result += size;
        }
    return result;
}

//======================================================================================================================

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
tBIZ_Transport::tBIZ_Transport(void)
{
    ServerName = "s4.bizmania.ru";
    CookiesPath = "";
    CookiesFileName = "cookies.cok";
    TOR_proxyAddr = "127.0.0.1";
    TOR_ProxyPort = 9150;
    TOR_ProxyCmdPort = 9151;
    UseTOR = false;
    CanUseTOR = false;

    curl_handle = curl_easy_init();
    if (curl_handle)
    {
        CurlInit();
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

tBIZ_Transport::~tBIZ_Transport(void)
{
    curl_easy_cleanup(curl_handle);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool tBIZ_Transport::CurlInit(void)
{
    std::string cfile;
    cfile = CookiesPath;
    cfile += CookiesFileName;

    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, &write_callback);
    curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, &header_callback);
    //curl_easy_setopt(curl_handle, CURLOPT_COOKIEJAR, "d:\\TEST2\\BIZ_Client_V2\\Debug\\cookies.cok");
    //curl_easy_setopt(curl_handle, CURLOPT_COOKIEFILE, "d:\\TEST2\\BIZ_Client_V2\\Debug\\cookies.cok");
    curl_easy_setopt(curl_handle, CURLOPT_COOKIEJAR, cfile.c_str());
    curl_easy_setopt(curl_handle, CURLOPT_COOKIEFILE, cfile.c_str());

    if (UseTOR) {
        std::string ps;
        ps = TOR_proxyAddr;
        ps += ":";
        ps += IntToStr(TOR_ProxyPort);
        curl_easy_setopt(curl_handle, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5);
        curl_easy_setopt(curl_handle, CURLOPT_PROXY, ps.c_str());
    }
    return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void tBIZ_Transport::SetCookiesPath(std::string path)
{
    curl_easy_cleanup(curl_handle);                     // при выполнении этой команды куки запишуться в файл
    CookiesPath = path;
    curl_handle = curl_easy_init();
    CurlInit();
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void tBIZ_Transport::SetServer(const char* ServName)
{
    //snprintf(ServerName, sizeof(ServerName), ServName);
    ServerName = ServName;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void tBIZ_Transport::SetUserPsw(const char* User, const char* Psw)
{
    UserLogin = User;
    UserPassword = Psw;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool tBIZ_Transport::Login(char *Captcha)
{
    bool result = false;
    CURLcode res;
    char addr[1024];
    snprintf(addr, sizeof(addr), "Логинимся на сервере %s как %s ...", ServerName.c_str(), UserLogin.c_str());
    LogMessage(addr, ML_WRK2);
    tHTML_Response *RSP = new tHTML_Response();

    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, RSP);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEHEADER, RSP);

    struct curl_httppost* post = NULL;
    struct curl_httppost* last = NULL;

    sprintf(addr, "%s/user/login/", ServerName.c_str());
    curl_easy_setopt(curl_handle, CURLOPT_URL, addr);
    curl_formadd(&post, &last,
        CURLFORM_COPYNAME, "login",
        CURLFORM_COPYCONTENTS, UserLogin.c_str(),
        CURLFORM_END);
    curl_formadd(&post, &last,
        CURLFORM_COPYNAME, "password",
        CURLFORM_COPYCONTENTS, UserPassword.c_str(),
        CURLFORM_END);
    curl_formadd(&post, &last,
        CURLFORM_COPYNAME, "$post",
        CURLFORM_COPYCONTENTS, "on",
        CURLFORM_END);
    curl_easy_setopt(curl_handle, CURLOPT_HTTPPOST, post);
    res = curl_easy_perform(curl_handle);
    curl_formfree(post);
    if (CURLE_OK == res)
        if (0 == RSP->BodyLen) {          // При нормально принятом логине тела нет, т.к. в хеадере перенаправление на домашнюю страницу
            result = true;
        }

    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, NULL);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEHEADER, NULL);
    delete RSP;
    if (result) {
        LogMessage(" залогинились успешно.", ML_WRK2);
        curl_easy_cleanup(curl_handle);                     // при выполнении этой команды куки запишуться в файл
        curl_handle = curl_easy_init();
        CurlInit();
    }
    else LogMessage(" Залогиниться не удалось!", ML_WRK2);
    return result;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool tBIZ_Transport::POST(const char *URI, const char *Msg, tHTML_Response *Response)
{
    bool result = false;
    char addr[1024];
    char data[4096];
    int ndat;

    Response->Clear();

    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, Response);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEHEADER, Response);

    snprintf(addr, sizeof(addr), "%s%s", ServerName.c_str() , URI);
    curl_easy_setopt(curl_handle, CURLOPT_URL, addr);

    ndat = snprintf(data, sizeof(data), Msg);
    curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, data);
    curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, ndat);

    CURLcode cres = curl_easy_perform(curl_handle);
    if (CURLE_OK == cres) {
        if (Response->BodyLen)
            Response->BodyXCode(CP_UTF8, CP_ACP);       // приводим к норме кодировку страницы
        result = true;
    }
    else {
        snprintf(addr, sizeof(addr), "Error POST (%d): %s", result, curl_easy_strerror(cres));
        LogMessage(addr, ML_ERR2);
        result = false;
    }

    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, NULL);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEHEADER, NULL);

    return result;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool tBIZ_Transport::GetPublicPage(const char *URI, tHTML_Response *Response)
{
    bool result = false;
    char addr[1024];

    Response->Clear();

    curl_easy_setopt(curl_handle, CURLOPT_POST, 0);
    curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1);

    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, Response);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEHEADER, Response);

    snprintf(addr, sizeof(addr), "%s%s", ServerName.c_str(), URI);
    curl_easy_setopt(curl_handle, CURLOPT_URL, addr);

    CURLcode cres= curl_easy_perform(curl_handle);
    if (CURLE_OK == cres) {
        if (Response->BodyLen)
            Response->BodyXCode(CP_UTF8, CP_ACP);       // приводим к норме кодировку страницы
        result = true;
    }
    else {
        snprintf(addr, sizeof(addr),"Error GetPage (%d): %s",result, curl_easy_strerror(cres) );
        LogMessage(addr, ML_ERR2);
    }

    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, NULL);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEHEADER, NULL);

    return result;
}

//-----------------------------------------------------------------------------
//  Получить авторизованную страницу
//-----------------------------------------------------------------------------
bool tBIZ_Transport::GetAuthorizedPage(const char *URI, tHTML_Response *Response)
{
    bool res = false;
    Response->Clear();

    GetPublicPage(URI, Response);
    bool x= BIZ_CheckLogined(Response);
    if (x) {
        res = true;
    }else{
        if (Login()) {
            GetPublicPage(URI, Response);
            if (BIZ_CheckLogined(Response)) {
                res = true;
            }
        }

    }
    if (!res) {
        std::string s;
        s = "Не удалось получить авторизованную страницу ";
        s += URI;
        LogMessage(s.c_str(), ML_ERR2);
    }

    return res;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool tBIZ_Transport::GetInternetPage(const char *URI, tHTML_Response *Response)
{
    bool result = false;
    char addr[1024];

    Response->Clear();

    curl_easy_setopt(curl_handle, CURLOPT_POST, 0);
    curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1);

    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, Response);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEHEADER, Response);

    snprintf(addr, sizeof(addr), "%s",  URI);
    curl_easy_setopt(curl_handle, CURLOPT_URL, addr);


    CURLcode cres = curl_easy_perform(curl_handle);
    if (CURLE_OK == cres) {
        if (Response->BodyLen)
            Response->BodyXCode(CP_UTF8, CP_ACP);       // приводим к норме кодировку страницы
        result = true;
    }
    else {
        snprintf(addr, sizeof(addr), "Error GetPage (%d): %s", result, curl_easy_strerror(cres));
        LogMessage(addr, ML_ERR2);
    }

    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, NULL);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEHEADER, NULL);

    return result;
}



//-----------------------------------------------------------------------------
//  Получить из удаленного источника мой IP
//  Вариант 1 (сервер http://clientn.free-hideip.com/map/whatismyip.php)
//  Ответ на этой странице выглядит так: <p>Result:</p> <p>201.200.215.25</p>
//-----------------------------------------------------------------------------
DWORD tBIZ_Transport::CheckIP_1(void)
{
    DWORD res = 0;
    bool getres;
    char *pC1, *pC2, *pC3;
    std::string s;
    BYTE b1, b2, b3, b4;
    char ts[50];
    int x;
    tHTML_Response *RSP = new tHTML_Response();


    getres = GetInternetPage("http://clientn.free-hideip.com/map/whatismyip.php", RSP);
    if (getres) {
        pC1 = strstr(RSP->Body, "<html");
        if (!pC1) {
            s = "  не найден тэг <html> на странице GetIP_1";
            LogMessage(s.c_str(), ML_ERR2);
            return res;
        }
        pC1 = strstr(pC1, "Result:");
        if (!pC1) return res;
        pC1 = strstr(pC1+7, "<p>");
        if (!pC1) return res;
        pC1 += 3;
        pC3 = strstr(pC1, "</p>");
        if (!pC3) return res;

        pC2 = strstr(pC1, ".");
        if (!pC2) return res;
        if (pC2 > pC3) return res;
        memset(ts, 0, sizeof(ts));
        memcpy(ts, pC1, pC2 - pC1);
        x = atoi(SpaceRemove(ts));
        if (x > 0xFF) return res;
        b1 = x & 0xFF;

        pC1 = pC2 + 1;
        pC2 = strstr(pC1+1, ".");
        if (!pC2) return res;
        if (pC2 > pC3) return res;
        memset(ts, 0, sizeof(ts));
        memcpy(ts, pC1, pC2 - pC1);
        x = atoi(SpaceRemove(ts));
        if (x > 0xFF) return res;
        b2 = x & 0xFF;

        pC1 = pC2 + 1;
        pC2 = strstr(pC1 + 1, ".");
        if (!pC2) return res;
        if (pC2 > pC3) return res;
        memset(ts, 0, sizeof(ts));
        memcpy(ts, pC1, pC2 - pC1);
        x = atoi(SpaceRemove(ts));
        if (x > 0xFF) return res;
        b3 = x & 0xFF;

        pC1 = pC2 + 1;
        if (pC1 > pC3) return res;
        memset(ts, 0, sizeof(ts));
        memcpy(ts, pC1, pC3 - pC1);
        x = atoi(SpaceRemove(ts));
        if (x > 0xFF) return res;
        b4 = x & 0xFF;

        res = (b1 << 24) + (b2 << 16) + (b3 << 8) + b4;
    }
    return res;
}

//-----------------------------------------------------------------------------
//  Проверка наличия подключения через TOR proxy
//  Вариант 1
//-----------------------------------------------------------------------------
DWORD  tBIZ_Transport::TOR_Check_1(bool *TORpresent)
{
    bool resbool = false;
    DWORD res = 0;
    bool getres;
    char *pC1, *pC2, *pC3;
    std::string s;
    BYTE b1, b2, b3, b4;
    char ts[50];
    int x;
    tHTML_Response *RSP = new tHTML_Response();
    getres = GetInternetPage("https://check.torproject.org/?lang=en", RSP);
    if (getres) {
        pC1 = strstr(RSP->Body, "<html");
        if (!pC1) {
            s = "  не найден тэг <html> на странице GetTOR_1";
            LogMessage(s.c_str(), ML_ERR2);
            return res;
        }
        pC1 += 5;
        pC2 = strstr(pC1, "Congratulations");
        if (!pC2) {
            pC2 = strstr(pC1, "Sorry.");
            if (pC2) {
                resbool = false;
            }
        }
        resbool = true;
        if (TORpresent) *TORpresent = resbool;

        pC1 = strstr(pC1, "Your IP address appears to be:");
        if (!pC1) return res;
        pC1 = strstr(pC1 + 7, "<strong>");
        if (!pC1) return res;
        pC1 += 8;
        pC3 = strstr(pC1, "</strong>");
        if (!pC3) return res;

        pC2 = strstr(pC1, ".");
        if (!pC2) return res;
        if (pC2 > pC3) return res;
        memset(ts, 0, sizeof(ts));
        memcpy(ts, pC1, pC2 - pC1);
        x = atoi(SpaceRemove(ts));
        if (x > 0xFF) return res;
        b1 = x & 0xFF;

        pC1 = pC2 + 1;
        pC2 = strstr(pC1 + 1, ".");
        if (!pC2) return res;
        if (pC2 > pC3) return res;
        memset(ts, 0, sizeof(ts));
        memcpy(ts, pC1, pC2 - pC1);
        x = atoi(SpaceRemove(ts));
        if (x > 0xFF) return res;
        b2 = x & 0xFF;

        pC1 = pC2 + 1;
        pC2 = strstr(pC1 + 1, ".");
        if (!pC2) return res;
        if (pC2 > pC3) return res;
        memset(ts, 0, sizeof(ts));
        memcpy(ts, pC1, pC2 - pC1);
        x = atoi(SpaceRemove(ts));
        if (x > 0xFF) return res;
        b3 = x & 0xFF;

        pC1 = pC2 + 1;
        if (pC1 > pC3) return res;
        memset(ts, 0, sizeof(ts));
        memcpy(ts, pC1, pC3 - pC1);
        x = atoi(SpaceRemove(ts));
        if (x > 0xFF) return res;
        b4 = x & 0xFF;

        res = (b1 << 24) + (b2 << 16) + (b3 << 8) + b4;
    }
    return res;
}

//-----------------------------------------------------------------------------
//  Проверка наличия подключения через TOR proxy
//-----------------------------------------------------------------------------
DWORD  tBIZ_Transport::TOR_Check(bool *TORpresent)
{
    DWORD resaddr = 0;
    bool resflag = false;
    resaddr = TOR_Check_1(&resflag);
    if (resaddr)
        if (TORpresent)
            *TORpresent = resflag;
    return resaddr;
}

//-----------------------------------------------------------------------------
//  Подать команду TORу сменть IP
//-----------------------------------------------------------------------------
bool tBIZ_Transport::TOR_ChangeIP(void)
{
    bool res = false;
    SOCKET s;
    int rl, wl;
    unsigned char wb[1000];
    unsigned char rb[1000];
    char *pC;

    InitTCP();
    s = TCP_openccon((char*)TOR_proxyAddr.c_str(), TOR_ProxyCmdPort);
    if (INVALID_SOCKET != s) {
        wl = sprintf((char*)wb, "AUTHENTICATE\r\n");
        wl = TCP_writesoc(s, wb, wl);
        rl = TCP_readsoc(s, rb, sizeof(rb), 20);
        wl = sprintf((char*)wb, "signal NEWNYM\r\n");
        wl = TCP_writesoc(s, wb, wl);
        rl = TCP_readsoc(s, rb, sizeof(rb), 20);
        if (rl < 0) res = false;
        else {
            pC = strstr((char*)rb,"250 OK");
            if (pC) res = true;
            else  res = false;
        }
    }
    if (INVALID_SOCKET != s) 
        TCP_closesoc(s);
    FreeTCP();

    return res;
}

//-----------------------------------------------------------------------------
//  Сконфигурировать TOR proxy
//-----------------------------------------------------------------------------
bool tBIZ_Transport::TOR_SetUp(char *proxyAddr, int proxyPort, int proxyCmdPort)
{
    bool res = false;
    if (!proxyAddr) return res;
    TOR_proxyAddr = proxyAddr;
    TOR_ProxyPort = proxyPort;
    TOR_ProxyCmdPort = proxyCmdPort;
    CanUseTOR = true;
}

//-----------------------------------------------------------------------------
//  Включить использование TOR proxy
//-----------------------------------------------------------------------------
bool tBIZ_Transport::TOR_On(void)
{
    std::string ps;
    if (CanUseTOR) {
        ps = TOR_proxyAddr;
        ps += ":";
        ps += IntToStr(TOR_ProxyPort);
        curl_easy_setopt(curl_handle, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5);
        curl_easy_setopt(curl_handle, CURLOPT_PROXY, ps.c_str());
        UseTOR = true;
        return true;
    }
    else 
        return false;
}

//-----------------------------------------------------------------------------
//  Отключить использование TOR proxy
//-----------------------------------------------------------------------------
void tBIZ_Transport::TOR_Off(void)
{
    curl_easy_setopt(curl_handle, CURLOPT_PROXY, "");
    UseTOR = false;
}


//-----------------------------------------------------------------------------
//  Получить из удаленного источника мой IP
//  (Как видят меня извне)
//-----------------------------------------------------------------------------
DWORD tBIZ_Transport::CheckIP(void)
{
    DWORD res = 0;
	std::string str;
    res = CheckIP_1();
	str = "Текущий IP: ";
	str += GetIPStr(res);
	LogMessage(str.c_str(), ML_WRK2);
	return res;
}


//-----------------------------------------------------------------------------
//  Сменить исходящий IP адрес клиента
//-----------------------------------------------------------------------------
bool tBIZ_Transport::ChangeIP(void)
{
    bool res = false;
    DWORD CurIP, NewIP;
    int attempt;
    std::string s;

    LogMessage("Смена IP ...", ML_WRK2);

    attempt = 0;
    do {
        CurIP = CheckIP();
        attempt++;
    } while ((0 == CurIP) && (attempt < 3));

    if (0 == CurIP) return false;

    if (UseTOR) {
        // если мы используем TOR
        if (!TOR_ChangeIP()) return false;
    }
    else {
        // если мы не используем TOR
        // тут можно реализовать через переназначение proxy
        // из списка разрешенных для пользователя проксей
        return false;
    }

    attempt = 0;
    do {
        NewIP = CheckIP();
        attempt++;
    } while ((0 == NewIP) && (attempt < 3));

    if (NewIP != CurIP) {
        s = "Сменили IP с ";
        s += GetIPStr(CurIP);
        s += " на ";
        s += GetIPStr(NewIP);
        LogMessage((char*)s.c_str(), ML_WRK2);
        return true;
    }
    else {
        s = "... cменить IP ";
        s += CurIP;
        s += " не удалось !";
        LogMessage((char*)s.c_str(), ML_ERR2);        
        return false;
    }

}

//-----------------------------------------------------------------------------
//  Очистить куки (для curlib)
//-----------------------------------------------------------------------------
bool tBIZ_Transport::ClearCookies(void)
{
    bool res;
    std::string cfile;

    LogMessage("Очистить Cookies", ML_WRK2);

    cfile = CookiesPath;
    cfile += CookiesFileName;
    if (FileExists(cfile.c_str()))
        res = (0 == remove(cfile.c_str()));
    else
        res = true;
    if (res) {
        curl_easy_setopt(curl_handle, CURLOPT_COOKIELIST, "ALL");
        //curl_easy_setopt(curl_handle, CURLOPT_COOKIEJAR, cfile.c_str());
        //curl_easy_setopt(curl_handle, CURLOPT_COOKIEFILE, cfile.c_str());
    }else
        LogMessage("... не удалось очистить Cookies !", ML_ERR2);
    return res;
}

//-----------------------------------------------------------------------------
//  Сменить точку входа
//  для анонимизации клиента
//-----------------------------------------------------------------------------
bool tBIZ_Transport::ChangeEntryPoint(void)
{
    bool res = false;
    LogMessage("Сменить точку входа", ML_WRK1);
    res = ClearCookies();
    res &= ChangeIP();
    if (!res) LogMessage("Сменить точку входа не удалось", ML_ERR2);
    return res;
}
