// WinSocketClient.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include "exceptions/WSAException.h"
#include "log/Log.h"
#include "utils/SocketUtils.h"
#include "vector"
#include "iterator"
#include "exceptions\SystemException.h"
#include "limits"

struct msgHead {
    unsigned char	version;
    unsigned short	length;
    unsigned char	cmd;
};

int main()
{
    try {
        std::string sTmpIP;
        u_short uPort = 0;
        std::cout << "Enter server IP:";
        while (!getline(std::cin >> std::ws, sTmpIP)) {
            std::cout << "Enter server IP:";
            std::cin.clear();
            std::cin.sync();
        }
        std::cout << "Enter server port:";
        while (!(std::cin >> std::ws >> uPort, std::cin.ignore())) {
            std::cout << "Enter server port:";
            std::cin.clear();
            std::cin.ignore();
        }

        tstring sIP;
#if defined(_UNICODE) | defined(UNICODE)
        sIP = NS_Yutils::MBToWChar(sTmpIP);
#else
        sIP = sTmpIP;
#endif
        std::wcout << _T("connecting to ") << sIP << _T(":") << uPort << _T("...") << std::endl;

        WSADATA wsaData;
        WORD wVersionRequested = 0x202;
        if (0 != ::WSAStartup(wVersionRequested, &wsaData))
            throw YSystemException("BaseServer", "YBaseServer");

        SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (INVALID_SOCKET == s)
            throw YWSAException("SocketCli", "main");

        SOCKADDR sa = {};
        NS_Yutils::convertToSockAddr(&sa, sIP, uPort);

        if (SOCKET_ERROR == connect(s, &sa, sizeof(sa)))
            throw YWSAException("SocketCli", "main");

        std::cout << "connected" << std::endl;

        constexpr size_t max_send_buf_size = 640 * 1024;
        constexpr size_t max_recv_buf_size = 320 * 1024;
        std::vector<char> writeBuff(max_send_buf_size, 0);
        std::vector<char> recvBuff(max_recv_buf_size, 0);
        while (true) {
            std::cout << "Enter message:";
            std::string sContent;
            if (!getline(std::cin >> std::ws, sContent)) {
                std::cout << "invalid input. quit...";
                break;
            }

            msgHead head = {};
            head.version = 1;
            head.length = static_cast<unsigned short>(sContent.length());
            head.cmd = 0x01;

            writeBuff.assign(max_send_buf_size, 0);
            memcpy(&writeBuff[0], &head, sizeof(head));
            writeBuff.insert(writeBuff.begin() + sizeof(head), sContent.begin(), sContent.end());

            int nRet = send(s, &writeBuff[0], sizeof(head) + sContent.length(), 0);
            if (nRet <= 0)
                throw YWSAException("SocketCli", "main");

            recvBuff.assign(max_recv_buf_size, 0);
            nRet = recv(s, &recvBuff[0], max_recv_buf_size, 0);
            if (nRet <= 0)
                throw YWSAException("SocketCli", "main");

            std::string sResponse(std::string(recvBuff.begin(), recvBuff.end()).c_str());
            std::cout << sResponse << std::endl;
        }

        system("Pause");
    }
    catch (std::exception & e) {
        //LOGFATAL(e.what());
        std::cout << e.what() << std::endl;
    }

    WSACleanup();
}
