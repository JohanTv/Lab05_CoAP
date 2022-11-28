from flask import Flask, render_template, session
from aiocoap import *
from flask_socketio import SocketIO, emit
import asyncio
from threading import Lock

async_mode = None
app = Flask(__name__)
socketio = SocketIO(app, async_mode=async_mode)
thread = None
thread_lock = Lock()

def observe_callback(response):
    if response.code.is_successful():
        distance = response.payload.decode('ascii')
        socketio.emit('show_received_distance',
                      {'data': distance})
    else:
        socketio.emit('show_received_distance',
                      {'data': "ERROR"})

async def observe():
    global session
    context = await Context.create_client_context()
    request = Message(code=GET)
    request.set_request_uri('coap://192.168.100.6/alarm')
    request.opt.observe = 0
    observation_is_over = asyncio.Future()
    try:
        context_request = context.request(request)
        context_request.observation.register_callback(observe_callback)
        response = await context_request.response
        exit_reason = await observation_is_over
        print('Observation is over: %r' % exit_reason)
    finally:
        if not context_request.response.done():
            context_request.response.cancel()
        if not context_request.observation.cancelled:
            context_request.observation.cancel()

def observer():
    asyncio.run(observe())

@app.route("/")
def index():
    return render_template("index.html")

@socketio.event
def beginning(message):
    emit('show_received_distance',
         {'data': message['data']})

@socketio.event
def connect():
    global thread
    with thread_lock:
        if thread is None:
            thread = socketio.start_background_task(observer)
            
if __name__ == '__main__':
    socketio.run(app, port=8080, debug=False)
