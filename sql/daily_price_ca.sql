create view invest.nzx.price_daily
            WITH (timescaledb.continuous,
            timescaledb.refresh_lag = '-40m',
            timescaledb.refresh_interval = '30m'
            )
AS
SELECT time_bucket('1hour', invest.nzx.prices.time) as bucket,
       code,
       trunc(cast(avg(price) as numeric), 4)        as price_avg,
       max(price)                                   as price_max,
       min(price)                                   as price_min
FROM invest.nzx.prices
GROUP BY bucket, code;