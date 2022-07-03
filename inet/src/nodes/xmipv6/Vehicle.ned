


package inet.nodes.xmipv6;

import inet.nodes.ipv6.StandardHost6;
import inet.nodes.inet.WirelessHost;
import inet.networklayer.IManetRouting;

module Server extends WirelessHost
{
    networkLayer.xMIPv6Support = true;
    networkLayer.mipv6support.isMobileNode = true; //is Mobile Node
    networkLayer.mipv6support.isHomeAgent = false; //is Home Agent
    @display("i=device/Sensor");
    
    
    
    
}

