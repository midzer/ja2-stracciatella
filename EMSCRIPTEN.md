# Emscripten

## Build

```
emmake make
```

## Link

```
em++ -flto -O3 -fno-rtti */*.o */*/*.o -o index.html -sUSE_SDL=2 -sASYNCIFY -sASYNCIFY_IGNORE_INDIRECT -sASYNCIFY_ONLY=@../../../funcs.txt -sENVIRONMENT=web --preload-file Data/ --preload-file ja2.ini --preload-file NoIntro.txt -sINITIAL_HEAP=32mb -sSTACK_SIZE=262144 --closure 1 -sEXPORTED_RUNTIME_METHODS=['allocate','ALLOC_NORMAL']
```
