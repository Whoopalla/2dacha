#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "raymath.h"
#include "rlgl.h"

#include "colled.h"
#include <assert.h>

#undef RAYGUI_IMPLEMENTATION
#define GUI_WINDOW_FILE_DIALOG_IMPLEMENTATION
#include "gui_window_file_dialog.h"

#define COLLIDER_DEFAULT_COLOR ROSE

#define ROSE GetColor(0xFF006E7C)
#define AMBER GetColor(0xFFBE0B7C)
//{"Amber":"ffbe0b","Orange (Pantone)":"fb5607","Rose":"ff006e","Blue
// Violet":"8338ec","Azure":"3a86ff"}
//
typedef struct {
  ColliderType selectedType;
  int selectedIndex;
  Vector2 dragPoint;
  bool scalingMode;
  Prefab prefab;
} CollidersEditor;

const int windowWidth = 1600;
const int windowHeight = 1000;

Prefab prevPrefab;
char currentTexturePath[MAX_PATH];
char *selectFileDialog(char *dest) {
  GuiWindowFileDialogState fileDialogState =
      InitGuiWindowFileDialog(GetWorkingDirectory());

  bool exitWindow = false;

  char fileNameToLoad[512] = {0};
  while (!exitWindow) {
    exitWindow = WindowShouldClose();
    if (fileDialogState.SelectFilePressed) {
      strcpy(fileNameToLoad,
             TextFormat("%s" PATH_SEPERATOR "%s", fileDialogState.dirPathText,
                        fileDialogState.fileNameText));
      fileDialogState.SelectFilePressed = false;
      exitWindow = true;
    }

    BeginDrawing();
    ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

    if (fileDialogState.windowActive) {
      GuiLock();
    }

    fileDialogState.windowActive = true;

    GuiUnlock();

    GuiWindowFileDialog(&fileDialogState);
    EndDrawing();
  }
  strcpy(dest, fileNameToLoad);
  return dest;
}

void mouseNavigation(Camera2D *camera, int zoomMode) {
  if (IsKeyPressed(KEY_ONE))
    zoomMode = 0;
  else if (IsKeyPressed(KEY_TWO))
    zoomMode = 1;

  // Translate based on mouse right click
  if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
    Vector2 delta = GetMouseDelta();
    delta = Vector2Scale(delta, -1.0f / camera->zoom);
    camera->target = Vector2Add(camera->target, delta);
  }

  if (zoomMode == 0) {
    // Zoom based on mouse wheel
    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
      // Get the world point that is under the mouse
      Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), *camera);

      // Set the offset to where the mouse is
      camera->offset = GetMousePosition();

      // Set the target to match, so that the camera->maps the world space
      // point under the cursor to the screen space point under the cursor at
      // any zoom
      camera->target = mouseWorldPos;

      // Zoom increment
      float scaleFactor = 1.0f + (0.25f * fabsf(wheel));
      if (wheel < 0)
        scaleFactor = 1.0f / scaleFactor;
      camera->zoom = Clamp(camera->zoom * scaleFactor, 0.125f, 64.0f);
    }
  } else {
    // Zoom based on mouse left click
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      // Get the world point that is under the mouse
      Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), *camera);

      // Set the offset to where the mouse is
      camera->offset = GetMousePosition();

      // Set the target to match, so that the camera->maps the world space
      // point under the cursor to the screen space point under the cursor at
      // any zoom
      camera->target = mouseWorldPos;
    }
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
      // Zoom increment
      float deltaX = GetMouseDelta().x;
      float scaleFactor = 1.0f + (0.01f * fabsf(deltaX));
      if (deltaX < 0)
        scaleFactor = 1.0f / scaleFactor;
      camera->zoom = Clamp(camera->zoom * scaleFactor, 0.125f, 64.0f);
    }
  }
}

