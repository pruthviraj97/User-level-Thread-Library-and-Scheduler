#!/bin/sh
#Parameter 1 - scheduler
#Parameter 2 - number of threads
#OUTPUT_FILE = "/common/home/td508/Downloads/project2/output_file.txt"


make SCHED=PSJF
cd benchmarks
make clean
make
./genRecord.sh

echo "SCHED=PSJF"
for i in 2 25 50 75 100 125 150
do
	echo "vector_multiply $i"
	./vector_multiply $i

	echo "parallel_cal $i"
  ./parallel_cal $i

	echo "external_cal $i"
  ./external_cal $i

done

cd ..
make SCHED=MLFQ
cd benchmarks

echo "SCHED=MLFQ"
for i in 2 25 50 75 100 125 150
do
	echo "vector_multiply $i"
	./vector_multiply $i

	echo "parallel_cal $i"
  ./parallel_cal $i

	echo "external_cal $i"
  ./external_cal $i

done




#echo "vector_multiply"
#./vector_multiply $2

#echo "parallel_cal"
#./parallel_cal $2

#echo "external_cal"
#./genRecord.sh
#./external_cal $2
