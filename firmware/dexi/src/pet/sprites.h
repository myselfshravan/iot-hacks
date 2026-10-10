// Dexi's pixel art: a 16x16 ginger cat, drawn at 4x scale.
// Cell codes: '.' clear, 'o' outline, 'b' body (mood tint), 'l' belly,
// 'p' pink, 'w' white. Faces are overlays so every mood reuses the body.
#pragma once

namespace sprites {

constexpr int kSize = 16;

// Standing body, rows 0-13 (legs come from the leg frames below).
constexpr const char* kBody[14] = {
    "................",
    "..oo........oo..",
    ".opbo......obpo.",
    ".obbboooooobbbo.",
    "obbbbbbbbbbbbbbo",
    "obbbbbbbbbbbbbbo",
    "obbbbbbbbbbbbbbo",
    "obbbbbbbbbbbbbbo",
    "obbbbbbbbbbbbbbo",
    "obbbllllllllbbbo",
    "obbbllllllllbbbo",
    "obbbllllllllbbbo",
    ".obbbbbbbbbbbbo.",
    "..oooooooooooo..",
};

// Rows 14-15: standing, then the two walk frames.
constexpr const char* kLegs[3][2] = {
    {"...obo....obo...", "...ooo....ooo..."},
    {"..obo......obo..", "..ooo......ooo.."},
    {"....obo..obo....", "....ooo..ooo...."},
};

// Curled-up loaf for sleeping: rows 6-15 of the grid.
constexpr int kLoafTop = 6;
constexpr const char* kLoaf[10] = {
    "................",
    "..oo........oo..",
    ".opbo......obpo.",
    ".obbboooooobbbo.",
    "obbbbbbbbbbbbbbo",
    "obbbbbbbbbbbbbbo",
    "obbbllllllllbbbo",
    "obbbllllllllbbbo",
    "obbbbbbbbbbbbbbo",
    ".oooooooooooooo.",
};

// Eyes: 3 wide x 3 tall, left eye at column 3, row 5. The right eye is the
// mirror image at column 10.
constexpr int kEyeCol = 3, kEyeRow = 5, kRightEyeCol = 10;
constexpr const char* kEyes[][3] = {
    /* open   */ {"...", ".wo", ".oo"},
    /* big    */ {".wo", ".oo", ".oo"},
    /* closed */ {"...", "...", ".oo"},
    /* happy  */ {"...", ".o.", "o.o"},
    /* hearts */ {"...", "p.p", ".p."},
    /* angry  */ {"o..", ".oo", ".oo"},
    /* sleepy */ {"...", "...", "ooo"},
};

// Mouths: 4 wide x 2 tall at column 6, row 9 (on the belly).
constexpr int kMouthCol = 6, kMouthRow = 9;
constexpr const char* kMouths[][2] = {
    /* none    */ {"....", "...."},
    /* neutral */ {".oo.", "...."},
    /* smile   */ {"o..o", ".oo."},
    /* open    */ {"oooo", ".pp."},
    /* oh      */ {".oo.", ".oo."},
    /* frown   */ {".oo.", "o..o"},
    /* tongue  */ {".oo.", ".pp."},
};

}  // namespace sprites
