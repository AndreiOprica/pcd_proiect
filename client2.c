#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#define m 80
#define PORT 5001
#define SA struct sockaddr


void sendImg(int sockfd, int typeOperation, char *image) {
	printf("Send Type Of Operation\n");
	send(sockfd, &typeOperation, sizeof(int), 0);

	

    printf("Getting Picture Size\n");
    FILE *picture;
    picture = fopen(image, "r");

    int sizePic;
    fseek(picture, 0, SEEK_END);
    sizePic = ftell(picture);
    fseek(picture, 0, SEEK_SET);

    //Send Picture Size
    printf("Sending Picture Size\n");
    send(sockfd, &sizePic, sizeof(sizePic), 0);

    //Send Picture as Byte Array
    printf("Sending Picture as Byte Array\n");
    char send_buffer[1024]; // no link between BUFSIZE and the file size
	do {
        int nb = fread(send_buffer, 1, sizeof(send_buffer), picture);
        send(sockfd, send_buffer, nb, 0);
		//printf("Send %d bytes\n",nb);
	}while(!feof(picture));

    fclose(picture);
}

void receive(int sockfd, int typeOp, char *name) {
	
	int size;
	while(recv(sockfd, &size, sizeof(int), 0)<=0) {
		printf("I am sleeping\n");
	}
    
	char *tp = "6\0";

	if (typeOp == 1){
		tp = "1\0";
	} else if (typeOp == 2) {
		tp = "2\0";
	} else if (typeOp == 3) {
		tp = "3\0";
	} else if (typeOp == 4) {
		tp = "4\0";
	} else if (typeOp == 5) {
		tp = "5\0";
	}

	printf("Reading Picture Byte Array\n");
    char p_array[1024];


	char *dir_name = "Client2/";
	char *ext = ".png";
	int len = strlen(dir_name) + strlen(name) + strlen(ext) + 2;
	char *img_name = malloc(len);
	
	if (img_name == NULL) {
		abort();
	}

	strncpy(img_name, dir_name, len);
	strncat(img_name, name, len);
	strncat(img_name, tp, len);
	strncat(img_name, ext, len);

    FILE *image = fopen(img_name, "w");

    while (size>0) {
        int nb = recv(sockfd, p_array, 1024, 0);
        if(nb<0)
            continue;
        size= size-nb;
        //printf("I read %d bytes and %d size\n",nb, size);
        fwrite(p_array, 1, nb, image);
    }

	fclose(image);
}

int main(int argc, char *argv[]) {

	int sockfd, connfd, nready;
	struct sockaddr_in servaddr, cli;
	fd_set rset;
	char *img = argv[1];
	char *new_img_name = argv[2];
	int tp;

	if (argc != 3) {
		perror("Insificient Arguments");
		exit(1);
	}

	FILE *fp = fopen(img, "r");

	if (!fp) {
		perror("The picture can't be open");
		exit(2);
	}

	while (1) {

		// socket create and verification
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd == -1) {
			printf("socket creation failed...\n");
			exit(0);
		}
		else
			printf("Socket successfully created..\n");

		bzero(&servaddr, sizeof(servaddr));

		// assign IP, PORT
		servaddr.sin_family = AF_INET;
		servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
		servaddr.sin_port = htons(PORT);

		// connect the client socket to server socket
		if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
			printf("connection with the server failed...\n");
			exit(0);
		}
		else
			printf("connected to the server..\n");

		
			printf("Choose the transformation typing the number:\n");
			printf("1 - to transform to grayscale;\n");
			printf("2 - to apply gaussian blur;\n");
			printf("3 - to rotate 90 degrees;\n");
			printf("4 - to rotate 180 degrees;\n");
			printf("5 - to rotate 270 degrees;\n");
			printf("Any key - to exit.\n");
			printf("Choice: ");
			scanf("%d", &tp);


			if (tp >= 1 && tp <= 5) {
				sendImg(sockfd, tp, img);
				receive(sockfd, tp, new_img_name);
			} else {
				exit(0);
			}
	// close the socket
	close(sockfd);
	}
}