#include <iostream>
#include <vector>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include "Arinc429.h"

#define SERVER_BUFFER_SIZE 512
#define XPLANE_ADDRES "127.0.0.1"
#define PORT 49001
#define COM "\\\\.\\COM5"

int main()
{
    //-------Socket setup---------
    WSAData wsaData;
    int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (res != NO_ERROR) {
        std::cout << "WSAStartup failed with error " << res << std::endl;
        return 1;
    }

    SOCKET serverSocket = INVALID_SOCKET;
    serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (serverSocket == INVALID_SOCKET) {
        std::cout << "socket failed with error " << WSAGetLastError() << std::endl;
        return 1;
    }

    sockaddr_in serverAddr;
    sockaddr_in clientAddr;
    short port = 49001;
    int senderAddrSize = sizeof(clientAddr);

    serverAddr.sin_family = AF_INET;
    inet_pton(AF_INET, XPLANE_ADDRES, &serverAddr.sin_addr.s_addr);
    serverAddr.sin_port = htons(PORT);
    if (bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr))) {
        std::cout << "bind failed with error " << WSAGetLastError() << std::endl;
        return 1;
    }

    //-----------Serial port setup-----------
    HANDLE hComm;

    hComm = CreateFileA(COM,
        GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    if (hComm == INVALID_HANDLE_VALUE) {
        std::cout << "Error in opening serial port." << std::endl;
        return -1;
    }
    else {
        std::cout << "Serial port is opened." << std::endl;
    }

    //------Networking------
    int bytes_recived;
    char serverBuff[SERVER_BUFFER_SIZE];
    char serverBuffTrim[SERVER_BUFFER_SIZE];
    float xPlane12Data[SERVER_BUFFER_SIZE / sizeof(float)];
    std::vector<uint32_t> Arinc429Words;

    std::vector<bool> options(13);
    //user settings
    {
        system("cls");
        const int xPlaneDataOutputIndexes[] = { 3, 4, 18, 17, 20, 167, 16, 151, 44, 45, 47, 49, 50 };
        const std::string optionsText[] = { "Speeds (computed, equivalent, true air, true ground)                   xPlane OD Index: ",
                                            "Mach, Inertial Vertical Velocity                                       xPlane OD Index: ",
                                            "Angle of Attack (AOA), Angle of Slip (AOS)                             xPlane OD Index: ",
                                            "Pitch and roll angles, true and magnetic heading                       xPlane OD Index: ",
                                            "Latitude, longitude, altitude                                          xPlane OD Index: ",
                                            "Body pitch, roll and yaw acceleration                                  xPlane OD Index: ",
                                            "Body pitch, roll and yaw rate                                          xPlane OD Index: ",
                                            "Static pressure, static and total air temperature, air density ration  xPlane OD Index: ",
                                            "Engine pressure ratio                                                  xPlane OD Index: ",
                                            "Fuel flow                                                              xPlane OD Index: ",
                                            "Exhaust gas Temperature                                                xPlane OD Index: ",
                                            "Engine oil pressure                                                    xPlane OD Index: ",
                                            "Engine oil temperature                                                 xPlane OD Index: " };
        std::string input;

        for (int i = 0; i < 13; i++) {
            std::cout << "Choose xPlane12 output data to recive:" << std::endl;
            std::cout << optionsText[i] << xPlaneDataOutputIndexes[i] << "\n(y/n): ";
            std::cin >> input;

            while (input != "y" && input != "n") {
                std::cout << "Invalid input. Please enter 'y' or 'n': ";
                std::cin >> input;
            }

            options[i] = (input == "y");

            system("cls");
        }

        std::cout << "xPlane12 output data selected, make sure ONLY following output data indexes are selected:" << std::endl;
        for (int i = 0; i < 13; i++) {
            if (options[i]) {
                std::cout << xPlaneDataOutputIndexes[i] << "\t";
            }
        }
        std::cout << std::endl;
    }

    std::cout << std::endl;
    std::cout << "Reciving datagrams on " << XPLANE_ADDRES << ":" << PORT << std::endl;


    while (true) {
        // recive data from xplane
        bytes_recived = recvfrom(serverSocket, serverBuff, SERVER_BUFFER_SIZE, 0, (SOCKADDR*)&clientAddr, &senderAddrSize);
        if (bytes_recived == SOCKET_ERROR) {
            std::cout << "recvfrom failed with error " << WSAGetLastError() << std::endl;
        }

        // Convert data to float
        int numFloats = (bytes_recived - 9) / 4; //calculate amount of variables sent by xplane
        std::copy(serverBuff + 9, std::end(serverBuff), std::begin(serverBuffTrim));
        for (int i = 0; i < numFloats; ++i) {
            std::memcpy(&xPlane12Data[i], &serverBuffTrim[i * sizeof(float)], sizeof(float));
        }

        // convert float data to Arinc429 words
        xPlane12UDPtoArinc429(xPlane12Data, options, Arinc429Words);

        // send Arinc429 words over COM to client
        for (int j = 0; j < Arinc429Words.size(); j++) {
            //std::cout << Arinc429Words[j] << std::endl;
            WriteFile(hComm, &Arinc429Words[j], 4, NULL, NULL);
        }

    }

    // Serial port cleanup
    CloseHandle(hComm);

    // Socket cleanup
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}