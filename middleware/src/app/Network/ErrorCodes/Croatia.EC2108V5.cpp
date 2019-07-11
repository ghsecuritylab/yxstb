#include "NetworkAssertions_.h"
#include "NetworkManager_.h"
#include "NetworkErrorCode.h"

void NetworkErrorCodeRegister(NetworkManager& network)
{
    //Network
    network.addErrorCode(
        NET_NETWORK_DISCONNECT,
        new NetworkErrorCode(
            102001,
            SEND_ERROR_EVENT,
            "Error code:102001,Description: The STB is disconnected from an uplink device during wireline access.", 
            -1)
        );
    network.addErrorCode(
        NET_IP_CONFILICT,
        new NetworkErrorCode(
            102003,
            SEND_ERROR_EVENT,
            "Error code:102003,Description: Network connection fails due to IP address conflicts.", 
            -1)
        );

    network.addErrorCode(
        NET_SELFCHECK_FAILED,
        new NetworkErrorCode(
            102001,
            SEND_ERROR_EVENT,
            "Error code:102001,Description: The STB is disconnected from an uplink device during wireline access.",
            -1)
        ); //TODO, I don't know, use 10000 instead.

    //Dhcp
    network.addErrorCode(
        DHCP_XSOCKET_ERROR,
        new NetworkErrorCode(
            102004,
            SEND_ERROR_EVENT,
            "Error code:102004,Description: No response is sent back for DHCP Discover requests.", 
            -1)
        );
    network.addErrorCode(
        DHCP_DISCOVER_TIMEDOUT,
        new NetworkErrorCode(
            102045,
            SEND_ERROR_EVENT,
            "Error code:102045,Description: No response is sent back for DHCP Discover requests.",
            -1)
        );
    network.addErrorCode(
        DHCP_REQUEST_TIMEDOUT,
        new NetworkErrorCode(
            102046,
            SEND_ERROR_EVENT,
            "Error code:102046,Description: No response is sent back for DHCP requests.",
            -1)
        );
    network.addErrorCode(
        DHCP_RENEW_TIMEDOUT,
        new NetworkErrorCode(
            102045,
            SEND_ERROR_EVENT,
            "Error code:10011,Description: No response is sent back for DHCP requests.", 
            -1)
        );
    network.addErrorCode(
        DHCP_RENEW_TIMEDOUT,
        new NetworkErrorCode(
            102045,
            SEND_ERROR_EVENT,
            "Error code:102045,Description: No response is sent back for DHCP requests.", 
            -1)
        );

    //PPPoE
    network.addErrorCode(
        PPPOE_XSOCKER_ERROR,
        new NetworkErrorCode(
            102005,
            SEND_ERROR_EVENT,
            "Error code:102005,Description: No response is sent back for PPPoE PADI requests.",
            -1)
        );
    network.addErrorCode(
        PPPOE_PADI_TIMEDOUT,
        new NetworkErrorCode(
            102006,
            SEND_ERROR_EVENT,
            "Error code:102006,Description: No response is sent back for PPPoE PADI requests.", 
            -1)
        );

    network.addErrorCode(
        PPPOE_IPCP_FAILED,
        new NetworkErrorCode(
            102007,
            SEND_ERROR_EVENT,
            "Error code:102007,Description: No response is sent back for PPPoE PADI requests.", 
            -1)
        );
    network.addErrorCode(
        PPPOE_PADR_TIMEDOUT,
        new NetworkErrorCode(
            102007,
            SEND_ERROR_EVENT,
            "Error code:102007,Description: No response is sent back for PPPoE PADR requests.", 
            -1)
        );
    network.addErrorCode(
        PPPOE_LCP_TIMEDOUT,
        new NetworkErrorCode(
            102007,
            SEND_ERROR_EVENT,
            "Error code:102007,Description: No response is sent back for PPPoE PADR requests.",
            -1)
        );

    network.addErrorCode(
        PPPOE_AUTH_FAILED,
        new NetworkErrorCode(
            102007,
            SEND_ERROR_EVENT,
            "Error code:102007,Description: PPPoE Session negotiation fails.",
            -1)
        );

    //Wireless
    network.addErrorCode(
        WIFI_LOADING_FAILED,
        new NetworkErrorCode(
            10060,
            SEND_ERROR_EVENT,
            "Error code:10060,Description: Loading the wireless network adapter fails. A subscriber chooses the wireless access mode but no wireless network adapter is detected.",
            -1)
        );
    network.addErrorCode(
        WIFI_CONNECT_FAILED,
        new NetworkErrorCode(
            10061,
            SEND_ERROR_EVENT,
            "Error code:10061,Description: Connecting to the wireless network fails. A subscriber chooses the wireless access mode but no wireless network is available."
            -1)
        );
}
