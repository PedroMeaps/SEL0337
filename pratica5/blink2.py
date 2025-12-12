import RPi.GPIO as GPIO
from time import sleep
GPIO.setmode(GPIO.BCM)
GPIO.setup(18, GPIO.OUT)

while True:
	GPIO.output(18, True)
	print("LED on")
	sleep(6)
	GPIO.output(18, False)
	print("LED off")
	sleep(0.5) # 0.5s
