while read line;do
    if([ ${#line} -ge 5 ]&&[ ${#line} -le 20 ]&&echo $line|grep -q '^[A-Za-z][A-Za-z]*[0-9][a-zA-Z0-9]*$')
    then echo $line|grep -i -q -f fruits.txt &&echo "NO"|| echo "YES"
    else echo "no"
    fi
done <$1