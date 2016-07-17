
default: proofofconcept

CORE_FILES=reaCtive/core.h reaCtive/core.c
OPER_FILES=reaCtive/operators.h reaCtive/operators.c

proofofconcept: $(CORE_FILES) $(OPER_FILES) proofofconcept.c
	gcc -o proofofconcept reaCtive/core.c reaCtive/operators.c proofofconcept.c
