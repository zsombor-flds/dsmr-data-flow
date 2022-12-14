{{ config(
	materialized='incremental',
    indexes=[
      {'columns': ['period_ts'], 'unique': true},
    ],
    unique_key = 'period_ts',
	incremental_strategy='delete+insert'

)}}


with base as (

  select * from {{ ref('stg_dsmr__measure') }}

),

final as (
  select
    date_trunc('hour', measure_timestamp::timestamp) as period_ts,
    round(min(power_consumption_khw), 2) as period_start_power_consumption_khw,
    round(max(power_consumption_khw), 2) as period_end_power_consumption_khw,
    round(
      max(power_consumption_khw) - min(power_consumption_khw), 2
    ) as period_power_consumption_khw,
    round(avg(outside_temperature), 2) as period_avg_temperature
  from base
  group by 1
)

select *
from final
{% if is_incremental() %}
  where period_ts >= (select max(period_ts) from {{ this }})
{% endif %}