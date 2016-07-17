
default: proofofconcept

CORE_FILES=reaCtive/core.h reaCtive/core.c

proofofconcept: $(CORE_FILES) proofofconcept.c
	gcc -o proofofconcept reaCtive/core.c proofofconcept.c
