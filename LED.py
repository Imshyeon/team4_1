#!/usr/bin/env python
import RPi.GPIO as GPIO
import time

def led(pin, t):
    GPIO.setmode(GPIO.BOARD)
    GPIO.setup(pin, GPIO.OUT)

    GPIO.output(pin, True)
    time.sleep(t) 

    GPIO.cleanup(pin)

led(18, 1) #