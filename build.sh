set -x

gcc -Wall fuseJson.c cJSON.c $(pkg-config fuse json-c --cflags --libs) -o fuseJson -D_FILE_OFFSET_BITS=64

chmod +x fuseJson
