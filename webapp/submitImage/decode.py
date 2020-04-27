import cv2
import sys
import os

def char_generator(message):
    for c in message:
        yield ord(c)

def get_image(image_location):
    img = cv2.imread(image_location)
    return img

def gcd(x, y):
    while(y):
        x, y = y, x % y
    return x

def decode_image(img_location):
    img = cv2.imread(img_location)
    pattern = gcd(len(img), len(img[0]))
    message = ''
    for i in range(len(img)):
        for j in range(len(img[0])):
            if (i+1 * j+1) % pattern == 0:
                if img[i-1][j-1][0] != 0:
                    message = message + chr(img[i-1][j-1][0])
                else:
                    return message

if __name__ == "__main__":
    msg = decode_image(sys.argv[1]);
    print(msg);




