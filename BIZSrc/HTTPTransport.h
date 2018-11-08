#ifndef HHTPTRANSPORTH
#define HHTPTRANSPORTH

#include <string>
#include "include/curl.h"

//------------------------------------------------------------------------------
class tHTML_Response
{
protected:
public:
  tHTML_Response(void);
  ~tHTML_Response();

  char *Header;
  char *Body;

  int HeaderLen;
  int BodyLen;

  void ClearHeader(void);
  void ClearBody(void);
  void Clear(void);
  void AddHeader(char *buf, int len);
  void AddBody(char *buf, int len);
  void SetHeader(char *buf, int len);
  void SetHeader(char *buf);
  void SetBody(char *buf, int len);
  void SetBody(char *buf);
  void BodyXCode(UINT srcCodePage, UINT dstCodePage);           // ������������� ���� ������

  //__property char* Body = {read = fBody, write = SetBody};
  //__property char* Header = {read = fHeader, write = SetHeader};
};

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
class tBIZ_Transport 
{
private:
    DWORD CheckIP_1(void);

    DWORD TOR_Check_1(bool *TORpresent = NULL);
    std::string TOR_proxyAddr;
    int TOR_ProxyPort;
    int TOR_ProxyCmdPort;
    bool UseTOR;                                            // ������������ �� TOR ��� �������� �� ������
    bool CanUseTOR;                                         // ��������� ������������ TOR

protected:
    CURL *curl_handle;
    std::string CookiesPath;
    std::string CookiesFileName;
    void SetServer(const char* ServerName);                 // ���� �������� ���, �� ������������ � ����, ����� ����� ���� �������� � BIZ_Client
    void SetUserPsw(const char* User, const char* Psw);     // ---- ������� ���� ������������ �������� BIZ_Client->LoadPerson
    bool CurlInit(void);

public:
    tBIZ_Transport(void);
    ~tBIZ_Transport(void);

    std::string ServerName;
    std::string UserLogin;
    std::string UserPassword;

    void SetCookiesPath(std::string);
    bool Login(char *Captcha = NULL);                                               // ������������ � �������� ����
    bool POST(const char *URI, const char *Msg, tHTML_Response *Response);          // ��������� POST ������     
    bool GetAuthorizedPage(const char *URI, tHTML_Response *Response);              // �������� �������������� �������� �� URI (����������� ����������� ��������������)
    bool GetPublicPage(const char *URI, tHTML_Response *Response);                  // �������� ��������� �������� �� URI (�� ����������� ��������������)
    bool GetInternetPage(const char *URI, tHTML_Response *Response);                // �������� �������� �� � ����� BIZ

    DWORD CheckIP(void);                                                            // �������� ������� IP (�����, � �������� ���� �� ���� �������)
    bool ChangeIP(void);

    bool TOR_SetUp(char *proxyAddr, int proxyPort = 9150, int proxyCmdPort = 9151);
    bool TOR_On(void);
    void TOR_Off(void);
    DWORD TOR_Check(bool *TORpresent = NULL);                                        // ���������  ������� ������������� ����� TOR
    bool TOR_ChangeIP(void);

    bool ClearCookies(void);
    bool ChangeEntryPoint(void);
};



#endif

