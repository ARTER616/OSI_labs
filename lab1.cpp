#include <iostream>
#include <unistd.h> //pipe, fork, close, dup2, sleep
#include <csignal> //sigaction, sigprogmask, sigemptyset,

enum PIPES { READ, WRITE };
enum {BUF_SIZE = 512};
int interrupt_counter = 0;//счетчик прерываний

void interrupt_handler(){//обработчик прерываний
    interrupt_counter++;
    std::cout<<std::endl<<"Прерывание "<<interrupt_counter<<std::endl;
    if(interrupt_counter % 5 == 0){//проверка каждых 5 прерываний

        std::cout<<std::endl;
        system("du -s");


        //Код ниже выводит информацию о общем количестве блоков в межпроцессный канал
        /*
        int prc_desc[2];
        pid_t childpid;
        pipe(prc_desc);//Создание межпроцессного канала
        if((childpid = fork()) == 0){//создание параллельного процесса
            close(WRITE);//Закрытие стандартного вывода
            close(prc_desc[READ]);//Закрытие межпроцессного канала на чтение
            dup2(prc_desc[WRITE], WRITE);//Дублирование дескриптора межпроцессного канала на стандартный вывод
            close(prc_desc[WRITE]);//Закрытие канала на запись
            system("du -s");//Вывод общего количества блоков du -s
            exit(1);//Завершение с кодом 1
        }
        else if(childpid == -1){
            perror("Ошибка при создании дочернего процесса!");//Вернуть ошибку в случае возврата -1 при создании дочернего процесса
            exit(1);//Завершение программы с кодом 1
        }
        else{
            waitpid(childpid, nullptr, 0);//Ожидание завершения дочернего процесса
            close(prc_desc[WRITE]);//Закрытие межпроцессного канала на запись
            FILE* read_file = fdopen(prc_desc[READ], "r");
            int r;
            fscanf(read_file, "%d", &r);
            fclose(read_file);
            std::cout<<"Всего блоков: "<<r<<std::endl<<std::endl;
            wait(nullptr);
        }
         */
    }
}

int main(){
    //объявление обработчика прерываний
    struct sigaction keyboard_interrupt{};//обработчик специфичных системных сигналов с клавиатуры
    keyboard_interrupt.sa_handler = reinterpret_cast<void (*)(int)>(interrupt_handler);//замена на собственный обработчик
    sigemptyset(&keyboard_interrupt.sa_mask);//запрет использования других сигналов
    sigprocmask(0, nullptr, &keyboard_interrupt.sa_mask);//сохранение маски
    keyboard_interrupt.sa_flags = 0;//не выполнять другие действия
    sigaction(SIGINT, &keyboard_interrupt, nullptr);//запуск обработчика

    int prc_desc[2];
    pid_t childpid;
    pipe(prc_desc);//Создание межпроцессного канала
    if((childpid = fork()) == 0){//Распараллеливание процесса
        //дочерний процесс
        std::cout<<"Дочерний процесс: "<<childpid<<std::endl<<std::endl;
        close(WRITE);//Закрытие стандартного вывода
        close(prc_desc[READ]);//Закрытие межпроцессного канала на чтение
        dup2(prc_desc[WRITE], WRITE);//Дублирование дескриптора межпроцессного канала на стандартный вывод
        close(prc_desc[WRITE]);//Закрытие канала на запись
        system("du -c");//Вывод количества блоков
    }
    else if(childpid == -1){
        perror("Ошибка при создании дочернего процесса!");
        exit(1);
    }
    else{
        //Родительский процесс
        std::cout<<"Родительский процесс: "<<childpid<<std::endl<<std::endl;//fork передает pid дочернего процесса в родит

        //Если закомментировать строчку ниже - начнет выводить строки только после завершения дочернего процесса
        //иначе - начнет выводить сразу
        //waitpid(childpid, nullptr, 0);//Ожидание завершения дочернего процесса
        
        char buf[BUF_SIZE] = {};
        close(prc_desc[WRITE]);//Закрытие межпроцессного канала на запись
        int r = 0;
        dup2(prc_desc[READ], READ);//Дублирование дескриптора межпроцессного канала на чтение
        while(scanf("%d %[^\n]", &r, buf) != EOF){//считывание из межпроцессного канала
            if(r > 4 && strcmp(buf, "total") != 0 && strcmp(buf, ".") != 0){//проверка кол-ва блоков
                std::cout << r << " " << buf << std::endl;                  //для красоты исключены . и total
                sleep(1); //приостановка исполнения на 1 секунду
            }
        }
        memset(buf, 0, BUF_SIZE);//обнуление памяти под buf
        close(prc_desc[READ]);//Закрытие межпроцессного канала на чтение
        wait(nullptr); //блокирует родительский процесс до завершения всех дочерних
    }
    exit(1);//Завершение программы с кодом 1
}
