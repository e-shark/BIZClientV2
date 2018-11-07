#ifndef LOGUTILS_H
#define LOGUTILS_H

#define LogInSingleDir

#define ML_WRK1     0x0001      // �������� ����� �������� ���������
#define ML_WRK2     0x0002      // ���� �������� ������
#define ML_WRK3     0x0004      // ����������� ���������

#define ML_ERR1     0x0010      // ����������� ������
#define ML_ERR2     0x0020      // ������������ ������
#define ML_ERR3     0x0040      // ������ "����������"

#define ML_DBG1     0x0100      // �������� ����� �������
#define ML_DBG2     0x0200      // ������������� ����������
#define ML_DBG3     0x0400      // ����������� ���������� ����������

bool SetLogPath(const char * Path);

int LogMessage(const char* Message, int Level = ML_DBG1);
int LogMessageProv(const char* Prov, const char* Message, int Level);


#endif

