#include "box2d/box2d.h"
#include "box2d/collision.h"
#include "box2d/math_functions.h"
#include "box2d/types.h"
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>

#include <assert.h>
#include "colled.h"

#define GROUND_CUBES_COUNT 10

// TODO: convert prefab to a world obj

// 128 pixels per meter is a appropriate for this scene. The boxes are 128
// pixels wide.
float lengthUnitsPerMeter = 128.0f;
int width = 1920, height = 1080;

typedef struct Entity {
  Prefab prefab;
  b2BodyId bodyId;
  b2Vec2 extent;
  Texture texture;
} Entity;

Entity entities[100] = {0};
size_t entity_count = 0;

b2WorldId worldId;
b2Vec2 groundExtent, boxExtent, nivaExtent, wheelExtent;
b2Polygon boxPolygon;
Prefab wheelPrefab;

Texture groundTexture, boxTexture, nivaTexture, wheelTexture;

static void create_body(Vector2 loc) {
  printf("New body created at x: %f y: %f\n", loc.x, loc.y);
  Entity *entity = entities + entity_count++;
  entity->prefab = wheelPrefab;
  b2BodyDef bodyDef = b2DefaultBodyDef();
  bodyDef.type = b2_dynamicBody;
  bodyDef.position = (b2Vec2){loc.x, loc.y};
  entity->bodyId = b2CreateBody(worldId, &bodyDef);
  entity->texture = LoadTexture(entity->prefab.texturePath);
  entity->extent = boxExtent;
  b2ShapeDef shapeDef = b2DefaultShapeDef();
  b2CreatePolygonShape(entity->bodyId, &shapeDef, &boxPolygon);
}

static void DrawEntity(const Entity *entity) {
  b2Vec2 p = b2Body_GetWorldPoint(
      entity->bodyId, (b2Vec2){-entity->extent.x, -entity->extent.y});
  b2Rot rotation = b2Body_GetRotation(entity->bodyId);
  float radians = b2Rot_GetAngle(rotation);

  Vector2 ps = {p.x, p.y};
  DrawTextureEx(entity->texture, ps, RAD2DEG * radians, 1.0f, WHITE);
  // DrawRectanglePro(
  //     (Rectangle){p.x, p.y, entity->extent.x * 2, entity->extent.y * 2},
  //     (Vector2){0.0f, 0.0f}, RAD2DEG * radians, RED);
  //
  //  DrawCircleV(ps, 5.0f, BLACK);
  //  p = b2Body_GetWorldPoint(entity->bodyId, (b2Vec2){0.0f, 0.0f});
  //  ps = (Vector2){p.x, p.y};
  //  DrawCircleV(ps, 5.0f, BLUE);
  //  p = b2Body_GetWorldPoint(entity->bodyId,
  //                           (b2Vec2){entity->extent.x, entity->extent.y});
  //  ps = (Vector2){p.x, p.y};
  //  DrawCircleV(ps, 5.0f, RED);
}

