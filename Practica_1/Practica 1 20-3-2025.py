from machine import Pin
from time import sleep
#from dht import DHT22
from network import WLAN, STA_IF
from urequests import post

#dht_sensor = DHT22(Pin(15))

WEBAPP_URL = "https://script.google.com/macros/s/AKfycbx-JkhUm6-7CB9sH6Vua2YnlIItyjaMEPocYyWEfhlz3U5S-F9IyZktCa7sKS6ClCn_/exec"

def connect_wifi():
    sta_if = WLAN(STA_IF)
    sta_if.active(True)
    sta_if.connect("Cesar", "123456789")
    while not sta_if.isconnected():
        print(".", end = "")
        sleep(0.3)
    print("Wifi ok")

def send_to_sheets():
    data = {
        "temp" : 50,
        "hum" : "Datos random2"
    }

    try:
        response = post(WEBAPP_URL, json = data)
        print("Respuesta: ", response.text)
        response.close()
    except Exception as e:
        print("Error al enviar a la hojoa de calculo", e)

connect_wifi()
while True:
    try:
        #dht_sensor.measure()
        #temp = dht_sensor.temperature()
        #hum = dht_sensor.humidity()
        #print(f"Temperatura: {temp:.1f}°C")
        #print(f"Humedad: {hum:.1f}%")
        print("-"*20)
        send_to_sheets()
        
    except Exception as e:
        print("Error al leer el sensor", e)
    sleep(2)