#include "draw.hpp"

/* Some draw functions ported from libwiiu/draw.c */
void drawPixel(int x, int y, uint32_t colour) {
	OSScreenPutPixelEx(SCREEN_TV, x, y, colour);
	OSScreenPutPixelEx(SCREEN_DRC, x, y, colour);
}
void drawLine(int x1, int y1, int x2, int y2, uint32_t colour) {
	int x, y;

	if (x1 == x2) {
		if (y1 < y2)
			for (y = y1; y <= y2; y++)
				drawPixel(x1, y, colour);
		else
			for (y = y2; y <= y1; y++)
				drawPixel(x1, y, colour);
	}
	else if (y1 == y2){
		if (x1 < x2)
			for (x = x1; x <= x2; x++)
				drawPixel(x, y1, colour);
		else
			for (x = x2; x <= x1; x++)
				drawPixel(x, y1, colour);
	}
	else {
		/*OSScreenPutFontEx(SCREEN_DRC, 30, 0, ("x1: " + std::to_string(x1)).c_str());
		OSScreenPutFontEx(SCREEN_DRC, 30, 1, ("y1: " + std::to_string(y1)).c_str());
		OSScreenPutFontEx(SCREEN_DRC, 40, 0, ("x2: " + std::to_string(x2)).c_str());
		OSScreenPutFontEx(SCREEN_DRC, 40, 1, ("y2: " + std::to_string(y2)).c_str());*/

		bool a1 = false; //use algorithm 1?
		bool a2 = false; //use alghorithm 2?

		/*    |
		*   0 | 1
		*-----+-----
		*   2 | 0
		*     |    */

		a1 = (x2 > x1 && y1 > y2);
		a2 = (x1 > x2 && y2 > y1);

		if (x1 > x2) {
			int _x = x1;
			x1 = x2;
			x2 = _x;
			//OSScreenPutFontEx(SCREEN_DRC, 20, 17, "testX");
		}

		if ((y1 > y2 && !a1) || a2) {
			int _y = y1;
			y1 = y2;
			y2 = _y;
			//OSScreenPutFontEx(SCREEN_DRC, 14, 17, "testY");
		}

		float _x1 = x1, _y1 = y1;
		float _x2 = x2, _y2 = y2;

		/*OSScreenPutFontEx(SCREEN_DRC, 29, 3, ("_x1: " + std::to_string(x1)).c_str());
		OSScreenPutFontEx(SCREEN_DRC, 29, 4, ("_y1: " + std::to_string(y1)).c_str());
		OSScreenPutFontEx(SCREEN_DRC, 39, 3, ("_x2: " + std::to_string(x2)).c_str());
		OSScreenPutFontEx(SCREEN_DRC, 39, 4, ("_y2: " + std::to_string(y2)).c_str());*/

		float m = (_y2 - _y1) / (_x2 - _x1);
		//OSScreenPutFontEx(SCREEN_DRC, 10, 16, ("m: " + std::to_string(m)).c_str());

		float oldY = _y1;

		if (a1 || a2) { //Dividing function in 2 parts is much big code, but its a little bit faster
			for (int x = _x1; x <= _x2; x++) {
				for (int y = _y1; y >= _y2; y--) {
					float resultedY = x * m + (_y1 - m * _x1);
					for (int z = resultedY; z < oldY; z++) {
						drawPixel(x, z, colour);
					}
					drawPixel(x, resultedY, colour);
					oldY = resultedY;
				}
			}
		}
		else {
			for (int x = _x1; x <= _x2; x++) {
				for (int y = _y1; y <= _y2; y++) {
					float resultedY = x * m + (_y1 - m * _x1);
					for (int z = oldY; z < resultedY; z++) {
						drawPixel(x, z, colour);
					}
					drawPixel(x, resultedY, colour);
					oldY = resultedY;
				}
			}
		}
	}
}
void drawRect(int x1, int y1, int x2, int y2, uint32_t colour) {
	drawLine(x1, y1, x2, y1, colour);
	drawLine(x2, y1, x2, y2, colour);
	drawLine(x1, y2, x2, y2, colour);
	drawLine(x1, y1, x1, y2, colour);
}

