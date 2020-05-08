# This is a pretty quick and simple program, but it just goes to show how shockingly easy it is to spoof email addresses using SMTP.
# This is set up to use UVic's mailserver, but can be pretty easily altered to use some other server, and send to any email address.
# 
# Written by Graeme Prendergast, Spring 2019

from socket import *

sender = "spoof@uvic.ca"
recipient = " ??? "
msg = "\r\n Lorem ipsum dolor sit amet, lorem aeterno no sea, tritani vivendo signiferumque id ius. Vero laudem percipit ad mei. Ut sed quot audiam epicurei. Quo consul sanctus ad."
endmsg = "\r\n.\r\n"

mailServer = "smtp.uvic.ca"
port = 25

clientSocket = socket(AF_INET, SOCK_STREAM)
clientSocket.connect((mailServer, port))

recv = clientSocket.recv(1024)
print(recv)
if recv[:3] != '220':
	print('220 reply not received from server.')

heloCommand = 'HELO spoof \r\n'
clientSocket.send(heloCommand)
recv1 = clientSocket.recv(1024)
print(recv1)
if recv1[:3] != '250':
    print('250 reply not received from server.')

clientSocket.send("MAIL FROM: %s\n" % sender )
print( clientSocket.recv(1024) )

clientSocket.send("RCPT TO: %s\n" % recipient )
print( clientSocket.recv(1024) )

clientSocket.send("DATA\n")
print( clientSocket.recv(1024) )

clientSocket.send(msg)
clientSocket.send(endmsg)

clientSocket.send("QUIT")
print( clientSocket.recv(1024) )
