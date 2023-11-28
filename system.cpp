#include "header.h"

// get cpu id and information, you can use `proc/cpuinfo`
string CPUinfo()
{
    char CPUBrandString[0x40];
    unsigned int CPUInfo[4] = {0, 0, 0, 0};

    // unix system
    // for windoes maybe we must add the following
    // __cpuid(regs, 0);
    // regs is the array of 4 positions
    __cpuid(0x80000000, CPUInfo[0], CPUInfo[1], CPUInfo[2], CPUInfo[3]);
    unsigned int nExIds = CPUInfo[0];

    memset(CPUBrandString, 0, sizeof(CPUBrandString));

    for (unsigned int i = 0x80000000; i <= nExIds; ++i)
    {
        __cpuid(i, CPUInfo[0], CPUInfo[1], CPUInfo[2], CPUInfo[3]);

        if (i == 0x80000002)
            memcpy(CPUBrandString, CPUInfo, sizeof(CPUInfo));
        else if (i == 0x80000003)
            memcpy(CPUBrandString + 16, CPUInfo, sizeof(CPUInfo));
        else if (i == 0x80000004)
            memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
    }
    string str(CPUBrandString);
    return str;
}

// getOsName, this will get the OS of the current computer
const char *getOsName()
{
#ifdef _WIN32
    return "Windows 32-bit";
#elif _WIN64
    return "Windows 64-bit";
#elif __APPLE__ || __MACH__
    return "Mac OSX";
#elif __linux__
    return "Linux";
#elif __FreeBSD__
    return "FreeBSD";
#elif __unix || __unix__
    return "Unix";
#else
    return "Other";
#endif
}

/**
 * Retrieves the number of running processes on the system by counting the number of directories in the /proc directory
 * that have a numeric name.
 *
 * @return The number of running processes on the system.
 */
int getProcesses()
{
    int processes = 0;
    filesystem::path procPath ("/proc");
    for(const auto& entry: filesystem::directory_iterator(procPath))
    {
        if (entry.is_directory() && isdigit(entry.path().filename().string()[0]))
            ++processes;
    }
    return processes;
}

/**
 * Retrieves CPU statistics from the /proc/stat file.
 *
 * @param cpu_s A reference to a CPUStats object where the retrieved statistics will be stored.
 */
void getCPUStats(CPUStats &cpu_s)
{
    ifstream proceStat("/proc/stat");
    string line;
    getline(proceStat, line);
    istringstream iss(line);
    string cpuLabel;
    iss >> cpuLabel;
    iss >> cpu_s.user >> cpu_s.nice >> cpu_s.system >> cpu_s.idle >> cpu_s.iowait >> cpu_s.irq >> cpu_s.softirq >> cpu_s.steal >>cpu_s.guest >>cpu_s.guestNice; 
}

/**
 * Calculates the CPU usage percentage based on the difference between the current and previous CPU statistics.
 *
 * @param prev_cpu_s A reference to a CPUStats object containing the previous CPU statistics.
 * @return The CPU usage percentage.
 */
float getCPUUsage(CPUStats &prev_cpu_s)
{
    CPUStats curr_cpu_s;
    getCPUStats(curr_cpu_s);

    long long int prev_idle = prev_cpu_s.user + prev_cpu_s.nice + prev_cpu_s.system;
    long long int curr_idle = curr_cpu_s.user + curr_cpu_s.nice+curr_cpu_s.system;
   

    long long int prev_total = prev_cpu_s.user + prev_cpu_s.nice +
     prev_cpu_s.system + prev_cpu_s.idle + prev_cpu_s.iowait + prev_cpu_s.irq +
     prev_cpu_s.softirq + prev_cpu_s.steal + prev_cpu_s.guest + prev_cpu_s.guestNice;
     
     long long int curr_total = curr_cpu_s.user + curr_cpu_s.nice +
     curr_cpu_s.system + curr_cpu_s.idle + curr_cpu_s.iowait + curr_cpu_s.irq +
     curr_cpu_s.softirq + curr_cpu_s.steal + curr_cpu_s.guest + curr_cpu_s.guestNice;

     long long int total_diff = curr_total - prev_total;
     long long int idle_diff = curr_idle - prev_idle;

    float cpuUsage = 100.0f*static_cast<float>(idle_diff)/static_cast<float>(total_diff);

    prev_cpu_s = curr_cpu_s;
    return cpuUsage;
}

/**
 * Retrieves CPU statistics from the /proc/stat file and displays them using ImGui.
 *
 * This function retrieves CPU statistics and displays them in a plot using ImGui. It uses the getCPUUsage function to calculate the CPU usage percentage and stores it in the values array. The plot is updated based on the animate checkbox and the fps slider. The scale slider controls the maximum value displayed on the plot. The CPU usage percentage is displayed as overlay text on the plot.
 */
