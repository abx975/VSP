Start:

Navigate in shell to project folder „vspDREI“.

Use startscript for example:

./startStations.sh enp0s25 225.10.1.2 15000 1 2 A 3


Start Sniffer:
./sniffer/64bit/STDMAsniffer 225.10.1.2 15000 -adapt


End:

pkill -f out/artifacts/vspDREI_jar/vspDREI.jar
pkill -f java
