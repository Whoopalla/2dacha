#include "raylib.h"

#define MAX_PATH 100
#define MAX_COLLIDERS_COUNT 10

typedef enum { Circle, Box, Wheel } ColliderType;

typedef struct {
  ColliderType type;
} Collider;

typedef struct {
  Vector2 center;
  float radius;
  Color color;
} CircleCollider;

typedef struct {
  //ColliderType type;
  CircleCollider colliders[MAX_COLLIDERS_COUNT];
  int count;
} CircleColliders;

typedef struct {
  Vector2 center1;
  Vector2 center2;
  float radius;
} CapsuleCollider;

typedef struct {
  CapsuleCollider colliders[MAX_COLLIDERS_COUNT];
  int count;
} CapsuleColliders;

typedef struct {
  Vector2 pos;   
  Vector2 size;
} BoxCollieder;

typedef struct {
  BoxCollieder colliders[MAX_COLLIDERS_COUNT];
  int count;
} BoxColliders;

typedef struct {
  char texturePath[MAX_PATH];
  CircleColliders circleColliders;
  CapsuleColliders capsuleColliders;
  BoxColliders boxColliders;
} Prefab;

int DeserializePrefab(char path[MAX_PATH], Prefab *res);
int SerializePrefab(Prefab *prefab, char path[MAX_PATH]);
