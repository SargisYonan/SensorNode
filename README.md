#SensorNode

- define ALLFUNCACCESS before compilation to allow for external use of functions
  generally not needed outside of the scope of a system. For example, for the
  OneWire temperature sensor, we only need access to the initialization and read
  functions. Any access to other functions may allow for unfortunate mistakes
  down the line

- define DONOTUSE before compilation to allow for the use of functions that have
  been removed because they may have had some relevance in the past but no
  longer should be used as they may cause problematic functionality. These
  functions are kept solely as a reference and may be removed soon.
