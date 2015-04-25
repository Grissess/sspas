(* dangling ELSE binds to closest IF *)
program main( input, output );
  type derp := array[0..] of array[0..] of character;
  var a, b: derp;
  function foo( a: integer; x: real; z: integer): integer;
    procedure boo(a: real);
    begin
    end;
  begin
  end;
begin
  read(a);
  if ( a < 10 ) then
    if ( a >= 10 ) then
      a := 1
  else
      a := 0;
  write(a)
end.
