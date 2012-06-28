// name: eq7.mo
// keywords:
// status: correct
// cflags:   +d=scodeInst
//

model A
  Real x, y;
equation
  y = x;
end A;

model B
  A a[3](x = {1, 2, 3});
end B;
// Result:
// 
// EXPANDED FORM:
// 
// class B
//   Real a[1].x = 1;
//   Real a[1].y;
//   Real a[2].x = 2;
//   Real a[2].y;
//   Real a[3].x = 3;
//   Real a[3].y;
// equation
//   a[1].y = a[1].x;
//   a[2].y = a[2].x;
//   a[3].y = a[3].x;
// end B;
// 
// 
// Found 6 components and 0 parameters.
// class B
//   Real a[1].x = 1.0;
//   Real a[1].y;
//   Real a[2].x = 2.0;
//   Real a[2].y;
//   Real a[3].x = 3.0;
//   Real a[3].y;
// equation
//   a[1].y = a[1].x;
//   a[2].y = a[2].x;
//   a[3].y = a[3].x;
// end B;
// endResult
