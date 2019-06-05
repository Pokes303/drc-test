#include <coreinit/screen.h>
#include <string>
#include <vector>
#include <vpad/input.h>

void drawPixel(int x, int y, uint32_t colour);
void drawLine(int x1, int y1, int x2, int y2, uint32_t colour);
void drawRect(int x1, int y1, int x2, int y2, uint32_t colour);
void drawStraightXLine(int y, int x1, int x2, uint32_t colour);
void drawStraightYLine(int x, int y1, int y2, uint32_t colour);
void drawV3DGraphic(int y, int ySize, std::vector<VPADVec3D> v3d, float max, bool xA, bool yA, bool zA);
void drawFillRect(int x1, int y1, int x2, int y2, uint32_t colour);
void drawV2DGraphic(int y, int ySize, std::vector<VPADVec2D> v2d, float max, bool xA, bool yA);
void drawCircleCircum(int cx, int cy, int x, int y, uint32_t colour);
void drawCircle(int xCen, int yCen, int radius, uint32_t colour);
void drawFillCircle(int xCen, int yCen, int radius, uint32_t colour);