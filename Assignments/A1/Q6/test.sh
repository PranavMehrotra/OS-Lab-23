#!/bin/bash

# function to find all prime numbers using the Sieve of Eratosthenes algorithm
sieve_of_eratosthenes() {
    n=$1
    primes=( $(seq 2 $n) )
    for ((i=0; i<${#primes[@]}; i++))
    do
        if [[ ${primes[i]} -ne -1 ]]
        then
            echo ${primes[i]}
            for ((j=2*${primes[i]}; j<=n; j+=${primes[i]}))
            do
                primes[j-2]=-1
            done
        fi
    done
}

# call the function with the upper limit of 1e6
sieve_of_eratosthenes 100
