import serial
import threading

#connecta först bluetooth i windows och sedan måste man kolla i device manager -> ports och hitta vilka som dyker upp. 
PORT_OUT = "COM11" 
PORT_IN = "COM12"
BAUD_RATE = 115200

ser_out = serial.Serial(PORT_OUT, BAUD_RATE, timeout=1)
ser_in = serial.Serial(PORT_IN, BAUD_RATE, timeout=1)

def read_from_port(ser, label):
    while True:
        if ser.in_waiting:
            data = ser.readline().decode(errors='ignore').strip()
            if data:
                print(f"[{label}] {data}")
threading.Thread(target=read_from_port, args=(ser_out, "OUT"), daemon=True).start()
threading.Thread(target=read_from_port, args=(ser_in, "IN"), daemon=True).start()


ser_out.write(b"samst lol\n")

try:
    while True:
        pass
except KeyboardInterrupt:
    ser_out.close()
    ser_in.close()
    print("ejd.")

