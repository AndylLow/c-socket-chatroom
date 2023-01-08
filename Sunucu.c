#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 12345 // dinlenecek port
#define MAX_CLIENTS 4 // maximum istenci sayýsý
#define BUFFER_SIZE 256 // maximum mesaj boyutu

// login için kullanýcý structure'ý
struct User {
  char* username;
  char* password;
};

// kullanýcý listesi
struct User users[MAX_CLIENTS] = {
  {"user1", "pass1"},
  {"user2", "pass2"},
  {"user3", "pass3"},
  {"user4", "pass4"}
};


// istemciden gelen mesajlarý iþleme
void* process_thread(void* arg) {
  int sockfd = *(int*)arg;
  char buffer[BUFFER_SIZE];
  int n;

  // istemciden gelen mesajlarý listele
  while (1) {
    bzero(buffer, BUFFER_SIZE);
    n = read(sockfd, buffer, BUFFER_SIZE - 1);
    if (n < 0) {
      perror("ERROR soketten okuma");
      pthread_exit(NULL);
    }
    printf("Mesaj: %s\n", buffer);
  }

  pthread_exit(NULL);
}

//gelen istekleri dinleme threadi
void* listen_thread(void* arg) {
	int sockfd, newsockfd, portno;
	pthread_t process_tid;
  socklen_t clilen;
  char buffer[BUFFER_SIZE];
  struct sockaddr_in serv_addr, cli_addr;
  int n, i;

  // soket oluþtur
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    perror("ERROR soket açýlmadý");
    exit(1);
  }

  // adres structure'ýný teimzle
  bzero((char *) &serv_addr, sizeof(serv_addr));

  // adresi ve portu oluþtur
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(PORT);

  // soketi adres ve porta ata
  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    perror("ERROR soketi adres ve porta atama");
    exit(1);
  }

  // gelen istekleri dinle
  listen(sockfd, MAX_CLIENTS);
  clilen = sizeof(cli_addr);

  // gelen baðlantýyý kabul et
  while (1) {
    // yeni baðlantý kabul et
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) {
      perror("ERROR kabul edilemedi");
      exit(1);
    }

    // kullanýcýya giriþ yap
    bzero(buffer, BUFFER_SIZE);
    n = read(newsockfd, buffer, BUFFER_SIZE - 1);
    if (n < 0) {
      perror("ERROR soketten okunamadý");
      exit(1);
    }
    char* username = strtok(buffer, ":");
    char* password = strtok(NULL, ":");
    int authenticated = 0;
    for (i = 0; i < MAX_CLIENTS; i++) {
      if (strcmp(username, users[i].username) == 0 && strcmp(password, users[i].password) == 0) {
        authenticated = 1;
        break;
      }
    }

    if (!authenticated) {
      close(newsockfd);
      continue;
    }

    // giriþ yapýldýðný gönder
    n = write(newsockfd, "Basarili giris!", BUFFER_SIZE);
    if (n < 0) {
      perror("ERROR sokete yazýlmadý");
      exit(1);
    }

    // gelen mesajlarý iþleme threadi olustur
    if (pthread_create(&process_tid, NULL, process_thread, &newsockfd) != 0) {
      perror("ERROR isleme threadi olusturulamadi");
      pthread_exit(NULL);
    }
  }

  close(sockfd);
	pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
  pthread_t listen_tid;

  // dinleme threadi 
  if (pthread_create(&listen_tid, NULL, listen_thread, NULL) != 0) {
    perror("ERROR dinleme threadi hatasý");
    exit(1);
  }

  // dinleme threadini belke
  if (pthread_join(listen_tid, NULL) != 0) {
    perror("ERROR dinleme threadine birleþmedi");
    exit(1);
  }

  return 0;
}


