#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include <random>
#include <iomanip>
#include <cxxopts.hpp>

using namespace std;

void printLine(string str1, string str2){
   cout<<"Here is the "<<str1<<" list, with the "<<str2<<" time of each job:"<<endl;
}

void generateRandomJobs(int seed,int maxLen,int jNum,vector<pair<int,string>>& jobList){
    
    mt19937 gen(seed);
    uniform_int_distribution<> dis(1, maxLen);

    for(int i=0; i<jNum; i++){
       string randJobs = to_string(dis(gen));
       cout<<"Job"<<" "<<i<<" "<<"(length = "<<randJobs<<")"<<endl;
       jobList.push_back({i,randJobs});
    }
}

vector<pair<int,string>> parseString(const string& List){

    vector<pair<int,string>> parseList;
    string num = "";
    int count = 0;

    for(int i=0;i<List.size();i++){
        if(List[i] != ','){
           num += List[i];
        }
        else{
            parseList.push_back({count,num});
            num = "";
            count++;
        } 
    }
    parseList.push_back({count,num});

    return parseList;
}

void printList(const vector<pair<int,string>>& list,string str){
    for(int i=0; i<list.size(); i++){
        cout<<"Job"<<" "<<i<<" "<<"("<<str<<" = "<<list[i].second<<")"<<endl;
    }
}

void checkArrive(string aList,vector<pair<int,string>>& arrive,vector<pair<int,string>>& jobList){

   if(aList == "") cout<<"All jobs (Arrive at = 0)"<<endl;
   else {
        arrive = parseString(aList);
        if(arrive.size() == jobList.size()) {
            printLine("arrival","arrive");
            printList(arrive,"arrive at");
        }
        else cout<<"\nNum of jobs not same as arrive's num.\n";
    }

}

void sortArriveTime(vector<pair<int,string>>& arrive){
    sort(arrive.begin(),arrive.end(),[](const auto& a,const auto& b){
        return stoi(a.second) < stoi(b.second);
    });
}

/*-----------------------------------------------------FIFO--------------------------------------------------------*/
void FIFO(const vector<pair<int,string>>& jobList,const vector<pair<int,string>>& arrive,vector<float>& Response,vector<float>& Turnaround,vector<float>& Wait){

    int ST=0,AT=0;

    for(int i=0; i<jobList.size(); i++){

        float runtime = stof(jobList[i].second);
        if(arrive.size() != 0) AT = stoi(arrive[i].second);
        if(arrive.size() != 0 && AT > ST) ST = AT;

        cout<<"  [time  "<<ST<<"] "<<"Run job "<<jobList[i].first<<" for "<<fixed<<setprecision(2)<<runtime<<" secs (arrive at "
        <<AT<<")"<<endl;
        
        Response.push_back(ST - AT); // Response = cpu access 1st - arrival time
        Turnaround.push_back((ST+runtime) - AT); // Turnaround = Completion time - arrival time
        Wait.push_back(((ST+runtime) - AT) - runtime); // Wait = TAT - runtime
        ST+=runtime; // Update cpu ST (Start time)
    }

}

/*-----------------------------------------------------RR-------------------------------------------------------*/
void RR(const vector<pair<int,string>>& jobList,const vector<pair<int,string>>& arrive,vector<float>& Response,vector<float>& Turnaround,vector<float>& Wait,int quantum){
    
    queue<pair<int,int>> readyQ;
    vector<int> jobLength(jobList.size(),0);
    int runFor=0 , AT=0 , count=1 , firstJob=jobList[0].first;
    
    if(arrive.size() != 0) runFor = stoi(arrive[firstJob].second);
    Turnaround = vector<float>(jobList.size(),0.0);
    Response = vector<float>(jobList.size(),-1);
    Wait = vector<float>(jobList.size(),0.0); 
    readyQ.push({jobList[0].first,stoi(jobList[0].second)});

    while(!readyQ.empty()){

        pair<int,int> firstElem = readyQ.front();
        int jobNum = firstElem.first;
        int runtime = firstElem.second;
        readyQ.pop();
    
        if(arrive.size() != 0) AT = stoi(arrive[jobNum].second);
        if(jobLength[jobNum] == 0) jobLength[jobNum] = runtime;
        if(Response[jobNum] == -1) Response[jobNum] = runFor - AT; // 1st cpu access - arrival
        
        cout<<"  [time  "<<runFor;

        if(runtime > quantum){
            runtime -= quantum;
            runFor += quantum;
        }
        else{
            runFor += runtime;
            runtime = 0;
            Turnaround[jobNum] = runFor - AT; // completion - arrival
            Wait[jobNum] = (runFor - AT) - jobLength[jobNum]; // TAT - jobLength
        }
        
        for(int i=count; i<jobList.size();){
            int arrTime = 0, jN = jobList[i].first;
            if(arrive.size() != 0) arrTime =  stoi(arrive[jN].second);
            if(arrTime <= runFor){
                readyQ.push({jN,stoi(jobList[i].second)});
                i = ++count;
            }
            else break;
        }

        if(runtime != 0) readyQ.push({jobNum,runtime});

       cout<<" - "<<runFor<<"] "<<" Run job "<<jobNum<<" (Arrive at = "<<AT<<")"<<endl;
        
    }
}

