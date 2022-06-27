#include <stdio.h>
#include <vips/vips.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

#define SOCK_PATH "echo_socket"
#define PORT 5001
#define SA struct sockaddr
#define m 80

clock_t t;

VipsImage* grayscale(VipsImage* img) {
    VipsImage *scRGB;
    VipsImage *gray;

    if (vips_sRGB2scRGB(img, &scRGB, NULL))
        vips_error_exit(NULL);

    if (vips_scRGB2BW(scRGB, &gray, NULL))
        vips_error_exit(NULL);

    return gray;
}

VipsImage* gaussianblur(VipsImage* img) {
    VipsImage *gauss;
    VipsImage *copy;

    if (vips_copy(img, &copy, NULL))
        vips_error_exit(NULL);

    if (vips_gaussblur(copy, &gauss, 5.00, NULL))
        vips_error_exit(NULL);

    return gauss;
}

VipsImage* rotation90(VipsImage* img) {
    VipsImage *rotated90;
    VipsImage *copy;

    if (vips_copy(img, &copy, NULL))
        vips_error_exit(NULL);

    if (vips_rot90(copy, &rotated90, NULL))
        vips_error_exit(NULL);

    return rotated90; 
}

VipsImage* rotation180(VipsImage* img) {
    VipsImage *rotated180;
    VipsImage *copy;

    if (vips_copy(img, &copy, NULL))
        vips_error_exit(NULL);


    if (vips_rot180(copy, &rotated180, NULL))
        vips_error_exit(NULL);

    return rotated180;

}

VipsImage* rotation270(VipsImage* img) {
    VipsImage *rotated270;
    VipsImage *copy;

    if (vips_copy(img, &copy, NULL))
        vips_error_exit(NULL);


    if (vips_rot270(copy, &rotated270, NULL))
        vips_error_exit(NULL);

    return rotated270;

}

char *randstring(size_t length) {

    static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";        
    char *randomString = NULL;

    if (length) {
        randomString = malloc(sizeof(char) * (length +1));

        if (randomString) {            
            for (int n = 0;n < length;n++) {            
                int key = rand() % (int)(sizeof(charset) -1);
                randomString[n] = charset[key];
            }

            randomString[length] = '\0';
        }
    }

    return randomString;
}

void sendImg(int connfd, char filename[255]) {

    char folder[255] = "serverOut/";
    strcat(folder, filename);
    printf("Getting Picture Size\n");
    FILE *picture;
    picture = fopen(folder, "rb");

    int sizePic;
    fseek(picture, 0, SEEK_END);
    sizePic = ftell(picture);
    fseek(picture, 0, SEEK_SET);

    //Send Picture Size
    printf("Sending Picture Size\n");
    send(connfd, &sizePic, sizeof(sizePic), 0);
    printf("Sended Picture Size: %d\n", sizePic);

    printf("Sending Picture as Byte Array\n");
    char send_buffer[1024]; // no link between BUFSIZE and the file size
	do {

        int nb = fread(send_buffer, 1, sizeof(send_buffer), picture);
        send(connfd, send_buffer, nb, 0);

	}while (!feof(picture));

    fclose(picture);
    printf("Sended Picture as Byte Array\n");
}

int func(int connfd) {

    printf("readinf type of operation ");
    int type;
    recv(connfd, &type, sizeof(int),0);
    printf("%d \n",type);

    if (type < 1 || type > 5) {
        return 1;
    }


    printf("Reading Picture Size\n");
    int size;
    recv(connfd, &size, sizeof(int),0);
    printf("Received Picture Size: %d\n", size);

    //Read Picture Byte Array
    printf("Reading Picture Byte Array\n");
    char p_array[1024];
    char *filename;
    filename = randstring(8);
    strcat(filename, ".png");

    printf("%s\n", filename);

    char folder[255] = "serverIn/";
    strcat(folder, filename);

    printf("%s\n", folder);

    FILE *image = fopen(folder, "wb");
    int nb;
    while (size>0) {

        nb = recv(connfd, p_array, 1024, 0);
        if (nb<0)
            continue;
        size= size-nb;
        
        fwrite(p_array, 1, nb, image);

    }

    fclose(image);

    VipsImage *in;

    printf("Image proccesing started\n");
    if (!(in = vips_image_new_from_file(folder, NULL)))
        vips_error_exit(NULL);

    printf("image loaded\n");

    VipsImage *out;
    switch (type) {
    case 1:
        out = grayscale(in);
        break;
    case 2:
        out =  gaussianblur(in);
        break;
    case 3:
        out = rotation90(in);
        break;
    case 4:
        out = rotation180(in);
        break;
    case 5:
        out = rotation270(in);
        break;
    default:
        break;
    }
    
    printf("image processed\n");

    char folder1[255] = "serverOut/";
    strcat(folder1, filename);

    if (vips_image_write_to_file( out, folder1, NULL))
        vips_error_exit(NULL);

    printf("Image proccesing ended\n");

    g_object_unref(in);
    g_object_unref(out); 

    
    sendImg(connfd, filename);

    return 0;
}


void *clientHandler(void* pconfd){
    int connfd = *(int*)pconfd;
    free(pconfd);
    // Function for chatting between client and server
    if(func(connfd) == 1) {
        printf("Client disconnected");
    }
}

void *inetClient(){
    int sockfd, connfd, len, nready;
    struct sockaddr_in servaddr, cli;
    fd_set rset;

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
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);
   
    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
        printf("socket bind failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully binded..\n");


    if ((listen(sockfd, 50)) != 0) {
        printf("Listen failed...\n");
        exit(0);
    }
    else
        printf("Server listening..\n");

    while (TRUE) {

        len = sizeof(cli);

        

        // Accept the data packet from client and verification
        connfd = accept(sockfd, (SA*)&cli, &len);
        if (connfd < 0) {
            printf("server accept failed...\n");
            exit(0);
        }
        else
            printf("server accept the client...\n");
        
        pthread_t t;
        int *pconfd = malloc(sizeof(int));

        *pconfd = connfd;

        pthread_create(&t, NULL, clientHandler, pconfd);

        
        
    }

    // After chatting close the socket
    close(sockfd);
}

void *adminClient(){
    printf("thread admin\n");
}

int main( int argc, char **argv ) {
    

    if(VIPS_INIT(argv[0]))
        vips_error_exit( NULL );
   
    pthread_t thread_id[2];
    t = clock();

    pthread_create(&thread_id[0], NULL, adminClient, NULL);
    pthread_create(&thread_id[1],NULL, inetClient, NULL);

    for(int i=0;i<2;i++)
        pthread_join(thread_id[i],NULL);

    return(0);
}