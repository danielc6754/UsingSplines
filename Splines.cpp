using namespace std;

#include "ConsoleEngine.h"
#include <string>

struct sPoint2D {
	float x;
	float y;

};

struct sSpline {
	vector<sPoint2D> points;
	// if we want splines to be loops, then need to treat it dfferently
	sPoint2D GetSplinePoint(float t, bool bLooped = false) {
		int p0, p1, p2, p3;
		if (!bLooped) {
			p1 = (int)t + 1;
			p2 = p1 + 1;
			p3 = p1 + 2;
			p0 = p1 - 1;
		}

		// wrap indices around all the points in vector
		else {
			p1 = (int)t;
			p2 = (p1 + 1) % points.size();
			p3 = (p2 + 2) % points.size();
			p0 = p1 >= 1 ? p1 - 1 : points.size() - 1;
		}
		// t is going above 1, so need to accomadate for this. // want whats left over
		t = t - (int)t;

		float tt = t * t;
		float ttt = tt * t;

		float q0 = -ttt + 2.0f * tt - t; // -x^3 + 2x^2 - x
		float q1 = 3.0f * ttt - 5.0f * tt + 2.0f; // 3x^3 -5 x^2 + 2
		float q2 = -3.0f * ttt + 4.0f * tt + t; // -3x^3 - 4x^2 + x
		float q3 = ttt - tt; // x^3 - x^2

		float tx = 0.5f * (points[p0].x * q0 + points[p1].x * q1 + points[p2].x * q2 + points[p3].x * q3);
		float ty = 0.5f * (points[p0].y * q0 + points[p1].y * q1 + points[p2].y * q2 + points[p3].y * q3);

		return { tx, ty };
	}
	// Calculating gradient
	sPoint2D GetSplineGradient(float t, bool bLooped = false) {
		int p0, p1, p2, p3;
		if (!bLooped) {
			p1 = (int)t + 1;
			p2 = p1 + 1;
			p3 = p1 + 2;
			p0 = p1 - 1;
		}

		// wrap indices around all the points in vector
		else {
			p1 = (int)t;
			p2 = (p1 + 1) % points.size();
			p3 = (p2 + 2) % points.size();
			p0 = p1 >= 1 ? p1 - 1 : points.size() - 1;
		}
		// t is going above 1, so need to accomadate for this. // want whats left over
		t = t - (int)t;

		float tt = t * t;
		float ttt = tt * t;

		float q0 = -3.0f * tt + 4.0f * t - 1; // -3x^2 + 4x - 1
		float q1 = 9.0f * tt - 10.0f * t; // 9x^2 - 10x
		float q2 = -9.0f * tt + 8.0f * t + 1; // -9x^2 - 8x + 1
		float q3 = 3.0f * tt - 2.0f * t; // 3x^2 - 2x

		float tx = 0.5f * (points[p0].x * q0 + points[p1].x * q1 + points[p2].x * q2 + points[p3].x * q3);
		float ty = 0.5f * (points[p0].y * q0 + points[p1].y * q1 + points[p2].y * q2 + points[p3].y * q3);

		return { tx, ty };
	}
};

class SpLines : public ConsoleTemplateEngine {
public:
	SpLines() {
		m_sAppName = L"SpLines";
	}
private:
	sSpline path;
	int nSelectedPoint = 0;
	float fMarker = 0.0f;

protected:
	virtual bool OnUserCreate() {
		//path.points = { {10, 41}, {40, 41}, {70, 41}, {100, 41} };
		path.points = { {10, 41}, {20, 41}, {30, 41}, {40, 41}, {50, 41}, {60, 41}, {70, 41}, {80, 41}, {90, 41}, {100, 41} };

		return true;
	}

	virtual bool OnUserUpdate(float fElapsedTime) {
		// Flear Screen
		Fill(0, 0, ScreenWidth(), ScreenHeight(), L' ');
		
		// Handle input
		if (m_keys[L'X'].bReleased) {
			nSelectedPoint++;
			if (nSelectedPoint >= path.points.size())
				nSelectedPoint = 0;
		}

		if (m_keys[L'Z'].bReleased) {
			nSelectedPoint--;
			if (nSelectedPoint < 0)
				nSelectedPoint = path.points.size() - 1;
		}

		if (m_keys[VK_LEFT].bHeld)
			path.points[nSelectedPoint].x -= 30.0f * fElapsedTime;
		if (m_keys[VK_RIGHT].bHeld)
			path.points[nSelectedPoint].x += 30.0f * fElapsedTime;
		if (m_keys[VK_UP].bHeld)
			path.points[nSelectedPoint].y -= 30.0f * fElapsedTime;
		if (m_keys[VK_DOWN].bHeld)
			path.points[nSelectedPoint].y += 30.0f * fElapsedTime;

		// Agent Inputs
		if (m_keys[L'A'].bHeld)
			fMarker -= 5.0f * fElapsedTime;
		if (m_keys[L'S'].bHeld)
			fMarker += 5.0f * fElapsedTime;

		if (fMarker >= (float)path.points.size())
			fMarker -= (float)path.points.size();
		if (fMarker < 0.0f)
			fMarker += (float)path.points.size();

		// Draw Spline
		for (float t = 0.0f; t < (float)path.points.size(); t += 0.005f) {
			sPoint2D pos = path.GetSplinePoint(t, true);
			Draw(pos.x, pos.y);
		}

		// Draw Points
		for (int i = 0; i < path.points.size(); i++) {
			Fill(path.points[i].x - 1, path.points[i].y - 1, path.points[i].x + 2, path.points[i].y + 2, PIXEL_SOLID, FG_RED);
			DrawString(path.points[i].x, path.points[i].y, to_wstring(i));
		}

		// Highlight control point
		Fill(path.points[nSelectedPoint].x - 1, path.points[nSelectedPoint].y - 1, path.points[nSelectedPoint].x + 2, path.points[nSelectedPoint].y + 2, PIXEL_SOLID, FG_YELLOW);
		DrawString(path.points[nSelectedPoint].x, path.points[nSelectedPoint].y, to_wstring(nSelectedPoint));

		// Draw Agent to demonstrate gradient
		sPoint2D p1 = path.GetSplinePoint(fMarker, true);
		sPoint2D g1 = path.GetSplineGradient(fMarker, true);
		// Get angle and convert to radian
		float r = atan2(-g1.y, g1.x);
		DrawLine(5.0f * sin(r) + p1.x, 5.0f * cos(r) + p1.y, -5.0f * sin(r) + p1.x, -5.0f * cos(r) + p1.y, PIXEL_SOLID, FG_BLUE);

		return true;
	}
};

int main() {
	SpLines game;
	game.ConstructConsole(160, 80, 10, 10);
	game.Start();

	return 0;
}