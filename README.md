# pcd_proiect

Compile server in C:
gcc -o main.out  main.c `pkg-config vips --cflags --libs`

Run server in C:
./main.out

Compile client in C:
gcc -o client2 client2.c

Run client in C:
./client2 path_to_image name_processed_image

Run client in Python:
python3 client3.py path_to_image name_processed_image

To see the commands correct, view this file as source blob.
