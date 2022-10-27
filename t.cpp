#include <string> // c_str()
#include <string.h> //strtok()
#include <unistd.h> // write(), pipe()
#include <stdlib.h> // setenv()
#include <iostream>
#include <sys/wait.h> //wait(), waitpid()
#include <vector>
#include <fcntl.h>
#include <algorithm> //remove_if

#define READ_END 0
#define WRITE_END 1



using namespace std;


void SplitToVectorHaveDelimeter(vector<string> &v, string lineString, string delim);
void SplitToVectorNotHaveDelimeter(vector<string> &v, string lineString, string delim);
void vectorPrint(vector<string> &words);
void initialize();
void createPipe(int (&)[2]);
void pipeConnect_LtoR(vector<pid_t> &instrs_pids, int (&last_pipe)[2], int (&otherCommendPipe)[2], char **instrP, bool pipeStdErr);
void pipeEnd_LtoR(vector<pid_t> &instrs_pids, int (&last_pipe)[2], char **instrP);
void directToWriteFile(vector<pid_t> &instrs_pids, int (&last_pipe)[2], char **instrP, string writeFileName);
string cleanWhiteString(string s);
bool is_number(const std::string& s);


void childHandler(int signo);

char** stringToDoubleChar(vector<string> &v);

struct pipeDelayNum_t{
    int fd[2];
    pid_t pid;
    int count;
};


int main(int argc,char** argv){
    extern char **environ; //enviroment variable list
    bool isExit = false;
    vector<pipeDelayNum_t> pipeDelayNums;


    signal(SIGCHLD,childHandler);
    initialize();

    while(!isExit){
        string input_line;
        write(1, "% ", 2);      
        getline(std::cin, input_line);

        //handle input is \n
        if(input_line.size() == 0){
            continue;
        }

        //convert a line string to instructions
        vector<string> instrs;
        vector<pid_t> instrs_pids;                
        SplitToVectorHaveDelimeter(instrs, input_line, ">|!");
        //!vectorPrint(instrs);
        
        int instrs_index = 0;
        int last_pipe[2];   
        int otherCommendPipe[2];
        bool directToFile = false;

        last_pipe[READ_END] = 0, last_pipe[WRITE_END] = 0;   
        otherCommendPipe[READ_END] = 0, otherCommendPipe[WRITE_END] = 0;

        //count delayPipe
        for(int i = 0; i < pipeDelayNums.size(); i++){
            pipeDelayNums.at(i).count--;
        }
        for(int i = 0; i < pipeDelayNums.size(); i++){
            //!cout << "index: " << i <<" count: " << pipeDelayNums.at(i).count << endl;
            if(pipeDelayNums.at(i).count == 0){
               //! cout << "count = 0" << endl; 
                last_pipe[READ_END] = pipeDelayNums.at(i).fd[READ_END];
                last_pipe[WRITE_END] = pipeDelayNums.at(i).fd[WRITE_END];
                pipeDelayNums.erase(pipeDelayNums.begin() + i);
                break;
            }
        }
            
        while(instrs_index < instrs.size()){                   
            //each loop handle one instruction. 
            bool pipeStdErr = false;           
            //!cout << "instr.:" << instrs.at(instrs_index) << endl;
            
            //convert "instrion expresion" to "instr and option"
            vector<string> instr;
            SplitToVectorNotHaveDelimeter(instr, instrs.at(instrs_index), " ");
            
            //special instruction   
            if(instr[0].compare("exit") == 0){
                isExit = true;
                break;
            }else if(instr[0].compare("setenv") == 0){
                setenv(instr.at(1).c_str(), instr.at(2).c_str(), 1);
                break;
            }else if(instr[0].compare("printenv") == 0){             
                
                
                cout << getenv(instr.at(1).c_str())  << endl;
                break;
            }

            //Options is String , so we need to convert them to char *const arr[].
            char **instrP;
            instrP = stringToDoubleChar(instr);         
            

            //start handle this instruction
            if((instrs_index + 1 < instrs.size())){
                //cout << "stage1 " << instr.at(instrs_index) << " index "  << instrs_index << "next: " << instrs.at(instrs_index + 1) <<endl;
                if(instrs.at(instrs_index + 1).compare("|") == 0 || instrs.at(instrs_index + 1).compare("!") == 0){
                    if(instrs.at(instrs_index + 1).compare("!") == 0){
                        pipeStdErr = true;
                    }

                    if(is_number(cleanWhiteString(instrs.at(instrs_index + 2)))){
                        //it is |nubmer => it must be in end. 
                        int number = atoi(instrs.at(instrs_index + 2).c_str());
                        
                        bool needAddToDelayNums = true;
                        for(int i = 0; i < pipeDelayNums.size(); i++){
                            if(pipeDelayNums.at(i).count == number){
                                otherCommendPipe[READ_END] = pipeDelayNums.at(i).fd[READ_END];
                                otherCommendPipe[WRITE_END] = pipeDelayNums.at(i).fd[WRITE_END];
                                pipeConnect_LtoR(instrs_pids, last_pipe, otherCommendPipe, instrP, pipeStdErr);
                                needAddToDelayNums = false;
                                break;
                            }
                        }
                        if(needAddToDelayNums){
                            //!cout << "add to vector: " << number << endl;
                            pipeDelayNum_t newPipeDelayNum;
                            pipeConnect_LtoR(instrs_pids, last_pipe, otherCommendPipe, instrP, pipeStdErr);
                            newPipeDelayNum.fd[READ_END] = last_pipe[READ_END];
                            newPipeDelayNum.fd[WRITE_END] = last_pipe[WRITE_END];
                            newPipeDelayNum.pid = instrs_pids.at(instrs_pids.size() - 1);
                            instrs_pids.pop_back();
                            newPipeDelayNum.count = number;
                            pipeDelayNums.push_back(newPipeDelayNum);
                            //!cout << "number:" << number << endl;                           
                        }

                        break;
                    }else{
                        // not |number                         
                        pipeConnect_LtoR(instrs_pids, last_pipe, otherCommendPipe, instrP, pipeStdErr);
                    }            

                }else if(instrs.at(instrs_index + 1).compare(">") == 0){
                    // ">" must be in end.
                    //!cout << "stage <" << endl;
                    directToWriteFile(instrs_pids, last_pipe, instrP, cleanWhiteString(instrs.at(instrs_index + 2)));
                    break;
                }
                instrs_index += 2;                                   
            }else{
                //end instruction
                //!cout << "stage2 " << instrs_index <<endl;
                pipeEnd_LtoR(instrs_pids, last_pipe, instrP);
                instrs_index += 1;
            }
        }

    }

    return 0;
}

