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
    int fd;
    int id;
    std::string name;
    sockaddr_in ipAddr;
};



class UserManager {
    std::map<int, User*> idUserMap;
    std::map<int, User*> fdUserMap;
    std::unordered_map<std::string, std::string> ipUsernameMap;

   public:
    UserManager();

    User* addUser(int fd, sockaddr_in ipAddr);
    void removeUserById(int id);
    void removeUserByFd(int fd);
    User* getUserById(int id);
    User* getUserByFd(int fd);

    std::map<int, User*> getIdUserMap();
    std::map<int, User*> getFdUserMap();


   private:
    int idMin();
};