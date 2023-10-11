import websocket
import rel

# websocket.enableTrace(True)

class WebSocket:
    def __init__(self, url, **kwargs):
        self.ws = websocket.WebSocketApp(url, **kwargs)

    def run_forever(self):
        self.ws.run_forever(dispatcher=rel, reconnect=5)
        rel.signal(2, rel.abort) # Keyboard interrupt
        rel.dispatch()

