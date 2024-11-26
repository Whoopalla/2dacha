#include "colled.h"
#include <stdio.h>
#include <stdlib.h>

int DeserializePrefab(char path[MAX_PATH], Prefab *res) {
  printf("Deserialized prefab: %s\n", path);
  FILE *f = fopen(path, "r");
  if (f == NULL) {
    perror("ERROR: could not open prefab for read");
    return -1;
  }
  printf("Deserialized texture path");
  if (fread(&(res->texturePath), 1, MAX_PATH, f) != MAX_PATH) {
    perror("ERROR: deserializing texture path");
    fclose(f);
    return -1;
  }
  // Circles
  if (fread(&(res->circleColliders.colliders), sizeof(CircleCollider),
            MAX_COLLIDERS_COUNT, f) != MAX_COLLIDERS_COUNT) {
    perror("ERROR: deserializing prefab circle collieders");
    fclose(f);
    return -1;
  }
  if (fread(&(res->circleColliders.count), sizeof(res->circleColliders.count),
            1, f) != 1) {
    perror("ERROR: deserializing prefab circles count");
    fclose(f);
    return -1;
  }
  // Boxes
  if (fread(&(res->boxColliders.colliders), sizeof(BoxCollider), MAX_COLLIDERS_COUNT,
            f) != MAX_COLLIDERS_COUNT) {
    perror("ERROR: deserializing prefab box colliders");
    fclose(f);
    return -1;
  }
  if (fread(&(res->boxColliders.count), sizeof(res->boxColliders.count), 1,
            f) != 1) {
    perror("ERROR: deserializing prefab box colliders");
    fclose(f);
    return -1;
  }
  printf("DESERIALIZED: path: %s\n", res->texturePath);
  printf("count circles: %d | boxes: %d\n", res->circleColliders.count, res->boxColliders.count);
  for (size_t i = 0; i < MAX_COLLIDERS_COUNT; i++) {
    printf("\tcircle collider r: %f is wheel: %b pos: %f %f\n",
           res->circleColliders.colliders[i].radius,
           res->circleColliders.colliders[i].isWheel,
           res->circleColliders.colliders[i].center.x,
           res->circleColliders.colliders[i].center.y);
  }
  for (size_t i = 0; i < MAX_COLLIDERS_COUNT; i++) {
    printf("\tbox collider pos: %f %f size: %f %f\n",
           res->boxColliders.colliders[i].pos.x,
           res->boxColliders.colliders[i].pos.y,
           res->boxColliders.colliders[i].size.x,
           res->boxColliders.colliders[i].size.y);
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
  if (fwrite(&prefab->texturePath, 1, MAX_PATH, f) == 0) {
    perror("ERROR: serializing texture path");
    fclose(f);
    return -1;
  }
  // Circle
  if (fwrite(&prefab->circleColliders.colliders, sizeof(CircleCollider),
             MAX_COLLIDERS_COUNT, f) != MAX_COLLIDERS_COUNT) {
    perror("ERROR: serializing prefab circle colliders");
    fclose(f);
    return -1;
  }
  if (fwrite(&prefab->circleColliders.count,
             sizeof(prefab->circleColliders.count), 1, f) != 1) {
    perror("ERROR: serializing prefab circle count");
    fclose(f);
    return -1;
  }
  // Box
  if (fwrite(&prefab->boxColliders.colliders, sizeof(BoxCollider),
             MAX_COLLIDERS_COUNT, f) != MAX_COLLIDERS_COUNT) {
    perror("ERROR: serializing prefab box colliders");
    fclose(f);
    return -1;
  }
  if (fwrite(&prefab->boxColliders.count,
             sizeof(prefab->boxColliders.count), 1, f) != 1) {
    perror("ERROR: serializing prefab box count");
    fclose(f);
    return -1;
  }
  fclose(f);
  printf("Prefab serialized succsessfully\n");
  return 0;
}
