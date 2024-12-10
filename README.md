# CS224_ARQ

This repository contains the code for the project "Automatic Repeat Request on top of UDP Sockets" done during the course CS224 at IITB.

-> Compiling the code:
1) Open two terminal windows in the directory containing the program files
2) Run "gcc sender.c -o sender -Wall" command (without quotes) in the first terminal. 
3) Run "gcc receiver.c -o receiver -Wall" command (without quotes) in the second terminal.

-> Executing the code:
1) Run "sudo tc qdisk add dev lo root netem delay <Delay_in_milliseconds>" in any terminal. Give integer value to Delay_in_milliseconds.
2) Run "./receiver <ReceiverPort> <SenderPort> <PacketDropProbability>" command (without quotes) in the second terminal, where PacketDropProbability is float value in [0,1]. Give 49513, 49512 to ReceiverPort, SenderPort respectively (or any higher 16 bit value).
3) Run "./sender <SenderPort> <ReceiverPort> <RetransmissionTimer> <NoOfPacketsToBeSent>" command (without quotes) in first terminal, where Retranmission timer is float value in seconds, and NoPacketsToBeSent is an integer.
4) Run "sudo tc qdisk del dev lo root netem delay <Delay_in_milliseconds>" in any terminal. Give the same integer value as used earlier for Delay_in_milliseconds.

-> Code description:
Commented properly in the code

-> Note:
1) RetransmissionTimer must be smaller than SLEEP_TIMER (defined to be 10 seconds in receiver.c) otherwise receiver program will terminate while sender program waits for Acknowledgment.
2) References are written in each C file at the end.
