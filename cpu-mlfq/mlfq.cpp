#include <iostream>
#include <cxxopts.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include <deque>
#include <iomanip>
#include <random>

using namespace std;

struct Job{
    int jobNum;
    int startTime;
    int runTime;
    int ioFreq;
    int timeLeft;
    int qLevel;
    int allot;
    int ticksLeft;
    int response;
    int completion;
    int wait;
};

struct Pair{
    int jobN;
    string jobTy;
};

void generateRandomJobs(int seed,int maxLen,int maxIO,int jobsNum,vector<Job>& jobList,int numQueues,const vector<int>& allotList,const vector<int>& quantList,unordered_map<int,vector<Pair>>& jobArrive){

    mt19937 gen(seed);
    uniform_int_distribution<> disLen(1, maxLen);
    uniform_int_distribution<> disIO(1, maxIO);

    for(int i=0; i<jobsNum; i++){
      int runTime = disLen(gen);
      int ioFreq = disIO(gen);
      jobList.push_back({i,0,runTime,ioFreq,runTime,numQueues-1,allotList[numQueues-1],quantList[numQueues-1],-1,0,0});
      jobArrive[0].push_back({i,"JOB BEGINS"});
    }
}

vector<int> parseString(string& list){
    
    vector<int> parsedList;
    string num = "";

    for(int i=0; i<list.size(); i++){
        if(list[i] != ','){
           num+=list[i];
        }
        else{
          parsedList.push_back(stoi(num));
          num = "";
        }
    }
    parsedList.push_back(stoi(num));

    reverse(parsedList.begin(),parsedList.end()); //reverse arr for correct looping
    return parsedList;
}

int parseJobList(string jlist,vector<Job>& parsedJList,int numQueues,const vector<int>& allotList,const vector<int>& quantList,unordered_map<int,vector<Pair>>& jobArrive){

    string num = "";
    int jobNum = 0, cnt = 1, startTime = 0, runTime = 0, ioFreq = 0;
     
    for(int i=0; i<jlist.size(); i++){
        if(jlist[i] != ':' && cnt <= 3){
           if(jlist[i] != ','){
               num+=jlist[i];
           }
           else{
               if(cnt == 1) startTime = stoi(num);
               else if(cnt == 2) runTime = stoi(num);
               num = "" , cnt++;
           }
        }
        else if(jlist[i] == ':' && cnt == 3){
            ioFreq = stoi(num);
            parsedJList.push_back({jobNum,startTime,runTime,ioFreq,runTime,numQueues-1,allotList[numQueues-1],quantList[numQueues-1],-1,0,0});
            jobArrive[startTime].push_back({jobNum,"JOB BEGINS"});
            num = "", cnt = 1 , jobNum++;
        }
        else {
            parsedJList.clear();
            return 1;
        }
    }
    ioFreq = stoi(num);
    if(cnt == 3){
        parsedJList.push_back({jobNum,startTime,runTime,ioFreq,runTime,numQueues-1,allotList[numQueues-1],quantList[numQueues-1],-1,0,0});
        jobArrive[startTime].push_back({jobNum,"JOB BEGINS"});
    }
    else{
        parsedJList.clear();
        return 1;
    }
    
    return 0;
}

int findQ(vector<deque<Job>>& queue, int highQ){

   for(int i = highQ; i >= 0; i--){
      if(queue[i].size() > 0) return i;
   } 
   return -1;
}

