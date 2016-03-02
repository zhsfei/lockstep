#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <stdint.h>
#include "../shared.h"

static volatile bool Running;

#define CLIENT_MAX 4

struct client {
  int FD;
};

struct client_set {
  client Clients[CLIENT_MAX];
  int MaxFDPlusOne;
  ui32 Count;
};

static inline int MaxInt(int A, int B) {
  return A > B ? A : B;
}

static void HandleSigint(int signum) {
  Running = false;
}

#define TEST_BUFFER_SIZE 4096
ui8 TestBuffer[TEST_BUFFER_SIZE];

void UpdateClientSet(client_set *Set) {
  Set->MaxFDPlusOne = 0;
  for(ui32 I=0; I<Set->Count; ++I) {
    Set->MaxFDPlusOne = MaxInt(Set->MaxFDPlusOne, Set->Clients[I].FD + 1);
  }
}

int main() {
  fd_set ClientFDSet;

  Running = true;
  client_set ClientSet;
  ClientSet.Count = 0;
  ClientSet.MaxFDPlusOne = 0;

  signal(SIGINT, HandleSigint);

  int HostFD = socket(PF_INET, SOCK_STREAM, 0);
  if(HostFD < 0) {
    printf("Could not get socket.\n");
    return 1;
  }
  fcntl(HostFD, F_SETFL, O_NONBLOCK);

  struct sockaddr_in Address;
  memset(&Address, 0, sizeof(Address));
  Address.sin_len = sizeof(Address);
  Address.sin_family = AF_INET;
  Address.sin_port = htons(4321);
  Address.sin_addr.s_addr = INADDR_ANY;

  if(bind(HostFD, (struct sockaddr *)&Address, sizeof(Address)) < 0) {
    printf("Bind failed.\n");
    return 1;
  }

  if(listen(HostFD, 5) < 0) {
    printf("Listen failed.\n");
    return 1;
  }

  printf("Listening...\n");

  int SelectResult;
  struct timeval Timeout;
  while(Running) {
    FD_ZERO(&ClientFDSet);
    for(ui32 I=0; I<ClientSet.Count; ++I) {
      FD_SET(ClientSet.Clients[I].FD, &ClientFDSet);
    }

    Timeout.tv_sec = 0;
    Timeout.tv_usec = 5000;

    SelectResult = select(ClientSet.MaxFDPlusOne, &ClientFDSet, NULL, NULL, &Timeout);
    if(SelectResult == -1) {
      if(errno == errno_code_interrupted_system_call) {
        break;
      }
      else {
        printf("Select failed (%s).\n", strerror(errno));
        return 1;
      }
    }
    else if(SelectResult != 0) {
      for(ui32 I=0; I<ClientSet.Count; ++I) {
        client *Client = ClientSet.Clients + I;
        if(FD_ISSET(Client->FD, &ClientFDSet)) {
          ssize_t Result = recv(Client->FD, TestBuffer, TEST_BUFFER_SIZE, 0); // TODO: Loop until you have all
          if(Result == 0) {
            ClientSet.Count--;
            UpdateClientSet(&ClientSet);
            printf("Disconnected.\n");
          }
          else {
            printf("Got something\n");
          }
        }
      }
    }

    int ClientFD = accept(HostFD, NULL, NULL);
    if(ClientFD != -1) {
      ClientSet.Clients[0].FD = ClientFD;
      ClientSet.Count++;
      ClientSet.MaxFDPlusOne = MaxInt(ClientFD + 1, ClientSet.MaxFDPlusOne);
      printf("Someone connected!\n");
    }
  }

  close(HostFD);
  printf("\nGracefully terminated.");
  return 0;
}