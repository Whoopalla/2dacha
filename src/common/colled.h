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
  CircleCollider colliders[MAX_COLLIDERS_COUNT];
  int count;
} CircleColliders;

typedef struct {
  char texturePath[MAX_PATH];
  CircleColliders circleColliders;
} Prefab;

typedef struct {
  CircleColliders circleColliders;
} Colliders;

Prefab DeserializePrefab(char path[MAX_PATH]);
void SerializePrefab(Prefab *prefab, char path[MAX_PATH]);