void directToWriteFile(vector<pid_t> &instrs_pids, int (&last_pipe)[2], char **instrP, string writeFileName){
    pid_t pid;

    if((pid = fork()) < 0){
        perror("fork error");
    }else if(pid == 0){
        //child
        if(last_pipe[READ_END] != 0 && last_pipe[WRITE_END] != 0){ 
            //mean that last pipe exist.
            close(last_pipe[WRITE_END]);

            dup2(last_pipe[READ_END], fileno(stdin));
            close(last_pipe[READ_END]);                                                
        }

        dup2(open(writeFileName.c_str(), O_CREAT |  O_TRUNC | O_WRONLY, 0666), fileno(stdout));        
        execvp(instrP[0], instrP);
        
        string str(instrP[0]);
        cout << "Unknown command: [" << str << "]." << endl;
        exit(0);
    }else{
        //parent
        if(last_pipe[READ_END] != 0 && last_pipe[WRITE_END] != 0){ 
            //mean that last pipe exist.
            close(last_pipe[READ_END]);
            close(last_pipe[WRITE_END]);
        }

        instrs_pids.push_back(pid);

        int status;
        waitpid(pid, &status, 0);
    }
}

int c = 0;

void pipeConnect_LtoR(vector<pid_t> &instrs_pids, int (&last_pipe)[2], int (&otherCommendPipe)[2], char **instrP, bool pipeStdErr){
    //!cout << "pipeConnect" << endl;
    int new_pipe[2];
    if(otherCommendPipe[READ_END] == 0 && otherCommendPipe[WRITE_END] == 0){
        createPipe(new_pipe);
        //c++;
        //cout << c << endl;
        //cout << "W" << new_pipe[0] << "R" << new_pipe[1] << endl;
    }else{        
    
        new_pipe[READ_END] = otherCommendPipe[READ_END];
        new_pipe[WRITE_END] = otherCommendPipe[WRITE_END];
    }

    pid_t pid;

    if((pid = fork()) < 0){
        //perror("fork error(Connect)");
        waitpid(-1, NULL, 0);
    }else if(pid == 0){
        //child
        if(last_pipe[READ_END] != 0 && last_pipe[WRITE_END] != 0){ 
            //mean that last pipe exist.
            close(last_pipe[WRITE_END]);

            dup2(last_pipe[READ_END], fileno(stdin));
            close(last_pipe[READ_END]);                                                
        }

        close(new_pipe[READ_END]);
        dup2(new_pipe[WRITE_END], fileno(stdout));
        if(pipeStdErr){            
            dup2(new_pipe[WRITE_END], fileno(stderr));
        }
        close(new_pipe[WRITE_END]);

        execvp(instrP[0], instrP);
        
        string str(instrP[0]);
        cerr << "Unknown command: [" << str << "]." << endl;
        exit(0);
    }else{
        //parent
        if(last_pipe[READ_END] != 0 && last_pipe[WRITE_END] != 0){ 
            //mean that last pipe exist.
            close(last_pipe[READ_END]);
            close(last_pipe[WRITE_END]);
        }

        instrs_pids.push_back(pid);

        last_pipe[READ_END] = new_pipe[READ_END];
        last_pipe[WRITE_END] = new_pipe[WRITE_END];
    }
}

