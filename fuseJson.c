/* taken from https://github.com/fntlnz/fuse-example, and modified */


#define FUSE_USE_VERSION 26

#include <stdio.h>
#include <fuse.h>
#include <string.h>
#include <errno.h>
#include <json.h>
#include "cJSON.h"

typedef struct {
    char* name;
    int inode;
} Entry;

typedef struct {
    int inode;
    char* type;
    Entry* entries;
    int numEntries;
    char* data;
} Node;


int arraySize;
cJSON* root;
Node* nodes;
char *filecontent;

static int getattr_callback(const char *path, struct stat *stbuf) {
  memset(stbuf, 0, sizeof(struct stat));

  if (strcmp(path, "/") == 0) {
    stbuf->st_mode = S_IFDIR | 0755;
    stbuf->st_nlink = 2;
    return 0;
  }

  if (strcmp(path, "/output.json") == 0) {
    stbuf->st_mode = S_IFREG | 0777;
    stbuf->st_nlink = 1;
    stbuf->st_size = strlen(filecontent);
    return 0;
  }

  return -ENOENT;
}

static int readdir_callback(const char *path, void *buf, fuse_fill_dir_t filler,
    off_t offset, struct fuse_file_info *fi) {

  if (strcmp(path, "/") == 0) {

	  filler(buf, ".", NULL, 0);
	  filler(buf, "..", NULL, 0);
	  filler(buf, "output.json", NULL, 0);
	  return 0;
  }

  return -ENOENT ;
}

static int open_callback(const char *path, struct fuse_file_info *fi) {
  return 0;
}

static int read_callback(const char *path, char *buf, size_t size, off_t offset,
    struct fuse_file_info *fi) {

  if (strcmp(path, "/output.json") == 0) {
    size_t len = strlen(filecontent);
    if (offset >= len) {
      return 0;
    }

    if (offset + size > len) {
      memcpy(buf, filecontent + offset, len - offset);
      return len - offset;
    }

    memcpy(buf, filecontent + offset, size);
    return size;
  }

  return -ENOENT;
}

static struct fuse_operations fuse_example_operations = {
  .getattr = getattr_callback,
  .open = open_callback,
  .read = read_callback,
  .readdir = readdir_callback,
};

cJSON* createJsonFromNodeArray(Node* nodes, int numNodes) {
    cJSON* jsonNodesArray = cJSON_CreateArray();

    for (int i = 0; i < numNodes; i++) {
        cJSON* jsonNode = cJSON_CreateObject();
        cJSON_AddNumberToObject(jsonNode, "inode", nodes[i].inode);
        cJSON_AddStringToObject(jsonNode, "type", nodes[i].type);

        if (nodes[i].entries != NULL && nodes[i].numEntries > 0) {
            cJSON* entriesArray = cJSON_CreateArray();
            for (int j = 0; j < nodes[i].numEntries; j++) {
                cJSON* entry = cJSON_CreateObject();
                cJSON_AddStringToObject(entry, "name", nodes[i].entries[j].name);
                cJSON_AddNumberToObject(entry, "inode", nodes[i].entries[j].inode);
                cJSON_AddItemToArray(entriesArray, entry);
            }
            cJSON_AddItemToObject(jsonNode, "entries", entriesArray);
        }

        if (nodes[i].data != NULL) {
            cJSON_AddStringToObject(jsonNode, "data", nodes[i].data);
        }

        cJSON_AddItemToArray(jsonNodesArray, jsonNode);
    }

    return jsonNodesArray;
}

void saveNodesToJsonFile(Node* nodes, int numNodes) {
    cJSON* jsonNodesArray = createJsonFromNodeArray(nodes, numNodes);
    filecontent = cJSON_Print(jsonNodesArray);

    // FILE* file = fopen(filename, "w");
    // if (file != NULL) {
    //     fputs(filecontent, file);
    //     fclose(file);
    // }
    // free(filecontent);
    cJSON_Delete(jsonNodesArray);
}

void parseEntries(cJSON* entriesArray, Entry** entries, int* numEntries) {
    if (entriesArray == NULL || !cJSON_IsArray(entriesArray)) {
        *entries = NULL;
        *numEntries = 0;
        return;
    }

    int size = cJSON_GetArraySize(entriesArray);
    *numEntries = size;
    *entries = malloc(size * sizeof(Entry));

    for (int i = 0; i < size; i++) {
        cJSON* entry = cJSON_GetArrayItem(entriesArray, i);
        cJSON* name = cJSON_GetObjectItem(entry, "name");
        cJSON* inode = cJSON_GetObjectItem(entry, "inode");

        if (name != NULL && inode != NULL) {
            (*entries)[i].name = strdup(name->valuestring);
            (*entries)[i].inode = inode->valueint;
        }
    }
}

void parseJson(const char* jsonString) {
    root = cJSON_Parse(jsonString);
    if (root == NULL) {
        printf("JSON 파싱 오류: %s\n", cJSON_GetErrorPtr());
        return;
    }

    if (cJSON_IsArray(root)) {
        arraySize = cJSON_GetArraySize(root);
        nodes = malloc(arraySize * sizeof(Node));

        for (int i = 0; i < arraySize; i++) {
            cJSON* item = cJSON_GetArrayItem(root, i);
            if (item != NULL) {
                cJSON* inode = cJSON_GetObjectItem(item, "inode");
                cJSON* type = cJSON_GetObjectItem(item, "type");
                cJSON* entries = cJSON_GetObjectItem(item, "entries");
                cJSON* data = cJSON_GetObjectItem(item, "data");

                if (inode != NULL && type != NULL) {
                    nodes[i].inode = inode->valueint;
                    nodes[i].type = strdup(type->valuestring);
                }

                if (entries != NULL) {
                    parseEntries(entries, &(nodes[i].entries), &(nodes[i].numEntries));
                }

                if (data != NULL) {
                    nodes[i].data = strdup(data->valuestring);
                }
            }
        }
        // // 메모리 해제
        // for (int i = 0; i < arraySize; i++) {
        //     free(nodes[i].type);
        //     for (int j = 0; j < nodes[i].numEntries; j++) {
        //         free(nodes[i].entries[j].name);
        //     }
        //     free(nodes[i].entries);
        //     free(nodes[i].data);
        // }
        // free(nodes);
    }
}

int main(int argc, char *argv[])
{
	// struct json_object * fs_json = json_object_from_file("examplejson.json") ; 
  FILE *file = fopen("examplejson.json","r");
    if(file == NULL){
        printf("파일을 열 수 없음.");
        return 1;
    }
    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    rewind(file);

    char* jsonfile = (char*)malloc(size+1);
    if(jsonfile == NULL){
        printf("메모리를 할당할 수 없습니다.");
        return 1;
    }
    int read_size = fread(jsonfile, 1, size, file);
    if(read_size != size){
        printf("파일을 읽어올 수 없음");
        return 1;
    }

    fclose(file);
    jsonfile[size] = '\0';

    parseJson(jsonfile);
    for (int i = 0; i < arraySize; i++) {
      printf("Node %d: inode: %d, type: %s\n", i, nodes[i].inode, nodes[i].type);

      for (int j = 0; j < nodes[i].numEntries; j++) {
          printf("  Entry %d: name: %s, inode: %d\n", j, nodes[i].entries[j].name, nodes[i].entries[j].inode);
      }

      if (nodes[i].data != NULL) {
          printf("  Data: %s\n", nodes[i].data);
      }
    }

    
    saveNodesToJsonFile(nodes, arraySize);

  return fuse_main(argc, argv, &fuse_example_operations, NULL);
}