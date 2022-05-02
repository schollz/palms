test: testI1P testAll testFC
	rm foo
testI1P:
	g++ -c I1P.h -o foo
testAll:
	g++ -c Allpass.cpp -o foo
testFC:
	g++ -c FeedbackComb.cpp -o foo
