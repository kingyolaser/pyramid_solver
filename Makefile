
pyramid: pyramid.cpp
	g++ -Wall -O3 -o $@ pyramid.cpp

pyramid_test: pyramid.cpp
	g++ -g -Wall -DTEST -o $@ pyramid.cpp -lcppunit


run: pyramid
	./$<

bench: pyramid
	time ./$< "7" 3j 620 280j 4j0ak 7k5q58 12k8462 4q17q0jkq716593394863599
	time ./$< "2" q5 29k 710k 40657 kkq63j 7aj353j 895q44898j22q96137401860
	time ./$< "9" 29 845 j200 75j5k 7a04qa kq32671 j3k89j4467829a6653q3qk80
	time ./$< "6" 9q 47q 4j96 10784 j3851k 6j5q70k 27151043kkjq036598223892

bench_noanswer: pyramid
	time ./$< "2" 12 kk8 qaq5 94187 465830 5qk78j9 0262j41jq049577j6339306k

test: pyramid_test
	./$< --test

clean:
	rm -f pyramid.exe
	rm -f pyramid_test.exe
	rm -f *.stackdump
