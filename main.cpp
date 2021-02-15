#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <cmath>
#include <iomanip>
#include <bits/stdc++.h>

using namespace std;

/*
Design:
    variable no of high priority and low priority elements depending
    a set can be : full of high priority elements
                 : full of low priority elements
                 : high priority and low priority elements
*/

int CYCLE = 0;                      // global variable for cycle counts

// power of 2 in x
int powerOfTwo(int &x){
    int y=0;
    if(x>=1){
        while(x%2==0){
            x/=2;
            y++;
        }
    }
    return y;
}

//  return index if tag matched and negative on no match
int check_tag(vector<tuple<bool,int,int,bool,int>> cache,int s_index,int e_index, int tag){
    for(int i=s_index;i<e_index;i++){
        if(get<0>(cache.at(i))==1){
            if(get<1>(cache.at(i))==tag)
                return i;                                       // index for matched tag
            else if (i == e_index -1) return -1;                // -1 : in case set is full and no tag is matched
        }
        else return -2;                                         // -2 : in case set is empty and no tag is matched
    }
}

//  find index of first invalid index in a set
int find_invalid(vector<tuple<bool,int,int,bool,int>> cache,int s_index,int e_index){
    for(int i=s_index;i<e_index;i++){
        if(get<0>(cache.at(i))==0)
            return i;
    }
}
//  general information about a set
tuple<bool,int,int,int,int,int> INFO(vector<tuple<bool,int,int,bool,int>> cache,int s_index,int e_index){
    int set_length = e_index-s_index;
    bool filled = true;
    int x = -1;                 // highest priority index among LPG
    int y = -1;                 // highest priority among LPG
    int z = -1;                 // lowest priority index among HPG
    int t = INT_MAX;            // lowest priority among HPG
    int lruLP = -1;
    for(int i=s_index;i<e_index;i++){
        if(get<0>(cache.at(i))==1){
            if(get<4>(cache.at(i)) > y && get<4>(cache.at(i)) < set_length){
                x = i;
                y = get<4>(cache.at(i));
                if(get<4>(cache.at(i))==0) lruLP = i;
            }
            else if(get<4>(cache.at(i)) >= set_length && get<4>(cache.at(i)) < t){
                z = i;
                t = get<4>(cache.at(i));
            }
        }
        else { filled = false; break; }
    }
    return make_tuple(filled,x,y,z,t,lruLP);
}

//  find highest prority among LPG
int find_highest_among_LP(vector<tuple<bool,int,int,bool,int>> cache,int s_index,int e_index){
    int x = -1;
    int set_length = e_index-s_index;
    for(int i=s_index;i<e_index;i++){
        if(get<0>(cache.at(i))==1){
            if(get<4>(cache.at(i)) > x && get<4>(cache.at(i)) < set_length) x = get<4>(cache.at(i));
        }
        if(get<0>(cache.at(i))==0) break;
    }
    return x;
}

//  update write buffer called on end of every cycle
void updateWB(long long int* memo, queue<tuple<int,int,int>> &WB, int wbz, int P){
    queue<tuple<int,int,int>> wb1;

    for(int j=0; j< WB.size(); j++){
        tuple<int,int,int> z = WB.front();
        if(get<2>(z) < P)  { get<2>(z)++; wb1.push(z); WB.pop(); }
        else if(get<2>(z) == P) {
            memo[get<0>(z)] = get<1>(z);
            WB.pop();
        }
    }
    WB = wb1;

}

//  check if the buffer is free or not
bool wbfreed(long long int* memo, queue<tuple<int,int,int>> WB, int wbz){
    if(WB.size() < wbz) return true;
    else false;
}

//  facilitate stalling of read
bool readStall(int &sc, int P){
    if(sc < P) return false;
    else true;
}

//  refresh status of cache on end of every cycle -- updating priorty among HPG
void refreshCache(vector<tuple<bool,int,int,bool,int>> &cache, int c_assoc, int T){
    int num_sets = cache.size()/c_assoc;
    for(int k=0;k<num_sets;k++){
        for(int w= k*c_assoc; w< k*c_assoc + c_assoc; w++){
            if(get<0>(cache.at(w))==1){
                int priority = get<4>(cache.at(w));
                if(priority > c_assoc) get<4>(cache.at(w))--;
                if(priority = c_assoc ) {
                    int x= find_highest_among_LP(cache,k*c_assoc,k*c_assoc + c_assoc);
                    get<4>(cache.at(w)) = x + 1;
                }
            }
        }
    }
}

