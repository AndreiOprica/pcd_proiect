# pcd_proiect

Compile server in C:
gcc -o main.out  main.c `pkg-config vips --cflags --libs`

Run server in C:
./main.out

Compile admin client:
gcc -o client1 client1.c

Run admin client:
./client1

Compile client in C:
gcc -o client2 client2.c

Run client in C:
./client2 path_to_image name_processed_image

Run client in Python:
python3 client3.py path_to_image name_processed_image

To see the commands correct, view this file as source blob.
To work properly the program needs in the project folder three folders:
  - 1st named: serverIn
  - 2nd named: serverOut
  - 3rd named: Client
