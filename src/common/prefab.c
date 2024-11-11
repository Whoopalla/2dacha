#include "colled.h"
#include <stdio.h>
#include <stdlib.h>

int DeserializePrefab(char path[MAX_PATH], Prefab *res) {
  FILE *f = fopen(path, "r");
  if (f == NULL) {
    perror("ERROR: could not open prefab for read");
    return -1;
  }
  if (fread(&(res->texturePath), 1, MAX_PATH, f) != MAX_PATH) {
    perror("ERROR: deserializing prefab path");
    fclose(f);
    return -1;
  }
  printf("Deserialized texture path: %s\n", res->texturePath);
  if (fread(&(res->circleColliders), sizeof(CircleCollider), MAX_COLLIDERS_COUNT, f) !=
      MAX_COLLIDERS_COUNT) {
    perror("ERROR: deserializing prefab circles");
    fclose(f);
    return -1;
  }
  if (fread(&(res->circleColliders.count), sizeof(res->circleColliders.count), 1, f) !=
      1) {
    perror("ERROR: deserializing prefab circles count");
    fclose(f);
    return -1;
  }
  printf("DESERIALIZED: path: %s\n", res->texturePath);
  for (size_t i = 0; i < MAX_COLLIDERS_COUNT; i++) {
    printf("\tcircle collider r: %f pos: %f %f\n",
           res->circleColliders.colliders[i].radius,
           res->circleColliders.colliders[i].center.x,
           res->circleColliders.colliders[i].center.y);
  }
  fclose(f);
  return 0;
}

int SerializePrefab(Prefab *prefab, char path[MAX_PATH]) {
  FILE *f = fopen(path, "w+");
  if (f == NULL) {
    perror("ERROR: Could not create file for prefab");
    return -1;
  }
  if (fwrite(prefab->texturePath, 1, MAX_PATH, f) == 0) {
    perror("ERROR: serializing prefab path");
    fclose(f);
    return -1;
  }
  if (fwrite(prefab->circleColliders.colliders, sizeof(CircleCollider), MAX_COLLIDERS_COUNT,
             f) != MAX_COLLIDERS_COUNT) {
    perror("ERROR: serializing prefab circles");
    fclose(f);
    return -1;
  }
  if (fwrite(&prefab->circleColliders.count, sizeof(prefab->circleColliders.count), 1,
             f) != 1) {
    perror("ERROR: serializing prefab circle count");
    fclose(f);
    return -1;
  }
  fclose(f);
  printf("Prefab serialized succsessfully\n");
  return 0;
}
