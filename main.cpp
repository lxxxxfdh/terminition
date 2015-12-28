#include <iostream>
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "loopAnalysis/termLoopPass.h"
#include "llvm/IR/LegacyPassManager.h"
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
using namespace std;
using namespace llvm;
using namespace termloop;
bool EndWith(const string& str,const string& strEnd)
{
    if(str.empty() || strEnd.empty())
    {
        return false;
    }
    if(str.size()<strEnd.size())
        return false;
    return str.compare(str.size()-strEnd.size(),strEnd.size(),strEnd)==0?true:false;
}
void List(char *path, map<string,string> * files)
{
    struct dirent* ptr = NULL;
    DIR *pDir;
    char base[1000];
    pDir=opendir(path);
    string p;
    while (NULL != (ptr=readdir(pDir)))
    {
        if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)    ///current dir OR parrent dir
             continue;


            if (ptr->d_type==8)
            {
                //printf("普通文件:%s\n", ptr->d_name);
               // if(EndWith(ptr->d_name,".c")) {
                    string pt = p.assign(path).append("/").append(ptr->d_name);
                    string filename = strtok(ptr->d_name, ".");
                    files->insert(pair<string, string>(pt, filename));
              //  }
            }
            else if(ptr->d_type==4)
            {
                memset(base,'\0',sizeof(base));
                strcpy(base,path);
                strcat(base,"/");
                strcat(base,ptr->d_name);
                //printf("子目录：%s\n",base);
                List(base,files);
            }

    }
    closedir(pDir);
}
int main(int argc,char *argv[]) {
    //cout << "Hello, World!" << endl;
    LLVMContext &Context = getGlobalContext();
    SMDiagnostic Err;
 /*   char* p1="/home/xie/terminition_test/testreg.ll";
    p1="/home/xie/ClionProjects/terminition/test/incAndCons/be_be_decreg.ll"; //aboveDecSinglereg.ll  belowDecSinglereg.ll   aboveIncSinglereg.ll  belowIncSinglereg.ll
    p1="/home/xie/ClionProjects/terminition/test/incAndDec/be_abreg.ll";
    p1="/home/xie/ClionProjects/terminition/test/conAndCon/be_bereg.ll";*/


    if(argc!=2){
        errs()<<"Need one directory as the arguments! \r\n";
        return 0;
    }
    string argu=argv[1];



    legacy::PassManager pm;
    pm.add(createLoopPass());


    clock_t b1 = clock();
    string path=argv[1]; //"/home/xie/SNU-real-time/fft1/fft1reg.ll";
    std::unique_ptr<Module> Mod = parseIRFile(path, Err, Context);
    if (Mod) {
        pm.run(*Mod);
    }else{
        assert(false);
    }
    clock_t  b2=clock();
    printf("Time comsumption is  %05.3f  milliseconds\r\n\r\n",(double)(b2-b1)/1000);


    return 0;
    if(access(argu.c_str(),F_OK)==-1) { //dir not exists
        errs()<<argu<<" not found\r\n";
        return 0;
    }



    //create temporary ir directory
    int MAX_PATH=100;
    char*  buffer=new char[MAX_PATH];
    char   rootDir[MAX_PATH];

    getcwd(buffer, MAX_PATH);
    strcpy(rootDir,buffer);
    strcat(buffer,"/irtemp/");
    if(access(buffer,F_OK)==-1){ //dir not exists
        if (mkdir(buffer,0777))
            assert(false&&"Cannot create temp");
    }







    //errs()<<buffer;
    string r=rootDir, r1=buffer;

    clock_t start = clock();

    string cmd=r+"/generateir.sh "+argu+" "+r1;

    system(cmd.c_str());













    //for file
    double end = (double)(clock() - start)/1000;

    //errs()<<" Generate IR time: "<<(double)end <<"seconds!\r\n";
    map<string,string>  files;
    List(buffer,&files);
    sleep(1);
    clock_t s1=clock();
    int i=1;
    for(map<string,string>::iterator it=files.begin();it!=files.end();it++){

        errs()<<i<<") "<<it->second<<".c"<<"\r\n";
        std::unique_ptr<Module> Mod = parseIRFile(it->first, Err, Context);
        if (Mod) {

            pm.run(*Mod);


        }
        else {
            std::cout << "Mod is null" << std::endl;
        }/**/
        clock_t  s2=clock();

        printf("Time comsumption is  %05.3f  milliseconds\r\n\r\n",(double)(s2-s1)/1000);

        s1=s2;
        i++;


        //errs()<<"\r\n";
    }



    //traverse(root);

    return 0;
}