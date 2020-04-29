import cv2
import sys

def char_generator(message):
    for c in message:
        yield ord(c)

def gcd(x, y):
    while(y):
        x, y = y, x % y
    return x

def encode_image(image_location, msg):
    img = cv2.imread(image_location)
    msg_gen = char_generator(msg)
    pattern = gcd(len(img), len(img[0]))
    for i in range(len(img)):
        for j in range(len(img[0])):
            if (i+1 * j+1) % 2 == 0:
                try:
                    img[i-1][j-1][0] = next(msg_gen)
                except StopIteration:
                    img[i-1][j-1][0] = 0
                    cv2.imwrite("public/ENCODED.png", img);

if __name__ == "__main__":
    encode_image(sys.argv[1], sys.argv[2]);


