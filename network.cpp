#include "header.h"

map<string, Net> net_map;
time_t net_last_retrieval_time = 0;
time_t net_current_time = time(nullptr);
bool net_needs_refresh = ((net_map.empty()) || difftime(net_current_time, net_last_retrieval_time) >= REFRESH_INTERVAL);

/**
 * Retrieves and displays the IPv4 network information.
 * This function uses the getifaddrs function to retrieve the network interface information,
 * and then iterates through the interfaces to find the ones with IPv4 addresses.
 * The IPv4 addresses are displayed using the ImGui library.
 */
void getIpv4Network(Networks *networks)
{
    IP4 ip;

    ImGui::Spacing();
    ImGui::Text("IPV4 Network:");

    struct ifaddrs *ifaddr, *ifa;
    getifaddrs(&ifaddr);

    for(ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
    {
        if(ifa->ifa_addr == nullptr) continue;
        if(ifa->ifa_addr->sa_family == AF_INET)
        {
            ip.name = ifa->ifa_name;
            struct sockaddr_in *addr = reinterpret_cast<struct sockaddr_in*>(ifa->ifa_addr);
            inet_ntop(AF_INET, &(addr->sin_addr), ip.addressBuffer, INET_ADDRSTRLEN);
            ImGui::Text("   %s : %s", ip.name, ip.addressBuffer);
            networks->ip4s.push_back(ip);
        }
    }
    freeifaddrs(ifaddr);
}

/*
* This function fills the net_map with network data obtained from the /proc/net/dev file.
*/
void fillRXTXDatas()
{
    map<string, Net> new_net_map;
        ifstream dev_file("/proc/net/dev");
        if (dev_file.is_open())
        {
            string line;
            getline(dev_file, line);
            getline(dev_file, line);

            while (getline(dev_file, line))
            {
                Net net;
                string interface;
                stringstream net_ss(line);
                net_ss >> interface;
                net_ss >> net.received.bytes >> net.received.packets >> net.received.errs >> net.received.drop >> net.received.fifo >> net.received.colls >> net.received.carrier >> net.received.compressed >> net.transmited.bytes >> net.transmited.packets >> net.transmited.errs >> net.transmited.drop >> net.transmited.fifo >> net.transmited.frame >> net.transmited.compressed >> net.transmited.multicast;

                new_net_map[interface] = move(net);
            }
            dev_file.close();
        }
        net_map = move(new_net_map);
}

/*
* This function uses ImGui to create a table called "TX" with 9 columns.
* The columns represent different network data metrics such as bytes, packets, errors, drops, etc.
* The function then iterates over the net_map, which is a map containing network data obtained from the /proc/net/dev file.
* For each entry in the map, the function adds a new row to the table and sets the values of each column using the corresponding data from the Net struct.
*/
void getTXTable()
{
    
    if (ImGui::BeginTable("TX", 9))
    {
        ImGui::TableSetupColumn("Interface");
        ImGui::TableSetupColumn("Bytes");
        ImGui::TableSetupColumn("Packets");
        ImGui::TableSetupColumn("Errs");
        ImGui::TableSetupColumn("Drop");
        ImGui::TableSetupColumn("Fifo");
        ImGui::TableSetupColumn("Frame");
        ImGui::TableSetupColumn("Compressed");
        ImGui::TableSetupColumn("Multicast");
        ImGui::TableHeadersRow();

        for (const auto &pair : net_map)
        {
            const Net &datas = pair.second;
            
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text(pair.first.c_str());
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%d", datas.transmited.bytes);
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%d", datas.transmited.packets);
                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%d", datas.transmited.errs);
                ImGui::TableSetColumnIndex(4);
                ImGui::Text("%d", datas.transmited.drop);
                ImGui::TableSetColumnIndex(5);
                ImGui::Text("%d", datas.transmited.fifo);
                ImGui::TableSetColumnIndex(6);
                ImGui::Text("%d", datas.transmited.frame);
                ImGui::TableSetColumnIndex(7);
                ImGui::Text("%d", datas.transmited.compressed);
                ImGui::TableSetColumnIndex(8);
                ImGui::Text("%d", datas.transmited.multicast);
        }
        ImGui::EndTable();
    }
}

/*
 * This function uses ImGui to create a table called "RX" with 9 columns.
 * The columns represent different network data metrics such as bytes, packets, errors, drops, etc.
 * The function then iterates over the net_map, which is a map containing network data obtained from the /proc/net/dev file.
 * For each entry in the map, the function adds a new row to the table and sets the values of each column using the corresponding data from the Net struct.
 */
void getRXTable()
{
    
    if (ImGui::BeginTable("RX", 9))
    {
        ImGui::TableSetupColumn("Interface");
        ImGui::TableSetupColumn("Bytes");
        ImGui::TableSetupColumn("Packets");
        ImGui::TableSetupColumn("Errs");
        ImGui::TableSetupColumn("Drop");
        ImGui::TableSetupColumn("Fifo");
        ImGui::TableSetupColumn("Colls");
        ImGui::TableSetupColumn("Carrier");
        ImGui::TableSetupColumn("Compressed");
        ImGui::TableHeadersRow();

        for (const auto &pair : net_map)
        {
            const Net &datas = pair.second;
            
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text(pair.first.c_str());
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%d", datas.received.bytes);
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%d", datas.received.packets);
                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%d", datas.received.errs);
                ImGui::TableSetColumnIndex(4);
                ImGui::Text("%d", datas.received.drop);
                ImGui::TableSetColumnIndex(5);
                ImGui::Text("%d", datas.received.fifo);
                ImGui::TableSetColumnIndex(6);
                ImGui::Text("%d", datas.received.colls);
                ImGui::TableSetColumnIndex(7);
                ImGui::Text("%d", datas.received.carrier);
                ImGui::TableSetColumnIndex(8);
                ImGui::Text("%d", datas.received.compressed);
        }
        ImGui::EndTable();
    }
}

/*
* This function retrieves network data and populates the network table in the Networks struct.
* If the network data needs to be refreshed, it calls the fillRXTXDatas() function.
* The network table is displayed using ImGui::TreeNode and ImGui::TreePop functions.
*/
void getNetworkTable(Networks *networks)
{
        if (net_needs_refresh)
        {
            net_last_retrieval_time = net_current_time;
            fillRXTXDatas();
        }
    if(ImGui::TreeNode("Network table"))
    {
        if(ImGui::TreeNode("RX"))
        {
            getRXTable();
            ImGui::TreePop();
        }
        if(ImGui::TreeNode("TX"))
        {
            getTXTable();
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }
}

// draw Progress Bar with RX data.
void drawRXProgress()
{
    for (const auto &pair : net_map)
    {
        const Net &datas = pair.second;
        char rx_overlay[50];
        string unit;
        long double rx_value = datas.received.bytes;

        if (rx_value >= pow(1024, 3))
        {
            // rx_value /= pow(1000, 3); //value for wsl
            rx_value /= pow(1024, 3);
            unit = "GB";
        }
        else if (rx_value >= pow(1024, 2))
        {
            // rx_value /= pow(1000, 2); //value for wsl
            rx_value /= pow(1024, 2);
            unit = "MB";
        }
        else if (rx_value >= 1024)
        {
            // rx_value /= 1000; //value for wsl
            rx_value /= 1024;
            unit = "KB";
        }
        else
        {
            unit = "bytes";
        }

        sprintf(rx_overlay, "%.2Lf %s", rx_value, unit.c_str());
        long double rx_progress = (long double)datas.received.bytes / 2000000000.0;
        ImGui::Text("%s", pair.first.c_str());
        ImGui::Spacing();
        ImGui::ProgressBar(rx_progress, ImVec2(-1.0f, 0.0f), rx_overlay);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY());
        ImGui::Text("0 GiB");
        ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
        ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - string("2.0GB").size());
        ImGui::SetCursorPosY(ImGui::GetCursorPosY());
        ImGui::Text("2.0GB");
        ImGui::Spacing();
        ImGui::Spacing();
    }
}

