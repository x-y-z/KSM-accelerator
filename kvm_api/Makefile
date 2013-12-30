GCC=gcc
FLAGS=-shared -fPIC
LIBS=-ldl

ksm_mm.so: ksm_mm.c
	$(GCC) $(FLAGS) -o $@ $^ $(LIBS)