DIRS = spl xfs-interface xsm_dev expl

all:
	set -e; for d in $(DIRS); do $(MAKE) -C $$d ; done

clean:
	@cd spl && make clean
	@cd xfs-interface && make clean
	@cd xsm_dev && make clean
	@cd expl && make clean
