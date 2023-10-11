import cv2
from cvzone.HandTrackingModule import HandDetector
from websocket import ABNF
from mywebsocket import WebSocket
import numpy as np
from threading import Thread
from queue import Queue

def findHand():
    while True:
        try:
            # Get image from queue
            img = findHandQueue.get()

            # Check if image is None
            if img is None:
                continue

            # Find hand
            hands, frame = detector.findHands(img)

            # if hands:
            #     hand1 = hands[0]
            #     fingers = detector.fingersUp(hand1)
                # myWebSocket.ws.send(fingers, ABNF.OPCODE_BINARY)

            # Flip image
            # frame = cv2.flip(frame, 1)

            # cv2.imshow("Image", frame)
            newImg = cv2.imencode(".jpg", frame)[1]
            myWebSocket.ws.send(newImg, ABNF.OPCODE_BINARY)
            if (cv2.waitKey(1) & 0xFF) == ord('q'):
                break
        except:
            pass
        

def on_data(ws, data, data_type, continue_flag):
    # print("Received data")
    nparr = np.frombuffer(data, np.uint8)
    img = cv2.imdecode(nparr, cv2.IMREAD_COLOR)

    findHandQueue.put_nowait(img)

def on_error(ws, error):
    print(error)

url = "ws://localhost:1880/opencv" # URL of Node-RED server
myWebSocket = WebSocket(url, on_error=on_error, on_data=on_data)
detector = HandDetector(detectionCon=0.5, maxHands=1)
findHandQueue = Queue()

if __name__ == "__main__":
    t1 = Thread(target=findHand)
    t1.daemon = True
    t1.start()
    myWebSocket.run_forever()
    cv2.destroyAllWindows()
