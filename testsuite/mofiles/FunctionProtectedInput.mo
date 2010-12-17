// name: FunctionProtectedInput
// status: incorrect

model FunctionProtectedInput

function fn
  protected Real r;
  input Real inR;
  output Real outR;
algorithm
  outR := inR;
end fn;

  Real r, r2;
equation
  r = fn(r2);
end FunctionProtectedInput;

// Result:
// Error processing file: FunctionProtectedInput.mo
// [FunctionProtectedInput.mo:8:3-8:17:writable] Error: inR was declared protected but should be public.
// [FunctionProtectedInput.mo:16:3-16:13:writable] Error: Class fn not found in scope FunctionProtectedInput (looking for a function or record).
// 
// # Error encountered! Exiting...
// # Please check the error message and the flags.
// 
// Execution failed!
// endResult
