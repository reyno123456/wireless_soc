#
# To generate depend file(.depend) for *.c *.cpp
# To make by rules for *.s *.c *.cpp
#

_depend: $(obj).depend

$(obj).depend: $(BUILD_DIR)/config.mk $(SRC_C) $(SRC_CXX)
	@rm -f $@
	@for f in $(SRC_C); do \
		g=`basename $$f | sed -e 's/\(.*\)\.\w/\1.o/'`; \
		$(CC) -MM $(CFLAGS) -E -MQ $(_obj)$$g $$f >> $@ ; \
	done
	@for f in $(SRC_CXX); do \
		g=`basename $$f | sed -e 's/\(.*\)\.\w\w\w/\1.cpp.o/'`; \
		$(CXX) -MM $(CXXFLAGS) -E -MQ $(_obj)$$g $$f >> $@ ; \
        done

# make all .s
 %.ao:    %.s
	@echo "Compling: " $(addsuffix .s, $(basename $(notdir $@)))
	@$(AS) $(ASFLAGS) $< -o $@

# make all .c
%.o:    %.c
	@echo "Compling: " $(addsuffix .c, $(basename $(notdir $@)))
	@$(CC) $(CFLAGS) -c $< -o $@

# make all .cpp
%.cpp.o:    %.cpp
	@echo "Compling: " $(basename $(notdir $@))
	$(CXX) $(CXXFLAGS) -c $< -o $@

