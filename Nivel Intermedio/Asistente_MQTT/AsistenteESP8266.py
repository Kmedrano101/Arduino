# -*- coding: utf-8 -*-
"""
Created on Wed Feb 24 18:01:13 2021

@author: Kevin Medrano Ayala
"""

# Importamos Librerias
import time
import paho.mqtt.client as mqtt
import serial, time
import keyboard

import speech_recognition as sr
from gtts import gTTS
import os

# Direccion IP del Broker MQTT
brokerIP ="192.168.1.200"
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    client.subscribe("$SYS/#")
def on_message(client, userdata, message):
    print(message.payload.decode("utf-8"))
    time.sleep(1)
    #print("received message =", str(message.payload.decode("utf-8")))

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect(brokerIP, 1883, 60)
#client.loop_forever() # En caso de no tener un blucle infinito

# Definir funcion de callback
print("Sistema conectadod!")
def grabarAudio():
    r = sr.Recognizer() # Crear objeto para reconocimiento
    # Abrir microfono y empezar a grabar
    with sr.Microphone() as source:
        print("###########Di algo###########")
        audio = r.listen(source)
    
    # Google reconocimiento
    data =''
    try:    
        data = r.recognize_google(audio,language="es-ES")
        print("Tu dices: "+ data)
    except sr.UnknownValueError:
        print("No se entiende el audio")
    except sr.RequestError as e:
        print("Resultado consulta ",e)
    return data

def respuestaAsistente(texto):
    global n_play
    myobj = gTTS(text=texto, lang="es", slow=False)
    myobj.save('asistenteRespuesta.mp3')
    os.system("asistenteRespuesta.mp3")

# Palabra de activacion 
def palabraActivacion(text):
    lista_palabras = ['asistente','hada','amor','hola']
    text = text.lower()
    
    for palabra in lista_palabras:
        if palabra in text:
            return True
    return False

def encender_apagar_Led(texto):
    
    lista = texto.split()
    for i in range(0,len(lista)):
        try:
            if (lista[i].lower() == 'encender' and lista[i+1].lower()=='luz'):
                # Enviar orden de encendido
                client.publish("casa/led1_active","1")
                respuesta = "Luz encendido"
                respuestaAsistente(respuesta)
        except:
            pass
        try:
            if (lista[i].lower() == 'apagar' and lista[i+1].lower()=='luz'):
                # Enviar orden de apagado
                client.publish("casa/led1_active","0")
                respuesta = "Luz apagado"
                respuestaAsistente(respuesta)
        except:
            pass

# Mensaje inicial
respuesta = "Hola he sido desarrollado por Kevin Medrano Ayala"
respuestaAsistente(respuesta)

while True:
    texto = grabarAudio()
    respuesta = ''        
    if(palabraActivacion(texto)== True):
        print("Activaste el asistente virtual")    
        if 'encender' or 'apagar'in texto:
            encender_apagar_Led(texto)
    if keyboard.is_pressed('esc'):
        break
    
client.disconnect() 
