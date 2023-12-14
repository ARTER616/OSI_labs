#include <unistd.h>//access() sleep() 
#include <sys/sem.h> //<sys/type.h> key_t <sys/ipc.h> IPC_CREAT, IPC_EXCL; semget(), struct sembuf(), semop() 
#include <sys/shm.h> //<sys/type.h> key_t <sys/ipc.h> IPC_RMD; shmget(), shmdt(), shmat() 
#include <sys/msg.h>
#include <iostream>// <stdio.h> popen() pclose() struct FILE fread() <cstdlib>  exit() cstdlib() 
#include <string>//class string 
 
#define MY_ID 2002 
 
void collectData(std::string &ps_output) { 
    FILE *fp; 
    fp = popen("ps -af", "r"); 
    if (fp == nullptr) { 
        std::cout << "Ошибка выполнения команды!\n"; 
        exit(2); 
    } 
    char tmp; 
    size_t retVal; 
    while ((retVal = fread(&tmp, 1, sizeof(tmp), fp)) != 0) { 
        ps_output += tmp; 
    } 
    pclose(fp); 
    ps_output += "_______"; 
} 
 
int main() { 
    const char *pathname = "."; 
    key_t key; 
    while (true) { 
        key = ftok(pathname, MY_ID); 
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
        if ((semid = semget(key, 1, 0666)) == -1) { 
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
    sembuf operation; 
    operation.sem_num = 0; //номер семафора - первый
    operation.sem_op = -1; //semval += -14
    operation.sem_flg = 0; //флаги не установлены
    semop(semid, &operation, 1); 
    while (true) { 
        if ((shmid = shmget(key, 4096*4096, 0666)) == -1) { 
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
    char *pointer = (char *)shmat(shmid, nullptr, 0); 
    if (pointer == nullptr) { 
        std::cout << "Ошибка выполнения shmat()\n"; 
        exit(6); 
    } 

    std::string tmp; 
    collectData(tmp); 
 
 
    //получение идентификатора процесса создателя РОП
    shmid_ds sharedMemoryInfo; //буффер объект структуры smdid_ds
    shmctl(shmid, IPC_STAT, &sharedMemoryInfo);//получение информации о РОП, копирует информацию о сегменте в буффер
 
    tmp += "\nИдентификатор процесса создателя РОП: ";
    tmp += std::to_string(sharedMemoryInfo.shm_cpid);//получение поля shm_cpid - идентификатора процесса создателя

    for (size_t i = 2048*4096; i < tmp.size()+2048*4096 && i < 4096*4096; ++i) { 
        pointer[i] = tmp[i-2048*4096]; 
    } 
    tmp = pointer + 2048*4096; 
    if (shmdt(pointer) != 0) { 
            std::cout << "Ошибка выполнения shmdt()\n"; 
            exit(4); 
    } 
    std::cout << tmp << "\n"; 
    operation.sem_num = 0; 
    operation.sem_flg = 0; 
    operation.sem_op = 2;
    semop(semid, &operation, 1); 
}
