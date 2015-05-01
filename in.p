program example(input, output);
var x, y: integer;

function gcd(a, b: integer): integer;
begin
    if b = 0 then gcd := a
    else gcd := gcd(b, a mod b)
end;

procedure nonlocal_varacc();
begin
    x := 4
end;

procedure read(a: array[0..] of integer, b: array[0..] of integer);
begin
end;

procedure writeln(a: integer);
begin
end;

begin
    read(@x, @y);
    writeln(gcd(x, y));
    nonlocal_varacc();
    writeln(x)
end.

