
ATTACH 'd:\GIT\BIZ_Client_V2\Debug\bizdb_home.sdb' AS zB2;

DROP TABLE IF EXISTS tmp1;

CREATE TEMPORARY TABLE IF NOT EXISTS tmp1 AS SELECT * FROM main.ExchangeState LIMIT 1;

DELETE FROM tmp1;

INSERT INTO tmp1 (PurchasePrice, SellingPrice, TimeStamp, ServerTime, ServerTimeStr) 
SELECT PurchasePrice, SellingPrice, TimeStamp, ServerTime, ServerTimeStr FROM zB2.ExchangeState 
WHERE TimeStamp NOT IN (SELECT TimeStamp FROM main.ExchangeState);

INSERT INTO tmp1 (PurchasePrice, SellingPrice, TimeStamp, ServerTime, ServerTimeStr) 
SELECT PurchasePrice, SellingPrice, TimeStamp, ServerTime, ServerTimeStr FROM main.ExchangeState;

DELETE FROM main.ExchangeState;

INSERT INTO main.ExchangeState (PurchasePrice, SellingPrice, TimeStamp, ServerTime, ServerTimeStr) 
SELECT PurchasePrice, SellingPrice, TimeStamp, ServerTime, ServerTimeStr FROM tmp1
ORDER BY TimeStamp;

DROP TABLE IF EXISTS tmp1;
DETACH zB2;