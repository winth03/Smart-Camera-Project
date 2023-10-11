from cvzone.HandTrackingModule import HandDetector
from mywebsocket import WebSocket
from threading import Thread
from websocket import ABNF
from queue import Queue
import numpy as np
import cv2

def findHand():
    while True:
        try:
            # Get image from queue
            img = imgQueue.get()

            # Check if image is None
            if img is None:
                continue

            # Find hand
            hands, frame = detector.findHands(img)

            if hands:
                hand1 = hands[0]
                fingers = detector.fingersUp(hand1)
                fingers = fingers.count(1)
                fingerSocket.ws.send(fingers, ABNF.OPCODE_BINARY)

            # cv2.imshow("Image", frame)
            newImg = cv2.imencode(".jpg", frame)[1]
            imageSocket.ws.send(newImg, ABNF.OPCODE_BINARY)
            if (cv2.waitKey(1) & 0xFF) == ord('q'):
                break
        except:
            pass
        

def on_data(ws, data, data_type, continue_flag):
    # print("Received data")
    nparr = np.frombuffer(data, np.uint8)
    img = cv2.imdecode(nparr, cv2.IMREAD_COLOR)

    imgQueue.put_nowait(img)

def on_error(ws, error):
    print(error)

url = "ws://localhost:1880" # URL of Node-RED server
imageSocket = WebSocket(f"{url}/image", on_error=on_error, on_data=on_data)
fingerSocket = WebSocket(f"{url}/opencv", on_error=on_error, on_data=on_data)
detector = HandDetector(detectionCon=0.5, maxHands=1)
imgQueue = Queue()

if __name__ == "__main__":
    t1 = Thread(target=findHand)
    t1.daemon = True
    t1.start()
    imageSocket.run_forever()
    cv2.destroyAllWindows()
