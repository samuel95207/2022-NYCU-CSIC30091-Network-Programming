#include <arpa/inet.h>
#include <memory.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <deque>
#include <map>
#include <string>
#include <unordered_map>


class MultiProcServer;

struct Message {
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

    static std::deque<Message> messageQueue;

    int pid;
    int fd;



   public:
    MessageManager();

    void run(MultiProcServer& server);
    void setPidFd(int pid, int fd);

    static bool setupSharedMemory();
    static void addMessage(const Message& message);
    static void closedSharedMemory();

   private:
    static bool readFromSharedMemory(bool lock = true);
    static bool writeToSharedMemory(bool lock = true);
};