#include "headers.h"

using namespace std;

// char lines[2000000][50]; // command lines
struct Line lines[2000000];
int lineRecords[2000000][3];
int records[2000000][3]; // finished cycle of every stage 
list<WindowMember> window;
int rstation_busy_num[3] = {0, 0, 0};

TMSLSimulator::TMSLSimulator() {
    this->lineNum = 0;

    for (int i=0; i<12; ++i) {
        this->rStations[i].Busy = false;
        this->rStations[i].Qj = -1;
        this->rStations[i].Qk = -1;
        this->rStations[i].LineNum = -1;
        this->rStations[i].Fu = -1;
    }
    for (int i=0; i<7; ++i) {
        this->fuStates[i].owner = -1;
        this->fuStates[i].lineIndex = -1;
    }
    for (int i=0; i<33; ++i) {
        this->registers[i].stat = -1;
    }

    this->logFile = fopen("printState.csv", "w");
    if (this->logFile == NULL) {
        printf("Open printSate.csv failed!\n");
        exit(-1);
    }

}

void TMSLSimulator::readLines(char* inputFile) {
    this->ifname = inputFile;
    this->ifn = inputFile;
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

    this->lineNum = 0;
    char lineBuffer[100];
    char ops[4][15]; // ops[0] is command name
    char *ptr,*retptr;
    while (fscanf(fp, "%s", lineBuffer) != EOF) {
        ptr = lineBuffer;
        int j = 0;	 
        // separate line by commas
        while ((retptr=strtok(ptr, ",")) != NULL) {
            strcpy(ops[j], retptr);
            printf("%s\n", ops[j]);
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
    if ((rstation_busy_num[0] + rstation_busy_num[1] + rstation_busy_num[2]) >= 12) return ret;
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
        // if (rstation_busy_num[0] >= 6 && rstation_busy_num[1] >= 3 && rstation_busy_num[2] >= 3) break; 
        // else launched = false; 
        break;
    }

    return ret;
}

void TMSLSimulator::doClocks() {
    int clock = 1;
    int cur_line_to_launch = 0;
    int old_cur_line_to_launch = 0;
    while (true) {
        // printf("%d\n",cur_line_to_launch);
        if (cur_line_to_launch >= this->lineNum && window.empty()) break;

        this->doCollect(clock);

        if (cur_line_to_launch < this->lineNum) {
            // launch
            old_cur_line_to_launch = cur_line_to_launch;
            cur_line_to_launch = this->doLaunchLines(old_cur_line_to_launch);
            for (int i = old_cur_line_to_launch; i < cur_line_to_launch; ++i) {
                lineRecords[i][0] = clock;
            }
        }

        // grab FU
        for (std::list<WindowMember>::iterator it=window.begin(); it != window.end(); ++it) {
            this->grabFU((*it).line_index, (*it).rs_index);
        }
        // do FU
        this->doFU(clock);

        // print states, used for debug
        printState(clock);

        clock++;
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
                        if (cur_l.cmd == 'D' && cur_rs.Vk == 0) this->fuStates[k].cycle_todo = 2;
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
            if (cur_rs.Qj == -1) { // Operands is ready
                for (int k = 0; k < 3; ++k) {
                    if (this->fuStates[k].owner == -1) {
                        cur_rs.Fu = k;
                        this->fuStates[k].owner = rst;
                        this->fuStates[k].cycle_todo = 2;
                        this->fuStates[k].lineIndex = l;
                        break;
                    }
                }
            }
        } else {
            printf("Invalid command type when grab FU in line %d\n", l);
            exit(-1);
        }
    } 
}

void TMSLSimulator::doFU(int clock) {
    for (int i = 0; i < 7; ++i) {
        FUState& cur_fu = this->fuStates[i];
        Line& cur_l = lines[cur_fu.lineIndex];
        if (cur_fu.owner != -1) cur_fu.cycle_todo--;
        // Actual compute?
        // TODO
        if (cur_fu.owner != -1 && cur_fu.cycle_todo == 0) {
            switch (cur_l.cmd) {
                case 'A':
                    cur_fu.result = this->rStations[cur_fu.owner].Vj + this->rStations[cur_fu.owner].Vk;
                    break;
                case 'S':
                    cur_fu.result = this->rStations[cur_fu.owner].Vj - this->rStations[cur_fu.owner].Vk;
                    break;
                case 'M':
                    cur_fu.result = this->rStations[cur_fu.owner].Vj * this->rStations[cur_fu.owner].Vk;
                    break;
                case 'D':
                    if (this->rStations[cur_fu.owner].Vk == 0) {
                        cur_fu.result = this->rStations[cur_fu.owner].Vj;
                    } else cur_fu.result = this->rStations[cur_fu.owner].Vj / this->rStations[cur_fu.owner].Vk;
                    break;
                case 'L':
                    cur_fu.result = cur_l.op[0];
                    break;
                case 'J':
                    break;
                default:
                    printf("Unrecognized command when doFU in line %d!\n", cur_fu.lineIndex);
                    exit(-1);
                    break;
            }
            lineRecords[cur_fu.lineIndex][1] = clock;
        }
    }
}

