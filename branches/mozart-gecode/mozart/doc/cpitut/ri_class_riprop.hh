  ...
  ri_float operator <= (ri_float f) {
    _u = min(_u, f);
    return getWidth();
  }
  
  ri_float operator >= (ri_float f) {
    _l = max(_l, f);
    return getWidth();
  }
  
  ri_float lowerBound(void) { return _l; }
  ri_float upperBound(void) { return _u; }
  ...
}; // class RI

