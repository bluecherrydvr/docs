#ifneq ($(GLABALCOMPILE),yes)
#	override TOPDIR := ./../
#	sinclude	$(TOPDIR)rules.make	
#endif

export

SUBDIRS = eventcore mediasdk isilsdk
.PHONY: subdirs $(SUBDIRS)
subdirs: $(SUBDIRS)
$(SUBDIRS):
	@echo -e "EVENTCOREINCPATH : $(EVENTCOREINCPATH) CFLAGS: $(CFLAGS) \n\t"
	$(MAKE) -C $@

.PHONY: clean
clean:
	@for i in $(SUBDIRS); do $(MAKE) -C $$i clean;done
