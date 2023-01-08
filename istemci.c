#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 12345 // ba�lan�lacak port
#define BUFFER_SIZE 256 // maximum mesaj boyutu


//sunucudan gelen mesajlar� ekrana yazcak thread
void* process_thread(void* arg) {
  int sockfd = *(int*)arg;
  char buffer[BUFFER_SIZE];
  int n;

  // istemciden gelen mesajlar� listele
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

int main(int argc, char *argv[]) {
  int sockfd, portno;
  
	pthread_t process_tid;
  struct sockaddr_in serv_addr;
  char buffer[BUFFER_SIZE];
  int n;

  // gereken arg�manlar� kontrol et
  if (argc < 3) {
    fprintf(stderr,"kullan�m %s isim �ifre\n", argv[0]);
    exit(1);
  }

  // soket olu�tur
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    perror("ERROR soket a��lmad�");
    exit(1);
  }

  // adres structure'� olu�tur
  bzero((char *) &serv_addr, sizeof(serv_addr));

  // adres ve portu olu�tur
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(PORT);

  // sunucuya ba�lan
  if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
    perror("ERROR ba�lant�");
    exit(1);
  }

  // isim �ifreyi sunucuya yolla
  sprintf(buffer, "%s:%s", argv[1], argv[2]);
  n = write(sockfd, buffer, BUFFER_SIZE);
  if (n < 0) {
    perror("ERROR sokete yaz�lmad�");
      exit(1);
  }
  
  // sunucudan cevab� al
  bzero(buffer, BUFFER_SIZE);
  n = read(sockfd, buffer, BUFFER_SIZE - 1);
  if (n < 0) {
    perror("ERROR soketten okunamad�");
    exit(1);
  }

  // giri� ba�ar�l� m�
  if (strcmp(buffer, "Ba�ar�l� giri�!") != 0) {
    printf("Giri� ba�ar�s�z\n");
    close(sockfd);
    return 1;
  }
  
  socklen_t svlen;
  int newsockfd;
  newsockfd = accept(sockfd, (struct sockaddr *) &serv_addr, &svlen);
    if (newsockfd < 0) {
      perror("ERROR kabul edilemedi");
      exit(1);
    }
    
  //sunucudan gelen mesajlar i�in thread a�
  if (pthread_create(&process_tid, NULL, process_thread, &newsockfd) != 0) {
      perror("ERROR isleme threadi olusturulamadi");
      pthread_exit(NULL);
    }

  // sunucuya mesajlar� yolla
  while (1) {
    bzero(buffer, BUFFER_SIZE);
    printf("Mesaj� girin: ");
    fgets(buffer, BUFFER_SIZE - 1, stdin);
    n = write(sockfd, buffer, BUFFER_SIZE);
    if (n < 0) {
      perror("Sokete yaz�lamad�");
      exit(1);
    }
  }

  close(sockfd);
  return 0;
}


