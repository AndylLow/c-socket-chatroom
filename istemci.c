#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 12345 // baðlanýlacak port
#define BUFFER_SIZE 256 // maximum mesaj boyutu


//sunucudan gelen mesajlarý ekrana yazcak thread
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

int main(int argc, char *argv[]) {
  int sockfd, portno;
  
	pthread_t process_tid;
  struct sockaddr_in serv_addr;
  char buffer[BUFFER_SIZE];
  int n;

  // gereken argümanlarý kontrol et
  if (argc < 3) {
    fprintf(stderr,"kullaným %s isim þifre\n", argv[0]);
    exit(1);
  }

  // soket oluþtur
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    perror("ERROR soket açýlmadý");
    exit(1);
  }

  // adres structure'ý oluþtur
  bzero((char *) &serv_addr, sizeof(serv_addr));

  // adres ve portu oluþtur
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(PORT);

  // sunucuya baðlan
  if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
    perror("ERROR baðlantý");
    exit(1);
  }

  // isim þifreyi sunucuya yolla
  sprintf(buffer, "%s:%s", argv[1], argv[2]);
  n = write(sockfd, buffer, BUFFER_SIZE);
  if (n < 0) {
    perror("ERROR sokete yazýlmadý");
      exit(1);
  }
  
  // sunucudan cevabý al
  bzero(buffer, BUFFER_SIZE);
  n = read(sockfd, buffer, BUFFER_SIZE - 1);
  if (n < 0) {
    perror("ERROR soketten okunamadý");
    exit(1);
  }

  // giriþ baþarýlý mý
  if (strcmp(buffer, "Baþarýlý giriþ!") != 0) {
    printf("Giriþ baþarýsýz\n");
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
    
  //sunucudan gelen mesajlar için thread aç
  if (pthread_create(&process_tid, NULL, process_thread, &newsockfd) != 0) {
      perror("ERROR isleme threadi olusturulamadi");
      pthread_exit(NULL);
    }

  // sunucuya mesajlarý yolla
  while (1) {
    bzero(buffer, BUFFER_SIZE);
    printf("Mesajý girin: ");
    fgets(buffer, BUFFER_SIZE - 1, stdin);
    n = write(sockfd, buffer, BUFFER_SIZE);
    if (n < 0) {
      perror("Sokete yazýlamadý");
      exit(1);
    }
  }

  close(sockfd);
  return 0;
}


