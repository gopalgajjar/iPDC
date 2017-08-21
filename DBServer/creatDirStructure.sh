#! /bin/bash
# Breaks a string in to tokens with separator
# usage: breakToken $1 $2

creatDirAndMove ()
{
    IFS=$2
    arr2=$1
    let k=0
    for x in $arr2
    do
        let k=k+1
        b[k]=$x
    done
    d=${b[k-4]}/${b[k-3]}/${b[k-2]}/${b[$k-1]}
    mkdir -p ${b[k-4]}/${b[k-3]}/${b[k-2]}/${b[$k-1]} 
    unset IFS
    mv $1 $d
}

for file in $@ 
do
    creatDirAndMove $file "-."
done
