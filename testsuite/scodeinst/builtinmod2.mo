// name: builtinmod2.mo
// keywords:
// status: incorrect
// cflags:   +d=scodeInst
//


model A
  Real x(final start = 1.0);
end A;

model B
  extends A(x(start = 2.0));
end B;

// Result:
// Error processing file: builtinmod2.mo
// [builtinmod2.mo:13:15-13:26:writable] Notification: From here:
// [builtinmod2.mo:9:16-9:27:writable] Error: Trying to override final component start with modifier 2.0
// 
// Error: Error occurred while flattening model B
// 
// # Error encountered! Exiting...
// # Please check the error message and the flags.
// 
// Execution failed!
// endResult
