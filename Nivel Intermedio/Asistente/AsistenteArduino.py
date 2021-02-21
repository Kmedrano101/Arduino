# -*- coding: utf-8 -*-
"""
Created on Fri Feb 19 11:36:35 2021

@author: Kevin Medrano Ayala
"""

# Comunicacion con Arduino con Asistente Personal

# Importamos Librerias
import serial, time
import serial.tools.list_ports
import json
import keyboard

import speech_recognition as sr
from gtts import gTTS
import os

#Mostrar lista de puertos
ports = serial.tools.list_ports.comports()
for port, desc, hwid in sorted(ports):
    print("{}: {} [{}]".format(port, desc, hwid))

# Definimos el objeto para comunicar con arduino
arduino = serial.Serial('COM8', 9600)
time.sleep(2)

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
        print("Resultado consulta "+e)
    return data

def respuestaAsistente(texto):
    #print(texto)
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
        if (lista[i].lower() == 'encender' and lista[i+1].lower()=='led'):
            # Enviar orden de encendido
            arduino.write('{"Led": 1}'.encode('ascii'))
            respuesta = "Led encendido"
            respuestaAsistente(respuesta)
        if (lista[i].lower() == 'apagar' and lista[i+1].lower()=='led'):
            # Enviar orden de apagado
            arduino.write('{"Led": 0}'.encode('ascii'))
            respuesta = "Led apagado"
            respuestaAsistente(respuesta)
       
def obtener_temperatura(texto):
    global _temperatura
    lista = texto.split() # cual es la temperatura
    for i in range(0,len(lista)):
        if(lista[i].lower() == 'cuál' and lista[i+1].lower()=='es' and lista[i+3].lower()=='temperatura'):
            # Enviar orden de encendido
            respuesta = "La temperatura del ambiente es de "+str(_temperatura)+" grados centrigrados"
            respuestaAsistente(respuesta)
        if(lista[i].lower() == 'dime' and lista[i+1].lower()=='la' and lista[i+2].lower()=='temperatura'):
            # Enviar orden de encendido
            respuesta = "La temperatura del ambiente es de "+str(_temperatura)+" grados centrigrados"
            respuestaAsistente(respuesta)
        
_estadoLed = 0
_temperatura = 0

arduino.reset_input_buffer()

while True:
    
    texto = grabarAudio()
    respuesta = ''        
    if(palabraActivacion(texto)== True):
        print("Activaste el asistente virtual")    
        if 'encender' or 'apagar'in texto:
            encender_apagar_Led(texto)
        if 'cuál' or 'dime'in texto:
            obtener_temperatura(texto)
    # Recepcion de datos desde Arduino
    datos = arduino.readline()
    data_json = json.loads(datos)
    print(datos)
    #print("Estado LED: ",data_json['Led'])
    _estadoLed = data_json['Led']
    _temperatura = data_json['temperatura']
    time.sleep(.001)
    arduino.reset_input_buffer()
    if keyboard.is_pressed('esc'):
        break

arduino.close()