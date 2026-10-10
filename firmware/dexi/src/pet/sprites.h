// Dexi's pixel art: a chibi blob in the style of Claude Code's Clawd and the
// Codex pets: chunky solid body, no outline, tiny bar eyes, stubby limbs.
// 16x12 cells drawn at 4x. Cell codes: '.' clear, 'b' body, 'h' highlight,
// 'd' shade, 'o' ink (eyes/mouth), 'p' pink, 'w' white.
// Faces are overlays so every mood reuses the body.
#pragma once

namespace sprites {

constexpr int kCols = 16, kRows = 12;

// Body rows 0-9; arms (cols 0 and 15) and legs (rows 10-11) are separate.
constexpr const char* kBody[10] = {
    "...hhhhhhhhhh...",
    "..hbbbbbbbbbbb..",
    ".hbbbbbbbbbbbbb.",
    ".bbbbbbbbbbbbbb.",
    ".bbbbbbbbbbbbbb.",
    ".bbbbbbbbbbbbbb.",
    ".bbbbbbbbbbbbbb.",
    ".bbbbbbbbbbbbbb.",
    ".dbbbbbbbbbbbbd.",
    "..dddddddddddd..",
};

// Rows 10-11: standing, then two walk frames.
constexpr const char* kLegs[3][2] = {
    {"..dd.dd..dd.dd..", "..dd.dd..dd.dd.."},
    {"..dd.dd..dd.dd..", "..dd.....dd....."},
    {"..dd.dd..dd.dd..", ".....dd.....dd.."},
};

// Arm nubs: rows for down (resting) and up (waving).
constexpr int kArmRowDown = 5, kArmRowUp = 3;

// Eyes: 3x3 at rows 2-4, left eye from col 4, right eye mirrored from col 9.
constexpr int kEyeCol = 4, kEyeRow = 2, kRightEyeCol = 9;
constexpr const char* kEyes[][3] = {
    /* open   */ {"...", ".o.", ".o."},
    /* big    */ {".o.", ".o.", ".o."},
    /* closed */ {"...", "...", "oo."},
    /* happy  */ {"...", ".o.", "o.o"},
    /* hearts */ {"...", "p.p", ".p."},
    /* angry  */ {"o..", ".o.", ".o."},
    /* sleepy */ {"...", "...", "oo."},
};

// Mouths: 4x2 at col 6, row 6. Mostly empty: the eyes do the talking.
constexpr int kMouthCol = 6, kMouthRow = 6;
constexpr const char* kMouths[][2] = {
    /* none    */ {"....", "...."},
    /* neutral */ {"....", "...."},
    /* smile   */ {"o..o", ".oo."},
    /* open    */ {".oo.", ".oo."},
    /* oh      */ {".oo.", ".oo."},
    /* frown   */ {".oo.", "o..o"},
    /* tongue  */ {".oo.", ".pp."},
};

}  // namespace sprites
