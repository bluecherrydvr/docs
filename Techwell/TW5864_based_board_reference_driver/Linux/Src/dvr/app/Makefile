


SUBDIRS = netmsg  pbapp src  test
.PHONY: subdirs $(SUBDIRS)
subdirs: $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@

.PHONY: clean
clean:
	@for i in $(SUBDIRS); do $(MAKE) -C $$i clean;done
