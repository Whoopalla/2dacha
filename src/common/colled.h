#include "raylib.h"

#define MAX_PATH 100
#define MAX_COLLIDERS_COUNT 10

typedef enum { Circle, Box } ColliderType;

typedef struct {
  ColliderType type;
} Collider;

typedef struct {
  Vector2 center;
  float radius;
  bool isWheel;
} CircleCollider;

typedef struct {
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
} BoxCollider;

typedef struct {
  BoxCollider colliders[MAX_COLLIDERS_COUNT];
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