int main(void) {
  InitWindow(width, height, "box2d-raylib");

  SetTargetFPS(60);

  b2SetLengthUnitsPerMeter(lengthUnitsPerMeter);

  b2WorldDef worldDef = b2DefaultWorldDef();

  // Realistic gravity is achieved by multiplying gravity by the length unit.
  worldDef.gravity.y = 9.8f * lengthUnitsPerMeter;
  worldId = b2CreateWorld(&worldDef);

  nivaTexture = LoadTexture("./res/niva.png");
  wheelTexture = LoadTexture("./res/wheel.png");
  boxTexture = LoadTexture("./res/box.png");
  groundTexture = LoadTexture("./res/ground.png");
  b2Vec2 groundSize = {groundTexture.width, groundTexture.height};

  nivaExtent = (b2Vec2){0.5f * nivaTexture.width, 0.5f * nivaTexture.height};
  wheelExtent = (b2Vec2){0.5f * wheelTexture.width, 0.5f * wheelTexture.height};
  groundExtent = (b2Vec2){0.5f * groundSize.x, 0.5f * groundSize.y};
  boxExtent = (b2Vec2){0.5f * boxTexture.width, 0.5f * boxTexture.height};

  // These polygons are centered on the origin and when they are added to a body
  // they will be centered on the body position.
  b2Polygon nivaPolygon = b2MakeBox(nivaExtent.x, nivaExtent.y);
  b2Polygon wheelPolygon = b2MakeBox(wheelExtent.x, wheelExtent.y);
  b2Polygon groundPolygon = b2MakeBox(groundExtent.x, groundExtent.y);
  boxPolygon = b2MakeBox(boxExtent.x, boxExtent.y);
  wheelPrefab = DeserializePrefab("./res/prefabs/wheel.prefab");

  for (size_t i = 0; i < GROUND_CUBES_COUNT; i++) {
    Entity *ground = &entities[entity_count++];
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.position = (b2Vec2){(2.0f * i + 2.0f) * groundExtent.x,
                                height - groundExtent.y - 100.0f};
    ground->bodyId = b2CreateBody(worldId, &bodyDef);
    ground->extent = groundExtent;
    ground->texture = groundTexture;
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    b2CreatePolygonShape(ground->bodyId, &shapeDef, &groundPolygon);
  }

  Entity *niva = &entities[entity_count++];
  b2BodyDef bodyDef = b2DefaultBodyDef();
  bodyDef.position = (b2Vec2){600.0f, 300.0f};
  bodyDef.type = b2_dynamicBody;
  niva->bodyId = b2CreateBody(worldId, &bodyDef);
  niva->texture = nivaTexture;
  niva->extent = nivaExtent;
  b2ShapeDef shapeDef = b2DefaultShapeDef();
  b2CreatePolygonShape(niva->bodyId, &shapeDef, &nivaPolygon);

  Entity *left_wheel = &entities[entity_count++];
  b2Circle circle = {{0.0f, 0.0}, wheelExtent.x};
  shapeDef.friction = 1.5f;
  bodyDef.position = (b2Vec2){600.0f - niva->extent.x, 600.0f};
  bodyDef.allowFastRotation = true;
  left_wheel->extent = wheelExtent;
  left_wheel->texture = wheelTexture;
  left_wheel->bodyId = b2CreateBody(worldId, &bodyDef);
  b2CreateCircleShape(left_wheel->bodyId, &shapeDef, &circle);

  Entity *right_wheel = &entities[entity_count++];
  shapeDef.friction = 1.5f;
  bodyDef.position = (b2Vec2){600.0f + niva->extent.x, 600.0f};
  bodyDef.allowFastRotation = true;
  right_wheel->extent = wheelExtent;
  right_wheel->texture = wheelTexture;
  right_wheel->bodyId = b2CreateBody(worldId, &bodyDef);
  b2CreateCircleShape(right_wheel->bodyId, &shapeDef, &circle);

  b2Vec2 axis = {0.0f, 1.0f};
  b2Vec2 pivot = b2Body_GetPosition(left_wheel->bodyId);

  b2WheelJointDef wheel_joint = b2DefaultWheelJointDef();

  float throttle = 0.0f;
  float speed = 35.0f;
  float torque = 2.5f;
  float hertz = 5.0f;
  float dampingRatio = 0.7f;

  wheel_joint.bodyIdA = niva->bodyId;
  wheel_joint.bodyIdB = left_wheel->bodyId;
  wheel_joint.localAxisA = b2Body_GetLocalVector(wheel_joint.bodyIdA, axis);
  wheel_joint.localAnchorA = b2Body_GetLocalPoint(wheel_joint.bodyIdA, pivot);
  wheel_joint.localAnchorB = b2Body_GetLocalPoint(wheel_joint.bodyIdB, pivot);
  wheel_joint.motorSpeed = 0;
  wheel_joint.maxMotorTorque = torque;
  wheel_joint.enableMotor = true;
  wheel_joint.hertz = hertz;
  wheel_joint.dampingRatio = dampingRatio;
  wheel_joint.lowerTranslation = -0.25f;
  wheel_joint.upperTranslation = 0.25f;
  wheel_joint.enableLimit = true;
  b2JointId left_wheel_joint = b2CreateWheelJoint(worldId, &wheel_joint);

  pivot = b2Body_GetPosition(right_wheel->bodyId);
  wheel_joint.bodyIdA = niva->bodyId;
  wheel_joint.bodyIdB = right_wheel->bodyId;
  wheel_joint.localAxisA = b2Body_GetLocalVector(wheel_joint.bodyIdA, axis);
  wheel_joint.localAnchorA = b2Body_GetLocalPoint(wheel_joint.bodyIdA, pivot);
  wheel_joint.localAnchorB = b2Body_GetLocalPoint(wheel_joint.bodyIdB, pivot);
  wheel_joint.motorSpeed = 0;
  wheel_joint.maxMotorTorque = torque;
  wheel_joint.enableMotor = true;
  wheel_joint.hertz = hertz;
  wheel_joint.dampingRatio = dampingRatio;
  wheel_joint.lowerTranslation = -0.25f;
  wheel_joint.upperTranslation = 0.25f;
  wheel_joint.enableLimit = true;
  b2JointId right_wheel_joint = b2CreateWheelJoint(worldId, &wheel_joint);

  Camera2D camera = {0};
  b2Vec2 camera_target = b2Body_GetPosition(niva->bodyId);
  camera.target = (Vector2){camera_target.x, camera_target.y};
  camera.offset = (Vector2){width / 2.0f, height / 2.0f};
  camera.rotation = 0.0f;
  camera.zoom = 1.0f;

  while (!WindowShouldClose()) {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      create_body(GetMousePosition());
    }

    b2World_Step(worldId, GetFrameTime(), 4);

    camera_target = b2Body_GetPosition(niva->bodyId);
    camera.target = (Vector2){camera_target.x, camera_target.y};
    camera.zoom += ((float)GetMouseWheelMove() * 0.05f);

    BeginDrawing();
    ClearBackground(DARKGRAY);

    //BeginMode2D(camera);
    for (int i = 0; i < entity_count; ++i) {
      DrawEntity(entities + i);
      b2Vec2 pos = b2Body_GetPosition(entities[i].bodyId);
      printf("Entity x: %f y: %f\n", pos.x, pos.y);
    }

    //EndMode2D();
    EndDrawing();
  }

  UnloadTexture(groundTexture);
  UnloadTexture(boxTexture);

  CloseWindow();

  return 0;
}
