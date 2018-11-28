#ifndef BIZEEXCEPTIONSH
#define BIZEEXCEPTIONSH

//---------------------------------------------------------------------------
#include <exception>
#include <string>

using namespace std;

class BIZException : public exception
{

public:
    /* Exception.Create */
    BIZException(const std::string Msg) : exception(), Message(Msg)
    { }

public:
    /* TObject.Destroy */
    virtual ~BIZException() throw() {}

public:
    std::string Message;
};


//---------------------------------------------------------------------------

class EDBException : public BIZException
{
public:
    EDBException(const std::string Msg) : BIZException(Msg.c_str()) { }
    virtual ~EDBException() throw() {}
};


//---------------------------------------------------------------------------
class EParserException : public BIZException
{
public:
    long Cod;

    EParserException(const std::string Msg, long cod = 100l) : BIZException(Msg.c_str()), Cod(cod) {};
    EParserException(long cod) : BIZException("Error"), Cod(cod) { };
    virtual ~EParserException() throw() {}
};


//---------------------------------------------------------------------------

#endif