void PRINTCACHE(vector<tuple<bool,int,int,bool,int>> cache, int c_assoc, int num_sets){
    cout<<"\n"<<endl;
    for(int i=0; i<num_sets; i++){
        cout<<"\nSet := "<<i<<endl;
        cout<<"+--------------------------------------------------------------------------------------+"<<endl;
        for(int j= i*c_assoc; j< i*c_assoc + c_assoc; j++){
            tuple<bool,int,int,bool,int> z = cache.at(j);
            string priority;
            if(get<0>(z)==false) priority = "NOT APPLICABLE";
            else{
                int p = get<4>(z);
                if( p >= 0 && p < c_assoc) priority = "LP";
                if( p >= c_assoc) priority = "HP";
            }
            cout<<"valid status:= "<<(get<0>(z))<<" tag:= "<<(get<1>(z))<<" data:= "<<(get<2>(z))<<" dirty status:= "<<(get<3>(z))<<" Priority:= "<<(get<4>(z))<<endl;
            cout<<"+--------------------------------------------------------------------------------------+"<<endl;
        }
    }
}

//  main
int main(){

    int c_size, b_size, c_assoc, T;

    vector<vector<string>> memoReq;                                         // memory access requests in a vector of vector of string

    ifstream myfile ("t2");                                                 // input file
    string line;
    int k=0;

    //  file the memoReq vector from input stream
    while ( getline (myfile,line)) {
        if(line != ""){
            cout<<line<<endl;
            stringstream iss(line);
            string word;
            bool comment=false;
            while(iss >> word) {
                if(word[0] == '#') {comment=true; break;}
                if(k==0) { c_size  = stoi(word); k++ ; comment=true; break;}
                if(k==1) { b_size  = stoi(word); k++ ; comment=true; break;}
                if(k==2) { c_assoc = stoi(word); k++ ; comment=true; break;}
                if(k==3) { T       = stoi(word); k++ ; comment=true; break;}
                break;
            }
            if( (k>3) && comment==false){
                //cout<<"AAYA"<<endl;
                vector<string> words;
                int last=0;
                for(int i=0;i<line.length();i++){
                    if(line[i]== ',' || line[i]==' ' || (i==line.length()-1)) {
                        if(i == line.length()-1) words.push_back(line.substr(last, i - last +1));
                        else if(last != i) {
                            words.push_back(line.substr(last, (i-last)));
                            last = i+1;
                        }
                        else last = i+1;
                    }
                }
                //cout<<words.size()<<endl;
                //for(int k=0; k< words.size(); k++) cout<<words.at(k)<<endl;
                memoReq.push_back(words);
            }
        }
    }

    cout<<endl;
    bool stall = false;                                                 // stall signal in case of write buffer waitinf for being freed

    int numCacheBlcks = c_size/b_size;                                                      // number of blocks in cache memory
    cout<<"numCacheBlcks: "<<numCacheBlcks<<endl;
    vector<tuple<bool,int,int,bool,int>> cache;                                             // tuple of (valid, tag, data, dirty status)
    for(int i=0;i < numCacheBlcks; i++) cache.push_back(make_tuple(false,0,0,false,0));     // initialize cache with (valid=0,tag=0,data=0,dirty status=0,priority)

    int wbz = numCacheBlcks/2;                                  // size of write buffer defined as half the size of cache memory
    cout<<"wbz: "<<wbz<<endl;
    queue<tuple<int,int,int>> WB;                               // (address,data,cycles)

    int numMemoBlcks = 4*numCacheBlcks;                         // size of main memory defined as 4 times that of cache memory
    cout<<"numMemoBlcks: "<<numMemoBlcks<<endl;
    long long int memo[numMemoBlcks];                           // main memory as an array of int
    memo[0] = 100;
    memo[2] = 300;
    int m_size = numMemoBlcks;                                  // m_size = number of blocks in main memory
    int num_sets = c_size /(c_assoc*b_size);                    // number of sets in cache memory
    cout<<"num_sets: "<<num_sets<<endl;

    // int addr_length = powerOfTwo(m_size);
    // int off_length = powerOfTwo(b_size);
    // int set_length = powerOfTwo(num_sets);

    int P=10;                                                   // mat: number of cycles taken for accessing the memory

    int readResult;                                             // value read on read request

    int pc=0;                                                   // memory request index

    // executing the memory access requests
    while(pc < memoReq.size()){

        cout<<memoReq.at(pc).at(0)<<", "<<memoReq.at(pc).at(1)<<endl;

        string type = memoReq.at(pc).at(1);                                         // type of request
        int data;
        if(type.compare("W")==0) data = stoi(memoReq.at(pc).at(2));                 // data for write request
        int blckAddr  = stoi(memoReq.at(pc).at(0));                                 // memory block address
        int set_index = blckAddr % num_sets;                                        // set index for cache memory
        int tag       = blckAddr / num_sets;                                        // tag value

        if((blckAddr >= 0) && blckAddr<(m_size)){
            int index = set_index*c_assoc;                                          // index for 1st element of corresponding set in cache
            int tag_index = check_tag(cache,index,(index+c_assoc),tag);             // return index if tag matched else negative value

            int e_index = index + c_assoc;
            tuple<bool,int,int,int,int,int> info = INFO(cache,index,e_index);
            //cout<<"\n"<<get<0>(info)<<" "<<get<1>(info)<<" "<<get<2>(info)<<" "<<get<3>(info)<<" "<<get<4>(info)<<" "<<get<5>(info)<<" "<<"\n"<<endl;
            bool filled = get<0>(info);

            // Case: set is full
            if(filled == true){

                // Case: tag not found
                if(tag_index == -1){

                    int lru;
                    if(get<1>(info) != -1) lru = get<5>(info);             // Case: all elements are not HP
                    else if(get<1>(info) == -1) lru = get<5>(info);        // Case: all elements are HP

                    // placing write request in Write Buffer for dirty element
                    bool ds = get<3>(cache.at(lru));
                    if(ds == true) {
                        int wbt = 0;
                        int waddr = index + (numCacheBlcks)*get<1>(cache.at(lru));
                        int wdata = get<2>(cache.at(lru));
                        if(WB.size() < wbz) WB.push(make_tuple(waddr,wdata,wbt));
                        else stall=true;
                        while(stall==true){
                            ::CYCLE ++;
                            updateWB(memo,WB, wbz,P);
                            if( wbfreed( memo, WB, wbz) == true ) stall=false;
                        }
                        WB.push(make_tuple(waddr,wdata,wbt));
                    }

                    // Read
                    if(type.compare("R")==0){
                        bool foundInWB = false;
                        queue<tuple<int,int,int>> wb2 = WB;

                        // finding read element in write buffer if present
                        for(int k = 0; k<WB.size(); k++){
                            tuple<int,int,int> q = wb2.front();
                            if(get<0>(q) == blckAddr) {
                                foundInWB = true;
                                readResult = get<1>(q);
                                if(get<1>(info) != -1)      cache.at(lru) = make_tuple(true,tag,get<1>(q),false, get<2>(info) );
                                else if(get<1>(info) == -1) cache.at(lru) = make_tuple(true,tag,get<1>(q),false, 0  );
                                break;
                            }
                            else { wb2.pop(); }
                        }

                        // stall for element to arrive
                        if(foundInWB == false){
                            int stallCycles = 0;
                            //::CYCLE++;
                            while ( readStall(stallCycles, P) == false ){
                                ::CYCLE++;
                                updateWB(memo,WB, wbz,P);
                                stallCycles++;
                            }
                            readResult = memo[blckAddr];
                            if(get<1>(info) != -1)      cache.at(lru) = make_tuple(true,tag,memo[blckAddr],false, get<2>(info) );
                            else if(get<1>(info) == -1) cache.at(lru) = make_tuple(true,tag,memo[blckAddr],false, 0  );
                        }
                    }

                    // Write
                    else if(type.compare("W")==0){
                        if(get<1>(info) != -1)      cache.at(lru) = make_tuple(true,tag,data,true, get<2>(info) );
                        else if(get<1>(info) == -1) cache.at(lru) = make_tuple(true,tag,data,true, 0  );
                    }

                    // arrange priority among LPG
                    if(get<1>(info) != -1)
                        for(int k= index; k< index + c_assoc;k++)
                            if(k!=lru) if(get<4>(cache.at(k)) > 0 && get<4>(cache.at(k)) < c_assoc) get<4>(cache.at(k))--;

                }
                // Case: tag foung
                else if(tag_index != -2){
                    // Read
                    if(type.compare("R")==0){
                        readResult = get<2>(cache.at(tag_index));
                        bool ds = get<3>(cache.at(tag_index));                              // retain dirty status
                        int prevPrior = get<4>(cache.at(tag_index));
                        if(prevPrior < c_assoc){
                            for(int k=index; k< e_index;k++){
                                if(get<4>(cache.at(k)) > prevPrior && get<4>(cache.at(k)) < c_assoc){
                                    get<4>(cache.at(k))--;
                                }
                            }
                        }
                        cache.at(tag_index) = make_tuple(true,tag,data,ds,T+ c_assoc-1);    // moving element to HPG or retaining as HP
                    }
                    // Write
                    if(type.compare("W")==0){
                        int prevPrior = get<4>(cache.at(tag_index));
                        if(prevPrior < c_assoc){
                            for(int k=index; k< e_index;k++){
                                if(get<4>(cache.at(k)) > prevPrior && get<4>(cache.at(k)) < c_assoc){
                                    get<4>(cache.at(k))--;
                                }
                            }
                        }
                        cache.at(tag_index) = make_tuple(true,tag,data,true,T+ c_assoc-1);  // moving element to HPG as dirty
                    }
                }
            }
            // Case: partially filled set
            else{

                // Case: tag not found
                if(tag_index == -2){
                    //cout<<"AAYA!"<<endl;
                    int insert_index = find_invalid(cache, index, index + c_assoc);
                    // Read
                    if(type.compare("R")==0){
                        int stallCycles = 0;
                        //::CYCLE++;
                        // stall for memory read
                        while ( readStall(stallCycles, P) == false ){
                            ::CYCLE++;
                            updateWB(memo,WB, wbz,P);
                            stallCycles++;
                        }
                        readResult = memo[blckAddr];
                        //cout<<get<2>(info)+1<<endl;
                        int p = find_highest_among_LP(cache,index,e_index);
                        //cout<<p<<endl;
                        cache.at(insert_index) = make_tuple(true,tag,memo[blckAddr],false, p+1);
                    }
                    // Write
                    if(type.compare("W")==0){
                        //
                        //cout<<get<2>(info)+1<<endl;
                        int p = find_highest_among_LP(cache,index,e_index);
                        cache.at(insert_index) = make_tuple(true,tag,data,true, p+1);
                    }
                }
                // Case: tag found
                else if (tag_index != -1){
                    // Read
                    if(type.compare("R")==0){
                        readResult = get<2>(cache.at(tag_index));
                        bool ds = get<3>(cache.at(tag_index));
                        int prevPrior = get<4>(cache.at(tag_index));
                        if(prevPrior < c_assoc){
                            for(int k=index; k< e_index;k++){
                                if(get<4>(cache.at(k)) > prevPrior && get<4>(cache.at(k)) < c_assoc){
                                    get<4>(cache.at(k))--;
                                }
                            }
                        }
                        cache.at(tag_index) = make_tuple(true,tag,data,ds,T+ c_assoc-1);        // move to HPG or retaining as HP
                    }
                    // Write
                    if(type.compare("W")==0){
                        int prevPrior = get<4>(cache.at(tag_index));
                        if(prevPrior < c_assoc){
                            for(int k=index; k< e_index;k++){
                                if(get<4>(cache.at(k)) > prevPrior && get<4>(cache.at(k)) < c_assoc){
                                    get<4>(cache.at(k))--;
                                }
                            }
                        }
                        cache.at(tag_index) = make_tuple(true,tag,data,true,T+ c_assoc-1);      // move to HPG or retaining as HP
                    }
                }
            }

            pc++;                                       // next access request index
            ::CYCLE++;                                  // increment cycle
            cout<<"cycles: "<<(::CYCLE)<<endl;
            updateWB(memo,WB, wbz,P);                   // update write buffer
            refreshCache(cache, c_assoc, T);            // update cache
        }
        else break;
    }
    PRINTCACHE(cache, c_assoc, num_sets);
    return 0;
}