void drawStraightXLine(int y, int x1, int x2, uint32_t colour) {
	if (x1 > x2) {
		int _x = x1;
		x1 = x2;
		x2 = _x;
	}

	for (int i = x1; i <= x2; i++)
		drawPixel(i, y, colour);
}void drawStraightYLine(int x, int y1, int y2, uint32_t colour) {
	if (y1 != y2) {
		if (y1 > y2) {
			for (int i = y2; i < y1; i++) {
				drawPixel(x, i, colour);
			}
		}
		else if (y1 < y2) {
			for (int i = y2; i > y1; i--) {
				drawPixel(x, i, colour);
			}
		}
	}
	else
		drawPixel(x, y1, colour);
}
void drawV3DGraphic(int y, int ySize, std::vector<VPADVec3D> v3d, float max, bool xA, bool yA, bool zA) {
	drawRect(0, y, 853, y + ySize, 0xFFFFFFFF);
	drawStraightXLine(y + ySize / 2, 1, 852, 0x7F7F7FFF);

	int vsize = static_cast<int>(v3d.size());
	if (vsize < 2 || vsize > 851)
		return;

	int oldXCalc = y + ySize / 2 + (-v3d[0].x / max * ySize / 2);
	int oldYCalc = y + ySize / 2 + (-v3d[0].y / max * ySize / 2);
	int oldZCalc = y + ySize / 2 + (-v3d[0].z / max * ySize / 2);

	for (int i = 1; i < vsize; i++) {//In order: x>y>z
		if (zA) {
			int zCalc = y + ySize / 2 + (-v3d[i].z / max * ySize / 2);
			drawStraightYLine(i, zCalc, oldZCalc, 0x0000FFFF);
			oldZCalc = zCalc;
		}

		if (yA) {
			int yCalc = y + ySize / 2 + (-v3d[i].y / max * ySize / 2);
			drawStraightYLine(i, yCalc, oldYCalc, 0x00FF00FF);
			oldYCalc = yCalc;
		}

		if (xA) {
			int xCalc = y + ySize / 2 + (-v3d[i].x / max * ySize / 2);
			drawStraightYLine(i, xCalc, oldXCalc, 0xFF0000FF);
			oldXCalc = xCalc;
		}
	}
}
void drawFillRect(int x1, int y1, int x2, int y2, uint32_t colour) {
	if (x1 > x2) {
		int _x = x1;
		x1 = x2;
		x2 = _x;
	}
	if (y1 > y2) {
		int _y = y1;
		y1 = y2;
		y2 = _y;
	}
	for (int x = x1; x <= x2; x++) {
		for (int y = y1; y <= y2; y++) {
			drawPixel(x, y, colour);
		}
	}
}
void drawV2DGraphic(int y, int ySize, std::vector<VPADVec2D> v2d, float max, bool xA, bool yA) {
	drawRect(0, y, 853, y + ySize, 0xFFFFFFFF);
	drawStraightXLine(y + ySize / 2, 1, 852, 0x7F7F7FFF);

	int vsize = static_cast<int>(v2d.size());
	if (vsize < 2 || vsize > 851)
		return;

	int oldXCalc = y + ySize / 2 + (-v2d[0].x / max * ySize / 2);
	int oldYCalc = y + ySize / 2 + (-v2d[0].y / max * ySize / 2);

	for (int i = 1; i < vsize; i++) {//In order: x>y

		if (yA) {
			int yCalc = y + ySize / 2 + (-v2d[i].y / max * ySize / 2);
			drawStraightYLine(i, yCalc, oldYCalc, 0xFFFF00FF);
			oldYCalc = yCalc;
		}

		if (xA) {
			int xCalc = y + ySize / 2 + (-v2d[i].x / max * ySize / 2);
			drawStraightYLine(i, xCalc, oldXCalc, 0xFF00FFFF);
			oldXCalc = xCalc;
		}
	}
}
void drawCircleCircum(int cx, int cy, int x, int y, uint32_t colour) {
	if (x == 0) {
		drawPixel(cx, cy + y, colour);
		drawPixel(cx, cy - y, colour);
		drawPixel(cx + y, cy, colour);
		drawPixel(cx - y, cy, colour);
	}
	if (x == y) {
		drawPixel(cx + x, cy + y, colour);
		drawPixel(cx - x, cy + y, colour);
		drawPixel(cx + x, cy - y, colour);
		drawPixel(cx - x, cy - y, colour);
	}
	if (x < y) {
		drawPixel(cx + x, cy + y, colour);
		drawPixel(cx - x, cy + y, colour);
		drawPixel(cx + x, cy - y, colour);
		drawPixel(cx - x, cy - y, colour);
		drawPixel(cx + y, cy + x, colour);
		drawPixel(cx - y, cy + x, colour);
		drawPixel(cx + y, cy - x, colour);
		drawPixel(cx - y, cy - x, colour);
	}
}
void drawCircle(int xCen, int yCen, int radius, uint32_t colour) {
	int x = 0;

	int y = radius;
	int p = (5 - radius * 4) / 4;

	drawCircleCircum(xCen, yCen, x, y, colour);

	while (x < y) {
		x++;

		if (p < 0) {
			p += 2 * x + 1;
		}
		else {
			y--;
			p += 2 * (x - y) + 1;
		}

		drawCircleCircum(xCen, yCen, x, y, colour);
	}
}
void drawFillCircle(int xCen, int yCen, int radius, uint32_t colour){
	drawCircle(xCen, yCen, radius, colour);

	int x, y;

	for (y = -radius; y <= radius; y++) {
		for (x = -radius; x <= radius; x++)
			if (x*x + y * y <= radius * radius + radius * .8f)
				drawPixel(xCen + x, yCen + y, colour);
	}

}