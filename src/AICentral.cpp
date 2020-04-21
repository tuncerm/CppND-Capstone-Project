#include "AICentral.h"

AICentral::AICentral() {
    for (int i = 0; i < 20; ++i) {
        for (int j = 0; j < 32; ++j) {
            _map[i][j] = MapObject::kDark;
        }
    }
}

void AICentral::AddToMap(int row, int col, MapObject ob) {
    _map[row][col] = ob;
}

AICentral::MapObject AICentral::ReadFromMap(int row, int col) {
    return _map[row][col];
}
