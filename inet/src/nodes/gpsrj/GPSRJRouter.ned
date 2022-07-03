//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package inet.nodes.gpsrj;

import inet.networklayer.routing.gpsrj.IGPSRJ;
import inet.nodes.inet.WirelessHost;

//
// TODO
//
module GPSRJRouter extends WirelessHost
{
    parameters:
        @display("i=device/cellphone");
        IPForward = true;
        wlan[*].mgmtType = default("Ieee80211MgmtAdhoc");
        string gpsrType = default("GPSR");
    submodules:
        gpsrj: <gpsrjType> like IGPSRJ {
            @display("p=527,287");
        }
    connections:
        gpsrj.ipOut --> networkLayer.transportIn++;
        gpsrj.ipIn <-- networkLayer.transportOut++;
}
