SHELL := /bin/bash
NODES := 16
EXEC := main nodes C.csv

default: build
## -----------------    Build     -----------------

build: main.c
	source /opt/nfs/config/source_bupc_2021.4.sh &&	upcc -gupc -Wc,"-fPIE" -network=udp -pthreads=4 ./main.c -o main

## -----------------    Create nodes    -----------------
nodes:
	/opt/nfs/config/station206_name_list.sh 1 16 > nodes

## Running


##        ----------    Run with verbose    ----------
run_verb:
	source /opt/nfs/config/source_bupc_2021.4.sh &&	UPC_NODEFILE=nodes upcrun -c 4 -N 4 -n $(NODES) ./main -v

##        ----------    Run without verbose    ----------
run:
	source /opt/nfs/config/source_bupc_2021.4.sh &&	UPC_NODEFILE=nodes upcrun -c 4 -N 4 -n $(NODES) ./main


## Cleaning

## -----------------    Clean    -----------------
clean:
	rm -f *.o $(EXEC) 