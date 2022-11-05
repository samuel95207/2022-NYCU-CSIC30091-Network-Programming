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
#include <deque>

struct Message{
    int pid;
    std::string type;
    std::string value;
};


class MessageManager {
    static const key_t SHM_KEY = ((key_t)7892);
    static const int SHM_SIZE = 65536;
    static const int SHM_PERMS = 0666;

    static const key_t SEM_KEY = ((key_t)7893);

    static int shmid;
    static char* shmBuf;
    static int sem;


    int pid;
    int fd;

    std::deque<Message> messageQueue;


   public:
    MessageManager();

    void run();
    void addMessage(const Message& message);

    void setPidFd(int pid, int fd);

    bool setupSharedMemory();

    static void closedSharedMemory();

   private:
    bool readFromSharedMemory(bool lock = true);
    bool writeToSharedMemory(bool lock = true);
};