void updateColliders(CollidersEditor *cEditor, Camera2D camera) {
  Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), camera);

  // Input
  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    for (int i = 0; i < MAX_COLLIDERS_COUNT; i++) {
      if (CheckCollisionPointCircle(
              mousePos, cEditor->prefab.circleColliders.colliders[i].center,
              cEditor->prefab.circleColliders.colliders[i].radius)) {
        cEditor->selectedIndex = i;
        cEditor->dragPoint = Vector2Subtract(
            mousePos, cEditor->prefab.circleColliders.colliders[i].center);
        break;
      } else {
        cEditor->selectedIndex = -1;
      }
    }
  }

  if (!cEditor->scalingMode && IsKeyDown(KEY_LEFT_CONTROL) &&
      IsKeyPressed(KEY_S) && cEditor->selectedIndex != -1) {
    cEditor->scalingMode = true;
  }
  if (cEditor->scalingMode && (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) ||
                               IsMouseButtonPressed(MOUSE_BUTTON_LEFT))) {
    cEditor->scalingMode = false;
  }

  // Update
  if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && cEditor->selectedIndex != -1) {
    cEditor->prefab.circleColliders.colliders[cEditor->selectedIndex].center =
        Vector2Subtract(mousePos, cEditor->dragPoint);
  }
  if (cEditor->scalingMode) {
    Vector2 pos =
        cEditor->prefab.circleColliders.colliders[cEditor->selectedIndex]
            .center;
    cEditor->prefab.circleColliders.colliders[cEditor->selectedIndex].radius =
        Clamp(pos.y - mousePos.y, 20, 500);
  }

  // Draw
  for (int i = 0; i < MAX_COLLIDERS_COUNT; i++) {
    DrawCircleV(cEditor->prefab.circleColliders.colliders[i].center,
                cEditor->prefab.circleColliders.colliders[i].radius,
                cEditor->prefab.circleColliders.colliders[i].color);
    if (cEditor->selectedIndex == i) {
      DrawCircleV(
          cEditor->prefab.circleColliders.colliders[i].center,
          cEditor->prefab.circleColliders.colliders[i].radius,
          Fade(cEditor->prefab.circleColliders.colliders[i].color, .5f));
    }
  }
}

void SavePrefab(Prefab *prefab) {
  char path[MAX_PATH];
  printf("Saving prefab with texture path: %s\n", prefab->texturePath);
  sprintf(path, "./res/prefabs/%s.prefab",
          GetFileNameWithoutExt(prefab->texturePath));
  FILE *f = fopen(path, "w+");
  if (f == NULL) {
    perror("Could not create file for prefab");
    exit(1);
  }
  if (fwrite(prefab->texturePath, 1, MAX_PATH, f) == 0) {
    perror("ERROR: serializing prefab");
    exit(1);
  }
  if (fwrite(prefab->circleColliders.colliders, sizeof(CircleCollider),
             MAX_COLLIDERS_COUNT, f) != MAX_COLLIDERS_COUNT) {
    perror("ERROR: serializing prefab");
    exit(1);
  }
  fclose(f);
  printf("Prefa serialized succsessfully\n");
}

Prefab DeserializePrefab(char path[MAX_PATH]) {
  Prefab res;
  FILE *f = fopen(path, "r");
  if (f == NULL) {
    printf("Printf could not open prefab for read");
    exit(1);
  }
  if (fread(&(res.texturePath), 1, MAX_PATH, f) != MAX_PATH) {
    perror("ERROR: deserializing prefab");
    exit(1);
  }
  printf("Deserialized texture path: %s\n", res.texturePath);
  if (fread(&(res.circleColliders), sizeof(CircleCollider), MAX_COLLIDERS_COUNT,
            f) != MAX_COLLIDERS_COUNT) {
    perror("ERROR: deserializing prefab");
    exit(1);
  }
  printf("DESERIALIZED: path: %s\n", res.texturePath);
  for (size_t i = 0; i < MAX_COLLIDERS_COUNT; i++) {
    printf("\tcircle collider r: %f pos: %f %f\n",
           res.circleColliders.colliders[i].radius,
           res.circleColliders.colliders[i].center.x,
           res.circleColliders.colliders[i].center.y);
  }
  return res;
}

