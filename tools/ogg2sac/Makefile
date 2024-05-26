CFLAGS = -Wno-return-type -Wno-unused-variable -Wno-invalid-offsetof

ifeq ($(OS),Windows_NT)
	CFLAGS += -Dset_compile=0
	INCLUDE = -I C:/libs/fan/include
	LINK =
	RM = del
	OUTPUT ="a.exe"
else
	CFLAGS += -Dset_compile=1
	INCLUDE =
	LINK =
	RM = rm
	OUTPUT = a.exe
endif

debug:
	clang $(CFLAGS) -g main.c -o $(OUTPUT) $(INCLUDE) $(LINK)

release:
	clang $(CFLAGS) $(ReleaseFlags) main.c -o $(OUTPUT) $(INCLUDE) $(LINK)

clean:
	$(RM) $(OUTPUT)
