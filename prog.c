#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/errno.h>
//#include <sys/wait.h>
/*#define SCRIPT "\
#!/bin/bash \n\
LIMIT=`ulimit -u` \n\
ACTIVE=`ps -u amber | wc -l`\n\
echo $LIMIT >> dane.txt\n\
echo " " >> dane.txt\n\
echo $ACTIVE >> dane.txt\n\
" */

key_t key;
int semafor;
int x;
pid_t w;

void koniec(char text[]) {
  perror(text);
  if(EXIT_FAILURE == 0) exit(0);
  exit(EXIT_FAILURE);
}

static void utworz_nowy_semafor(key_t key) {
  if((semafor = semget(key, 1, 0600 | IPC_CREAT)) == -1) koniec("Błąd przy tworzeniu nowego semafora!\n");
  printf("Semafor został utworzony: %d\n", semafor);
}

static void ustaw_semafor() {
  if((semctl(semafor, 0, SETVAL, 1)) == -1) koniec("Nie można ustawić semafora!\n");
  printf("Semafor został ustawiony.\n");
}

static void wait_semafor(int potomki) {
  int i;
  for (i = 0; i < potomki; i++) {
      w = wait(&x);
      if (w == -1) {
        koniec("Wait error");
      } else
        printf("PID potomka: %d, Kod powrotu = %d\n", w, x);
  }
}

static void usun_semafor() {
  if((semctl(semafor, 0, IPC_RMID)) == -1) koniec("Nie można usunąć semafora!\n");
  printf("Semafor został usunięty.\n");
}

int main(int argc, char* argv[]) {
 //system(SCRIPT);
  if (argc != 4) {
    printf("Nieprawidlowa liczba argumentow!\n");
    exit(5);
  }
  FILE *file;   
  file = fopen("dane.txt", "r");
  int limit, active;
  fscanf(file, "%d", &limit);
  fscanf(file, "%d", &active);
file=fopen("dane.txt", "w");
fclose(file);
  printf("Max liczba procesow: %d, Liczba aktywnych procesow: %d\n", limit-active-1, active);

  if (atoi(argv[2]) >= limit-active-1 || atoi(argv[2]) < 0) {
    printf("Podana ilosc procesow przekracza maksymalna liczbe procesow do utworzenia albo jest ujemna!");
    printf("Max ilosc do utworzenia : %d\n", limit-active-2);
        exit(0);
  }
  if (atoi(argv[3]) < 0) {
    printf("Podana ilosc sekcji krytycznych jest nieodpowiednia!");
    exit(0);
  }
  if((key = ftok(".", 'A')) == -1) koniec("[MAIN] Błąd tworzenia klucza!\n");
  printf("[MAIN] Tworzę semafora...\n");
  utworz_nowy_semafor(key);
  printf("[MAIN] Ustawiam semafora...\n");
  ustaw_semafor();

  printf("[MAIN] Tworzę %d potomkow...\n", atoi(argv[2]));
  int potomki = atoi(argv[2]);
  int i, id,j;
  for(i = 0; i < potomki; i++) {
    id = fork();
    switch(id) {
        case -1:
            printf("Błąd przy tworzeniu potomka");
                for(j=0;j<i;j++)wait(0);
                exit(1);
            break;
        case 0: //Proces potomny
            if((execl(argv[1], argv[1], argv[3], NULL)) == -1) koniec("[MAIN] Błąd przy execl!\n");
            break;
        default: //Proces macierzysty
         //wait(1);
        break;
    }
  }

  wait_semafor(potomki);
  usun_semafor();
  //file = fopen("dane.txt", "w");
  //fclose(file);
  exit(0);
}
