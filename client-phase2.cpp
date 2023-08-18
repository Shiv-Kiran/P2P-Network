// Koustubh Rao, error corrections are omitted (probably not needed?)
#include <bits/stdc++.h>
#include <dirent.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#define MAX 120
#define SA struct sockaddr


using namespace std;

int main(int argc, char *argv[]) {


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //////////////////////////// PRE WORK ///////////////////////////////////
    FILE* demo;
    demo = fopen(argv[1], "r");
    struct dirent *de;
    DIR *dr = opendir(argv[2]);
    int zd = 0;
    while ((de = readdir(dr)) != NULL){
        zd++;
        if(zd > 2){
            if(strcmp(de->d_name,"Downloaded") != 0){
                printf("%s\n", de->d_name);
            }
        }
    }
    char c;
    bool b = true;
    bool i, p;
    i = true;
    p = true;
    int id, port, uid;
    id = 0;
    port = 0;
    uid = 0;
    int N = 0;
    while(b) {
        c = fgetc(demo);
        if(c == '\n') break;
        if(i){
            if(c == ' ') i = false;
            else{
                int yj = c - '0';
                id = id*10;
                id = id + yj;
            }
        }
        else if(p){
            if(c == ' ') p = false;
            else{
                int yj = c - '0';
                port = port*10;
                port = port + yj;
            }
        }
        else{
            int yj = c - '0';
            uid = uid*10;
            uid = uid + yj;
        }
    }
    while(b){
        c = fgetc(demo);
        if(!(c>=48 && c<=57)) break;
        N = N*10;
        int yj = c - '0';
        N = N + yj;
    }
    // c = fgetc(demo);
    int ID[N];
    int UID[N];
    int PORT[N];
    for(int j = 0; j < N; j++){
        ID[j] = 0;
        PORT[j] = 0;
        UID[j] = 0;
        int z = 0;
        while(true){
            c = fgetc(demo);
            if(c == ' ' || !(c>=48 && c<=57)){
                z++;
                if(z == 2) break;
            }
            else{
                if(z == 0){
                    int yj = c - '0';
                    ID[j] = ID[j]*10;
                    ID[j] = ID[j] + yj;
                }
                else if(z == 1){
                    int yj = c - '0';
                    PORT[j] = PORT[j]*10;
                    PORT[j] = PORT[j] + yj;
                }
            }
        }
    }
    int NF = 0;
    c = fgetc(demo);
    while(true){
        c = fgetc(demo);
        if(!(c>=48 && c<=57)) break;
        NF = NF*10;
        int yj = c - '0';
        NF = NF + yj;   
    }
    string file[NF];
    int depth[NF];
    int uni[NF];
    string md5[NF];
    for(int k = 0; k < NF; k++){
        file[k] = "";
        md5[k] = "0";
        depth[k] = 0;
        uni[k] = 0;
        // MD5 hash
        while(true){
            c = fgetc(demo);
            if(!(c>=33 && c<=126)) break;
            file[k] = file[k] + c; 
        }
    }

    mkdir("Downloaded",0777);

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // ********** MAIN SOCKET ****************
    int opt = 1;
    int sockfd;   // main socket descripter of the client
    struct sockaddr_in servaddr;
    unsigned int len = sizeof(servaddr);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port);
    bind(sockfd, (SA*)&servaddr, sizeof(servaddr));
    listen(sockfd, N);                     // listen with max needed capacity

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //************* CONNECTING WITH NEIGHBOURS ***************

    vector<int> accep, connec;
    // fd_set master; 
    fd_set read_fds;
    int max_fd; 
    FD_ZERO(&read_fds);
    for (int j = 0; j < N; j++) {
        int clifd;
        struct sockaddr_in cliaddr;
        clifd = socket(AF_INET, SOCK_STREAM, 0);
        bzero(&cliaddr, sizeof(cliaddr));
        cliaddr.sin_family = AF_INET;
        string addr = "127.0.0.1";  // to be changed to the IP of neighbours
        inet_pton(AF_INET, addr.c_str(), &cliaddr.sin_addr); 
        cliaddr.sin_port = htons(PORT[j]);
        while (connect(clifd, (struct sockaddr *)&cliaddr, sizeof(cliaddr)) < 0) {
            sleep(0.10);
        }
        connec.push_back(clifd);
    }

    

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //************** WORKSPACE ***************

    while(true){
        FD_SET(sockfd, &read_fds);
        max_fd = sockfd;
        for (int pl = 0; pl < accep.size(); pl++) {
            int acc_fd = accep[pl];
            FD_SET(acc_fd, &read_fds);
            if (acc_fd > max_fd) {
                max_fd = acc_fd;
            } 
        }  
               
        for (int pl = 0; pl < connec.size(); pl++) {
            int con_fd = connec[pl];
            FD_SET(con_fd, &read_fds);
            if (con_fd > max_fd) {
                max_fd = con_fd;
            } 
        }                
            
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);

        // send relevant info to ur neighbour 

        if (FD_ISSET(sockfd, &read_fds)) {
            int cli_fd_1 = accept(sockfd, (struct sockaddr *)&servaddr, (socklen_t *)&len);                    
            accep.push_back(cli_fd_1);  
            char buff[MAX];
            sprintf(buff, "u %d %d %d", id, uid, port);
            send(cli_fd_1, buff, MAX, 0);
            bzero(buff, MAX);
        } 


        // receive relevant info from ur neighbour

        for (int pl = 0; pl < connec.size(); pl++) {
            int cli_fd = connec[pl];                  
            if (FD_ISSET(cli_fd, &read_fds)) {
                char buff[MAX];
                bzero(buff, MAX);
                // other side is already closed case
                if (read(cli_fd, buff, MAX) == 0) {
                    FD_CLR(connec[pl], &read_fds);
                    close(connec[pl]);
                    connec[pl] = -1;
                    connec.erase(connec.begin() + pl);
                    pl = pl - 1;
                } 
                // other side is open => msg transfer not yet done
                else {
                    if(buff[0] == 'u'){
                        int x, y, z;
                        sscanf(buff, "u %d %d %d", &x, &y, &z);
                        for(int ko = 0; ko < N; ko++){
                            if(ID[ko] == x) UID[ko] = y;
                        }
                        bzero(buff, MAX);
                        int pol = 0;
                        sprintf(buff, "f %s %d", file[0].c_str(), pol);
                        send(cli_fd, buff, MAX, 0);
                        bzero(buff, MAX);
                    }
                    if(buff[0] == '1'){
                        int u;
                        int x;
                        sscanf(buff, "1 %d %d", &u, &x);
                        if(depth[x] == 0){
                            uni[x] = u;
                            depth[x] = 1;
                        }
                        else{
                            uni[x] = min(uni[x],u);
                        }
                        bzero(buff, MAX);
                        if(x == NF -1){
                            buff[0] = 'b';
                            send(cli_fd, buff, MAX, 0);
                            bzero(buff, MAX);
                            FD_CLR(connec[pl], &read_fds);
                            close(connec[pl]);
                            connec[pl] = -1;
                            connec.erase(connec.begin() + pl);
                        }
                        else{
                            sprintf(buff, "f %s %d", file[x+1].c_str(), x+1);
                            send(cli_fd, buff, MAX, 0);
                            bzero(buff, MAX);
                        }
                    }
                    if(buff[0] == '0'){
                        int u;
                        int x;
                        sscanf(buff, "0 %d %d", &u, &x);
                        bzero(buff, MAX);
                        if(x == NF -1){
                            buff[0] = 'b';
                            send(cli_fd, buff, MAX, 0);
                            bzero(buff, MAX);
                            FD_CLR(connec[pl], &read_fds);
                            close(connec[pl]);
                            connec[pl] = -1;
                            connec.erase(connec.begin() + pl);
                        }
                        else{
                            sprintf(buff, "f %s %d", file[x+1].c_str(), x+1);
                            send(cli_fd, buff, MAX, 0);
                            bzero(buff, MAX);
                        }
                    }
                }
            }
        }

        for (int pl = 0; pl < accep.size(); pl++) {
            int cli_fd = accep[pl];                   
            if (FD_ISSET(cli_fd, &read_fds)) {
                char buff[MAX];
                bzero(buff, MAX);
                // other side is already closed case
                if (read(cli_fd, buff, MAX) == 0) {
                    FD_CLR(accep[pl], &read_fds);
                    close(accep[pl]);
                    accep[pl] = -1;
                    accep.erase(accep.begin() + pl);
                    pl = pl - 1;
                } 
                // other side is open => msg transfer not yet done
                else {
                    if(buff[0] == 'f'){
                        char p[40];
                        bzero(p, 40);       // error region
                        int x;
                        sscanf(buff, "f %s %d", p, &x);
                        bzero(buff, MAX);
                        DIR *dr = opendir(argv[2]);
                        struct dirent *de;
                        char uj = '0';
                        while ((de = readdir(dr)) != NULL){
                            if(strcmp(de->d_name,p) == 0){
                                uj = '1';
                                break;
                            }
                        }
                        closedir(dr);
                        sprintf(buff, "%c %d %d", uj, uid, x);
                        send(cli_fd, buff, MAX, 0);
                        bzero(buff, MAX);
                    }

                    else if(buff[0] == 'b'){
                        FD_CLR(accep[pl], &read_fds);
                        close(accep[pl]);
                        accep[pl] = -1;
                        accep.erase(accep.begin() + pl);
                        pl = pl - 1;
                    }
                }
            }

        }

        // end of duty

        if (connec.empty() && accep.empty()) {
            for(int ko = 0; ko < N; ko++){
                printf("Connected to %d with unique-ID ", ID[ko]);
                printf("%d",UID[ko]);
                printf(" on port %d\n", PORT[ko]);
            }
            vector<tuple<string, int, string, int>> v;
            for(int ko = 0; ko < NF; ko++){
                v.push_back(make_tuple(file[ko], uni[ko], md5[ko], depth[ko]));
            }
            sort(v.begin(), v.end());
            for(int ko = 0; ko < NF; ko++){
                printf("Found %s at %d with MD5 %s at depth %d\n", (get<0>(v[ko])).c_str(), (get<1>(v[ko])), (get<2>(v[ko])).c_str(), (get<3>(v[ko])));
            }
            break;
        }
    }

    

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //************ END ****************

    closedir(dr);
    fclose(demo);
    return 0;
}