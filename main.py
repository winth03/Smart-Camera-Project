from websockets.server import WebSocketServerProtocol, serve
from cvzone.HandTrackingModule import HandDetector
from threading import Thread
from queue import Queue
import numpy as np
import asyncio
import base64
import cv2
import json


def findHand():
    while True:
        try:
            # Get image from queue
            img = imgQueue.get()

            # Check if image is None
            if img is None:
                continue

            # Decode image
            nparr = np.frombuffer(img, np.uint8)
            img = cv2.imdecode(nparr, cv2.IMREAD_COLOR)

            # Find hand
            hands, frame = detector.findHands(img)

            if hands:
                hand1 = hands[0]
                fingers = detector.fingersUp(hand1)
                fingers = fingers.count(1)
            else:
                fingers = -1

            # cv2.imshow("Image", frame)
            newImg = cv2.imencode(".jpg", frame)[1]
            newImg = base64.b64encode(newImg)
            newImg = newImg.decode("utf-8")

            # Send json data to Node-RED server
            jsonData = json.dumps({"image": newImg, "fingers": fingers})
            print(jsonData)

            if (cv2.waitKey(1) & 0xFF) == ord("q"):
                break
        except:
            pass


async def handler(ws: WebSocketServerProtocol):
    async for data in ws:
        if type(data) == bytes:
            imgQueue.put_nowait(data)


detector = HandDetector(detectionCon=0.5, maxHands=1)
imgQueue = Queue()


async def main():
    async with serve(handler, port=8000):
        await asyncio.Future()  # run forever


if __name__ == "__main__":
    t1 = Thread(target=findHand)
    t1.daemon = True
    t1.start()
    asyncio.run(main())
    cv2.destroyAllWindows()
