CFLAGS = -std=c++2a -Wno-return-type -Wno-unused-variable -Wno-invalid-offsetof

INCLUDE = -I /usr/include/opus/
LINK = -lopus -lpthread -lpulse-simple
RM = rm
OUTPUT = a.exe

ReleaseFlags = -s -fdata-sections -ffunction-sections -Wl,--gc-sections -mmmx -msse -msse2 -msse3 -mssse3 -msse4 -msse4.1 -O3

debug:
	clang++ $(CFLAGS) -g main.cpp -o $(OUTPUT) $(INCLUDE) $(LINK)

release:
	clang++ $(CFLAGS) $(ReleaseFlags) main.cpp -o $(OUTPUT) $(INCLUDE) $(LINK)

clean:
	$(RM) $(OUTPUT)
