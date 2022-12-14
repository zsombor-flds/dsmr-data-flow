with source as (

  select * from {{ source('public', 'sensor_log') }}

),

renamed as (

  select
    cast(power_timestamp as timestamp) as measure_timestamp,
    power_device_name,
    cast(substring(power_consumption, 0, 11) as numeric) as power_consumption_khw,
    cast(substring(l1_phase_voltage, 0, 6) as numeric) as l1_phase_voltage,
    cast(substring(l2_phase_voltage, 0, 6) as numeric) as l2_phase_voltage,
    cast(substring(l3_phase_voltage, 0, 6) as numeric) as l3_phase_voltage,
    cast(substring(l1_phase_current, 0, 4) as numeric) as l1_phase_current,
    cast(substring(l2_phase_current, 0, 4) as numeric) as l2_phase_current,
    cast(substring(l3_phase_current, 0, 4) as numeric) as l3_phase_current,
    cast(humidity as numeric) as outside_humidity,
    cast(temperature as numeric) as outside_temperature
  from source

)

select * from renamed
