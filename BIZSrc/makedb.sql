

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
-- для каждого юзера свой список прокси. в списке прокси отмечен предпочитаемый, или всем выставлены сигнатуры (с наименьшей сигнатурой предпочитаемый)

--===============================================================================================================
-- Справочники

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
	PID		INTEGER,				-- если подразделение, то ссылка на предприятие (Units). если предприятие, то ссылка на компанию (Units)  .если компания, то ссылка на игрока (Person)
	Type		INTEGER NOT NULL,
	City		INTEGER,
	Name		TEXT,
	Company		INTEGER,
	Corporation	INTEGER,
	Image		TEXT
)

-- Архив данных по юнитам
CREATE TABLE UnitsInfo(
	UID		INTEGER NOT NULL
	TS		DATETIME,		-- Время, когда была сделана запись (реал)
	STS		BLOOB,			-- Время на сервере
	UnitData	BLOOB			-- Объектные данные
)


-- Архив данных по компании
CREATE TABLE CompanyInfo(
	СID		INTEGER,
	TS		DATETIME,		-- Время, когда была сделана запись (реал)
	STS		BLOOB,			-- Время на сервере
	UnitData	BLOOB			-- Объектные данные
)

-- Список заданий для сервиса
CREATE TABLE Schedule(
	ID		INTEGER PRIMARY KEY ASC NOT NULL,
	СID		INTEGER,
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

-- История курса ИО
CREATE TABLE ExchangeState(
	PurchasePrice	REAL,		-- закупочная цена, (по которой можно купить) (по которой биржа продает)
	SellingPrice	REAL,		-- продажная цена, (по которой можно продать) (по которой биржа покупает)
	TimeStamp	DATETIME,	-- метка времени данных
	ServerTime	BLOB,		-- исходная структура с данными времени сервера
	ServerTimeStr	VARCHAR (15)	-- строка отобража.щая время сервера
)

CREATE TABLE options(
	Server          TEXT,
	HLenQPExRate    INTEGER,
	HLenCompInfo    INTEGER
	
)

-- Таблица коэффициентов
CREATE TABLE Factors(
	Type		INTEGER NOT NULL,	-- тип коэффициент
	Object		INTEGER NOT NULL,	-- идентификатор объекта (например ID товара ,0xFFFF - все товары, 0xFENN NN- ID группы товара, 0xFDNN NN- ID массива для перечня товаров)
	LocationType	INTEGER NOT NULL,	-- где применять (1-магазин, 2-город, 3-регион, 4-страна, 5-для всей компании, 10-для группы магазинов из массива)
	Location	INTEGER NOT NULL,	-- идентификатор локации (ID магазина, или ID массива)
	int		INTEGER NOT NULL		-- величина коэффициента
)

--INSERT INTO Factors (Type,Object,LocationType,Location,Value) VALUES ();
--INSERT INTO Factors (Type,Object,LocationType,Location,Value) VALUES ();
--INSERT INTO Factors (Type,Object,LocationType,Location,Value) VALUES ();

-- Пояснение коэффициентов
CREATE TABLE FactorsTypeDescrioption(
	Type		INTEGER NOT NULL,	-- тип коэффициент
	Name		TEXT NOT NULL,		-- название коэффициента
	Unit		TEXT,			-- название единиц измерения
	Desription	TEXT,			-- описание коэффициента
	DesriptionExt	TEXT			-- описание коэффициента
)

INSERT INTO FactorsTypeDescrioption (Type,Name,Unit,Desription,DesriptionExt) VALUES(1,"Price","","","Цена продукта (товара)");
INSERT INTO FactorsTypeDescrioption (Type,Name,Unit,Desription,DesriptionExt) VALUES(2,"Price Dumping","%","Превышение над средней ценой","На сколько процентов больше нужно выставить цену продуктов при корректироваке цены (-99% .. +99%)");
INSERT INTO FactorsTypeDescrioption (Type,Name,Unit,Desription,DesriptionExt) VALUES(3,"Price Permitted Excess","%","Максимальное превышение цены над средней","На сколько процентов цена на продукт может превышать среднюю цену по городу (-99% .. +99%)");
INSERT INTO FactorsTypeDescrioption (Type,Name,Unit,Desription,DesriptionExt) VALUES(4,"Quality","","Качество","Необходимое качество продукта");
INSERT INTO FactorsTypeDescrioption (Type,Name,Unit,Desription,DesriptionExt) VALUES(5,"Quality Dumping","%","Превышение качества","На сколько процентов лучше по качеству продукты нужно пытаться закупать (-99% .. +99%)");
INSERT INTO FactorsTypeDescrioption (Type,Name,Unit,Desription,DesriptionExt) VALUES(6,"Quality Minimum","","Минимальное качество","Минимальное допустимое качество для магазина");
INSERT INTO FactorsTypeDescrioption (Type,Name,Unit,Desription,DesriptionExt) VALUES(7,"Quality Maximum","","Максимальное качество","Максимальное допустимое качество для магазина");
INSERT INTO FactorsTypeDescrioption (Type,Name,Unit,Desription,DesriptionExt) VALUES(8,"Quality Minimum Dumping","%","Минимальное занижение качества","Минимальное допустимое качество для магазина относительно среднего");
INSERT INTO FactorsTypeDescrioption (Type,Name,Unit,Desription,DesriptionExt) VALUES(9,"Quality Maximum Dumping","%","Максимальное завышение качества","Максимальное допустимое качество для магазина относительно среднего");



-- Таблица для хранения массивов
CREATE TABLE Arrays(
	ArrayID		INTEGER NOT NULL,	-- ID массива
	Key		INTEGER,		-- ключ элемента массива (если нужен)
	Value		REAL NOT NULL		-- значение элемента массива
)
-- Тип таблицы
CREATE TABLE ArrayType(
	ArrayID		INTEGER NOT NULL,	-- ID массива
	ArrayType	INTEGER NOT NULL,	-- тип массива
)

-- Конфигурационные параметры
CREATE TABLE Config(
	ID		INTEGER NOT NULL,	-- ID параметра
	KEY		TEXT NOT NULL,		-- текстовый ключ параметра
	Value		TAXT NOT NULL		-- значение параметра
}
INSERT INTO Config (ID,KEY,Value) VALUES(1,"UserTOR",1);		-- Использовать TOR proxy
INSERT INTO Config (ID,KEY,Value) VALUES(10,"ShopsAutoSupply",0);	-- Включать автоснабжение магазинов (на странице товаров) на N дней (0-выключено)
INSERT INTO Config (ID,KEY,Value) VALUES(2,"FileLogLevel","333");
INSERT INTO Config (ID,KEY,Value) VALUES(3,"DBLogLevel","000");
--INSERT INTO Config (ID,KEY,Value) VALUES(,"",);
--INSERT INTO Config (ID,KEY,Value) VALUES(,"",);
--INSERT INTO Config (ID,KEY,Value) VALUES(,"",);



