#!/bin/bash

memory_counter=0
out_counter=0
err_counter=0
counter=0
for f in $2/*.in
do
	((counter++))
	valgrind --log-file=aux ./$1 < $f > aux.out 2>aux.err

	if grep -q -c 'in use at exit: 0 bytes in 0 blocks' aux
	then
		echo "${f%.in} memory OK"
	else
		echo "${f%in} memory leak"
		((memory_counter++))
	fi

	if cmp -s ${f%in}out aux.out
	then
		echo "${f%in}out OK"
	else
		echo "${f%in}out WRONG"
		((out_counter++))
	fi

	if cmp -s ${f%in}err aux.err
	then
		echo "${f%in}err OK"
	else
		echo "${f%in}err WRONG"
		((err_counter++))
	fi
done

echo "memory leak: $memory_counter/$counter"
echo "wrong out: $out_counter/$counter"
echo "wrong err: $err_counter/$counter"

rm aux.out aux.err aux
