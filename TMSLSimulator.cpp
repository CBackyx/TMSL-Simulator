#include <headers.h>

using namespace std;

// char lines[2000000][50]; // command lines
struct Line lines[2000000];
int records[2000000][3]; // finished cycle of every stage 
list<WindowMember> window;
int rstation_busy_num[3] = {0, 0, 0};

TMSLSimulator::TMSLSimulator() {
    for (int i=0; i<12; ++i) {
        this->rStations[i].Busy = false;
        this->rStations[i].Qj = -1;
        this->rStations[i].Qk = -1;
        this->rStations[i].LineNum = -1;
        this->rStations[i].Fu = -1;
    }
    for (int i=0; i<7; ++i) {
        this->fuStates[i].owner = -1;
    }
    for (int i=0; i<33; ++i) {
        this->registers[i].stat = -1;
    }
}

void TMSLSimulator::readLines(char* inputFile) {
    this->ifname = inputFile;

    FILE *fp;
    string inputFileName = inputFile;
    inputFileName = "TestCase/" + inputFileName;
    char curInput[50];
    strcpy(curInput, inputFileName.c_str());
    fp = fopen(curInput, "r");
    if (fp == NULL) {
        printf("Open %S failed!\n", inputFileName.c_str());
        exit(-1);
    }

    int cnt = 0;
    char lineBuffer[100];
    char ops[4][15]; // ops[0] is command name
    char *ptr,*retptr;
    while (fscanf(fp, "%s", lineBuffer) != EOF) {
        ptr = lineBuffer;
        int j = 0;	 
        // separate line by commas
        while ((retptr=strtok(ptr, ",")) != NULL) {
            strcpy(ops[j], retptr);
            j++;
            ptr = NULL;
        }
        lines[this->lineNum].cmd = ops[0][0];
        switch (ops[0][0]) {
            case 'A':
                sscanf(ops[1], "R%d", &lines[this->lineNum].rg[0]);
                sscanf(ops[2], "R%d", &lines[this->lineNum].rg[1]);
                sscanf(ops[3], "R%d", &lines[this->lineNum].rg[2]);
                break;
            case 'S':
                sscanf(ops[1], "R%d", &lines[this->lineNum].rg[0]);
                sscanf(ops[2], "R%d", &lines[this->lineNum].rg[1]);
                sscanf(ops[3], "R%d", &lines[this->lineNum].rg[2]);
                break;
            case 'M':
                sscanf(ops[1], "R%d", &lines[this->lineNum].rg[0]);
                sscanf(ops[2], "R%d", &lines[this->lineNum].rg[1]);
                sscanf(ops[3], "R%d", &lines[this->lineNum].rg[2]);
                break;
            case 'D':
                sscanf(ops[1], "R%d", &lines[this->lineNum].rg[0]);
                sscanf(ops[2], "R%d", &lines[this->lineNum].rg[1]);
                sscanf(ops[3], "R%d", &lines[this->lineNum].rg[2]);
                break;
            case 'L':
                sscanf(ops[1], "R%d", &lines[this->lineNum].rg[0]);
                sscanf(ops[2], "%x", &lines[this->lineNum].op[0]);
                break;
            case 'J':
                sscanf(ops[1], "%x", &lines[this->lineNum].op[0]);
                sscanf(ops[2], "R%d", &lines[this->lineNum].rg[0]);
                sscanf(ops[3], "%x", &lines[this->lineNum].op[1]);                
                break;
            default:
                printf("Unrecognized command!\n");
                exit(-1);
                break;
        }
        this->lineNum++;
    }
}

