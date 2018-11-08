
SELECT TimeStamp, PurchasePrice, SellingPrice, ServerTimeStr,ServerTime FROM
(
SELECT TimeStamp, ServerTime, ServerTimeStr,
PurchasePrice, lag(PurchasePrice) OVER win as pPP, lead(PurchasePrice) OVER win as nPP,
SellingPrice, lag(SellingPrice) OVER win as pSP, lead(SellingPrice) OVER win as nSP
FROM ExchangeState
WINDOW win AS (ORDER BY TimeStamp)
)
where NOT(PurchasePrice=pPP and PurchasePrice=nPP and SellingPrice=pSP and SellingPrice=pSP);
