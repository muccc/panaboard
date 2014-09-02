from PIL import Image
import PIL.ImageOps 
import serial
ser = serial.Serial('/dev/ttyACM1', 500000, timeout=100)
ser.flushOutput()
ser.flushInput()
x = ser.read(288720)
ser.close()
im = Image.frombuffer("1", (1440, 1604), x, 'raw', "1", 0, 1)
im = PIL.ImageOps.invert(PIL.ImageOps.grayscale(im))
im.save("test.png", "PNG")
