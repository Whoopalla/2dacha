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

#define DEFAULT_COLLIDER_COLLOR ROSE

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

Rectangle windowRec = {0, 0, windowWidth, windowHeight};
Rectangle viewRec = {0, 0, (int)windowWidth * 0.75f, windowHeight};
Rectangle controlRec = {(int)windowWidth * 0.75f, 0, (int)windowWidth * 0.25f,
                        windowHeight};
Vector2 scalingPoint;
Vector2 scalingAnchorSize = {20.0f, 20.0f};
Vector2 scalingAnchorPos;

Texture viewBackground;

Prefab prevPrefab;
char currentTexturePath[MAX_PATH];
static char *selectFileDialog(char *dest) {
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
    // ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

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

static void mouseNavigation(Camera2D *camera, int zoomMode) {
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

static void updateColliders(CollidersEditor *Editor, Camera2D camera) {
  Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), camera);
  Vector2 mousePosWindow = GetWorldToScreen2D(mousePos, camera);

  // Input
  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    // So that click on control panel would not deselect elem
    if (CheckCollisionPointRec(mousePosWindow, viewRec)) {
      Editor->selectedIndex = -1;
      Editor->selectedType = -1;
    }
    Editor->dragPoint = (Vector2){0.0f, 0.0f};
    for (int i = 0; i < Editor->prefab.circleColliders.count; i++) {
      if (CheckCollisionPointCircle(
              mousePos, Editor->prefab.circleColliders.colliders[i].center,
              Editor->prefab.circleColliders.colliders[i].radius)) {
        Editor->selectedType = Circle;
        Editor->selectedIndex = i;
        Editor->dragPoint = Vector2Subtract(
            mousePos, Editor->prefab.circleColliders.colliders[i].center);
        break;
      }
    }
    for (int i = 0; i < Editor->prefab.boxColliders.count; i++) {
      if (CheckCollisionPointRec(
              mousePos,
              (Rectangle){Editor->prefab.boxColliders.colliders[i].pos.x,
                          Editor->prefab.boxColliders.colliders[i].pos.y,
                          Editor->prefab.boxColliders.colliders[i].size.x,
                          Editor->prefab.boxColliders.colliders[i].size.y})) {

        Editor->selectedType = Box;
        Editor->selectedIndex = i;
        Editor->dragPoint = Vector2Subtract(
            mousePos, Editor->prefab.boxColliders.colliders[i].pos);
        break;
      }
    }
  }

  // Scaling mode
  // TODO: should cancel scaling when clicked on not selected elem
  if (Editor->selectedIndex != -1) {
    switch (Editor->selectedType) {
    case Circle:
      if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S)) {
        Editor->scalingMode = true;
      }
      break;
    case Box:
      if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        scalingAnchorPos = Vector2Add(
            Editor->prefab.boxColliders.colliders[Editor->selectedIndex].pos,
            Editor->prefab.boxColliders.colliders[Editor->selectedIndex].size);
        scalingAnchorPos = Vector2Subtract(scalingAnchorPos, scalingAnchorSize);

        printf("scaling anchor pos: %f %f\n", scalingAnchorPos.x,
               scalingAnchorPos.y);
        printf("scaling anchor size: %f %f\n", scalingAnchorSize.x,
               scalingAnchorSize.y);

        // DrawRectangle(scalingAnchorPos.x, scalingAnchorPos.y,
        //    scalingAnchorSize.x, scalingAnchorSize.y, GREEN);
        if (CheckCollisionPointRec(
                mousePos,
                (Rectangle){scalingAnchorPos.x, scalingAnchorPos.y,
                            scalingAnchorSize.x, scalingAnchorSize.y})) {
          Editor->scalingMode = true;
          printf("scaling mode for box\n");
        } else {
          Editor->scalingMode = false;
          printf("scaling mode turned off\n");
        }
      }
      break;
    }
  }
  if (Editor->scalingMode && Editor->selectedIndex == -1) {
    Editor->scalingMode = false;
  }

  // Update
  if (!Editor->scalingMode && IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
      Editor->selectedIndex != -1 &&
      !CheckCollisionPointRec(mousePosWindow, controlRec)) {
    switch (Editor->selectedType) {
    case Circle:
      Editor->prefab.circleColliders.colliders[Editor->selectedIndex].center =
          Vector2Subtract(mousePos, Editor->dragPoint);
      break;
    case Box:
      Editor->prefab.boxColliders.colliders[Editor->selectedIndex].pos =
          Vector2Subtract(mousePos, Editor->dragPoint);
      break;
    }
  }
  if (Editor->scalingMode) {
    switch (Editor->selectedType) {
      Vector2 pos;
    case Circle:
      pos = Editor->prefab.circleColliders.colliders[Editor->selectedIndex]
                .center;
      Editor->prefab.circleColliders.colliders[Editor->selectedIndex].radius =
          Clamp(pos.y - mousePos.y, 20, 500);
      break;
    case Box:
      if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        Vector2 diff = Vector2Subtract(
            mousePos,
            Vector2Add(
                Editor->prefab.boxColliders.colliders[Editor->selectedIndex]
                    .pos,
                Editor->prefab.boxColliders.colliders[Editor->selectedIndex]
                    .size));
        printf("diff: %f %f\n", diff.x, diff.y);
        Editor->prefab.boxColliders.colliders[Editor->selectedIndex]
            .size = Vector2Add(
            Editor->prefab.boxColliders.colliders[Editor->selectedIndex].size,
            diff);
      }
      break;
    }
  }

  // Draw colliders
  for (int i = 0; i < Editor->prefab.circleColliders.count; i++) {
    if (Editor->prefab.circleColliders.colliders[i].isWheel) {
      DrawCircleV(
          Editor->prefab.circleColliders.colliders[i].center,
          Clamp(Editor->prefab.circleColliders.colliders[i].radius, 10, 50),
          BLACK);
    }
    if (Editor->selectedType == Circle && Editor->selectedIndex == i) {
      DrawCircleV(Editor->prefab.circleColliders.colliders[i].center,
                  Editor->prefab.circleColliders.colliders[i].radius,
                  Fade(DEFAULT_COLLIDER_COLLOR, .5f));
    } else {
      DrawCircleV(Editor->prefab.circleColliders.colliders[i].center,
                  Editor->prefab.circleColliders.colliders[i].radius,
                  DEFAULT_COLLIDER_COLLOR);
    }
  }
  for (int i = 0; i < Editor->prefab.boxColliders.count; i++) {
    DrawRectangleV(Editor->prefab.boxColliders.colliders[i].pos,
                   Editor->prefab.boxColliders.colliders[i].size,
                   DEFAULT_COLLIDER_COLLOR);
    if (Editor->selectedType == Box && Editor->selectedIndex == i) {
      DrawRectangleV(Editor->prefab.boxColliders.colliders[i].pos,
                     Editor->prefab.boxColliders.colliders[i].size,
                     Fade(DEFAULT_COLLIDER_COLLOR, .5f));
    }
  }
}

