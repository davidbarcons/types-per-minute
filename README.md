# types-per-minute

Displays a text in a variable font (.ttf) which depends on 3 parameters:
- Heart rate
- Oxygen saturation
- Body temperature

These vital signs are collected with an arduino equipped with a MAX30102 sensor. We have written an arduino code "tpm_max30102.ino" to collect them.

We make use of a python server "server.py" to read the vital signs from the arduino, processes them 
and print them on a simple web template 'index.html'.

List of requirements:
- Arduino board (tested with Arduino UNO) and MAX30102 sensor (see https://lastminuteengineers.com/max30102-pulse-oximeter-heart-rate-sensor-arduino-tutorial/?utm_content=cmp-true)
- Python 3 with the packages: numpy, flask, flask_socketio, serial.
- Arduino IDE or equivalent.
- A web browser.
