from flask import Flask, render_template
from flask_socketio import SocketIO, emit
import serial
import numpy as np

########################################################
hrat_limits = [40, 120]
osat_limits = [90, 100]
temp_limits = [28, 36]
font_limits = [100,600]
#################################################

hrat_range = np.linspace(hrat_limits[0],hrat_limits[1],100)
osat_range = np.linspace(osat_limits[0],osat_limits[1],100)
temp_range = np.linspace(temp_limits[0],temp_limits[1],100)
font_range = np.linspace(font_limits[0],font_limits[1],100)


app = Flask(__name__)
app.config['SECRET_KEY'] = 'secret_key'
socketio = SocketIO(app)

arduino = serial.Serial()
arduino.baudrate = 115200
arduino.port = 'COM3'

arduino.open()


            
@app.route('/')
def index():
    return render_template('index.html')

@socketio.on('connect')
def on_connect():
    print('Client connected')

@socketio.on('disconnect')
def on_disconnect():
    print('Client disconnected') 

def update_value():
    while True:
        if arduino.in_waiting: # Only runs if there a new value in the buffer
            rawString = arduino.readline().decode().strip() # Decode the string
            if rawString[0] not in ['I','M','P']: # Avoid other messages in the arduino code
                rawdata = rawString.split(";")
                rawdata = [float(d) for d in rawdata]
                # Interpolate data to font range parameters
                data = []
                data.append(np.interp(rawdata[0], hrat_range, font_range))
                data.append(np.interp(rawdata[1], osat_range, np.flipud(font_range)))
                data.append(np.interp(rawdata[2], temp_range, font_range))
            
                #socketio.emit('update', {'rawdata': rawdata, 'data': data}, namespace='/')
                socketio.emit('update', [rawdata, data], namespace='/')


if __name__ == '__main__':
    socketio.start_background_task(update_value)
    socketio.run(app, allow_unsafe_werkzeug=True)