void TMSLSimulator::doCollect(int clock) {
    for (int i = 0; i < 7; ++i) {
        FUState& cur_fu = this->fuStates[i];
        if (cur_fu.owner != -1 && cur_fu.cycle_todo == 0) {
            // write records
            lineRecords[cur_fu.lineIndex][2] = clock;
            // broadcast result
            for (int k = 0; k < 12; ++k) {
                if (this->rStations[k].Qj == cur_fu.owner) {
                    this->rStations[k].Qj = -1;
                    this->rStations[k].Vj = cur_fu.result; // TODO
                } 
                if (this->rStations[k].Qk == cur_fu.owner) {
                    this->rStations[k].Qk = -1;
                    this->rStations[k].Vk = cur_fu.result; // TODO 
                }
            }
            for (int k = 0; k < 33; ++k) {
                if (this->registers[k].stat == cur_fu.owner) {
                    this->registers[k].value = cur_fu.result;
                    this->registers[k].stat = -1;
                }
            }
            // reclaim resources
            this->rStations[cur_fu.owner].Busy = false;
            if (cur_fu.owner < 6) rstation_busy_num[0]--;
            else if (cur_fu.owner < 9) rstation_busy_num[1]--;
            else rstation_busy_num[2]--;
            for (std::list<WindowMember>::iterator it=window.begin(); it != window.end(); ++it) {
                if ((*it).line_index == cur_fu.lineIndex) {
                    window.erase(it);
                    break;
                }
            }
            this->rStations[cur_fu.owner].Fu = -1;
            cur_fu.owner = -1;
            cur_fu.lineIndex = -1;
        }
    }
}

void TMSLSimulator::writeRecords() {
    FILE *fp;
    fp = fopen((this->ifn + "command_record.txt").c_str(), "w");
    if (fp == NULL) {
        printf("Open data_prepare.txt failed!\n");
        exit(-1);
    }
    for (int i = 0; i < this->lineNum; ++i) fprintf(fp, "%d %d %d\n", lineRecords[i][0], lineRecords[i][1], lineRecords[i][2]);
    fclose(fp);    
}

void TMSLSimulator::printState(int clock) {
    fprintf(this->logFile, "clock %d\n", clock);
    fprintf(this->logFile, ",Busy,Op,Vj,Vk,Qj,Qk\n");
    for (int i = 0; i < 6; ++i) {
        if (this->rStations[i].Busy) fprintf(this->logFile, "Ars%d,yes,%c,0x%x,0x%x,%d,%d\n", i, 
            this->rStations[i].Op, this->rStations[i].Vj, this->rStations[i].Vk, this->rStations[i].Qj, this->rStations[i].Qk);
        else fprintf(this->logFile, "Ars%d,no,%c,0x%x,0x%x,%d,%d\n", i,
            this->rStations[i].Op, this->rStations[i].Vj, this->rStations[i].Vk, this->rStations[i].Qj, this->rStations[i].Qk);
    }
    for (int i = 6; i < 9; ++i) {
        if (this->rStations[i].Busy) fprintf(this->logFile, "Mrs%d,yes,%c,0x%x,0x%x,%d,%d\n", i-6,
            this->rStations[i].Op, this->rStations[i].Vj, this->rStations[i].Vk, this->rStations[i].Qj, this->rStations[i].Qk);
        else fprintf(this->logFile, "Mrs%d,no,%c,0x%x,0x%x,%d,%d\n", i-6,
            this->rStations[i].Op, this->rStations[i].Vj, this->rStations[i].Vk, this->rStations[i].Qj, this->rStations[i].Qk);
    }
    fprintf(this->logFile, "\n");
    fprintf(this->logFile, ",Busy,Address\n");   
    for (int i = 9; i < 12; ++i) {
        if (this->rStations[i].Busy) fprintf(this->logFile, "LB%d,yes,0x%x\n", i-9, this->rStations[i].Addr);
        else fprintf(this->logFile, "LB%d,no,0x%x\n", i-9, this->rStations[i].Addr);
    }
    fprintf(this->logFile, "\n");

    for (int i = 1; i <= 32; ++i) fprintf(this->logFile, "R%d,", i);
    fprintf(this->logFile, "\n");
    for (int i = 1; i <= 32; ++i) fprintf(this->logFile, "%d,", this->registers[i].stat);
    fprintf(this->logFile, "\n");
    for (int i = 1; i <= 32; ++i) fprintf(this->logFile, "0x%x,", this->registers[i].value);
    fprintf(this->logFile, "\n");

    fprintf(this->logFile, ",Current Line,Operation,Cycle Remained\n");
    for (int i = 0; i < 3; ++i) {
        fprintf(this->logFile, "Add%d,%d,%c,%d\n", 
            i, this->fuStates[i].lineIndex, lines[this->fuStates[i].lineIndex].cmd, this->fuStates[i].cycle_todo);
    }

    for (int i = 3; i < 5; ++i) {
        fprintf(this->logFile, "Mult%d,%d,%c,%d\n", 
            i, this->fuStates[i].lineIndex, lines[this->fuStates[i].lineIndex].cmd, this->fuStates[i].cycle_todo);
    } 

    for (int i = 5; i < 7; ++i) {
        fprintf(this->logFile, "Load%d,%d,%c,%d\n", 
            i, this->fuStates[i].lineIndex, lines[this->fuStates[i].lineIndex].cmd, this->fuStates[i].cycle_todo);
    } 

    fprintf(this->logFile, "\n");

    // fprintf(this->logFile, "window\n");
    // for (std::list<WindowMember>::iterator it=window.begin(); it != window.end(); ++it) {
    //     fprintf(this->logFile, "%d,%d\n",(*it).line_index, (*it).rs_index);
    // }
    // fprintf(this->logFile, "\n");

    fflush(this->logFile);
}