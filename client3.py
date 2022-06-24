from calendar import c
from dis import code_info
import io
import math
from pydoc import cli
from secrets import choice
import sys
import os.path
import os
import socket



def sendimg(client, choice, image_path):
    print('Client sending')
    val = (choice).to_bytes(4, sys.byteorder)

    client.send(val)
    
    file = open(image_path, 'rb', buffering=1024)
    size = os.stat(image_path).st_size
    print('Image size: ' + str(size))
    file.seek(0, io.SEEK_END)

    file.seek(0, io.SEEK_SET)

    sizePic = (size).to_bytes(4, sys.byteorder)
    client.send(sizePic)

    print('Image size sended')
    
    print("Sending image")

    
    image_data = file.read(1024)

    while size > 0:
        client.send(image_data)
        image_data = file.read(1024)
        size -= 1024

    file.close()
    print('Client sended')



def receive(client, choice, image_name):
    img_name = 'Client/' + image_name + str(choice) + '.png'
    print('Receiving image size')
    size = client.recv(4)
    value = int.from_bytes(size, 'little')
    print('Image size: ' + str(value))

    file = open(img_name, 'wb')

    print('Receiving image')

    while value > 0:
        image_data = client.recv(1024)
        value = value - 1024
        file.write(image_data)

    file.close()
    print('Received image')



if __name__ == '__main__':

    if len(sys.argv) != 3:
        print('Arguments incorrect.')
        exit()

    image_path = sys.argv[1]
    image_name = sys.argv[2]

    if os.path.exists(image_path) == False:
        print('Image file do not exist.')
        exit()

    while True:
        client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client.connect(('127.0.0.1', 5001))

        print('connected to the server..')
        print('')
        print('Choose the transformation typing the number:')
        print('1 - to transform to grayscale;')
        print('2 - to apply gaussian blur;')
        print('3 - to rotate 90 degrees;')
        print('4 - to rotate 180 degrees;')
        print('5 - to rotate 270 degrees;')
        print('Any key - to exit')
		   
        choice = int(input('Choice: '))
        
        if choice >= 1 and choice <= 5:
            sendimg(client, choice, image_path)
            receive(client, choice, image_name)
        else:
            exit()

        client.close()