int main(int argc, char* argv[]){

    try{
        cxxopts::Options options("JobScheduler", "Simulates a job scheduler with various policies");
        options.add_options()
            ("s,seed", "The random seed", cxxopts::value<int>()->default_value("0"))
            ("j,jobs", "Number of jobs in the system", cxxopts::value<int>()->default_value("3"))
            ("l,jlist", "Comma-separated list of job runtimes", cxxopts::value<string>()->default_value(""))
            ("a,aList","Comma-separated arrival time of each job",cxxopts::value<string>()->default_value(""))
            ("m,maxlen", "Max length of a job", cxxopts::value<int>()->default_value("10"))
            ("p,policy", "Scheduling policy to use (SJF, FIFO, RR)", cxxopts::value<string>()->default_value("FIFO"))
            ("q,quantum", "Time slice length for RR policy", cxxopts::value<int>()->default_value("1"))
            ("c,compute", "Compute answers for me", cxxopts::value<bool>()->default_value("false"))
            ("h,help", "Print help");

        auto result = options.parse(argc,argv);

        if(result.count("help")){
          cout<<options.help()<<endl;
          return 0;
        }
        
        int seed = result["seed"].as<int>();
        int jobs = result["jobs"].as<int>();
        string jList = result["jlist"].as<string>();
        string aList = result["aList"].as<string>();
        int maxLen = result["maxlen"].as<int>();
        string policy = result["policy"].as<string>();
        int quantum = result["quantum"].as<int>();
        bool compute = result["compute"].as<bool>();

        cout<<"ARG Policy "<<policy<<endl;
        vector<pair<int,string>> jobList;
        vector<pair<int,string>> arrive;
        
        if(jList == ""){
           cout<<"ARG jobs "<<jobs<<endl;
           cout<<"ARG maxlen "<<maxLen<<endl;
           cout<<"ARG seed "<<seed<<"\n\n";
           printLine("job","run");
           generateRandomJobs(seed,maxLen,jobs,jobList);
           checkArrive(aList,arrive,jobList);
        }
        else {
           cout<<"ARG jlist "<<jList<<"\n\n";
           printLine("job","run");
           jobList = parseString(jList);
           printList(jobList,"length");
           checkArrive(aList,arrive,jobList);
        }
        
        if(compute && (arrive.size() == 0 || (arrive.size() == jobList.size()))){

            cout<<"\n** Solutions **\n\n";
            cout<<"Execution trace:"<<endl;

            vector<float> Turnaround, Response, Wait;
            float avgTAT=0, avgRT=0, avgWT=0                                                                                                                                                ;
            int jNum = jobList.size();

            if(policy == "FIFO"){

                if(arrive.size() != 0){

                    // sort jobs as per arrival time 1st in 1st out
                    sort(jobList.begin(),jobList.end(),[&](const auto& a, const auto& b){
                        return stoi(arrive[a.first].second) < stoi(arrive[b.first].second);
                    });
                    sortArriveTime(arrive); // sort arrive time 
                }

                FIFO(jobList,arrive,Response,Turnaround,Wait);
            }
            else if(policy == "SJF"){

                if(arrive.size() == 0){

                    // sort as per job shortest 1st (all arrive at 0)
                    sort(jobList.begin(),jobList.end(),[](const auto& a, const auto& b){
                       return stoi(a.second) < stoi(b.second);
                    });
                }

                else{
                    // sort job as per arrive time n shortest 1st if same arrive time
                    sort(jobList.begin(),jobList.end(),[&](const auto& a, const auto& b){
                        if(arrive[a.first].second == arrive[b.first].second) return stoi(a.second) < stoi(b.second);
                        else return stoi(arrive[a.first].second) < stoi(arrive[b.first].second);
                    });
                    sortArriveTime(arrive);
                }

                FIFO(jobList,arrive,Response,Turnaround,Wait);
            }
            else if(policy == "RR"){
                
                if(arrive.size() != 0){

                    // sort jobs as per arrival time 1st in 1st out
                    sort(jobList.begin(),jobList.end(),[&](const auto& a, const auto& b){
                        return stoi(arrive[a.first].second) < stoi(arrive[b.first].second);
                    });
                }
                RR(jobList,arrive,Response,Turnaround,Wait,quantum);
            }

            
            if(policy == "FIFO" || policy == "SJF" || policy == "RR"){
               
               cout<<"\nFinal statistics:\n";
               int jobNum;

               for(int i=0; i<jobList.size(); i++){
                   if(policy == "RR") jobNum = i;
                   else jobNum = jobList[i].first;
                   cout<<"  Job  "<<jobNum<<" -- Response: "<<Response[i]<<"  Turnaround "<<Turnaround[i]<<"  Wait "<<
                   Wait[i]<<endl;
                   avgTAT+=Turnaround[i];
                   avgRT+=Response[i];
                   avgWT+=Wait[i];
                }
  
               cout<<"\n  Average -- Response: "<<(avgRT/jNum)<<"  Turnaround "<<(avgTAT/jNum)<<"  Wait "<<(avgWT/jNum)<<endl;
            }
            else cout<<"Error: Policy "<<policy<<" is not available."<<endl;

        }


    }
    catch(const cxxopts::exceptions::exception& e){
       cerr<<"Parsing err: "<<e.what()<<endl;
       return 1;
    }
    catch(exception& e){
       cerr<<"General err: "<<e.what()<<endl;
       return 1;
    }
    
}