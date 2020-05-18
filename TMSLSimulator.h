#ifndef TMSLSIMULATOR_H
#define TMSLSIMULATOR_H

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

struct ReserveStation;
struct FUState;
struct Register;
struct WindowMember;
struct Line;

struct ReserveStation {
    bool Busy;
    char Op;
    int Vj;
    int Vk;
    int Qj;
    int Qk;
    int Addr;
    int Fu;
    int LineNum;
    bool Inqueue;
    vector<pair<int, char>> Q_RS;
    vector<int> Q_R;
};

struct FUState {
    int lineIndex;
    int cycle_todo;
    int owner;
    int result;
};

struct Register {
    int value;
    int stat;
};

struct WindowMember {
    int line_index;
    int rs_index;
};

struct Line {
    char cmd;
    int rg[3];
    int op[2]; // instant number
};

class TMSLSimulator {

    char *ifname;
    int lineNum;
    struct ReserveStation rStations[12];
    struct FUState fuStates[7]; // for that the register index starts from 1
    struct Register registers[33];
    FILE* logFile;

    public:
        string ifn;

        TMSLSimulator();
        ~TMSLSimulator() {}
        void readLines(char* inputFile);
        int doLaunchLines(int start);
        void doClocks();
        void writeRecords();
        void printState(int clock);
        void grabFU(int l, int rst);
        void doFU(int clock);
        void doCollect(int clock);
        void dequeFU();
        
};

#endif