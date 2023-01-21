n=100
primes=($(seq 0 $n))
for((i=2;((i*i))<=n;i++));do
    if((${primes[i]}!=-1));then
        for((j=((i*i));j<=n;j+=i));do
            primes[j]=-1
        done
    fi
done
while read line;do
    j=0
    for((i=2;i<=line;i++));do
        if((${primes[i]}!=-1));then
            echo -n $i "";
        fi
    done
    echo "";
done <"input.txt" >"output.txt"