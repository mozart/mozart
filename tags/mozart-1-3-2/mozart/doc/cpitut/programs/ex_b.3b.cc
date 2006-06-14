#define ReplaceOnUnify(EQ01, EQ02, EQ12) \
  if (mayBeEqualVars()) {                \
    if (OZ_isEqualVars(_x, _y)) {        \
      return (EQ01);                     \
    }                                    \
    if (OZ_isEqualVars(_x, _z)) {        \
      return (EQ02);                     \
    }                                    \
    if (OZ_isEqualVars(_y, _z)) {        \
      return (EQ12);                     \
    }                                    \
  }