static void SavePrefab(Prefab *prefab) {
  char path[MAX_PATH];
  printf("Saving prefab with texture path: %s\n", prefab->texturePath);
  sprintf(path, "./res/prefabs/%s.prefab",
          GetFileNameWithoutExt(prefab->texturePath));
  SerializePrefab(prefab, path);
}

int selectedColliderSelectDropBox = 0;
bool isEditModeColliderSelectDropBox = false;
float dropBoxYPos;

char *textColliderSelecteDropBox = "Circle;Capsule;Box";
typedef enum {
  circleCollider = 0,
  capsuleCollider,
  boxCollider
} colliderDropDownValues;

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

  Texture currentTexture = {0};
  Vector2 currentTexturePos = {.0f, .0f};

  CollidersEditor Editor = {0};
  Editor.selectedIndex = -1;
  CircleCollider defaultCircleCollider = {(Vector2){.0f, .0f}, 50, false};
  CapsuleCollider defaultCapsuleCollider = {(Vector2){.0f, .0f},
                                            (Vector2){30.0f, 30.0f}, 50.0f};
  BoxCollider defaultBoxCollider = {(Vector2){.0f, .0f},
                                    (Vector2){50.0f, 50.0f}};

  while (!WindowShouldClose()) {
    mouseNavigation(&camera, zoomMode);

    BeginDrawing();
    ClearBackground(LIGHTGRAY);

    if (isEditModeColliderSelectDropBox) {
      GuiLock();
    } else if (!isEditModeColliderSelectDropBox) {
      GuiUnlock();
    }

    BeginMode2D(camera);
    rlPushMatrix();
    rlTranslatef(0, 250 * 300, 0);
    rlRotatef(90, 1, 0, 0);
    DrawGrid(1000, 300);
    rlPopMatrix();
    EndMode2D();

    // DRAW UI
    DrawRectangleRec(controlRec, GRAY);

    float controlPanelVerticalOffset = 50;
    int controlPanelVertElemCount = 0;
    if (GuiButton((Rectangle){controlRec.x + controlRec.width * 0.25f,
                              controlRec.y + controlPanelVerticalOffset +
                                  100 * controlPanelVertElemCount++,
                              controlRec.width / 2.0f, 100},
                  "Edit Prefab")) {
      char prefabPath[MAX_PATH];
      selectFileDialog(prefabPath);
      if (IsFileExtension(prefabPath, ".prefab")) {
        prevPrefab = Editor.prefab;
        if (DeserializePrefab(prefabPath, &Editor.prefab) == 0) {
          UnloadTexture(currentTexture);
          currentTexture = LoadTexture(Editor.prefab.texturePath);
          printf("Selected prefab path: %s\n", prefabPath);
        } else {
          Editor.prefab = prevPrefab;
        }
      } else {
        printf("Selected prefab path: %s\n", prefabPath);
        printf("Select file with .prefab extension\n");
      }
    }

    if (GuiButton((Rectangle){controlRec.x + controlRec.width * 0.25f,
                              controlRec.y + controlPanelVerticalOffset +
                                  100 * controlPanelVertElemCount++,
                              controlRec.width / 2.0f, 100},
                  "Select Texture")) {
      char newTexturePath[MAX_PATH];
      selectFileDialog(newTexturePath);
      if (IsFileExtension(newTexturePath, ".png")) {
        UnloadTexture(currentTexture);
        strcpy(Editor.prefab.texturePath, newTexturePath);
        currentTexture = LoadTexture(newTexturePath);
      }
      if (IsTextureReady(currentTexture)) {
        GuiWindowBox(windowRec, "Texture was loaded.");
      } else {
        GuiWindowBox(windowRec, "Texture was not loaded");
      }
    }

    if (GuiButton((Rectangle){controlRec.x + controlRec.width * 0.25f,
                              controlRec.y + controlPanelVerticalOffset +
                                  100 * controlPanelVertElemCount++,
                              controlRec.width / 2.0f, 100},
                  "Save Prefab")) {
      SavePrefab(&Editor.prefab);
    }

    controlPanelVertElemCount++;
    if (GuiButton((Rectangle){controlRec.x + controlRec.width * 0.25f,
                              controlRec.y + controlPanelVerticalOffset +
                                  100 * controlPanelVertElemCount++ + 50,
                              controlRec.width / 2.0f, 100},
                  "Add collider")) {
      switch (selectedColliderSelectDropBox) {
      case circleCollider:
        Editor.prefab.circleColliders
            .colliders[Editor.prefab.circleColliders.count++] =
            defaultCircleCollider;
        break;
      case capsuleCollider:
        Editor.prefab.capsuleColliders
            .colliders[Editor.prefab.capsuleColliders.count++] =
            defaultCapsuleCollider;
        break;
      case boxCollider:
          printf("added box collider\n");
        Editor.prefab.boxColliders
            .colliders[Editor.prefab.boxColliders.count++] = defaultBoxCollider;
        break;
      }
    }

    if (GuiDropdownBox((Rectangle){controlRec.x + controlRec.width * 0.25f,
                                   controlRec.y + controlPanelVerticalOffset +
                                       100 * controlPanelVertElemCount++ - 100,
                                   controlRec.width / 2.0f, 50},
                       textColliderSelecteDropBox,
                       &selectedColliderSelectDropBox,
                       isEditModeColliderSelectDropBox)) {
      isEditModeColliderSelectDropBox = !isEditModeColliderSelectDropBox;
    }

    GuiSetStyle(CHECKBOX, BORDER_COLOR_NORMAL, ColorToInt(BLACK));
    if (Editor.selectedIndex != -1 && Editor.selectedType == Circle &&
        GuiCheckBox(
            (Rectangle){controlRec.x + controlRec.width * 0.25f,
                        controlRec.y + controlPanelVerticalOffset +
                            100 * controlPanelVertElemCount++,
                        30, 30},
            "Is a wheel",
            &Editor.prefab.circleColliders.colliders[Editor.selectedIndex]
                 .isWheel)) {
    }

    if (Editor.selectedIndex != -1 &&
        GuiButton((Rectangle){controlRec.x + controlRec.width * 0.25f,
                              controlRec.y + controlPanelVerticalOffset +
                                  100 * controlPanelVertElemCount++ + 50,
                              controlRec.width / 2.0f, 100},
                  "Remove collider")) {
      switch (Editor.selectedType) {
      case Circle:
        assert(Editor.prefab.circleColliders.count >= 0);
        Editor.prefab.circleColliders.count--;
        Editor.selectedIndex = -1;
        break;
      case Box:
        assert(Editor.prefab.boxColliders.count >= 0);
        Editor.prefab.boxColliders.count--;
        Editor.selectedIndex = -1;
        break;
      }
    }

    BeginMode2D(camera);

    if (IsTextureReady(currentTexture)) {
      DrawTexture(currentTexture, currentTexturePos.x, currentTexturePos.y,
                  WHITE);
    }

    updateColliders(&Editor, camera);

    EndMode2D();

    EndDrawing();
  }

  UnloadTexture(currentTexture);
  UnloadTexture(viewBackground);
  CloseWindow();
  return 0;
}
