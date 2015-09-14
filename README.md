# BACnet-Galileo
EasyPHY Reference stack for Intel Galileo development board

Contains Eclipse Workspace set up for Intel's IoT Development Kit IDE 
 
Steps to get to a running BACnet stack on the Galileo. Assuming windows dev platform, you are connected to Galileo via Ethernet:

1. Clone this repository on your machine: git clone https://github.com/EasyPHY/BACnet-Galileo.git c:\temp\bacnet
2. Start Intelis IoT Development IDE from its batch file
3. Switch workspace to the above: c:\temp\bacnet
4. Import the project that is in that folder
5. Compile
6. Set up a IP (SSH) debug connection under run->debug configuration, then 'Setup Connection'
7. Run Eclipse debugger !
8. Browse to the BACnet Stack over the network connection using a BACnet Client

Questions: info@connect-ex.com