void getCPUTabbed()
{
    const int GSIZE = 100;
    static CPUStats prev_cpu_s ={0,0,0,0,0,0,0,0,0,0};
    static int fps = 1;
    static int index = 0;
    static float timer =0.0f;
    static float scale = 100.0f;
    static float values[100] = {0};
    static bool animate = true;
    static float cpu_usage;
    char overlay_text[32];

    ImGui::Checkbox("Animate", &animate);
    ImGui::SliderInt("FPS", &fps, 0, 60);
    ImGui::SliderFloat("scale max", &scale, 0, 100);

    if(animate)
    {
        timer += ImGui::GetIO().DeltaTime;
        if(timer > 1.0f /fps)
        {
        cpu_usage = getCPUUsage(prev_cpu_s);
        values[index] = cpu_usage;
        index = (index + 1) % GSIZE;
        timer -= 1.0/fps;
        }
    }
        sprintf(overlay_text, "CPU Usage: %.2f%%", cpu_usage);
    ImGui::PlotLines("CPU", values, GSIZE, index, overlay_text, 0.0f, scale, ImVec2(0, 100));
}

/**
 * Retrieves the speed of the fan from the specified file.
 *
 * @return The speed of the fan as a string.
 */
string getSpeedFan()
{
    ifstream speedFan("/sys/class/hwmon/hwmon7/fan1_input");
    string speed;
    getline(speedFan, speed);
    return speed;
}

/**
 * Retrieves the current fan level from the system.
 *
 * @return The current fan level, either "auto" or "manual".
 */
string getFanLevel()
{
    ifstream fanLevel("/sys/class/hwmon/hwmon7/pwm1_enable");
    string level;
    getline(fanLevel, level);
    int lvl = atoi(level.c_str());
    string output = (lvl == 1) ? "auto" : "manual";
    return output;
}

/**
 * Retrieves fan statistics and displays them using ImGui.
 * The function retrieves the fan speed, status, and level from external functions.
 * It then displays the fan status, level, and speed in RPM using ImGui.
 * The function also provides options to animate the fan speed graph and adjust the FPS and scale.
 * The fan speed is plotted on a graph using ImGui's PlotLines function.
 */
void getFanTabbed()
{
    const int GSIZE = 100;
    static int fps = 1;
    static int index = 0;
    static float timer = 0.0f;
    static float scale = 2000.0f;
    static float values[100]={0};
    static bool animate = true;

    const char *speed_fan = getSpeedFan().c_str();
    const char *status_fan = (atoi(speed_fan) > 0 ) ? "enabled" : "disabled";
    const char *level_fan = getFanLevel().c_str();
   
    ImGui::Text("Status: %s         Level: %s         Speed: %s RPM", status_fan, level_fan, speed_fan);
    ImGui::Checkbox("Animate", &animate);
    ImGui::SliderInt("FPS", &fps, 0, 60);
    ImGui::SliderFloat("scale max", &scale, 0, 10000);

    if(animate)
    {
        timer += ImGui::GetIO().DeltaTime;
        if(timer > 1.0f / fps)
        {
            values[index] = stof(speed_fan);
            index = (index + 1) % GSIZE;
            timer -= 1.0/fps;
        }
    }

    char overlay_text[32];
    sprintf(overlay_text, "Speed: %.0f RPM", stof(speed_fan));
    ImGui::PlotLines("CPU", values, GSIZE, index, overlay_text, 0.0f, scale, ImVec2(0, 100));
}

/**
 * Retrieves the CPU temperature from the /sys/class/thermal/thermal_zone0/temp file.
 *
 * @return The CPU temperature in degrees Celsius.
 */
float getCPUTemp()
{
    string temp;
    ifstream cpuTemp("/sys/class/thermal/thermal_zone0/temp");
    string line;
    getline(cpuTemp, line);
    istringstream iss(line);
    iss >> temp;
    return stof(temp)/1000.00;
}

/**
 * Retrieves CPU temperature and displays it using ImGui.
 * Allows the user to animate the temperature graph, adjust the FPS, and scale the maximum value.
 */
void getThermalTabbed()
{
    const int GSIZE = 100;
    static int fps = 1;
    static int index = 0;
    static float timer = 0.0f;
    static float scale = 100.0f;
    static float values[100]={0};
    static bool animate = true;

    float cpu_temp = getCPUTemp();

    ImGui::Text("Temperature: %.1f", cpu_temp);
    ImGui::Checkbox("Animate", &animate);
    ImGui::SliderInt("FPS", &fps, 0, 60);
    ImGui::SliderFloat("scale max", &scale, 0, 100);

    if(animate)
    {
        timer += ImGui::GetIO().DeltaTime;
        if(timer > 1.0f / fps)
        {
            values[index] = cpu_temp;
            index = (index + 1) % GSIZE;
            timer -= 1.0/fps;
        }
    }
    char overlay_text[32];
    sprintf(overlay_text, "Temp: %.1f °C", cpu_temp);
    ImGui::PlotLines("CPU", values, GSIZE, index, overlay_text, 0.0f, scale, ImVec2(0, 100));
}

// Draw Container in system window
void drawTabbedContainer()
{
    if(ImGui::BeginTabBar("##TabBar"))
    {   
        // CPU tabbed
        if (ImGui::BeginTabItem("CPU"))
        {
            getCPUTabbed();
            ImGui::EndTabItem();
        }
        // Fan tabbed
        if (ImGui::BeginTabItem("Fan"))
        {
            ImGui::Text("Fan informations");
            getFanTabbed();
            ImGui::EndTabItem();
        }
        // Thermal tabbed
        if (ImGui::BeginTabItem("Thermal"))
        {
            ImGui::Text("Thermal informations");
            getThermalTabbed();
            ImGui::EndTabItem();
        }
    ImGui::EndTabBar();
    }
}
