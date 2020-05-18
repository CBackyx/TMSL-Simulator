#include "headers.h"

using namespace std;

// char lines[2000000][50]; // command lines
struct Line lines[2000000];
int lineRecords[2000000][3];
int records[2000000][3]; // finished cycle of every stage 
bool recordsWritten[2000000];

list<WindowMember> window;
int rstation_busy_num[3] = {0, 0, 0};
bool hasJump = false;

queue<int> A_FU_q;
queue<int> M_FU_q;
queue<int> L_FU_q;

int cur_line_to_launch = 0;
int old_cur_line_to_launch = 0;

TMSLSimulator::TMSLSimulator() {
    this->lineNum = 0;

    this->search_clock = false;
    this->performance = false;

    for (int i=0; i<12; ++i) {
        this->rStations[i].Busy = false;
        this->rStations[i].Qj = -1;
        this->rStations[i].Qk = -1;
        this->rStations[i].LineNum = -1;
        this->rStations[i].Fu = -1;
        this->rStations[i].Inqueue = false;
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
            // printf("%s\n", ops[j]);
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
                // printf("hhhhhhhhhhhhhhhhhhhhhh\n");
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
    for (int i = 0;i < this->lineNum; ++i) recordsWritten[i] = false;
}

int TMSLSimulator::doLaunchLines(int start) {
    int ret = start;
    char ops[4][15]; // ops[0] is command name
    char *ptr,*retptr;
    WindowMember cur_wm;
    bool launched = false;
    if ((rstation_busy_num[0] + rstation_busy_num[1] + rstation_busy_num[2]) >= 12) return ret;
    if (hasJump) return ret;
    for (int i=start; i<this->lineNum; ++i) {
        // printf("hhhhhhhhhhhhhhhhhhhhhh\n");
        Line cur = lines[i];
        if (lines[i].cmd == 'A' || lines[i].cmd == 'S') {
            if (rstation_busy_num[0] >= 6) return ret;
            for (int k=0; k<6; ++k) {
                if (!this->rStations[k].Busy) {
                    // printf("hhhhhhhhhhhhhhhhhhhhhh\n");
                    rstation_busy_num[0]++;
                    this->rStations[k].Busy = true;
                    if (this->registers[cur.rg[1]].stat != -1) {
                        this->rStations[k].Qj = this->registers[cur.rg[1]].stat;
                        this->rStations[this->registers[cur.rg[1]].stat].Q_RS.push_back(make_pair(k, 0));
                    }
                    else {
                        this->rStations[k].Vj = this->registers[cur.rg[1]].value;
                        this->rStations[k].Qj = -1;
                    }
                    if (this->registers[cur.rg[2]].stat != -1) {
                        this->rStations[k].Qk = this->registers[cur.rg[2]].stat;
                        this->rStations[this->registers[cur.rg[2]].stat].Q_RS.push_back(make_pair(k, 1));
                    }
                    else {
                        this->rStations[k].Vk = this->registers[cur.rg[2]].value;
                        this->rStations[k].Qk = -1;
                    }
                    this->rStations[k].Op = lines[i].cmd;

                    this->registers[cur.rg[0]].stat = k;
                    this->rStations[k].Q_R.push_back(cur.rg[0]);

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
            if (rstation_busy_num[1] >= 3) return ret;
            for (int k=6; k<9; ++k) {
                if (!this->rStations[k].Busy) {
                    rstation_busy_num[1]++;
                    this->rStations[k].Busy = true;
                    if (this->registers[cur.rg[1]].stat != -1) {
                        this->rStations[k].Qj = this->registers[cur.rg[1]].stat;
                        this->rStations[this->registers[cur.rg[1]].stat].Q_RS.push_back(make_pair(k, 0));
                    }
                    else {
                        this->rStations[k].Vj = this->registers[cur.rg[1]].value;
                        this->rStations[k].Qj = -1;
                    }
                    if (this->registers[cur.rg[2]].stat != -1) {
                        this->rStations[k].Qk = this->registers[cur.rg[2]].stat;
                        this->rStations[this->registers[cur.rg[2]].stat].Q_RS.push_back(make_pair(k, 1));
                    }
                    else {
                        this->rStations[k].Vk = this->registers[cur.rg[2]].value;
                        this->rStations[k].Qk = -1;
                    }
                    this->rStations[k].Op = lines[i].cmd;

                    this->registers[cur.rg[0]].stat = k;
                    this->rStations[k].Q_R.push_back(cur.rg[0]);

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
            // printf("hhhhhhhhhhhhhhhhhhhhhh\n");
            if (rstation_busy_num[2] >= 3) return ret;
            for (int k=9; k<12; ++k) {
                if (!this->rStations[k].Busy) {
                    rstation_busy_num[2]++;
                    this->rStations[k].Busy = true;
                    this->rStations[k].Op = lines[i].cmd;

                    this->registers[cur.rg[0]].stat = k;
                    this->rStations[k].Q_R.push_back(cur.rg[0]);

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
            if (rstation_busy_num[0] >= 6) return ret;
            for (int k=0; k<6; ++k) {
                if (!this->rStations[k].Busy) {
                    rstation_busy_num[0]++;
                    this->rStations[k].Busy = true;
                    if (this->registers[cur.rg[0]].stat != -1) {
                        this->rStations[k].Qj = this->registers[cur.rg[0]].stat;
                        this->rStations[this->registers[cur.rg[0]].stat].Q_RS.push_back(make_pair(k, 0));
                    }
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
                    // printf("set Jump hhhhhhhhhhhhhhhhhhhhhh\n");
                    hasJump = true;
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
    while (true) {
        // printf("clock %d\n", clock);
        // if ((cur_line_to_launch & 0xfff) == 0xfff) printf("%d\n",cur_line_to_launch);
        if (cur_line_to_launch >= this->lineNum && window.empty()) break;

        this->doCollect(clock);

        // if (!hasJump) printf("No Jump!\n");
        if (cur_line_to_launch < this->lineNum) {
            // launch
            if (!hasJump) {
                old_cur_line_to_launch = cur_line_to_launch;
                cur_line_to_launch = this->doLaunchLines(old_cur_line_to_launch);
                for (int i = old_cur_line_to_launch; i < cur_line_to_launch; ++i) {
                    if (!recordsWritten[i]) {
                        lineRecords[i][0] = clock;
                    }
                }
            }
        }

        // grab FU, launched time order
        for (std::list<WindowMember>::iterator it=window.begin(); it != window.end(); ++it) {
            this->grabFU((*it).line_index, (*it).rs_index);
        }

        // deque FU, ready order
        this->dequeFU();

        // do FU
        this->doFU(clock);

        // print states, used for debug
        if (this->search_clock && this->t_clock == clock) {
            this->printState(clock);
        } 

        clock++;
    }

    if (clock <= this->t_clock) {
        printf("The searched clock is out of bound!/n");
        exit(-1);
    }
}

void TMSLSimulator::grabFU(int l, int rst) {
    ReserveStation& cur_rs = this->rStations[rst];
    Line& cur_l = lines[l];
    if (cur_rs.Fu == -1 && !cur_rs.Inqueue) {
        if (cur_l.cmd == 'A' || cur_l.cmd == 'S') {
            if (cur_rs.Qj == -1 && cur_rs.Qk == -1) { // Operands is ready
                cur_rs.Inqueue = true;
                A_FU_q.push(rst);
            }
        } else if (cur_l.cmd == 'M' || cur_l.cmd == 'D') {
            if (cur_rs.Qj == -1 && cur_rs.Qk == -1) { // Operands is ready
                cur_rs.Inqueue = true;
                M_FU_q.push(rst);
            }
        } else if (cur_l.cmd == 'L') {
            cur_rs.Inqueue = true;
            L_FU_q.push(rst);            
        } else if (cur_l.cmd == 'J') {
            if (cur_rs.Qj == -1) { // Operands is ready
                cur_rs.Inqueue = true;
                A_FU_q.push(rst);   
            }
        } else {
            printf("Invalid command type when grab FU in line %d\n", l);
            exit(-1);
        }
    }
}

void TMSLSimulator::dequeFU() {
    // deque
    for (int k = 0; k < 3; ++k) {
        if (this->fuStates[k].owner == -1) {
            int crst = 0;
            if (!A_FU_q.empty()) {
                crst = A_FU_q.front();
                A_FU_q.pop();
            } else break;
            ReserveStation& cur_rs = this->rStations[crst];
            cur_rs.Fu = k;
            cur_rs.Inqueue = false;
            this->fuStates[k].owner = crst;
            this->fuStates[k].cycle_todo = 4;
            if (cur_rs.Op == 'J') this->fuStates[k].cycle_todo = 2;
            this->fuStates[k].lineIndex = cur_rs.LineNum;
        }
    } 

    for (int k = 3; k < 5; ++k) {
        if (this->fuStates[k].owner == -1) {
            int crst = 0;
            if (!M_FU_q.empty()) {
                crst = M_FU_q.front();
                M_FU_q.pop();
            } else break;
            ReserveStation& cur_rs = this->rStations[crst];
            cur_rs.Fu = k;
            cur_rs.Inqueue = false;
            this->fuStates[k].owner = crst;
            this->fuStates[k].cycle_todo = 5;
            if (cur_rs.Op == 'D' && cur_rs.Vk == 0) this->fuStates[k].cycle_todo = 2;
            this->fuStates[k].lineIndex = cur_rs.LineNum;
        }
    }

    for (int k = 5; k < 7; ++k) {
        if (this->fuStates[k].owner == -1) {
            int crst = 0;
            if (!L_FU_q.empty()) {
                crst = L_FU_q.front();
                L_FU_q.pop();
            } else break;
            ReserveStation& cur_rs = this->rStations[crst];
            cur_rs.Fu = k;
            cur_rs.Inqueue = false;
            this->fuStates[k].owner = crst;
            this->fuStates[k].cycle_todo = 4;
            this->fuStates[k].lineIndex = cur_rs.LineNum;
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
                    if (cur_l.op[0] == this->rStations[cur_fu.owner].Vj) cur_fu.result = 1;
                    else cur_fu.result = 2;
                    break;
                default:
                    printf("Unrecognized command when doFU in line %d!\n", cur_fu.lineIndex);
                    exit(-1);
                    break;
            }
            if (!recordsWritten[cur_fu.lineIndex]) {
                lineRecords[cur_fu.lineIndex][1] = clock;
            }
        }
    }
}

void TMSLSimulator::doCollect(int clock) {
    for (int i = 0; i < 7; ++i) {
        FUState& cur_fu = this->fuStates[i];
        if (cur_fu.owner != -1 && cur_fu.cycle_todo == 0) {
            ReserveStation& cur_rs = this->rStations[cur_fu.owner];
            // write records
            if (!recordsWritten[cur_fu.lineIndex]) {
                lineRecords[cur_fu.lineIndex][2] = clock;
                recordsWritten[cur_fu.lineIndex] = true;
            }
            // broadcast result
            int q_rs_size = cur_rs.Q_RS.size();
            int q_r_size = cur_rs.Q_R.size();

            for (int k = 0; k < q_rs_size; ++k) {
                if (cur_rs.Q_RS[k].second == 0) {
                    this->rStations[cur_rs.Q_RS[k].first].Qj = -1;
                    this->rStations[cur_rs.Q_RS[k].first].Vj = cur_fu.result;
                } else {
                    this->rStations[cur_rs.Q_RS[k].first].Qk = -1;
                    this->rStations[cur_rs.Q_RS[k].first].Vk = cur_fu.result;                    
                }
            }

            for (int k = 0; k < q_r_size; ++k) {
                if (this->registers[cur_rs.Q_R[k]].stat == cur_fu.owner) {
                    this->registers[cur_rs.Q_R[k]].stat = -1;
                    this->registers[cur_rs.Q_R[k]].value = cur_fu.result;
                }
            }

            // for (int k = 0; k < 12; ++k) {
            //     if (this->rStations[k].Qj == cur_fu.owner) {
            //         this->rStations[k].Qj = -1;
            //         this->rStations[k].Vj = cur_fu.result; // TODO
            //     } 
            //     if (this->rStations[k].Qk == cur_fu.owner) {
            //         this->rStations[k].Qk = -1;
            //         this->rStations[k].Vk = cur_fu.result; // TODO 
            //     }
            // }
            // for (int k = 0; k < 33; ++k) {
            //     if (this->registers[k].stat == cur_fu.owner) {
            //         this->registers[k].value = cur_fu.result;
            //         this->registers[k].stat = -1;
            //     }
            // }

            cur_rs.Q_R.clear();
            cur_rs.Q_RS.clear();

            if (lines[cur_fu.lineIndex].cmd == 'J') {
                // if is Jump inst
                hasJump = false;
                if (cur_fu.result == 1) {
                    // Jump success
                    cur_line_to_launch += (-1 + lines[cur_fu.lineIndex].op[1]);
                } else if (cur_fu.result == 2) {
                    // Jump fail
                    // did nothing
                } else {
                    printf("Unexpected Jump inst reult: %d\n", cur_fu.result);
                    exit(-1);
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
    fprintf(this->logFile, ",Busy,Op,Vj,Vk,Qj,Qk,FU,Line\n");
    for (int i = 0; i < 6; ++i) {
        if (this->rStations[i].Busy) fprintf(this->logFile, "Ars%d,yes,%c,0x%x,0x%x,%d,%d,%d,%d\n", i, 
            this->rStations[i].Op, this->rStations[i].Vj, this->rStations[i].Vk, this->rStations[i].Qj, this->rStations[i].Qk, this->rStations[i].Fu, this->rStations[i].LineNum);
        else fprintf(this->logFile, "Ars%d,no,%c,0x%x,0x%x,%d,%d,%d,%d\n", i,
            this->rStations[i].Op, this->rStations[i].Vj, this->rStations[i].Vk, this->rStations[i].Qj, this->rStations[i].Qk, this->rStations[i].Fu, this->rStations[i].LineNum);
    }
    for (int i = 6; i < 9; ++i) {
        if (this->rStations[i].Busy) fprintf(this->logFile, "Mrs%d,yes,%c,0x%x,0x%x,%d,%d,%d,%d\n", i-6,
            this->rStations[i].Op, this->rStations[i].Vj, this->rStations[i].Vk, this->rStations[i].Qj, this->rStations[i].Qk, this->rStations[i].Fu, this->rStations[i].LineNum);
        else fprintf(this->logFile, "Mrs%d,no,%c,0x%x,0x%x,%d,%d,%d,%d\n", i-6,
            this->rStations[i].Op, this->rStations[i].Vj, this->rStations[i].Vk, this->rStations[i].Qj, this->rStations[i].Qk, this->rStations[i].Fu, this->rStations[i].LineNum);
    }
    fprintf(this->logFile, "\n");
    fprintf(this->logFile, ",Busy,Address\n");   
    for (int i = 9; i < 12; ++i) {
        if (this->rStations[i].Busy) fprintf(this->logFile, "LB%d,yes,0x%x\n", i-9, this->rStations[i].Addr);
        else fprintf(this->logFile, "LB%d,no,0x%x\n", i-9, this->rStations[i].Addr);
    }
    fprintf(this->logFile, "\n");

    for (int i = 0; i <= 31; ++i) fprintf(this->logFile, "R%d,", i);
    fprintf(this->logFile, "\n");
    for (int i = 0; i <= 31; ++i) fprintf(this->logFile, "%d,", this->registers[i].stat);
    fprintf(this->logFile, "\n");
    for (int i = 0; i <= 31; ++i) fprintf(this->logFile, "0x%x,", this->registers[i].value);
    fprintf(this->logFile, "\n");

    fprintf(this->logFile, "\n");


    fprintf(this->logFile, ",Current Line,Operation,Cycle Remained,RStation\n");
    for (int i = 0; i < 3; ++i) {
        fprintf(this->logFile, "Add%d,%d,%c,%d,%d\n", 
            i, this->fuStates[i].lineIndex, lines[this->fuStates[i].lineIndex].cmd, this->fuStates[i].cycle_todo, this->fuStates[i].owner);
    }

    for (int i = 3; i < 5; ++i) {
        fprintf(this->logFile, "Mult%d,%d,%c,%d,%d\n", 
            i, this->fuStates[i].lineIndex, lines[this->fuStates[i].lineIndex].cmd, this->fuStates[i].cycle_todo, this->fuStates[i].owner);
    } 

    for (int i = 5; i < 7; ++i) {
        fprintf(this->logFile, "Load%d,%d,%c,%d,%d\n", 
            i, this->fuStates[i].lineIndex, lines[this->fuStates[i].lineIndex].cmd, this->fuStates[i].cycle_todo, this->fuStates[i].owner);
    } 

    fprintf(this->logFile, "\n");

    // fprintf(this->logFile, "window\n");
    // for (std::list<WindowMember>::iterator it=window.begin(); it != window.end(); ++it) {
    //     fprintf(this->logFile, "%d,%d\n",(*it).line_index, (*it).rs_index);
    // }
    // fprintf(this->logFile, "\n");

    // fprintf(this->logFile, "window\n");
    // if (!A_FU_q.empty()) fprintf(this->logFile, "A %d\n", A_FU_q.front());
    // if (!M_FU_q.empty()) fprintf(this->logFile, "M %d\n", M_FU_q.front());
    // if (!L_FU_q.empty()) fprintf(this->logFile, "L %d\n", L_FU_q.front());

    fprintf(this->logFile, "\n");    

    fflush(this->logFile);
}