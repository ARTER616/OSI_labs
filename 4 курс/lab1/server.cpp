#include <unistd.h> //sleep(), access()
#include <cstring> //strcpy()
#include <sys/msg.h> // <sys/types.h> [key_t] [ssize_t] <sys/ipc.h> ftok(); msgget(), msgsnd(), msgrcv()

#include <iostream> // <cstdio.h> fopen(), fclose(), fgetc(), EOF;
#include <string> // getline()
#include <sstream> //[istringstream]

#define MY_SND_TYPE 10

// структура сообщения с типом и текстом с заданной длиной
struct Message {
    long mtype;
    char mtext[2048];
};

//функция получения первого соощения (то есть файлов у которых в названии есть символ a)
std::string getMsg1() {
    FILE *fp;
    std::string msg_1;
    fp = popen("find . -maxdepth 1 -type f -name \"*a*\"", "r"); //открывает процесс, создавая канал, производя fork и вызывая командную оболочку, возвращает поток ввода-вывода
    if (fp == nullptr) {
        std::cout << "Error executing command shell\n";
        exit(2);
    }
    char tmp;
    size_t retVal;
    while ((retVal = fread(&tmp, 1, sizeof(tmp), fp)) != 0) {//читает файл по байтам в переменную tmp и записывает в textFile
        msg_1 += tmp;
    }
    pclose(fp);//ожидает завершения ассоциированного процесса и возвращает код выхода
    return msg_1;
}

//функция получения второго сообщения (то есть дат последнего изменения файлов)
std::string getMsg2(const std::string &messageToString) {
    std::istringstream iss(messageToString);
    std::string temp;
    std::string buffer;
    while (getline(iss, temp)) { //чтение потока istringstream в переменную temp (в данном случае используется для получения даты изменения каждого файла)
        std::string command = "date -r " + temp + " +%s";
        FILE *file = popen(command.c_str(), "r"); //открывает процесс, создавая канал, производя fork и вызывая командную оболочку, возвращает поток ввода-вывода
        if (file == nullptr) {
            std::cout << "Error executing command shell\n";
            exit(2);
        }
    char tmp;
    size_t retVal;
    std::string textFile;
    while ((retVal = fread(&tmp, 1, sizeof(tmp), file)) != 0) { //читает файл по байтам в переменную tmp и записывает в textFile
        textFile += tmp;
    }
    buffer += textFile;//запись всего в буфер
    pclose(file);//ожидает завершения ассоциированного процесса и возвращает код выхода
    }
    return buffer;
}

int main() {
    const char *pathname = ".";
    key_t key;
    while (true) {
        key = ftok(pathname, 'A');//преобразовывает имя файла и идентификатор в ключ для системных вызовов, возвращает key_t
        if (key != -1) {
            break;
        } else {
            std::cout << "Error creating unique id for message queue\n";
            sleep(20);
        }
    }
    int mqId = msgget(key, IPC_EXCL | IPC_CREAT | 0666); //получает идентификатор созданной очереди сообщений, принимает ключ созданный ftok
    //IPC_EXCL: Этот флаг указывает, что функция должна создать новую очередь сообщений, и если очередь с таким ключом уже существует, то возвращается ошибка. Это предотвращает случайные конфликты ключей.
    //IPC_CREAT: Этот флаг указывает, что если очередь с указанным ключом не существует, она должна быть создана. Если очередь уже существует, то этот флаг игнорируется.
    //0666: Это маска прав доступа (режим), устанавливающая права доступа к очереди сообщений. В данном случае, 0666 означает, что у всех пользователей (владельца, группы и остальных) есть права на чтение и запись в очередь сообщений.
    
    //проверка создания очереди
    if (mqId == -1) {
        std::cout << "Impossible create message queue\n";
        return 1;
    } else {
        std::cout << "Message queue ID: " << mqId << "\n";
    }
    std::string msg_1 = getMsg1();
    Message message1;
    message1.mtype = MY_SND_TYPE;
    memset(message1.mtext, 0, sizeof(message1.mtext));//установка всех байтов массива в 0
    strcpy(message1.mtext, msg_1.c_str());//запись стринга преобразованного в массив чаров в message1.mtext
    if (msgsnd(mqId, &message1, sizeof(message1.mtext), 0) == -1) {//отправка сообщения, флаги не установлены
        std::cout << "Trouble sending of message:\n" << message1.mtext << "\n";
    }
    std::string msg_2 = getMsg2(msg_1);
    Message message2;
    message2.mtype = MY_SND_TYPE;
    memset(message2.mtext, 0, sizeof(message2.mtext));
    strcpy(message2.mtext, msg_2.c_str());
    if (msgsnd(mqId, &message2, sizeof(message2.mtext), 0) == -1) {
        std::cout << "Trouble sending of message:\n" << message2.mtext << "\n";
    }
    exit(0);
}
