class RIWakeUp : public OZ_CtWakeUp {
public:
  static OZ_CtWakeUp wakeupMin(void) {
    OZ_CtWakeUp ri_wakeup_min;
    ri_wakeup_min.init();
    ri_wakeup_min.setWakeUp(0);
    return ri_wakeup_min;
  }
  static OZ_CtWakeUp wakeupMax(void) {
    OZ_CtWakeUp ri_wakeup_max;
    ri_wakeup_max.init();
    ri_wakeup_max.setWakeUp(1);
    return ri_wakeup_max;
  }
  static OZ_CtWakeUp wakeupMinMax(void) {
    OZ_CtWakeUp ri_wakeup_minmax;
    ri_wakeup_minmax.init();
    ri_wakeup_minmax.setWakeUp(0);
    ri_wakeup_minmax.setWakeUp(1);
    return ri_wakeup_minmax;
  }
};
