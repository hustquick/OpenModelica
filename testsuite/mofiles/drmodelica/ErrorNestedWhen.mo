// name:     ErrorNestedWhen
// keywords: when
// status:   correct
// 
// Nested whens are not allowed
// 

model ErrorNestedWhen
  Real x, y1, y2;
equation
  when x > 2 then
    when x > 3 then         // Error: when-clauses cannot be nested  
      y2 = sin(x);
    end when;
  end when;
end ErrorNestedWhen;    

// fclass ErrorNestedWhen
// Real x;
// Real y1;
// Real y2;
// equation
//   when x > 2.0 then
//   when x > 3.0 then
//   y2 = sin(x);
//   end when;
//   end when;
// end ErrorNestedWhen;