int main(void) {
  SetTargetFPS(60);

  InitWindow(windowWidth, windowHeight, "colled");
  Font font = LoadFont("res/fonts/Roboto/Roboto-Bold.ttf");
  if (!IsFontReady(font)) {
    perror("Could not load font");
    exit(1);
  }
  font.baseSize = 18;
  GuiSetFont(font);

  Camera2D camera = {0};
  camera.target = (Vector2){-800, -500};
  camera.zoom = .8f;
  int zoomMode = 0;

  Rectangle windowRec = {0, 0, windowWidth, windowHeight};
  Rectangle viewRec = {0, 0, (int)windowWidth * 0.75f, windowHeight};
  Rectangle controlRec = {(int)windowWidth * 0.75f, 0, (int)windowWidth * 0.25f,
                          windowHeight};

  Texture currentTexture = {0};
  Vector2 currentTexturePos = {.0f, .0f};

  CollidersEditor cEditor = {0};
  cEditor.selectedIndex = -1;
  CircleCollider defaultCircleCollider = {(Vector2){.0f, .0f}, 50,
                                          COLLIDER_DEFAULT_COLOR};

  while (!WindowShouldClose()) {
    mouseNavigation(&camera, zoomMode);

    BeginDrawing();
    ClearBackground(RAYWHITE);

    // UI
    GuiPanel(viewRec, "view");
    GuiPanel(controlRec, "control");
    DrawRectangleRec(controlRec, GRAY);
    if (GuiButton((Rectangle){controlRec.x + controlRec.width * 0.25f,
                              controlRec.y + 50, controlRec.width / 2.0f, 100},
                  "Edit Prefab")) {
      char prefabPath[MAX_PATH];
      selectFileDialog(prefabPath);
      if (IsFileExtension(prefabPath, ".prefab")) {
        prevPrefab = cEditor.prefab;
        cEditor.prefab = DeserializePrefab(prefabPath);
        UnloadTexture(currentTexture);
        currentTexture = LoadTexture(cEditor.prefab.texturePath);
        printf("Selected prefab path: %s\n", prefabPath);
      } else {
        printf("Selected prefab path: %s\n", prefabPath);
        printf("Select file with .prefab extension\n");
      }
    }

    if (GuiButton((Rectangle){controlRec.x + controlRec.width * 0.25f,
                              controlRec.y + 50 + 100, controlRec.width / 2.0f,
                              100},
                  "Select Texture")) {
      char newTexturePath[MAX_PATH];
      selectFileDialog(newTexturePath);
      if (IsFileExtension(newTexturePath, ".png")) {
        UnloadTexture(currentTexture);
        strcpy(cEditor.prefab.texturePath, newTexturePath);
        currentTexture = LoadTexture(newTexturePath);
      }
      if (IsTextureReady(currentTexture)) {
        GuiWindowBox(windowRec, "Texture was loaded.");
      } else {
        GuiWindowBox(windowRec, "Texture was not loaded");
      }
    }

    if (GuiButton((Rectangle){controlRec.x + controlRec.width * 0.25f,
                              controlRec.y + 50 + 200, controlRec.width / 2.0f,
                              100},
                  "Add circle collider")) {
      cEditor.prefab.circleColliders
          .colliders[cEditor.prefab.circleColliders.count++] =
          defaultCircleCollider;
    }

    if (GuiButton((Rectangle){controlRec.x + controlRec.width * 0.25f,
                              controlRec.y + 50 + 300, controlRec.width / 2.0f,
                              100},
                  "Save Prefab")) {
      SavePrefab(&cEditor.prefab);
      char dp[MAX_PATH];
      sprintf(dp, "./res/prefabs/%s.prefab",
              GetFileNameWithoutExt(currentTexturePath));
      printf("Deserializing: %s\n", dp);
      DeserializePrefab(dp);
    }

    BeginMode2D(camera);

    rlPushMatrix();
    rlTranslatef(0, 25 * 50, 0);
    rlRotatef(90, 1, 0, 0);
    DrawGrid(100, 50);
    rlPopMatrix();

    if (IsTextureReady(currentTexture)) {
      DrawTexture(currentTexture, currentTexturePos.x, currentTexturePos.y,
                  WHITE);
    }

    updateColliders(&cEditor, camera);

    EndMode2D();

    EndDrawing();
  }

  UnloadTexture(currentTexture);
  CloseWindow();
  return 0;
}
