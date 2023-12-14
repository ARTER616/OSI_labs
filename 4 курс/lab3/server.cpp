#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <iostream>
#include <string>
#include <sstream>
#include <map>

// bool getStatInfoClient() {
//   FILE *file = popen("sudo netstat -tulpan | grep \"udp\" | grep \"127.0.0.1:8070\" | wc -l", "r");
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

std::string collectData(const char *textFiles) {
  std::istringstream ss(textFiles);
  std::string temp;
  std::string out;
  std::map<std::string, size_t> map_;
  std::map<std::string, size_t> map_1;
  while (getline(ss, temp)) {
    char* p;
    strtol(temp.c_str(), &p, 10);
    if (*p == 0){
      break;
    }
    size_t pos = temp.find("@"); // Находим позицию "@" в строке
    std::string second = temp.substr(pos);
    std::string first = temp.substr(0, pos);
    if (map_.find(first) != map_.end()) {
      map_[first]++;
    } else {
      map_.emplace(first, 0);
    }
    if (map_1.find(second) != map_1.end()) {
      map_1[second]++;
    } else {
      map_1.emplace(second, 0);
    }
  }
  double x = (double)map_1.size()/map_.size();
  return std::to_string(x);
}

int main() {
    sockaddr_in server_addr;
    int server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket < 0) {
        std::cout << "Error in socket()\n";
        exit(1);
    }
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &(server_addr.sin_addr));
    server_addr.sin_port = htons(8090); // Порт сервера
    int size = sizeof(server_addr);
    if (bind(
            server_socket,
            (struct sockaddr *) &server_addr,
            size) < 0) {
        std::cout << "Error in bind()\n";
        exit(2);
    }

    std::cout << "Server ready to work...\n";

    sockaddr_in client_addr;
    client_addr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &(client_addr.sin_addr));
    client_addr.sin_port = htons(8070);
    // bool flagCLIENT = false;
    // while (true) {
    //   if (getStatInfoClient()) {
    //     break;
    //   } else {
    //     if (!flagCLIENT) {
    //       std::cout << "turn on client\n";
    //       sleep(5);
    //       flagCLIENT = true;
    //       continue;
    //     } else {
    //       std::cout << "client wasn't opened\n";
    //       std::cout << "Closing server...\n";
    //       close(server_socket);
    //       exit(10);
    //     }
    //   }
    // }

    char buffer[2048];
    memset(buffer, 0, sizeof(buffer));
    if (recvfrom(server_socket, buffer, sizeof(buffer), 0, nullptr, nullptr) == -1){
        std::cout << "Error recieving message!\n";
    }
    std::cout << "Answer from client:\n" << buffer << "\n";
    char response[2048];
    memset(response, 0, 2048);
    std::string ans = collectData(buffer);
    strcpy(response, ans.c_str());
    if (sendto(server_socket, response, sizeof(response), 0, (struct sockaddr*)&client_addr, sizeof(client_addr)) == -1){
        std::cout << "Error sending message!\n";
    }

    std::cout << "Closing server...\n";
    close(server_socket);

    return 0;
}