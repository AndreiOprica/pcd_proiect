#include <stdio.h>
#include <vips/vips.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#define PORT 5001
#define SA struct sockaddr
#define m 80


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
    FILE *image = fopen("serverOut/received_from_client.png", "wb");
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
    if (!(in = vips_image_new_from_file( "serverOut/received_from_client.png", NULL )))
        vips_error_exit(NULL);

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
    
    if (vips_image_write_to_file( out, "serverOut/output.png", NULL))
        vips_error_exit(NULL);

    printf("Image proccesing ended\n");

    g_object_unref(in);
    g_object_unref(out); 

    return 0;
}

void sendImg(int connfd) {

    printf("Getting Picture Size\n");
    FILE *picture;
    picture = fopen("serverOut/output.png", "rb");

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

int main( int argc, char **argv ) {
    int sockfd, connfd, len, nready;
    struct sockaddr_in servaddr, cli;
    fd_set rset;

    if(VIPS_INIT(argv[0]))
        vips_error_exit( NULL );
   
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

    len = sizeof(cli);


    while (TRUE) {

        // Now server is ready to listen and verification
        if ((listen(sockfd, 50)) != 0) {
            printf("Listen failed...\n");
            exit(0);
        }
        else
            printf("Server listening..\n");

        len = sizeof(cli);

        FD_SET(sockfd, &rset);

        // Accept the data packet from client and verification
        connfd = accept(sockfd, (SA*)&cli, &len);
        if (connfd < 0) {
            printf("server accept failed...\n");
            exit(0);
        }
        else
            printf("server accept the client...\n");

        // Function for chatting between client and server
        if(func(connfd) == 0) {
            sendImg(connfd);
        }
        
    }

    // After chatting close the socket
    close(sockfd);

    return(0);
}