void pipeEnd_LtoR(vector<pid_t> &instrs_pids, int (&last_pipe)[2], char **instrP){
    // don't ceate new pipe
    pid_t pid;

    if((pid = fork()) < 0){
        perror("fork error");
    }else if(pid == 0){
        //child
        if(last_pipe[READ_END] != 0 && last_pipe[WRITE_END] != 0){ 
            //mean that last pipe exist.
            close(last_pipe[WRITE_END]);
            dup2(last_pipe[READ_END], fileno(stdin));
            close(last_pipe[READ_END]);
        }

        execvp(instrP[0], instrP);
        
        string str(instrP[0]);
        cerr << "Unknown command: [" << str << "]." << endl;
        exit(0);
    }else{
        //parent
        if(last_pipe[READ_END] != 0 && last_pipe[WRITE_END] != 0){ 
            //mean that last pipe exist.
            close(last_pipe[READ_END]);
            close(last_pipe[WRITE_END]);
        }

        instrs_pids.push_back(pid);

        int status;
        //!cout << "ending" << endl;
        waitpid(pid, &status, 0);
    }
}


void createPipe(int (&fd)[2]){
    if(pipe(fd) < 0){
        perror("pipe create error");
    }
}

void initialize(){
    setenv("PATH", "bin:.", 1);
}

void SplitToVectorHaveDelimeter(vector<string> &v, string lineString, string delim){
    //delim also be saved in vector.
    bool run = true;
    while(run){
        int max_index = INT8_MAX;
        string char_delim;

        for(int j = 0; j < delim.size();  j++){
            int find_index = lineString.find(delim.at(j));
            //!cout << "find_index " << find_index << endl;
            if(max_index > find_index && find_index != string::npos){
                max_index = find_index;
                char_delim = delim.at(j); 
            }
        }

        if(max_index != INT8_MAX){
            //!cout << max_index << endl;
            v.push_back(lineString.substr(0, max_index));
            lineString = lineString.substr(max_index + 1);
            //!cout << "lineString" << lineString << endl;
            v.push_back(char_delim);
        }else{
            run = false;
        }

    }
    v.push_back(lineString);
    
}

void SplitToVectorNotHaveDelimeter(vector<string> &v, string lineString, string delim){
    char *token;
   
    char *cStr = new char[lineString.size() + 1];
    strcpy(cStr, lineString.c_str());
   
    token = strtok(cStr, delim.c_str());
    while(token != NULL){
        string str(token);
        v.push_back(str);
        token = strtok(NULL, delim.c_str());
    }
}

char** stringToDoubleChar(vector<string> &v){
    // last element is NULL
    char** instrP;
    instrP = (char**)malloc(sizeof(char*) * (v.size() + 1));
    for(int i = 0; i < v.size(); i++){
        instrP[i] = (char*)malloc(sizeof(char) * (v.at(i).size() + 1));
        strcpy(instrP[i], v.at(i).c_str());        
    }

    instrP[v.size()] = NULL;

    return instrP;
}

void vectorPrint(vector<string> &words){
    for(int i = 0; i < words.size(); i++){
        write(0, words.at(i).c_str(), words.at(i).size()); // need c_str() to make string convert to const void*
        write(0, "\n", 1);
    }
}

void childHandler(int signo){
    int status;
    while(waitpid(-1, &status, WNOHANG) > 0){

    }
}

bool is_number(const std::string& s){
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

string cleanWhiteString(string s){
    s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());    
    return s;
}
