#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctime>

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <map>

// bool getStatInfoServer() {
//   FILE *file = popen("sudo netstat -tulpan | grep \"udp\" | grep \"127.0.0.1:8090\" | wc -l", "r");
//   if (file == nullptr) {
//       std::cout << "Error executing command shell\n";
//       exit(2);
//   }
//   char tmp;
//   size_t retVal;
//   std::string perm;
//   //while ((retVal = fread(&tmp, 1, sizeof(tmp), file)) != 0) {
//   //    perm += tmp;
//   //}
//   fread(&tmp, 1, sizeof(tmp), file);
//   perm = tmp;
//   pclose(file);
//   if (perm == "1") { // "1\n"
//     return true;
//   }
//   return false;
// }

std::string DataToServer() {
  FILE *file1 = popen("mail -p | grep -P \'From:|Date:\'", "r");//открывает процесс, создавая канал, производя fork и вызывая командную оболочку, возвращает поток ввода-вывода
  if (file1 == nullptr) {
      std::cout << "Error executing command shell\n";
      exit(2);
  }
  char tmp;
  size_t retVal;
  std::string perm;
  while ((retVal = fread(&tmp, 1, sizeof(tmp), file1)) != 0) {
      perm += tmp;
  }
  pclose(file1);
  //
  FILE *file2 = popen("mail -p | grep -P \'From:\' | wc -l", "r");
  if (file2 == nullptr) {
      std::cout << "Error executing command shell\n";
      exit(2);
  }
  while ((retVal = fread(&tmp, 1, sizeof(tmp), file2)) != 0) {
      perm += tmp;
  }
  //
  pclose(file2);
  std::cout << perm;
  std::istringstream ss(perm);
  std::string temp;
  std::string out;
  struct tm tm_time = {0};
  time_t time_;
  while (getline(ss, temp)) {
    size_t pos = temp.find("Date:"); // Находим позицию "Date:" в строке
        char* p;
        strtol(temp.c_str(), &p, 10);
        if (pos != std::string::npos) {
            pos += 6; // Перемещаем позицию на позицию после "Date:"
            std::string dateStr = temp.substr(pos); // Создаем подстроку после "Date:"

            strptime(dateStr.c_str(), "%a, %d %b %Y %H:%M:%S", &tm_time);
            time_ = mktime(&tm_time);
        } else if (*p == 0){
            out += temp;
        }
        else {
            size_t pos1 = temp.find("From:"); // Находим позицию "From:" в строке
            pos1 += 6;
            std::string from_ = temp.substr(pos1); // Создаем подстроку после "From"
            time_t currentTime = std::time(0);
            if (currentTime - time_ < 604800) {
              out += from_ + "\n";
            }
        }
  }
  return out;
}

int main() {
    sockaddr_in client_addr;
    //creates connection endpoint and returns descriptor
    int client_socket = socket(AF_INET, SOCK_DGRAM, 0); //0 - default protocol (UDP)
    if (client_socket == -1) {
        std::cout << "Error in socket()\n";
        exit(1);
    }
    //sets address family to client_addr object
    client_addr.sin_family = AF_INET;

    //converts string os symbols (127.0.0.1) to network address in format AF_INET and stores structure in client_addr.sin_addr 
    inet_pton(AF_INET, "127.0.0.1", &(client_addr.sin_addr));
    //sets port of client 
    client_addr.sin_port = htons(8070); // converts from host byte order to network byte order
    int size = sizeof(client_addr);
    //binds a unique local name to the socket with size
    if (bind(client_socket, (struct sockaddr *) &client_addr, size) == -1) {
        std::cout << "Error in bind()\n";
        exit(2);
    }
    std::cout << "Client ready to work...\n";
    sockaddr_in server_addr;
    socklen_t server_addr_length = sizeof(server_addr);
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &(server_addr.sin_addr));
    //sets port of server
    server_addr.sin_port = htons(8090); // converts from host byte order to network byte order
    // bool flagSERVER = false;
    // while (true) {
    //   if (getStatInfoServer()) {
    //     break;
    //   } else {
    //     if (!flagSERVER) {
    //       std::cout << "turn on server\n";
    //       sleep(5);
    //       flagSERVER = true;
    //       continue;
    //     } else {
    //       std::cout << "server wasn't opened\n";
    //       std::cout << "Closing client...\n";
    //       close(client_socket);
    //       exit(10);
    //     }
    //   }
    // }

    char message1[2048];
    memset(message1, 0, sizeof(message1));
    std::string tmp = DataToServer();
    strcpy(message1, tmp.c_str());

    // if (sendto(client_socket, message1, sizeof(message1), 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
    //     std::cout << "Error sending message!\n";
    //     exit(10);
    // }
    char buffer[2048];
    memset(buffer, 0, sizeof(buffer));
    // recvfrom(client_socket, buffer, sizeof(buffer), 0, (struct sockaddr*)&server_addr, &server_addr_length);

    while (recvfrom(client_socket, buffer, sizeof(buffer), MSG_DONTWAIT, (struct sockaddr*)&server_addr, &server_addr_length) == -1) {
        std::cout << "Attemptimng access server...\n";
        if (sendto(client_socket, message1, sizeof(message1), 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
            std::cout << "Error sending message!\n";
            exit(10);
        }
        sleep(5);
    }
    std::cout << "Message sent!\n";
    std::cout << "Answer from server:\n" << std::setprecision(2) << std::fixed << strtod(buffer,nullptr) << "\n";

    std::cout << "Closing client...\n";
    close(client_socket);
    return 0;
}