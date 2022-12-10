with source as (

    select * from {{ source('public', 'sensor_log') }}

),

renamed as (

    select
        power_timestamp,
        power_device_name,
        power_breaker_status,
        power_consumption,
        l1_phase_voltage,
        l2_phase_voltage,
        l3_phase_voltage,
        l1_phase_current,
        l2_phase_current,
        l3_phase_current,
        humidity,
        temperature

    from source

)

select * from renamed