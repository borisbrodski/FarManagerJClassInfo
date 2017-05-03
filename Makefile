# Set this to the path to Headers.c directory. The path shouldn't containt any spaces.
PATH_TO_FAR_SDK=../PluginSDK/Headers.c

CPP_FILES := $(wildcard *.cpp)
H_FILES := $(wildcard *.h)
OBJ_FILES := $(addprefix ,$(notdir $(CPP_FILES:.cpp=.o)))

JClassInfo.dll: $(OBJ_FILES) plugin_rc.o
	g++.exe -shared -static -static-libgcc -static-libstdc++ -o $@ -Wl,--kill-at plugin.def plugin_rc.o $^

%.o: %.cpp $(H_FILES)
	g++.exe -c -o $@  -Wl,--kill-at "-I$(PATH_TO_FAR_SDK)" -DWIN32 -DUNICODE $<

plugin_rc.o: plugin.rc
	windres.exe --include $(PATH_TO_FAR_SDK) -o plugin_rc.o -O coff plugin.rc


clean:
	rm -rf *.o *.dll

