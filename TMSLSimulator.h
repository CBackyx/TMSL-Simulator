#ifndef TMSLSIMULATOR_H
#define TMSLSIMULATOR_H

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
};

struct FUState {
    int lineIndex;
    int cycle_todo;
    int owner;
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

    public:
        TMSLSimulator();
        ~TMSLSimulator() {}
        void readLines(char* inputFile);
        int doLaunchLines(int start);
        void doClocks();
        void writeRecords();
        void printState();
        void grabFU(int l, int rst);
};

#endif