#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mcc_generated_files/mcc.h"
#include "tcp_client_demo.h"
#include "mcc_generated_files/TCPIPLibrary/tcpv4.h"
#include "mcc_generated_files/TCPIPLibrary/ipv4.h"
#include "mcc_generated_files/TCPIPLibrary/tcpip_config.h"


sockaddr_in4_t remoteSocket;

void DEMO_TCP_Client(void)
{
    // create the socket for the TCP Client
    static tcpTCB_t port60TCB;
    
    // create the TX and RX Client's buffers
    static uint8_t rxdataPort60[50];
    static uint8_t txdataPort60[80];


    static time_t t_client;
    static time_t socketTimeout;
    
    uint16_t rx_len;
    socketState_t socketState;
    rx_len = 0;
    char strTmp[17];
    
    socketState = TCP_SocketPoll(&port60TCB);

    time(&t_client);

    switch(socketState)
    {
        case NOT_A_SOCKET:
            // Inserting and initializing the socket
            TCP_SocketInit(&port60TCB);
            break;
        case SOCKET_CLOSED:
            // if the socket is closed we will try to connect again
            // try to connect once at 2 seconds
            socketTimeout = t_client + 2;
            TCP_InsertRxBuffer(&port60TCB, rxdataPort60, sizeof(rxdataPort60));
            TCP_Connect(&port60TCB, &remoteSocket);
            break;
        case SOCKET_IN_PROGRESS:
            // if the socket is closed we will try to connect again
            if(t_client >= socketTimeout)
            {
                TCP_Close(&port60TCB);
            }
            break;
        case SOCKET_CONNECTED:
            // implement an echo client over TCP
            // check if the previous buffer was sent
            if (TCP_SendDone(&port60TCB))
            {
                rx_len = TCP_GetReceivedData(&port60TCB);
                // handle the incoming data
                if(rx_len > 0)
                {
                    // check for "led x on/off" command
                    if (rx_len > 16) {
                        rxdataPort60[16] = 0;
                    } else {
                        rxdataPort60[rx_len] = 0;
                    }

                    if(rxdataPort60[0] == 'l' && rxdataPort60[1] == 'e' && rxdataPort60[2] == 'd') {
                        if(rxdataPort60[6] == 'o' && rxdataPort60[7] == 'n') {
                                RA0_SetHigh();
                        }else {
                            if(rxdataPort60[6] == 'o' && rxdataPort60[7] == 'f' && rxdataPort60[8] == 'f') {
                                RA0_SetLow();
                            }
                        }
                    }
                    // reuse the RX buffer
                    TCP_InsertRxBuffer(&port60TCB, rxdataPort60, sizeof(rxdataPort60));
                }

                if(t_client >= socketTimeout)
                { 
                    // send board status message only once at 2 seconds
                    socketTimeout = t_client + 2;
                    sprintf(txdataPort60,"LED's state: %d\n", LATAbits.LATA0);
                    //send data back to the source
                    TCP_Send(&port60TCB, txdataPort60, strlen(txdataPort60));
                }
            }
            break;
        case SOCKET_CLOSING:
            TCP_SocketRemove(&port60TCB);
            break;
        default:
            break;
            // we should not end up here
    }
}

void TCP_Client_Initialize(){
    remoteSocket.addr.s_addr = MAKE_IPV4_ADDRESS(192, 168, 0, 16);
    remoteSocket.port = 60;
}