int TMSLSimulator::doLaunchLines(int start) {
    int ret = start;
    char ops[4][15]; // ops[0] is command name
    char *ptr,*retptr;
    WindowMember cur_wm;
    bool launched = false;
    if ((rstation_busy_num[0] + rstation_busy_num[1] + rstation_busy_num[2]) >= 12) return;
    for (int i=start; i<this->lineNum; ++i) {
        Line cur = lines[i];
        if (lines[i].cmd == 'A' || lines[i].cmd == 'S' || lines[i].cmd == 'J') {
            if (rstation_busy_num[0] >= 6) continue;
            for (int k=0; k<6; ++k) {
                if (!this->rStations[k].Busy) {
                    rstation_busy_num[0]++;
                    this->rStations[k].Busy = true;
                    if (this->registers[cur.rg[1]].stat != -1) this->rStations[k].Qj = this->registers[cur.rg[1]].stat;
                    else {
                        this->rStations[k].Vj = this->registers[cur.rg[1]].value;
                        this->rStations[k].Qj = -1;
                    }
                    if (this->registers[cur.rg[2]].stat != -1) this->rStations[k].Qk = this->registers[cur.rg[2]].stat;
                    else {
                        this->rStations[k].Vk = this->registers[cur.rg[2]].value;
                        this->rStations[k].Qk = -1;
                    }
                    this->rStations[k].Op = lines[i].cmd;
                    this->registers[cur.rg[0]].stat = k;
                    this->rStations[k].LineNum = i;
                    
                    cur_wm.line_index = i;
                    cur_wm.rs_index = k;
                    window.push_back(cur_wm);

                    ret++;
                    launched = true;
                    break;
                }
            }
        } else if (lines[i].cmd == 'M' || lines[i].cmd == 'D') {
            if (rstation_busy_num[1] >= 3) continue;
            for (int k=6; k<9; ++k) {
                if (!this->rStations[k].Busy) {
                    rstation_busy_num[1]++;
                    this->rStations[k].Busy = true;
                    if (this->registers[cur.rg[1]].stat != -1) this->rStations[k].Qj = this->registers[cur.rg[1]].stat;
                    else {
                        this->rStations[k].Vj = this->registers[cur.rg[1]].value;
                        this->rStations[k].Qj = -1;
                    }
                    if (this->registers[cur.rg[2]].stat != -1) this->rStations[k].Qk = this->registers[cur.rg[2]].stat;
                    else {
                        this->rStations[k].Vk = this->registers[cur.rg[2]].value;
                        this->rStations[k].Qk = -1;
                    }
                    this->rStations[k].Op = lines[i].cmd;
                    this->registers[cur.rg[0]].stat = k;
                    this->rStations[k].LineNum = i;
                    
                    cur_wm.line_index = i;
                    cur_wm.rs_index = k;
                    window.push_back(cur_wm);

                    ret++;
                    launched = true;
                    break;
                }
            }          
        } else if (lines[i].cmd == 'L') {
            if (rstation_busy_num[2] >= 3) continue;
            for (int k=9; k<12; ++k) {
                if (!this->rStations[k].Busy) {
                    rstation_busy_num[2]++;
                    this->rStations[k].Busy = true;
                    this->rStations[k].Op = lines[i].cmd;
                    this->registers[cur.rg[0]].stat = k;
                    this->rStations[k].LineNum = i;

                    this->rStations[k].Addr = cur.op[0];
                    
                    cur_wm.line_index = i;
                    cur_wm.rs_index = k;
                    window.push_back(cur_wm);

                    ret++;
                    launched = true;
                    break;
                }
            } 
        } else if (lines[i].cmd == 'J') {
            if (rstation_busy_num[0] >= 6) continue;
            for (int k=0; k<6; ++k) {
                if (!this->rStations[k].Busy) {
                    rstation_busy_num[0]++;
                    this->rStations[k].Busy = true;
                    if (this->registers[cur.rg[0]].stat != -1) this->rStations[k].Qj = this->registers[cur.rg[0]].stat;
                    else {
                        this->rStations[k].Vj = this->registers[cur.rg[0]].value;
                        this->rStations[k].Qj = -1;
                    }
                    this->rStations[k].Op = lines[i].cmd;
                    
                    this->rStations[k].LineNum = i;
                    
                    cur_wm.line_index = i;
                    cur_wm.rs_index = k;
                    window.push_back(cur_wm);

                    ret++;
                    launched = true;
                    break;
                }
            }   
        } else {
            printf("Invalid command type in line %d\n", i);
            exit(-1);
        }
        if (rstation_busy_num[0] >= 6 && rstation_busy_num[1] >= 3 && rstation_busy_num[2] >= 3) break; 
        if (!launched) break;
        else launched = false; 
    }

    return ret;
}

void TMSLSimulator::doClocks() {
    int cur_line_to_launch = 0;
    while (true) {
        if (cur_line_to_launch >= this->lineNum) break;
        // launch
        cur_line_to_launch = this->doLaunchLines(cur_line_to_launch);
        // grab FU
        for (std::list<WindowMember>::iterator it=window.begin(); it != window.end(); ++it) {
            this->grabFU((*it).line_index, (*it).rs_index);
        }
    }
}

void TMSLSimulator::grabFU(int l, int rst) {
    ReserveStation& cur_rs = this->rStations[rst];
    Line& cur_l = lines[l];
    if (cur_rs.Fu == -1) {
        if (cur_l.cmd == 'A' || cur_l.cmd == 'S') {
            if (cur_rs.Qj == -1 && cur_rs.Qk == -1) { // Operands is ready
                for (int k = 0; k < 3; ++k) {
                    if (this->fuStates[k].owner == -1) {
                        cur_rs.Fu = k;
                        this->fuStates[k].owner = rst;
                        this->fuStates[k].cycle_todo = 4;
                        this->fuStates[k].lineIndex = l;
                        break;
                    }
                }
            }
        } else if (cur_l.cmd == 'M' || cur_l.cmd == 'D') {
            if (cur_rs.Qj == -1 && cur_rs.Qk == -1) { // Operands is ready
                for (int k = 3; k < 5; ++k) {
                    if (this->fuStates[k].owner == -1) {
                        cur_rs.Fu = k;
                        this->fuStates[k].owner = rst;
                        this->fuStates[k].cycle_todo = 5;
                        this->fuStates[k].lineIndex = l;
                        break;
                    }
                }
            }
        } else if (cur_l.cmd == 'L') {
            for (int k = 5; k < 7; ++k) {
                if (this->fuStates[k].owner == -1) {
                    cur_rs.Fu = k;
                    this->fuStates[k].owner = rst;
                    this->fuStates[k].cycle_todo = 4;
                    this->fuStates[k].lineIndex = l;
                    break;
                }
            }
        } else if (cur_l.cmd == 'J') {

        } else {
            printf("Invalid command type when grab FU in line %d\n", l);
            exit(-1);
        }
    } 
}

void TMSLSimulator::writeRecords() {

}

void TMSLSimulator::printState() {

}