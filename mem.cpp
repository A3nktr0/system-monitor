#include "header.h"

const int REFRESH_INTERVAL = 1;

map<int, Proc> process_map;
vector<int> selected_rows;
time_t process_last_retrieval_time = 0;

/**
 * Retrieves memory statistics from the /proc/meminfo file and stores them in a Memory object.
 *
 * @param mem A pointer to a Memory object where the retrieved statistics will be stored.
 */
void getMemoryValues(Memory *mem)
{
       ifstream meminfo("/proc/meminfo");
       string line;
       int count = 0;

       while (getline(meminfo, line))
       {
              if (line.find("MemTotal") != string::npos)
              {
                     sscanf(line.c_str(), "MemTotal: %lld", &mem->total_ram);
                     count++;
              }
              else if (line.find("MemAvailable") != string::npos)
              {
                     sscanf(line.c_str(), "MemAvailable: %lld", &mem->used_ram);
                     count++;
              }
              else if (line.find("SwapTotal") != string::npos)
              {
                     sscanf(line.c_str(), "SwapTotal: %lld", &mem->total_swap);
                     count++;
              }
              else if (line.find("SwapFree") != string::npos)
              {
                     sscanf(line.c_str(), "SwapFree: %lld", &mem->used_swap);
                     mem->used_swap = mem->total_swap - mem->used_swap;
                     count++;
              }
              if (count == 4)
                     break;
       }
       mem->used_ram = (mem->total_ram - mem->used_ram);
}

/**
 * Retrieves memory statistics and displays them using ImGui.
 */
void getMemory()
{      
       Memory mem;
       getMemoryValues(&mem);

       char tr[20];
       char ts[20];
       sprintf(tr, "%.1f GiB", (float)mem.total_ram / 1024 / 1024);
       sprintf(ts, "%d MiB", (int)mem.total_swap / 1024);

       float ram_progress = (float)mem.used_ram / mem.total_ram;
       char ram_values[50];
       sprintf(ram_values, "%.1f GiB / %.1f GiB", (float)mem.used_ram / 1024 / 1024, (float)mem.total_ram / 1024 / 1024);

       ImGui::Text("Physic Memory (RAM) :");
       ImGui::Spacing();
       ImGui::ProgressBar(ram_progress, ImVec2(-1.0f, 0.0f), ram_values);
       ImGui::SetCursorPosY(ImGui::GetCursorPosY());
       ImGui::Text("0 GiB");
       ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
       ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - string(tr).size());
       ImGui::SetCursorPosY(ImGui::GetCursorPosY());
       ImGui::Text(tr);
       ImGui::Spacing();
       ImGui::Spacing();

       float swap_progress = (float)mem.used_swap / mem.total_swap;
       char swap_values[50];
       sprintf(swap_values, "%lld MiB / %lld MiB", mem.used_swap, mem.total_swap / 1024);

       ImGui::Text("Virtual Memory (SWAP) :");
       ImGui::Spacing();
       ImGui::ProgressBar(swap_progress, ImVec2(-1.0f, 0.0f), swap_values);
       ImGui::SetCursorPosY(ImGui::GetCursorPosY());
       ImGui::Text("0 MiB");
       ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
       ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - string(tr).size());
       ImGui::SetCursorPosY(ImGui::GetCursorPosY());
       ImGui::Text(ts);
       ImGui::Spacing();
       ImGui::Spacing();
}

/**
 * Retrieves disk usage statistics from the root directory.
 * Calculates the total disk space, free disk space, used disk space,
 * disk space usage progress, and displays them using ImGui.
 */
void getDiskUsage()
{
       struct statvfs buff;

       if (statvfs("/", &buff) == -1)
       {
              return;
       }

       unsigned long disk_total = buff.f_blocks * buff.f_frsize;
       unsigned long disk_free = buff.f_bfree * buff.f_frsize;
       unsigned long disk_used = disk_total - disk_free;

       double disk_total_gb = (double)(disk_total) / (1024 * 1024 * 1024);
       double disk_used_gb = (double)(disk_used) / (1024 * 1024 * 1024);
       float disk_progress = (float)disk_used / (float)disk_total;

       char ds[20];
       char disk_values[50];
       sprintf(ds, "%.f GiB", ceil(disk_total_gb));
       sprintf(disk_values, "%.f GiB / %.f GiB", ceil(disk_used_gb), ceil(disk_total_gb));

       ImGui::Text("Disk :");
       ImGui::Spacing();
       ImGui::ProgressBar(disk_progress, ImVec2(-1.0f, 0.0f), disk_values);
       ImGui::SetCursorPosY(ImGui::GetCursorPosY());
       ImGui::Text("0 GiB");
       ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
       ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - string(ds).size());
       ImGui::SetCursorPosY(ImGui::GetCursorPosY());
       ImGui::Text(ds);
       ImGui::Spacing();
       ImGui::Spacing();
}

/**
 * Retrieves the process table and displays it using ImGui.
 * The process table includes information such as PID, name, state, CPU usage, and memory usage.
 * The table can be filtered by process name.
 * The table is automatically refreshed at a specified interval.
 */