int main(int argc, char* argv[]){
     
    try{
        
        cxxopts::Options options("MLFQ simulator","A simulation of the MLFQ scheduling algorithm");

        options.add_options()
            ("s,seed", "The random seed", cxxopts::value<int>()->default_value("0"))
            ("n,numQueues", "Number of queues in MLFQ (if not using -Q)", cxxopts::value<int>()->default_value("3"))
            ("q,quantum", "Length of time slice (if not using -Q)", cxxopts::value<int>()->default_value("10"))
            ("a,allotment", "Length of allotment (if not using -A)", cxxopts::value<int>()->default_value("1"))
            ("Q,quantumList", 
                "Length of time slice per queue level, specified as x,y,z,... where x is the quantum length for the highest priority queue, y the next highest, and so forth", 
                cxxopts::value<std::string>()->default_value(""))
            ("A,allotmentList", 
                "Length of time allotment per queue level, specified as x,y,z,... where x is the # of time slices for the highest priority queue, y the next highest, and so forth", 
                cxxopts::value<std::string>()->default_value(""))
            ("j,numJobs", "Number of jobs in the system", cxxopts::value<int>()->default_value("3"))
            ("m,maxlen", "Max run-time of a job (if randomly generating)", cxxopts::value<int>()->default_value("100"))
            ("M,maxio", "Max I/O frequency of a job (if randomly generating)", cxxopts::value<int>()->default_value("10"))
            ("B,boost", "How often to boost the priority of all jobs back to high priority", cxxopts::value<int>()->default_value("0"))
            ("G,agingList", 
              "boost priority by 1 if job waiting increase its aging time given for each queue by comma seperated specified as x,y,z,... where x is for the highest priority queue, y the next highest, and so forth", 
              cxxopts::value<std::string>()->default_value(""))
            ("i,iotime", "How long an I/O should last (fixed constant)", cxxopts::value<int>()->default_value("5"))
            ("S,stay", "Reset and stay at same priority level when issuing I/O", cxxopts::value<bool>()->default_value("False"))
            ("I,iobump", 
                "If specified, jobs that finished I/O move immediately to front of current queue", 
                cxxopts::value<bool>()->default_value("False"))
            ("l,jlist", 
                "A comma-separated list of jobs to run, in the form x1,y1,z1:x2,y2,z2:... where x is start time, y is run time, and z is how often the job issues an I/O request", 
                cxxopts::value<std::string>()->default_value(""))
            ("c", "Compute answers for me", cxxopts::value<bool>()->default_value("false"))
            ("h,help", "Print usage");
        
        auto result = options.parse(argc,argv);

        if(result.count("help")){
          cout<<options.help()<<endl;
          return 0;
        }

        int seed = result["seed"].as<int>();
        int numQueues = result["numQueues"].as<int>();
        int quantum = result["quantum"].as<int>();
        int allotment = result["allotment"].as<int>();
        string quantumList = result["quantumList"].as<string>();
        string allotmentList = result["allotmentList"].as<string>();
        int numJobs = result["numJobs"].as<int>();
        int maxlen = result["maxlen"].as<int>();
        int maxio = result["maxio"].as<int>();
        int boost = result["boost"].as<int>();
        string aging = result["agingList"].as<string>();
        const int iotime = result["iotime"].as<int>();
        bool stay = result["stay"].as<bool>();
        bool iobump = result["iobump"].as<bool>();
        string jlist = result["jlist"].as<string>();
        bool compute = result["c"].as<bool>();

        vector<Job> jobList;
        vector<int> allotList;
        vector<int> quantList;
        vector<int> agingList;
        unordered_map<int,vector<Pair>> jobArrive;

        if(quantumList != ""){
            quantList = parseString(quantumList);
            numQueues = quantList.size();
        }
        else quantList = vector<int>(numQueues,quantum);

        if(allotmentList != ""){
            allotList = parseString(allotmentList);
            if(numQueues != allotList.size()){
                cout<<"Number of allotments must be same as number of quantums.\n";
                return 1;
            }
        }
        else allotList = vector<int>(numQueues,allotment);

        if(jlist == "") generateRandomJobs(seed,maxlen,maxio,numJobs,jobList,numQueues,allotList,quantList,jobArrive);
        else{
            parseJobList(jlist,jobList,numQueues,allotList,quantList,jobArrive);
            if(jobList.size() == 0){
                cout<<"Job must contain 3 comma-seperated inputs only startTime, runTime and ioFreq.\n";
                return 1;
            }
            numJobs = jobList.size();
        }

        if(aging != ""){
            agingList = parseString(aging);
            if(agingList.size() != numQueues){
                cout<<"Number of agingList must be same as number of queues.\n";
                return 1;
            }
        }

/*-------------------------------------------------List of Inputs----------------------------------------------------------*/

        cout<<"Here is the list of inputs:\n";
        cout<<"OPTIONS jobs "<<numJobs<<endl;
        cout<<"OPTIONS queues "<<numQueues<<endl;

        for(int i = numQueues - 1; i >= 0; i--){
            cout<<"OPTIONS allotments for queue "<<i<<" is "<<allotList[i]<<endl;
            cout<<"OPTIONS quantum length for queue "<<i<<" is "<<quantList[i]<<endl;
            if(aging != "") cout<<"OPTIONS aging threshold for queue "<<i<<" is "<<agingList[i]<<endl;
        }

        cout<<"OPTIONS boost "<<boost<<endl;
        cout<<"OPTIONS ioTime "<<iotime<<endl;
        cout<<"OPTIONS stayAfterIO "<<boolalpha<<stay<<endl;
        cout<<"OPTIONS iobump "<<boolalpha<<iobump<<endl;

        cout<<"\n\nFor each job, three defining characteristics are given:\n"
        <<" startTime : at what time does the job enter the system\n"
        <<" runTime   : the total CPU time needed by the job to finish\n"
        <<" ioFreq    : every ioFreq time units, the job issues an I/O\n"
        <<"             (the I/O takes ioTime units to complete)"<<endl;

        cout<<"\nJob List:"<<endl;

        for(int i=0; i<numJobs; i++){
          cout<<" Job "<<jobList[i].jobNum<<": startTime "<<jobList[i].startTime<<" - runTime "<<jobList[i].runTime<<" - ioFreq "<<jobList[i].ioFreq<<endl;
        }
        
        if(!compute){
            cout<<"\nCompute the execution trace for the given workloads.\n"
            <<"If you would like, also compute the response and turnaround\n"
            <<"times for each of the jobs.\n"<<endl;
                   
            cout<<"Use the -c flag to get the exact results when you are finished.\n\n";
            return 1;
        }

/*----------------------------------------------------Compute-------------------------------------------------------------*/
        cout<<"\n\nExecution Trace:\n\n";
        
        vector<deque<Job>> queue(numQueues);
        int finishedJobs = 0, currTime = 0, highQ = numQueues - 1;;

        while(finishedJobs < numJobs){

            //move all jobs at high queue at given time
            if(boost > 0 && currTime != 0 && (currTime % boost == 0)){
                
                cout<<"[ time "<<currTime<<" ] BOOST ( every "<<boost<<" )\n";
                
                //remove jobs from all other queues except highQ
                for(int q=highQ-1; q>=0; q--){
                   for(auto it = queue[q].begin(); it != queue[q].end(); ++it){
                       queue[highQ].push_back(move(*it));
                   }
                   queue[q].clear();
                }

                for(int i=0; i<numJobs; i++){

                    if(jobList[i].timeLeft > 0){
                        jobList[i].qLevel = highQ;
                        jobList[i].ticksLeft = quantList[highQ];
                        jobList[i].allot = allotList[highQ];
                    }

                }

            }

            //Check if job Arrive or ioDone at current time and push in queue
            if(jobArrive.find(currTime) != jobArrive.end()){ 
                for(Pair job:jobArrive[currTime]){

                    int priority = jobList[job.jobN].qLevel;
                    cout<<"[ time "<<currTime<<" ] "<<job.jobTy<<" by JOB "<<job.jobN<<endl;

                    //when io done then put it at front of queue
                    if( iobump && job.jobTy == "IO_DONE" ) queue[priority].push_front(jobList[job.jobN]);
                    else queue[priority].push_back(jobList[job.jobN]);

                }
            }

            //aging
            if(aging != ""){
                for(int q=highQ-1; q>=0; q--){
                    if(queue[q].size() != 0){
                        for(auto it=queue[q].begin(); it!=queue[q].end();){
                           Job &job = *it;

                            if((++jobList[job.jobNum].wait) > agingList[q]){
                                cout<<"[ time "<<currTime<<" ] Aging( job "<<job.jobNum<<" update to queue "<<q+1<<" )\n";
                                queue[q+1].push_back(job);
                                jobList[job.jobNum].qLevel = q+1;
                                jobList[job.jobNum].ticksLeft = quantList[q+1];
                                jobList[job.jobNum].allot = allotList[q+1];  
                                it = queue[q].erase(it);
                            }
                            else ++it;
                        }
                    }
                }
            }

            //Find high queue
            int currQ = findQ(queue,highQ);
            if(currQ == -1){
              cout<<"[ time "<<currTime<<" ] IDLE\n";
              currTime++;
              continue;
            }

            //Runnable job(using cpu)
            int currJob = queue[currQ].front().jobNum;
            jobList[currJob].ticksLeft--;
            jobList[currJob].timeLeft--;

            if(jobList[currJob].response == -1)  jobList[currJob].response = currTime - jobList[currJob].startTime;
            if(aging != "" && currQ != highQ) jobList[currJob].wait = 0;
            
            int ticksLeft = jobList[currJob].ticksLeft;
            int allotLeft = jobList[currJob].allot;
            int timeLeft = jobList[currJob].timeLeft;
            int runTime = jobList[currJob].runTime;
            int ioFreq = jobList[currJob].ioFreq;
            int qLevel = jobList[currJob].qLevel;

            cout<<"[ time "<<currTime<<" ]"<<" Run JOB "<<currJob<<" at PRIORITY "<<qLevel<<" [ TICKS "<<ticksLeft
            <<" ALLOT "<<allotLeft<<" TIME "<<timeLeft<<" (of "<<runTime<<" ) ]"<<endl;
            
            currTime++;

            //chck finished job
            if(timeLeft == 0){
                cout<<"[ time "<<currTime<<" ]"<<" FINISHED JOB "<<currJob<<endl;
                finishedJobs++;
                jobList[currJob].completion = currTime;
                queue[currQ].pop_front();
                continue;
            }
             
            //chck for I/O 
            bool ioIssued = false;
            if( ioFreq > 0 && ((runTime - timeLeft) % ioFreq == 0) ){

                cout<<"[ time "<<currTime<<" ]"<<" IO_START by JOB "<<currJob<<"\n"
                <<"IO DONE\n";
                ioIssued = true;
                queue[currQ].pop_front();
                
                //this put the job at same queue when request i/o which is bad (mlfq rule 4a n 4b)
                if(stay){
                    jobList[currJob].ticksLeft = quantList[currQ];
                    jobList[currJob].allot = allotList[currQ];
                }
                
                //time when io is done and put then job in queue as per qLvl
                int ioDT = currTime + iotime;
                jobArrive[ioDT].push_back({currJob,"IO_DONE"}); 

            }

            //if quantum complete at particular qlvl chng priority as per allotment left
            if(ticksLeft == 0){

               jobList[currJob].allot--;
               if(!ioIssued) queue[currQ].pop_front();
               int ql = currQ;

               if(jobList[currJob].allot == 0){
                  //job done at this q-level so move to nxt lvl
                  if(currQ > 0){
                    ql = currQ - 1;
                    jobList[currJob].qLevel = currQ - 1;
                  }
                  jobList[currJob].allot = allotList[ql];
               }

               jobList[currJob].ticksLeft = quantList[ql];
               if(!ioIssued) queue[ql].push_back(jobList[currJob]);
               if(aging != "") jobList[currJob].wait = 0;
            }
           
        }

        cout<<"\nFinal statistics:\n";
        float avgST = 0, avgRT = 0, avgTT = 0;

        for(int i=0; i<numJobs; i++){
          cout<<" Job "<<jobList[i].jobNum<<": startTime  "<<jobList[i].startTime<<" - response  "<<jobList[i].response
          <<" - turnaround  "<<(jobList[i].completion-jobList[i].startTime)<<endl;
          avgST+=jobList[i].startTime;
          avgRT+=jobList[i].response;
          avgTT+=(jobList[i].completion-jobList[i].startTime);
        }

        cout<<"\n Average: startTime "<<fixed<<setprecision(2)<<(avgST/numJobs)<<" - response "<<(avgRT/numJobs)<<" - turnaround "<<(avgTT/numJobs)<<endl<<endl;

    }
    catch(const cxxopts::exceptions::exception& e){
       cerr<<"Error parsing options: "<<e.what()<<endl;
       return 1;
    }
    catch(exception& e){
       cerr<<"General err: "<<e.what()<<endl;
       return 1;
    }

    return 0;
}