// draw Progress Bar with TX data.
void drawTXProgress()
{
    for (const auto &pair : net_map)
    {
        const Net &datas = pair.second;
        char tx_overlay[50];
        string unit;
        long double tx_value = datas.transmited.bytes;

        if (tx_value >= pow(1024, 3))
        {
            // tx_value /= pow(1000, 3); //value for wsl
            tx_value /= pow(1024, 3); 
            unit = "GB";
        }
        else if (tx_value >= pow(1024, 2))
        {
            // tx_value /= pow(1000, 2); //value for wsl
            tx_value /= pow(1024, 2);
            unit = "MB";
        }
        else if (tx_value >= 1024)
        {
            // tx_value /= 1000; //value for wsl
            tx_value /= 1024;
            unit = "KB";
        }
        else
        {
            unit = "bytes";
        }

        sprintf(tx_overlay, "%.2Lf %s", tx_value, unit.c_str());
        long double tx_progress = (long double)datas.transmited.bytes / 2000000000.0;
        ImGui::Text("%s", pair.first.c_str());
        ImGui::Spacing();
        ImGui::ProgressBar(tx_progress, ImVec2(-1.0f, 0.0f), tx_overlay);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY());
        ImGui::Text("0 GiB");
        ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
        ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - string("2.0GB").size());
        ImGui::SetCursorPosY(ImGui::GetCursorPosY());
        ImGui::Text("2.0GB");
        ImGui::Spacing();
        ImGui::Spacing();
    }
}

// Draw Container in network window
void drawNetworkTabbed()
{
    if (ImGui::BeginTabBar("##TabBar"))
    {
        // RX tabbed
        if (ImGui::BeginTabItem("Receive(RX)"))
        {
            drawRXProgress();
            ImGui::EndTabItem();
        }
        // TX tabbed
        if (ImGui::BeginTabItem("Transmit(TX)"))
        {
            drawTXProgress();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}
