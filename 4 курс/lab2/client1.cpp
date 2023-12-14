#include <unistd.h>//access() sleep() 
#include <sys/sem.h> //<sys/type.h> key_t <sys/ipc.h> IPC_CREAT, IPC_EXCL; semget(), struct sembuf(), semop() 
#include <sys/shm.h> //<sys/type.h> key_t <sys/ipc.h> IPC_RMD; shmget(), shmdt(), shmat() 
 
#include <iostream>// <stdio.h> popen() pclose() struct FILE fread() <cstdlib>  exit() cstdlib() 
#include <string>//class string 
 
#define MY_ID 2002 
 
//функция получения информации о процессах и записи в string
void collectData(std::string &ps_output) { 
    FILE *fp; 
    fp = popen("ps -ef | grep -v -P 'ttys|pts'", "r"); //открывает процесс, создавая канал, производя fork и вызывая командную оболочку, возвращает поток ввода-вывода
    if (fp == nullptr) { 
        std::cout << "Ошибка выполнения команды!\n"; 
        exit(2); 
    } 
    char tmp; 
    size_t retVal; 
    while ((retVal = fread(&tmp, 1, sizeof(tmp), fp)) != 0) { //читает файл по байтам в переменную tmp и записывает в textFile
        ps_output += tmp; 
    } 
    pclose(fp); //ожидает завершения ассоциированного процесса и возвращает код выхода
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
    bool flag = 0; 
    while (true) { 
        if ((semid = semget(key, 1, 0666)) == -1) { //получение идентификатора набора семафоров (в данном случае 1), без флагов но с правами на чтение и запись для всех
            if (flag) { 
                exit(1); 
            } 
            std::cout << "Сервер не открыт!\n"; 
            sleep(10); 
            flag = 1; 
        } else { 
            break; 
        } 
    } 
    while (true) { 
        if ((shmid = shmget(key, 4096*4096, 0666)) == -1) { //получение идентификатора РОП, созданного размером 4096, с правами на чтение и запись, флаги не установлены
            if (flag) { 
                exit(1); 
            } 
            std::cout << "Сервер не открыт!\n"; 
            sleep(10); 
            flag = 1; 
        } else { 
            break; 
        } 
    } 
 
    char *pointer = (char *)shmat(shmid, nullptr, 0); //подключение сегмента разделяемой памяти shmid к адресному пространству вызывающего процесса. т.к. shmaddr установлен в NULL, система выбирает неиспользованный адрес, флаги не установлены
    if (pointer == nullptr) { 
        std::cout << "Ошибка выполнения shmat()\n"; 
        exit(6); 
    } 

    std::string tmp; 
    collectData(tmp); 
    for (size_t i = 0; i < tmp.size() && i < 4096*4096; ++i) { //запись из tmp в РОП
        pointer[i] = tmp[i]; 
    } 
    tmp = pointer + 1; 
    if (shmdt(pointer) != 0) { //операция обратная shmat - отключает сегмент разделяемой памяти, находящийся по адресу pointer, от адресного пространства вызывающего процесса
            std::cout << "Ошибка выполнения shmdt()\n"; 
            exit(4); 
    } 
    std::cout << tmp << "\n"; 
    sembuf operation; 
    operation.sem_num = 0; //номер семафора - первый
    operation.sem_flg = 0; //флаги не установлены
    operation.sem_op = 1; //semval +=1
    semop(semid, &operation, 1); //выполнение операции над семафорами, 3й аргумент - кол-во структур sembuf
}
