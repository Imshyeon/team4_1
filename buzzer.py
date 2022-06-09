#!/usr/bin/env python
import RPi.GPIO as GPIO
import time

buzzer = 18
GPIO.setmode(GPIO.BCM)
GPIO.setup(buzzer, GPIO.OUT)
GPIO.setwarnings(False)

pwm = GPIO.PWM(buzzer, 51) # 262
pwm.start(50.0)
time.sleep(0.5)

pwm.stop()
GPIO.cleanup()