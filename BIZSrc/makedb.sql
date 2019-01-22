

CREATE TABLE Person(
	ID		INTEGER PRIMARY KEY ASC NOT NULL,
	Server		TEXT,
	User 	  	VARCHAR(50),
	Psw	  	VARCHAR(50),
	ProxyNAme 	TEXT,
	ProxyPort 	VARCHAR(50),
	ProxyUser 	VARCHAR(50),
	ProxyPwd 	VARCHAR(50)
)

CREATE TABLE Person(
	ID		INTEGER PRIMARY KEY ASC NOT NULL,
	Server		TEXT,
	User 	  	VARCHAR(50),
	Psw	  	VARCHAR(50),
	ProxyList	INTEGER,
	Company		INTEGER,
	TOR_IP	  	VARCHAR(18) DEFAULT ('127.0.0.1'),
	TOR_PORT 	INTEGER,
	TOR_cmdPORT 	INTEGER
)
-- ��� ������� ����� ���� ������ ������. � ������ ������ ������� ��������������, ��� ���� ���������� ��������� (� ���������� ���������� ��������������)

--===============================================================================================================
-- �����������

CREATE TABLE UnitsTypes(
	ID		INTEGER PRIMARY KEY ASC NOT NULL,
	Name		TEXT,
	Image		VARCHAR(20)
)

CREATE TABLE Cities(
	ID		INTEGER PRIMARY KEY ASC NOT NULL,
	Name		TEXT,
	Country		INTEGER
			REFERENCES Cities(ID),
	Region		INTEGER
			REFERENCES Cities(ID),
	beginner	BOOLEAN		
)

CREATE TABLE ProductsTypes(
	ID		INTEGER PRIMARY KEY ASC NOT NULL,
	Name		TEXT,
	Group		INTEGER,
			REFERENCES ProductsTypes(ID),
	Image		VARCHAR(20)
)

--===============================================================================================================

CREATE TABLE Units(
	UID		INTEGER PRIMARY KEY ASC NOT NULL,
	PID		INTEGER,				-- ���� �������������, �� ������ �� ����������� (Units). ���� �����������, �� ������ �� �������� (Units)  .���� ��������, �� ������ �� ������ (Person)
	Type		INTEGER NOT NULL,
	City		INTEGER,
	Name		TEXT,
	Company		INTEGER,
	Corporation	INTEGER,
	Image		TEXT
)

-- ����� ������ �� ������
CREATE TABLE UnitsInfo(
	UID		INTEGER NOT NULL
	TS		DATETIME,		-- �����, ����� ���� ������� ������ (����)
	STS		BLOOB,			-- ����� �� �������
	UnitData	BLOOB			-- ��������� ������
)


-- ����� ������ �� ��������
CREATE TABLE CompanyInfo(
	�ID		INTEGER,
	TS		DATETIME,		-- �����, ����� ���� ������� ������ (����)
	STS		BLOOB,			-- ����� �� �������
	UnitData	BLOOB			-- ��������� ������
)

-- ������ ������� ��� �������
CREATE TABLE Schedule(
	ID		INTEGER PRIMARY KEY ASC NOT NULL,
	�ID		INTEGER,
	Type		INTEGER,
	Desription	TEXT,
	ParamI1		INTEGER, 
	ParamI2		INTEGER, 
	ParamStr	TEXT, 
	ParamBin	BLOB, 
	Period		INTEGER,
	Start		DATETIME,
	LastRun		DATETIME,
	ResCod		INTEGER,
	Disabled	INTEGER
)

-- ������� ����� ��
CREATE TABLE ExchangeState(
	PurchasePrice	REAL,		-- ���������� ����, (�� ������� ����� ������) (�� ������� ����� �������)
	SellingPrice	REAL,		-- ��������� ����, (�� ������� ����� �������) (�� ������� ����� ��������)
	TimeStamp	DATETIME,	-- ����� ������� ������
	ServerTime	BLOB,		-- �������� ��������� � ������� ������� �������
	ServerTimeStr	VARCHAR (15)	-- ������ ��������.��� ����� �������
)

CREATE TABLE options(
	Server          TEXT,
	HLenQPExRate    INTEGER,
	HLenCompInfo    INTEGER
	
)

-- ������� �������������
CREATE TABLE Factors(
	Type		INTEGER NOT NULL,	-- ��� �����������
	Object		INTEGER NOT NULL,	-- ������������� ������� (�������� ID ������ ,0xFFFF - ��� ������, 0xFENN NN- ID ������ ������, 0xFDNN NN- ID ������� ��� ������� �������)
	LocationType	INTEGER NOT NULL,	-- ��� ��������� (1-�������, 2-�����, 3-������, 4-������, 5-��� ���� ��������, 10-��� ������ ��������� �� �������)
	Location	INTEGER NOT NULL,	-- ������������� ������� (ID ��������, ��� ID �������)
	int		INTEGER NOT NULL		-- �������� ������������
)

