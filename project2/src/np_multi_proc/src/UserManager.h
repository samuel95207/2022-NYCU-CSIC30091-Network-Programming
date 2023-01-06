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

    static std::map<int, User> idUserMap;
    static std::map<int, User> pidUserMap;
    static std::map<std::string, User> nameUserMap;

   public:
    UserManager();

    static User addUser(int pid, int fd, sockaddr_in ipAddr);
    static void removeUserById(int id);
    void removeUserByPid(int pid);

    static User getUserById(int id, bool lock = true);
    static User getUserByPid(int pid, bool lock = true);
    static User getUserByName(std::string name, bool lock = true);
    static std::map<int, User> getIdUserMap();

    static bool setNameById(int id, std::string name);

    static bool setupSharedMemory();
    static void closedSharedMemory();

    static bool readFromSharedMemory(bool lock = true);
    static bool writeToSharedMemory(bool lock = true);

    friend class BuildinCommand;

   private:
    static int idMin();
};