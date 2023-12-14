#include <unistd.h> //access() sleep()
#include <sys/sem.h> //<sys/type.h> key_t <sys/ipc.h> IPC_CREAT, IPC_EXCL; semget(), struct sembuf(), semop()
#include <sys/shm.h> //<sys/type.h> key_t <sys/ipc.h>; shmget(), shmdt(), shmat()
#include <cstring> //memset

#include <iostream> // <stdio.h> popen() pclose() struct FILE fread() <cstdlib>  exit()
#include <string> //class string
#include <sstream> //class istringstream

#define MY_ID 2002

//union для semctl
union semun {
		int val; //значение для SETVAL
		struct semid_ds *buf; //буффер для IPC_STAT, IPC_SET
		unsigned short *array; //массив для GETALL, SETALL
		struct seminfo *__buf; //буффер для IPC_INFO
	} semops;

//функция обрабатывающая разделитель 2го сообщения
std::pair<std::string, std::string> splitString(const std::string& input) {
    std::string delimiter = "_______";
    std::size_t delimiterPos = input.find(delimiter);
    if (delimiterPos != std::string::npos) {
        std::string firstPart = input.substr(0, delimiterPos);
        std::string secondPart = input.substr(delimiterPos + delimiter.length());
        return std::make_pair(firstPart, secondPart);
    } else {
        return std::make_pair(input, "");
    }
}

//функция фильтрации шаблона "S" в строках
std::string filterStrings(const std::string& input) {
    std::string result;
    std::string delimiter = "\n";
    std::size_t pos = 0;
    std::size_t found;
    while ((found = input.find(delimiter, pos)) != std::string::npos) {
        std::string line = input.substr(pos, found - pos);
        if (line.find('s') != std::string::npos || line.find('S') != std::string::npos) {
            result += line + delimiter;
        }
        pos = found + delimiter.length();
    }
    if (pos < input.length()) {
        std::string line = input.substr(pos);
        if (line.find('s') != std::string::npos || line.find('S') != std::string::npos) {
            result += line;
        }
    }
    return result;
}

std::string processData(std::string &data) {
    std::pair<std::string, std::string> data_ = splitString(data);
    std::string result = filterStrings(data_.first) + "\n" + data_.second;
    return result;
}

//очищает память под pointer
void clearSHM(char *pointer) {
    if (pointer != nullptr) {
        memset(pointer, 0, 4096*4096);
    } else {
        std::cout << "РОП удалена!\n";
        exit(3);
    }
}

//устанавливает значение 0 для 0го значения семафора по semid
void setSems(int semid) {
    semops.val = 0;
    if (semctl(semid, 0, SETVAL, semops) != 0) {
        std::cout << "Ошибка задания значения для semops!\n";
        exit(7);
    }
}

int main() {
    const char *pathname = ".";
    key_t key;
    while (true) {
        key = ftok(pathname, MY_ID); //преобразовывает имя файла и идентификатор в ключ для системных вызовов, возвращает key_t
        if (key != -1) {
            break;
        } else {
            std::cout << "Ошибка создания айди для РОП и семафоров!\n";
            sleep(20);
        }
    }
    int semid;
    int shmid;
    if ((semid = semget(key, 1,  IPC_CREAT | IPC_EXCL | 0666)) == -1) { //получение идентификатора набора семафоров (в данном случае 1), с флагами и правами на чтение и запись для всех
    //IPC_EXCL: Этот флаг указывает, что функция должна создать новую очередь сообщений, и если очередь с таким ключом уже существует, то возвращается ошибка. Это предотвращает случайные конфликты ключей.
    //IPC_CREAT: Этот флаг указывает, что если очередь с указанным ключом не существует, она должна быть создана. Если очередь уже существует, то этот флаг игнорируется.
        std::cout << "Ошибка создания семафоров!\n";
        exit(1);
    }
    if ((shmid = shmget(key, 4096*4096, IPC_CREAT | IPC_EXCL | 0666)) == -1) { //получение идентификатора РОП, созданного размером 4096, с флагами и правами на чтение и запись
    //IPC_EXCL: Этот флаг указывает, что функция должна создать новую очередь сообщений, и если очередь с таким ключом уже существует, то возвращается ошибка. Это предотвращает случайные конфликты ключей.
    //IPC_CREAT: Этот флаг указывает, что если очередь с указанным ключом не существует, она должна быть создана. Если очередь уже существует, то этот флаг игнорируется.
        std::cout << "Ошибка создания РОП!\n";
        exit(2);
    }
    char *pointer = (char *)shmat(shmid, nullptr, 0); //подключение сегмента разделяемой памяти shmid к адресному пространству вызывающего процесса. т.к. shmaddr установлен в NULL, система выбирает неиспользованный адрес, флаги не установлены
    clearSHM(pointer); //очищение памяти под pointer
    setSems(semid); //устанавливает значение первого семафора в 0

    sembuf operation;
    operation.sem_num = 0; //номер семафора - первый
    operation.sem_op = -2; //semval += -20
    operation.sem_flg = 0; //флаги не установлены
    semop(semid, &operation, 1); //выполнение операции над семафорами, 3й аргумент - кол-во структур sembuf
    std::string getMessage = pointer;
    std::cout << "Первое поступившее сообщение:\n" << processData(getMessage) << "\n";


    std::string getMessage1 = pointer + 2048*4096;
    std::cout << "Второе поступившее сообщение:\n" << processData(getMessage1);
    std::cout << "\n\nPID сервера: " << getpid() << "\n";
    if (shmdt(pointer) != 0) { //операция обратная shmat - отключает сегмент разделяемой памяти, находящийся по адресу pointer, от адресного пространства вызывающего процесса
            std::cout << "Ошибка выполнения shmdt()\n";
            exit(4);
    }
    shmctl(shmid, IPC_RMID, nullptr); //удаляет сегмент после отключения (пометка сегмента как удаленного)
    semctl(semid, 0, IPC_RMID); //удаляет из системы набор семафоров и структуры данных запускающие процессы находящиеся в режиме ожидания
}

