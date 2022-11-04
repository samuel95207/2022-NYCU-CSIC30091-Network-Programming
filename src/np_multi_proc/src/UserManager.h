#include <arpa/inet.h>
#include <memory.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <map>
#include <string>
#include <unordered_map>



struct User {
    int pid;
    int fd;
    int id;
    std::string name;
    std::string ipAddr;
};



class UserManager {
    static const key_t SHM_KEY = ((key_t)7890);
    static const int SHM_SIZE = 65536;
    static const int SHM_PERMS = 0666;

    static const key_t SEM_KEY = ((key_t)7891);

    static int shmid;
    static char* shmBuf;
    static int sem;


    std::map<int, User*> idUserMap;
    std::map<int, User*> pidUserMap;
    std::map<std::string, User*> nameUserMap;

   public:
    UserManager();

    User* addUser(int pid, int fd, sockaddr_in ipAddr);
    void removeUserById(int id);
    void removeUserByPid(int pid);

    User* getUserById(int id);
    User* getUserByPid(int pid);
    User* getUserByName(std::string name);
    bool setNameById(int id, std::string name);

    std::map<int, User*> getIdUserMap();
    std::map<int, User*> getPidUserMap();
    std::map<std::string, User*> getNameUserMap();


    bool setupSharedMemory();
    static void closedSharedMemory();

   private:
    bool readFromSharedMemory();
    bool writeToSharedMemory();


    int idMin();
};