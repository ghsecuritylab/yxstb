
DEFINES += -DHAVE_ASSERT_H

OBJS = 
OBJS += $(OBJDIR)/bit.obj
OBJS += $(OBJDIR)/fixed.obj
OBJS += $(OBJDIR)/frame.obj
OBJS += $(OBJDIR)/huffman.obj
OBJS += $(OBJDIR)/layer12.obj
OBJS += $(OBJDIR)/layer3.obj
OBJS += $(OBJDIR)/stream.obj
OBJS += $(OBJDIR)/synth.obj
OBJS += $(OBJDIR)/timer.obj
OBJS += $(OBJDIR)/version.obj

all: builddir $(TARGET)

builddir:
	$(MKDIR) $(OBJDIR)

$(TARGET): $(OBJS)
	$(AR) -rcus $@ $(OBJS)

#
# ±àÒë C ÎÄ¼þ
#
$(OBJDIR)/%.obj:$(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -c -o $@ $<

clean:
	$(RMDIR) $(OBJDIR)
	$(RM) $(TARGET)