The UDP proxy server with a domains blacklist feature to filter unwanted host names resolving.


This UDP proxyserver is a crossplatform application (at least it works on Windows10 and Ubuntu).


The proxy server reads its parameters during startup from the configuration file.


The configuration file contains the following parameters:

- IP address and port of upstream UDP server

- List of domain names to filter resolving ("blacklist")

- Type of UDP proxy server's response for blacklisted domains.


If a domain name from a client's request is not found in the blacklist, then the proxy server forwards the request to the upstream server, 
waits for a response, and sends it back to the client.


If a domain name from a client's request is found in the blacklist, then the proxy server responds with a response defined in the configuration file.


Programming language: C.


I also used here external open-source library cJSON (the Licence and the Link are in the "cjson" folder)


To build and run proxyserver - use one of the two instructions:

1. from the file location folder using any command line run

clang DNS_proxy_cerver_2.c   (clang compiller is required!)

./a  (to run in Windows)

./a.out (to run in Linux)

2. from the file location folder using any command line run

gcc DNS_proxy_cerver_2.c     (gcc compiller is required!)

./a  (to run in Windows)

./a.out (to run in Linux)


To configure proxysercver - change data inside "data.json" config file (you may change UPPSTREAM Server IP or port, blacklist IPs and response for blacklist IPs)

IMPORTANT!!! In case if changes UPPSTREAM port - it is necessary to change the port in the file TEST_ServUDP.c!!! (Othervise testing using TEST_ServUDP.c will be useless)


The proxyserver testing was done on "localhost" using main proxyserver and two additional servers: TEST_ServUDP.c (as a UPSTREAM Server) and TEST_ClientUDP.c (as a Client)


To test proxyserver:

1. run the compilled from DNS_proxy_cerver_2.c exe-file or out-file

2. run the compilled from TEST_ServUDP.c (upstream server) exe-file or out-file

3. run the compilled from TEST_ClientUDP.c (client) exe-file or out-file

On the 3-rd step Client will send though proxyserver to upstream server and the same way will receive response from upstream server, if client IP not in the black list, 

othervise - client will receive the error-message from the proxy server and upstream server will receive nothing.



