#include <stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/errno.h>
key_t key;
int semafor;

void koniec(char text[]) {
  perror(text);
  exit(EXIT_FAILURE);
}

static void semafor_p(int x) {
  struct sembuf bufor_sem;
  bufor_sem.sem_num=x;
  bufor_sem.sem_op=-1;
  bufor_sem.sem_flg=0;
  if ((semop(semafor, &bufor_sem, 1)) == -1) {
    if(errno == 4) {
      semafor_p(x);
    } else {
      koniec("Błąd przy zamykaniu semafora!\n");
    }
  }
  //printf("Semafor %d zostal zamkniety.\n", x);
}

static void semafor_v(int x) {
  struct sembuf bufor_sem;
  bufor_sem.sem_num=x;
  bufor_sem.sem_op=1;
  bufor_sem.sem_flg=0;
  if ((semop(semafor, &bufor_sem, 1)) == -1) {
    if(errno == 4) {
      semafor_v(x);
    } else {
      koniec("Błąd przy otwieraniu semafora!\n");
    }
  }
//  printf("Semafor %d zostal otwarty.\n", x);
}

static void utworz_nowy_semafor(key_t key) {
  if((semafor = semget(key, 1, 0600 | IPC_CREAT)) == -1) koniec("[P1] Błąd przy tworzeniu nowego semafora!\n");
  //printf("[P1] Semafor został utworzony: %d\n", semafor);
}

static void stworz_sekcje_kr(int sesjaKrytyczna) {
  int i;
  for(i = 1; i <= sesjaKrytyczna; i++) {
    printf("[PRZED] Przed sekcją krytyczną nr %d, PID procesu: %d\n", i, getpid());
    semafor_p(0);
    printf("[SK] Sekcja krytyczna nr %d, PID procesu: %d\n", i, getpid());
    sleep(1);
    int waiting = semctl(semafor, 0, GETNCNT);
    printf("Procesy oczekujace: %d\n", waiting);
    semafor_v(0);
    printf("[PO] Po sekcji krytycznej nr %d, PID procesu: %d\n", i, getpid());
  }
}

int main(int argc, char * argv[]) {
  if (argc != 2) {
    printf("Nieprawidlowa liczba argumentow!\n");
    exit(5);
  }
  if((key = ftok(".", 'A')) == -1) koniec("Błąd tworzenia klucza!\n");
  utworz_nowy_semafor(key);
  stworz_sekcje_kr(atoi(argv[1]));
  exit(0);
}