void getProcessTable()
{
       if (ImGui::TreeNode("Process Table"))
       {
              time_t process_current_time = time(nullptr);
              bool process_needs_refresh = (process_map.empty() || difftime(process_current_time, process_last_retrieval_time) >= REFRESH_INTERVAL);
              if (process_needs_refresh)
              {
                     process_last_retrieval_time = process_current_time;
                     updateProcessData();
              }
              ImGui::Text("Filter the process by name:");
              static ImGuiTextFilter filter;
              filter.Draw();

              if (ImGui::BeginTable("proc", 5))
              {
                     ImGui::TableSetupColumn("PID");
                     ImGui::TableSetupColumn("NAME");
                     ImGui::TableSetupColumn("STATE");
                     ImGui::TableSetupColumn("CPU");
                     ImGui::TableSetupColumn("MEM v/o");
                     ImGui::TableHeadersRow();

                     for (const auto &pair : process_map)
                     {
                            const Proc &process = pair.second;
                            if (filter.PassFilter(process.name.c_str()))
                            {
                                   ImGui::TableNextRow();
                                   ImGui::TableSetColumnIndex(0);
                                   bool is_selected = (find(selected_rows.begin(), selected_rows.end(), process.pid) != selected_rows.end());
                                   if (ImGui::Selectable(to_string(process.pid).c_str(), is_selected, ImGuiSelectableFlags_SpanAllColumns))
                                   {
                                          if (is_selected)
                                          {
                                                 selected_rows.erase(remove(selected_rows.begin(), selected_rows.end(), process.pid), selected_rows.end());
                                          }
                                          else
                                          {
                                                 selected_rows.push_back(process.pid);
                                          }
                                   }
                                   ImGui::TableSetColumnIndex(1);
                                   ImGui::Text(process.name.c_str());
                                   ImGui::TableSetColumnIndex(2);
                                   ImGui::Text(&process.state);
                                   ImGui::TableSetColumnIndex(3);
                                   ImGui::Text("%.2f", process.cpu_usage);
                                   ImGui::TableSetColumnIndex(4);
                                   ImGui::Text("%.2f", process.memory_usage);
                            }
                     }
                     ImGui::EndTable();
              }
              ImGui::TreePop();
       }
}

/**
 * Updates the process data by retrieving CPU and memory statistics for each process.
 * The CPU statistics are obtained from the /proc/stat file, while the memory statistics
 * are calculated using information from the /proc/[pid]/stat file.
 */
void updateProcessData()
{
       map<int, Proc> new_process_map;
       double uptime;
       filesystem::path proc_path("/proc");

       ifstream uptime_file("/proc/uptime");
       if (uptime_file.is_open())
       {
              string uptime_data;
              getline(uptime_file, uptime_data);
              stringstream uptime_ss(uptime_data);
              uptime_ss >> uptime;
              uptime_file.close();
       }

       for (const auto &entry : filesystem::directory_iterator(proc_path))
       {
              if (entry.is_directory() && isdigit(entry.path().filename().string()[0]))
              {
                     Proc curr;
                     // Get CPU usage for individual process.
                     ifstream proc_stat("/proc/" + entry.path().filename().string() + "/stat");
                     if (proc_stat.is_open())
                     {
                            string stat_data;
                            getline(proc_stat, stat_data);
                            stringstream ss(stat_data);
                            vector<string> datas;
                            string data;
                            while (ss >> data)
                            {
                                   datas.push_back(data);
                            }

                            curr.pid = stoi(datas[0]);
                            curr.name = std::move(datas[1]);
                            curr.state = datas[2].at(0);
                            curr.utime = stod(datas[13]);
                            curr.stime = stod(datas[14]);
                            curr.cutime = stod(datas[15]);
                            curr.cstime = stod(datas[16]);
                            curr.starttime = stod(datas[21]);
                            curr.vsize = stoul(datas[22]);
                            curr.rss = stod(datas[23]);

                            double hertz = sysconf(_SC_CLK_TCK);
                            double total_time = curr.utime + curr.stime + curr.cutime + curr.cstime;
                            double seconds = uptime - (curr.starttime / hertz);
                            double cpu_usage = 100.0 * ((total_time / hertz) / seconds);
                            curr.cpu_usage = cpu_usage;

                            // Get memory usage for individual process.
                            double memory_usage = (curr.rss) * sysconf(_SC_PAGESIZE) / 1024;
                            double totalMemory = sysconf(_SC_PHYS_PAGES) * sysconf(_SC_PAGESIZE) / 1024;
                            curr.memory_usage = (memory_usage / totalMemory) * 100.0;
                            if (curr.memory_usage > 100.0)
                            {
                                   curr.memory_usage /= 100000;
                            }                     
                            new_process_map[curr.pid] = move(curr);
                            datas.clear();
                            proc_stat.close();
                     }
              }
       }
       process_map = move(new_process_map);
}