--INSERT INTO Factors (Type,Object,LocationType,Location,Value) VALUES ();
--INSERT INTO Factors (Type,Object,LocationType,Location,Value) VALUES ();
--INSERT INTO Factors (Type,Object,LocationType,Location,Value) VALUES ();

-- ��������� �������������
CREATE TABLE FactorsTypeDescrioption(
	Type		INTEGER NOT NULL,	-- ��� �����������
	Name		TEXT NOT NULL,		-- �������� ������������
	Unit		TEXT,			-- �������� ������ ���������
	Desription	TEXT,			-- �������� ������������
	DesriptionExt	TEXT			-- �������� ������������
)

INSERT INTO FactorsTypeDescrioption (Type,Name,Unit,Desription,DesriptionExt) VALUES(1,"Price","","","���� �������� (������)");
INSERT INTO FactorsTypeDescrioption (Type,Name,Unit,Desription,DesriptionExt) VALUES(2,"Price Dumping","%","���������� ��� ������� �����","�� ������� ��������� ������ ����� ��������� ���� ��������� ��� �������������� ���� (-99% .. +99%)");
INSERT INTO FactorsTypeDescrioption (Type,Name,Unit,Desription,DesriptionExt) VALUES(3,"Price Permitted Excess","%","������������ ���������� ���� ��� �������","�� ������� ��������� ���� �� ������� ����� ��������� ������� ���� �� ������ (-99% .. +99%)");
INSERT INTO FactorsTypeDescrioption (Type,Name,Unit,Desription,DesriptionExt) VALUES(4,"Quality","","��������","����������� �������� ��������");
INSERT INTO FactorsTypeDescrioption (Type,Name,Unit,Desription,DesriptionExt) VALUES(5,"Quality Dumping","%","���������� ��������","�� ������� ��������� ����� �� �������� �������� ����� �������� �������� (-99% .. +99%)");
INSERT INTO FactorsTypeDescrioption (Type,Name,Unit,Desription,DesriptionExt) VALUES(6,"Quality Minimum","","����������� ��������","����������� ���������� �������� ��� ��������");
INSERT INTO FactorsTypeDescrioption (Type,Name,Unit,Desription,DesriptionExt) VALUES(7,"Quality Maximum","","������������ ��������","������������ ���������� �������� ��� ��������");
INSERT INTO FactorsTypeDescrioption (Type,Name,Unit,Desription,DesriptionExt) VALUES(8,"Quality Minimum Dumping","%","����������� ��������� ��������","����������� ���������� �������� ��� �������� ������������ ��������");
INSERT INTO FactorsTypeDescrioption (Type,Name,Unit,Desription,DesriptionExt) VALUES(9,"Quality Maximum Dumping","%","������������ ��������� ��������","������������ ���������� �������� ��� �������� ������������ ��������");



-- ������� ��� �������� ��������
CREATE TABLE Arrays(
	ArrayID		INTEGER NOT NULL,	-- ID �������
	Key		INTEGER,		-- ���� �������� ������� (���� �����)
	Value		REAL NOT NULL		-- �������� �������� �������
)
-- ��� �������
CREATE TABLE ArrayType(
	ArrayID		INTEGER NOT NULL,	-- ID �������
	ArrayType	INTEGER NOT NULL,	-- ��� �������
)

-- ���������������� ���������
CREATE TABLE Config(
	ID		INTEGER NOT NULL,	-- ID ���������
	KEY		TEXT NOT NULL,		-- ��������� ���� ���������
	Value		TAXT NOT NULL		-- �������� ���������
}
INSERT INTO Config (ID,KEY,Value) VALUES(1,"UserTOR",1);		-- ������������ TOR proxy
INSERT INTO Config (ID,KEY,Value) VALUES(10,"ShopsAutoSupply",0);	-- �������� ������������� ��������� (�� �������� �������) �� N ���� (0-���������)
INSERT INTO Config (ID,KEY,Value) VALUES(2,"FileLogLevel","333");
INSERT INTO Config (ID,KEY,Value) VALUES(3,"DBLogLevel","000");
--INSERT INTO Config (ID,KEY,Value) VALUES(,"",);
--INSERT INTO Config (ID,KEY,Value) VALUES(,"",);
--INSERT INTO Config (ID,KEY,Value) VALUES(,"",);



