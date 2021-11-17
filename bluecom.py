import os
import select
import bluetooth
import subprocess
class BluetoothComm:
    def __init__(self):
        self.server_socket=bluetooth.BluetoothSocket( bluetooth.RFCOMM )
        port = 1
        self.server_socket.bind(("",port))
        self.server_socket.listen(1)
        self.client_socket,address = self.server_socket.accept()
        print("Accepted connection from ",address)
    def read_comm(self):
        res = self.client_socket.recv(10)
        if len(res):
            return res
        else:
            return None
def main():
    blue_comm = BluetoothComm()
    while True:
        get = blue_comm.read_comm()
        f = open('modefile', 'wb') 
        f.write(get)
        f.close()
        f = open('modefile', 'a')
        f.write('\n\r')
        f.close()
main()

