using namespace std;

#include "ConsoleEngine.h"
#include <string>

class SpLines : public ConsoleTemplateEngine {
public:
	SpLines() {
		m_sAppName = L"SpLines";
	}
private:

protected:
	virtual bool OnUserCreate() {
		return true;
	}

	virtual bool OnUserUpdate(float fElapsedTime) {
		return true;
	}
};

int main() {
	SpLines game;
	game.ConstructConsole(90, 50, 12, 12);
	game.Start();

	return 0;
}