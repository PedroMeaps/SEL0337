import RPi.GPIO as GPIO
from time import sleep

GPIO.cleanup()
print("iniciado")
GPIO.setmode(GPIO.BCM)
ledPin = 12
GPIO.setup(ledPin, GPIO.OUT) #led

def main():

	tempo = 10
	GPIO.output(ledPin, 0)

	minutos, segundos = divmod(tempo, 60) #tempo/60

	try:
		while True:
			print("{:02d}:{:02d}".format(minutos, segundos), end='\r')
			sleep(1)
			segundos -= 1
			if(segundos == 0):
				minutos -= 1
				if(minutos < 0):
					break
				else:
					print("{:02d}:{:02d}".format(1, 0), end='\r')
					segundos=59
					sleep(1)
		GPIO.output(ledPin, True)
		print("led aceso")
		return
	except KeyboardInterrupt:
		GPIO.cleanup()
main()
