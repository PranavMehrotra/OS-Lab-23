rev $1 | gawk -F'\n' '{if(NF) lcm=(NR<2)?$1:(lcm*$1)/gcd(lcm,$1)} function gcd(a,b){return(b)?gcd(b,a%b):a} END{print lcm}' | bc