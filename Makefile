
default: proofofconcept

proofofconcept: reaCtive.h reaCtive.c proofofconcept.c
	gcc -o proofofconcept reaCtive.c proofofconcept.c
