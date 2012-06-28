// name: when1.mo
// keywords:
// status: correct
// cflags:   +d=scodeInst
//
// FAILREASON: Expand of when equations not implemented.
//

model A
  Real x, y;
  Boolean b, b2;
equation
  when {b, b2} then
    x = y;
  end when;
end A;
