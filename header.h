// To make sure you don't declare the function more than once by including the header multiple times.
#ifndef header_H
#define header_H

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <dirent.h>
#include <vector>
#include <iostream>
#include <cmath>
// lib to read from file
#include <fstream>
// for the name of the computer and the logged in user
#include <unistd.h>
#include <limits.h>
// this is for us to get the cpu information
// mostly in unix system
// not sure if it will work in windows
#include <cpuid.h>
// this is for the memory usage and other memory visualization
// for linux gotta find a way for windows
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
// for time and date
#include <ctime>
// ifconfig ip addresses
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <map>
#include <filesystem>
#include <algorithm>

using namespace std;
struct CPUStats
{
    long long int user;
    long long int nice;
    long long int system;
    long long int idle;
    long long int iowait;
    long long int irq;
    long long int softirq;
    long long int steal;
    long long int guest;
    long long int guestNice;
};

// processes `stat`
struct Proc
{
    int pid;
    string name;
    char state;
    long unsigned vsize;
    double rss; 
    double utime;
    double stime;
    double cutime;
    double cstime;
    double starttime; 
    double cpu_usage;
    double memory_usage;
};

struct IP4
{
    char *name;
    char addressBuffer[INET_ADDRSTRLEN];
};

struct Networks
{
    vector<IP4> ip4s;
    
};

struct TX
{
    int bytes;
    int packets;
    int errs;
    int drop;
    int fifo;
    int frame;
    int compressed;
    int multicast;
};

struct RX
{
    int bytes;
    int packets;
    int errs;
    int drop;
    int fifo;
    int colls;
    int carrier;
    int compressed;
};

struct Memory
{
    long long int total_ram;
    long long int used_ram;
    long long int total_swap;
    long long int used_swap;
};

struct Net
{
    RX received;
    TX transmited;
};

// student TODO : system stats

string CPUinfo();
const char *getOsName();
int getProcesses();
void getCPUStats(CPUStats &cpu_s);
float getCPUUsage(CPUStats &prev_cpu_s);
string getSpeedFan();
string getFanLevel();
float getCPUTemp();
void drawTabbedContainer();
void getCPUTabbed();
void getFanTabbed();
void getThermalTabbed();

// student TODO : memory and processes

void getMemory();
void getMemoryValues(Memory *mem);
void getDiskUsage();
void getProcessTable();
void updateProcessData();

// student TODO : network

void getIpv4Network(Networks *networks);
void getNetworkTable(Networks *networks);
void drawNetworkTabbed();

extern const int REFRESH_INTERVAL;

#endif