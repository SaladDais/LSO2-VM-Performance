//
// The Computer Language Shootout
// http://shootout.alioth.debian.org/
//
// contributed by bearophile, Jan 24 2006
// modified by Babbage Linden, Oct 10 2007
//

integer ack(integer x, integer y) 
{
  if(x == 0)
  {
    return y + 1;
  }
  
  if(y)
  {
    return ack(x - 1, ack(x, y - 1));
  }
  else
  {
    return ack(x - 1, 1);
  }
}

integer fib(integer n) 
{
  if (n < 2) 
  {
    return 1;
  }
  return fib(n - 2) + fib(n - 1);
}

float fibFP(float n) 
{
  if (n < 2.0) 
  {
    return 1.0;
  }
  return fibFP(n - 2.0) + fibFP(n - 1.0);
}

integer tak(integer x, integer y, integer z) 
{
  if (y < x) 
  {
    return tak(tak(x - 1, y, z), tak(y - 1, z, x), tak(z - 1, x, y));
  }
  return z;
}

float takFP(float x, float y, float z) 
{
    if (y < x)
    {
        return takFP( takFP(x-1.0, y, z), takFP(y-1.0, z, x), takFP(z-1.0, x, y) );
    }
    return z;
}

default
{
    state_entry()
    {
        integer n = 3;
        print("Ack(3," + (string)(n+1) + "): " + (string)(ack(3, n+1)));
        
        print("Tak(" + (string)(3 * n) + "," + (string)(2 * n) + "," + (string)n + "): " + (string)tak(3*n, 2*n, n));
        
        print("Fib(3): " + (string)fib(3));
        print("Tak(3.0,2.0,1.0): " + (string)takFP(3.0, 2.0, 1.0));
    }
}
