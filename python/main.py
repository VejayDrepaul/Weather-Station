import utime
from machine import Pin, ADC, I2C
from gpio_lcd import GpioLcd
from bmp280 import *

lcd = GpioLcd(rs_pin=Pin(16),
              enable_pin=Pin(17),
              d4_pin=Pin(18),
              d5_pin=Pin(19),
              d6_pin=Pin(20),
              d7_pin=Pin(21),
              num_lines=2, num_columns=16)

LM_35 = ADC(4)
bus = I2C(0, scl=Pin(1), sda=Pin(0), freq=200000)
BMP_280 = BMP280(bus)
BMP_280.use_case(BMP280_CASE_INDOOR)

g_led = Pin(5, Pin.OUT)
r_led = Pin(4, Pin.OUT)
g_led.value(0)
r_led.value(0)

def lm_35_temp():
    raw_adc = LM_35.read_u16()
    temp_c = (raw_adc * 3.3 / 65535 - 0.5) * 100  #convert the raw ADC value to temperature in Celsius

    return round(temp_c, 2)


def bmp_reading():
    temperature = BMP_280.temperature
    pressure = BMP_280.pressure
    p_hpa = pressure / 100
    #p_bar = pressure / 100000
    #p_mmHg = pressure / 133.3224

    return p_hpa


def altitude_calculation(atom_pressure):
    sea_level_pressure = 1013.25 
    pressure_ratio = atom_pressure / sea_level_pressure
    alt = 44330 * (1 - (pressure_ratio**(1 / 5.255)))

    return alt


while True:
    temp = lm_35_temp()
    pressure = bmp_reading()
    alt = altitude_calculation(pressure)

    if temp >= 25:
        g_led.value(0)
        r_led.value(1)
    else:
        r_led.value(0)
        g_led.value(1)
    
    print(f"Temperature: {temp} C")
    print(f"Pressure: {pressure} hPa")
    print(f"Altitude: {alt} m")
    lcd.clear()
    lcd.putstr(f"Temp: {round(temp, 2)} C")
    lcd.putstr(f"\nPa: {round(pressure, 2)} hPa")
    utime.sleep(10)
    lcd.clear()
    lcd.putstr(f"Alt: {round(alt, 2)} m")
    utime.sleep(5)