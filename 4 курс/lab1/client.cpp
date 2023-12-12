#include <unistd.h> //sleep(), access()
#include <cstring> // strcpy()
#include <sys/msg.h> //<sys/types.h> [key_t] [ssize_t]; <sys/ipc.h> ftok() IPC_CREAT, IPC_EXCL; msgget(), msgrcv(), msgsnd(), msgctl()

#include <iostream> //<stdio.h> FILE; <cstdlib> popen(), pclose(), exit()
#include <string>
#include <sstream> //[istringstream]
#include <vector>

#define MY_RCV_TYPE 10

// структура сообщения с типом и текстом с заданной длиной
struct Message {
    long mtype;
    char mtext[2048];
};

//вспомогательная функция для создания команды du
std::string makeCommand(std::string &__files) {
    std::istringstream iss(__files);
    std::string temp;
    std::string buffer;
    
    while (getline(iss, temp)) {
        std::string tmp = "\"" + temp + "\" ";
        buffer += tmp;
    }
    
    return buffer;
}

//функция определения наименьшего индекса
int findSmallestIndex(const std::vector<size_t>& nums) {
    if (nums.empty()) {
        return -1;
    }

    int smallestIndex = 0;
    int smallestValue = nums[0];

    for (size_t i = 1; i < nums.size(); ++i) {
        if (nums[i] < smallestValue) {
            smallestValue = nums[i];
            smallestIndex = i;
        }
    }
    return smallestIndex;
}

//функция определения наименьшего времени изменения и файла который ему соответствует
std::string findDate(std::string &msg2, std::string &msg1) {
    std::istringstream iss(msg2);
    std::string temp;
    std::vector<size_t> arrayDates;
    while (getline(iss, temp)) {
        arrayDates.push_back(std::stoul(temp));
    }
    int index = findSmallestIndex(arrayDates);
    std::istringstream ss1(msg1);
    int i = 0;
    if (index == -1) {
        return "";
    }
    while (getline(ss1, temp)) {
        if (index == i) {
            return temp;
        }
        i++;
    }
    return "";
}

//подсчет кол-ва блоков каждого файла
std::string findCountBlockFile(std::string &__files) {
    FILE *fp;
    if (__files.empty()){
      return "0\n";
    }  
    std::string command = "du -h -B 2048 " + makeCommand(__files) + "| awk '$1 <= 8 {print $0}' | wc -l";
    fp = popen(command.c_str(), "r");
    if (fp == nullptr) {
        std::cout << "Error executing command shell\n";
        exit(2);
    }
    char tmp;
    size_t retVal;
    std::string output;
    while ((retVal = fread(&tmp, 1, sizeof(tmp), fp)) != 0) {
        output += tmp;
    }
    pclose(fp);
    return output;
}

int main() {
    const char *pathname = ".";
    key_t key;
    while (true) {
        key = ftok(pathname, 'A');
        if (key != -1) {
            break;
        } else {
            std::cout << "Error creating unique id for message queue\n";
            sleep(5);
        }
    }
    int mqId;
    bool flag = 0;
    while (true) {
        mqId = msgget(key, 0);//выполнение без флагов
        if (mqId == -1) {
            if (flag) {
               std::cout << "Server didn't create mq\n";
                exit(3);
            }
            std::cout << "Server didn't create mq\n";
            sleep(5);
            flag = 1;
        } else {
            break;
        }
    }
    //std::string __files;
    //findCountBlockFile(__files);
    Message rcvMessage1, rcvMessage2;
    int flag2Msg = 0;
    while (true) {
        if (flag2Msg == 0) {
            if(msgrcv(mqId, &rcvMessage1, sizeof(rcvMessage1.mtext), MY_RCV_TYPE, 0) == -1) {//получение сообщения из очереди
                exit(3);
            } else {
                flag2Msg++;
                continue;
            }
        }
        if (flag2Msg == 1) {
            if(msgrcv(mqId, &rcvMessage2, sizeof(rcvMessage2.mtext), MY_RCV_TYPE, 0) == -1) {
                std::cout << "Error rcvMessage\n";
                exit(3);
            } else {
                flag2Msg++;
            }
        }
        if (flag2Msg == 2) {
            break;
        }
    }
    std::string firstMsg = rcvMessage1.mtext;
    std::string secondMsg = rcvMessage2.mtext;
    std::cout << "Files count with <= 2 blocks: " << findCountBlockFile(firstMsg);
    std::cout << "Oldest modification file: " << findDate(secondMsg, firstMsg) << "\n";
    struct msqid_ds msgbuf;
    if (msgctl(mqId, IPC_STAT, &msgbuf) == -1) {//получение статистики об очереди сообщений
        std::cout << "Problem with msgctl\n";
        exit(1);
    }
    std::cout << "Last read time: " << ctime(&msgbuf.msg_rtime);
    msgctl(mqId, IPC_RMID, nullptr);//очищение очереди сообщений
    return 0;
}
