#include "UnifiedControlSuiteWindow.h"

int main(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--simulate" || a == "--dsimulate" || a == "--point" || a == "--calc") {
            return runCli(argc, argv);
        }
    }
    QApplication app(argc, argv);
    UnifiedControlSuiteWindow win;
    win.show();
    return app.exec();
}
