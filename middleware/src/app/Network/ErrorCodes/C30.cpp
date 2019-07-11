#include "NetworkAssertions.h"
#include "NetworkManager.h"
#include "NetworkErrorCode.h"
#include "BrowserAgent.h"

void NetworkErrorCodeRegister(NetworkManager& network)
{
    //Network
    network.addErrorCode(
       NET_NETWORK_DISCONNECT,
        new NetworkErrorCode(
            10000,
            SHOW_MINI_ALERT,
            "Error code:10000,Description: The STB is disconnected from an uplink device during wireline access.", 
            Hippo::BrowserAgent::PromptDisconnectAlert)
        );
    network.addErrorCode(
        NET_IP_CONFILICT,
        new NetworkErrorCode(
            10001,
            OPEN_ERROR_PAGE,
            "Error code:10001,Description: Network connection fails due to IP address conflicts.", 
            -1)
        );

    network.addErrorCode(
        NET_SELFCHECK_FAILED,
        new NetworkErrorCode(
            10000,
            SHOW_MINI_ALERT,
            "Error code:10000,Description: The STB is disconnected from an uplink device during wireline access.",
            Hippo::BrowserAgent::PromptDisconnectAlert)
        ); //TODO, I don't know, use 10000 instead.

    //Dhcp
    network.addErrorCode(
        DHCP_XSOCKET_ERROR,
        new NetworkErrorCode(
            10010,
            OPEN_ERROR_PAGE,
            "Error code:10010,Description: No response is sent back for DHCP Discover requests.", 
            -1)
        );
    network.addErrorCode(
        DHCP_DISCOVER_TIMEDOUT,
        new NetworkErrorCode(
            10010,
            OPEN_ERROR_PAGE,
            "Error code:10010,Description: No response is sent back for DHCP Discover requests.",
            -1)
        );
    network.addErrorCode(
        DHCP_REQUEST_TIMEDOUT,
        new NetworkErrorCode(
            10011,
            OPEN_ERROR_PAGE,
            "Error code:10011,Description: No response is sent back for DHCP requests.",
            -1)
        );
    network.addErrorCode(
        DHCP_RENEW_TIMEDOUT,
        new NetworkErrorCode(
            10011,
            OPEN_ERROR_PAGE,
            "Error code:10011,Description: No response is sent back for DHCP requests.", 
            -1)
        );
    network.addErrorCode(
        DHCP_RENEW_TIMEDOUT,
        new NetworkErrorCode(
            10011,
            OPEN_ERROR_PAGE,
            "Error code:10011,Description: No response is sent back for DHCP requests.", 
            -1)
        );

    //PPPoE
    network.addErrorCode(
        PPPOE_XSOCKER_ERROR,
        new NetworkErrorCode(
            10021,
            OPEN_ERROR_PAGE,
            "Error code:10021,Description: No response is sent back for PPPoE PADI requests.",
            -1)
        );
    network.addErrorCode(
        PPPOE_PADI_TIMEDOUT,
        new NetworkErrorCode(
            10021,
            OPEN_ERROR_PAGE,
            "Error code:10021,Description: No response is sent back for PPPoE PADI requests.", 
            -1)
        );

    network.addErrorCode(
        PPPOE_IPCP_FAILED,
        new NetworkErrorCode(
            10021,
            OPEN_ERROR_PAGE,
            "Error code:10021,Description: No response is sent back for PPPoE PADI requests.", 
            -1)
        );
    network.addErrorCode(
        PPPOE_PADR_TIMEDOUT,
        new NetworkErrorCode(
            10022,
            OPEN_ERROR_PAGE,
            "Error code:10022,Description: No response is sent back for PPPoE PADR requests.", 
            -1)
        );
    network.addErrorCode(
        PPPOE_LCP_TIMEDOUT,
        new NetworkErrorCode(
            10022,
            OPEN_ERROR_PAGE,
            "Error code:10022,Description: No response is sent back for PPPoE PADR requests.",
            -1)
        );

    network.addErrorCode(
        PPPOE_AUTH_FAILED,
        new NetworkErrorCode(
            10023,
            OPEN_ERROR_PAGE,
            "Error code:10023,Description: PPPoE Session negotiation fails.",
            -1)
        );

    //Wireless
    network.addErrorCode(
        WIFI_LOADING_FAILED,
        new NetworkErrorCode(
            10060,
            SHOW_MINI_ALERT,
            "Error code:10060,Description: Loading the wireless network adapter fails. A subscriber chooses the wireless access mode but no wireless network adapter is detected.",
            Hippo::BrowserAgent::PromptWifiDisconnectAlert)
        );
    network.addErrorCode(
        WIFI_CONNECT_FAILED,
        new NetworkErrorCode(
            10061,
            OPEN_ERROR_PAGE,
            "Error code:10061,Description: Connecting to the wireless network fails. A subscriber chooses the wireless access mode but no wireless network is available."
            -1)
        );

    //GuangDong Customizate
    network.setCustomErrCode(true);
    network.addErrorCode(
        (28101 << 16) | PPPOE_AUTH_FAILED,
        new NetworkErrorCode(
            28101,
            OPEN_ERROR_PAGE,
            "Error code:28101,Description: The network account does not exist.",
            -1)
        );
    network.addErrorCode(
        (22102 << 16) | PPPOE_AUTH_FAILED,
        new NetworkErrorCode(
            22102,
            OPEN_ERROR_PAGE,
            "Error code:22102,Description: The password of the network account is incorrect.",
            -1)
        );
    network.addErrorCode(
        (22104 << 16) | PPPOE_AUTH_FAILED,
        new NetworkErrorCode(
            22104,
            OPEN_ERROR_PAGE,
            "Error code:22104,Description: The network account is formally suspended or suspended upon arrears.",
            -1)
        );
    network.addErrorCode(
        (22106 << 16) | PPPOE_AUTH_FAILED,
        new NetworkErrorCode(
            22106,
            OPEN_ERROR_PAGE,
            "Error code:22106,Description: The network account hangs.",
            -1)
        );
    network.addErrorCode(
        (22122 << 16) | PPPOE_AUTH_FAILED,
        new NetworkErrorCode(
            22122,
            OPEN_ERROR_PAGE,
            "Error code:22122 (or 22130),Description: The binding of the network account and port is incorrect.",
            -1)
        );
    network.addErrorCode(
        (22103 << 16) | PPPOE_AUTH_FAILED,
        new NetworkErrorCode(
            22103,
            OPEN_ERROR_PAGE,
            "Error code:22103,Description: The network account has been deregistered.",
            -1)
        );
    network.addErrorCode(
        10099,
        new NetworkErrorCode(
            10099,
            OPEN_ERROR_PAGE,
            "Error code:10099,Description: The error is unknown.",
            -1)
        );
}
