#include "colled.h"
#include <stdio.h>
#include <stdlib.h>

int DeserializePrefab(char path[MAX_PATH], Prefab *res) {
  FILE *f = fopen(path, "r");
  if (f == NULL) {
    printf("ERROR: could not open prefab for read");
    fclose(f);
    return -1;
  }
  if (fread(&(res->texturePath), 1, MAX_PATH, f) != MAX_PATH) {
    perror("ERROR: deserializing prefab");
    fclose(f);
    return -1;
  }
  printf("Deserialized texture path: %s\n", res->texturePath);
  if (fread(&(res->circleColliders), sizeof(CircleCollider),
            MAX_COLLIDERS_COUNT, f) != MAX_COLLIDERS_COUNT) {
    perror("ERROR: deserializing prefab");
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
    perror("ERROR: serializing prefab");
    return -1;
  }
  if (fwrite(prefab->circleColliders.colliders, sizeof(CircleCollider),
             MAX_COLLIDERS_COUNT, f) != MAX_COLLIDERS_COUNT) {
    perror("ERROR: serializing prefab");
    return -1;
  }
  fclose(f);
  printf("Prefab serialized succsessfully\n");
  return 0;
}
